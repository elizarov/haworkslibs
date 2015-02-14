/*
  Libfrary for SHT1X temperature / humidity sensors.
  Adapted from Weather Station Data Logger code.
*/

//============================================================================
//Weather Station Data Logger : Weather Shield for Arduino
//Copyright � 2010, Weber Anderson
// 
//This application is free software; you can redistribute it and/or
//modify it under the terms of the GNU Lesser General Public
//License as published by the Free Software Foundation; either
//version 3 of the License, or (at your option) any later version.
//
//This application is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR When PARTICULAR PURPOSE. See the GNU
//Lesser General Public License for more details.
//
//You should have received a copy of the GNU Lesser General Public
//License along with this library; if not, see <http://www.gnu.org/licenses/>
//
//=============================================================================

#include "SHT1X.h"
#include <avr/delay.h>

//
// to avoid damage to the SHT1x sensor,
// the DATA pin can only be driven low as an output. if we want to send a "HIGH"
// output, it must be done by setting the data pin as INPUT with the pull-up enabled.
// to accomplish this, the following sequences must be followed:
// 
// to go from high output or input mode to a low output:
// first set the output pin low. this disables the pull-up resistor which is not 
// necessarily desired, but it is necessary to preset the
// output pin to the low state. then, change the pin from input to output.
//
// to go from low output to high output or input mode, first change the pin to
// an input, then set the pin to high, enabling the pull-up resistor.
//
// as mentioned above, if a 10k pull-up resistor is added to the data line,
// this gets a lot simpler. just initialize the port data bit to zero and leave
// it there. Then data_low() is just switching the port bit to output mode, and
// data_high() is simply switching to input mode.
//

const int SHT1X_DATA_SETUP_US = 20;
inline void SHT1X::DATA_LOW()   { cli(); digitalWrite(_data_pin, LOW); pinMode(_data_pin, OUTPUT); sei(); _delay_us(SHT1X_DATA_SETUP_US); }
inline void SHT1X::DATA_HIGH()  { cli(); pinMode(_data_pin, INPUT); digitalWrite(_data_pin, HIGH); sei(); _delay_us(SHT1X_DATA_SETUP_US); }

const int SHT1X_DATA_HOLD_US = 20;
inline void SHT1X::CLOCK_HIGH() { cli(); digitalWrite(_clk_pin, HIGH); sei(); _delay_us(SHT1X_DATA_HOLD_US); }
inline void SHT1X::CLOCK_LOW()  { cli(); digitalWrite(_clk_pin, LOW ); sei(); _delay_us(SHT1X_DATA_HOLD_US); }

const uint8_t FAIL_RESET_THRESHOLD = 3;

//
// Sensirion data sheet notes that in high res mode (14-bit), no more than 1 msmt every
// 3.2 seconds should be made to keep self-heating below 0.1 degC.
//

// Here we do 2 * 400ms measurements followed by 9.2 sec cooldown to get new reading every 10 seconds

const unsigned long RESET_INTERVAL = Timeout::SECOND; // 1 sec
const unsigned long RETRY_INTERVAL = 100L; 
const unsigned long MEAS_INTERVAL = 400L; 
const unsigned long COOLDOWN_INTERVAL = 9.2 * Timeout::SECOND;

// these cmd defs also include the 3 address bits which are always equal to zero
const uint8_t SHT1X_MEAS_TEMP_CMD = 0x03;
const uint8_t SHT1X_MEAS_RH_CMD = 0x05;

const uint8_t SHT1X_STATE_MEAS_TEMP  = 0;
const uint8_t SHT1X_STATE_READ_TEMP  = 1;
const uint8_t SHT1X_STATE_MEAS_RH    = 2;
const uint8_t SHT1X_STATE_READ_RH    = 3;

const unsigned int INVALID = 0xffff;

