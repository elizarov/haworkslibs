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
//
// Hardware connections and notes:
//
// The SHT1x sensor only has two signals, clock and data. The clock goes to 
// Arduino pin 3 and the data pin goes to pin 4. It is a good idea to include a 
// 1K series resistor on both pins. If there is any problem with data integrity,
// a 20k pull-up resistor can be attached to the data pin. The SHT1x sensors 
// run on the 5V supply, so level translation is not required. For future improvment,
// if a 10k pull-up resistor is installed on the data line, then driving the line
// from Arduino is simpler -- the data port bit can be set to zero and left there.
// then just changing the port from output to input mode is all that is needed to
// send a zero or one, or read an input bit.
//

#ifndef SHT1X_H_
#define SHT1X_H_

#include <Arduino.h>
#include <FixNum.h>
#include <Timeout.h>

class SHT1X
{
public:
  typedef fixnum16_1 temp_t;
  typedef fixnum16_1 rh_t;

  SHT1X(uint8_t clock_pin, uint8_t data_pin);
  bool check();
  temp_t getTemp();
  rh_t getRH();
  uint16_t getState(); // state & HEX error code for debugging

private:
  void DATA_LOW();
  void DATA_HIGH();
  void CLOCK_LOW();
  void CLOCK_HIGH();
  void reset();
  bool send_cmd(byte cmd);
  void read_data(unsigned int *value, byte *crc);
  bool verify_crc8(byte cmd, unsigned int value, byte crc);
  bool done();
  float rdg2temp(unsigned int rdg);
  float rdg2rh(unsigned int rdg, float temp);

  uint8_t _clk_pin;
  uint8_t _data_pin;
  uint8_t _state;
  uint8_t _last_error;
  Timeout _timeout;
  uint8_t _fail_count;

  unsigned int _raw_temp;
  unsigned int _raw_rh;
};

#endif