SHT1X::SHT1X(uint8_t clk_pin, uint8_t data_pin) :
  _clk_pin(clk_pin),
  _data_pin(data_pin),
  _state(SHT1X_STATE_MEAS_TEMP),
  _timeout(0) // measure immediately
{
  //
  // setup pins for the SHT1x temp/RH sensor.
  //
  cli();
  pinMode(_clk_pin, OUTPUT);
  pinMode(_data_pin, INPUT);     // redundant as pins default to inputs. here for safety
  digitalWrite(_clk_pin, LOW);
  digitalWrite(_data_pin, HIGH); // enable pull-up resistor
  sei();
  reset();
}

//
// these functions are from the SHT1x data sheet
// there is a polynomial to convert temperature readings into 
// temperature.
//
float SHT1X::rdg2temp(unsigned int rdg)
{
  float g = (float)rdg - 7000.0F;
  float t = -40.1 + 0.01 * (float)rdg - 2.0e-8 * g * g;
  return t;
}
//
// For RH, there is a polynomial to convert readings into % RH
// There is also a smaller correction based on the temperature
//
float SHT1X::rdg2rh(unsigned int rdg, float temp)
{
  float so = (float)rdg;
  float rh_linear =  -2.0468F + so * (0.0367 -1.5955e-6 * so);
  float corr = (temp - 25.0F) * (0.01 + 0.00008 * so);
  return rh_linear + corr;
}

SHT1X::temp_t SHT1X::getTemp() {
  if (_raw_temp == INVALID)
    return temp_t::invalid;
  return rdg2temp(_raw_temp) * temp_t::multiplier;
}

SHT1X::rh_t SHT1X::getRH() {
  if (_raw_temp == INVALID || _raw_rh == INVALID)
    return rh_t::invalid;
  return rdg2rh(_raw_rh, rdg2temp(_raw_temp)) * rh_t::multiplier;
}

uint16_t SHT1X::getState() {
  return (_last_error << 8) | _state;
}

//
// none of the SHT1x I/O is interrupt driven. This interface is a total
// bit banger with no delays required so interrupts will not be of much use.
// The SHT1x's interface logic is all static, so it won't hurt anything if
// we get interrupted during an operation -- as long as the interrupt does not
// mess around with our I/O lines!
//
void SHT1X::reset() {
  DATA_HIGH();
  for (int k=0; k<10; k++) {
    CLOCK_HIGH();
    CLOCK_LOW();
  }    
  _fail_count = 0;
  _raw_temp = INVALID;
  _raw_rh = INVALID;
}

boolean SHT1X::send_cmd(byte cmd) {
  // transmission start sequence
  CLOCK_HIGH();
  DATA_LOW();
  CLOCK_LOW();
  CLOCK_HIGH();
  DATA_HIGH();
  CLOCK_LOW();  

  // clock out the bits...
  boolean datastate = true; // current state of data pin
  for (int k=0; k<8; k++) {
    // set the DATA pin output to match the data MSB
    // to avoid unnecessary noise on the DATA pin, only change the 
    // pin state if it is not equal to the desired state.
    //
    boolean da_bit = ((cmd & 0x80) == 0x80);
    boolean state_ok = datastate == da_bit;
    if (!state_ok) {
      if (da_bit)
        DATA_HIGH();
      else
        DATA_LOW();
      datastate = da_bit;
    }
    // toggle the clock
    CLOCK_HIGH();
    CLOCK_LOW();
    cmd = cmd << 1; // shift the next output bit into the MSB position
  }
  //
  // after sending 8 bits, the SHT1x sensor will drive the data line low as an
  // acknowledgement of the command.
  //
  if (!datastate) {
    DATA_HIGH(); // prepare to read
    datastate = true;
  }
  boolean ack = digitalRead(_data_pin) == LOW;
  // toggle clock once more to clear the ACK
  CLOCK_HIGH();
  CLOCK_LOW();

  if (!ack) {
    _last_error = 1; // NO ACK
    return false;
  }

  // verify that data has gone high again -- should we do this?
  if (digitalRead(_data_pin) == LOW) {
    _last_error = 2; // NO DATA LOW
    return false;
  }

  return true;
}

void SHT1X::read_data(unsigned int *value, byte *crc) {
  // caller must ensure data is ready to be read before calling this function
  // first data bit is ready to be read upon entry to this routine.
  byte x[3];
  for (byte m=0; m<3; m++) {
    x[m] = 0;
    for (byte k=0; k<8; k++) {
      x[m] = (x[m] << 1);
      if (digitalRead(_data_pin) == HIGH) x[m] |= 0x01U;
      CLOCK_HIGH();
      CLOCK_LOW();        
    }
    // got the byte. send an ACK with data low
    // except for the last byte, which has an ACK with data high
    if (m < 2) 
      DATA_LOW();
    CLOCK_HIGH();
    CLOCK_LOW();
    if (m < 2) 
      DATA_HIGH();
  }

  *value = ((unsigned int)x[0] << 8) | (unsigned int)x[1];
  *crc = x[2];
}

//
// this is from the Sensirion application note on CRC-8 calculation.
// it has been modified slightly to generate the CRC in the proper
// bit order, matching the ordering of bits in the CRC reported by the sensor.
//
// the crc computation includes the command byte that was sent
// in addition to the 16-bit data returned from the command. this provides
// an extra level of confirmation that the sensor is responding to 
// the proper command.
//
bool SHT1X::verify_crc8(byte cmd, unsigned int value, byte crc) {
  byte reg = 0;
  byte xmit[3];
  xmit[0] = cmd;
  xmit[1] = value >> 8;
  xmit[2] = value & 0xff;
  boolean b0, b;

  int k,m;
  for (m=0; m<3; m++) {
    for (k=0; k<8; k++) {
      b = (xmit[m] & 0x80) == 0x80;
      b0 = (reg & 0x01) == 0x01;
      reg = reg >> 1;
      if (b != b0) {
        reg |= 0x80;
        reg ^= 0x0C;
      }
      xmit[m] = xmit[m] << 1;
    }
  }
  if (crc != reg) {
    _last_error = 3; // BAD CRC
    return false;
  }
  return true;
}

bool SHT1X::done() {
  if (digitalRead(_data_pin) != LOW) {
    _last_error = 4; // NOT DONE
    return false;
  }
  return true;
}

bool SHT1X::check() {
  if (!_timeout.check())
     return false;

  if (_fail_count >= FAIL_RESET_THRESHOLD) {
    reset();
    _timeout.reset(RESET_INTERVAL);
    return false;
  }

  unsigned int data;
  byte crc;

  switch (_state) {
  case SHT1X_STATE_MEAS_TEMP:
    if (send_cmd(SHT1X_MEAS_TEMP_CMD)) {
      _state = SHT1X_STATE_READ_TEMP;
      _timeout.reset(MEAS_INTERVAL);
      return false; // quit method
    }  
    break;

  case SHT1X_STATE_READ_TEMP:
    if (done()) {
      read_data(&data, &crc);
      if (verify_crc8(SHT1X_MEAS_TEMP_CMD, data, crc)) {
        _raw_temp = data;
        _state = SHT1X_STATE_MEAS_RH;
        _timeout.reset(0); // immediately go to RH measurement next time
        return false; // quit method
      } 
    }  
    break;

  case SHT1X_STATE_MEAS_RH:
    if (send_cmd(SHT1X_MEAS_RH_CMD)) {
      _state = SHT1X_STATE_READ_RH;
      _timeout.reset(MEAS_INTERVAL);
      return false; // quit method
    }
    break;

  case SHT1X_STATE_READ_RH:
    if (done()) {
      read_data(&data, &crc);
      if (verify_crc8(SHT1X_MEAS_RH_CMD, data, crc)) {
        _raw_rh = data;
        _state = SHT1X_STATE_MEAS_TEMP;
        _last_error = 0;
        _timeout.reset(COOLDOWN_INTERVAL);
        return true; // all results are in (temp & rh)
      }
    }
  }
  // failed -- retry the same operation after a time
   _fail_count++;
   _timeout.reset(RETRY_INTERVAL);
  return false;
}
