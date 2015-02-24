// See DS3231.h for credits

#include "DS3231.h"
#include <avr/pgmspace.h>

#define DS3231_ADDRESS	      0x68 //I2C Slave address

/* DS3231 Registers. Refer Sec 8.2 of application manual */
#define DS3231_SEC_REG        0x00  
#define DS3231_MIN_REG        0x01  
#define DS3231_HOUR_REG       0x02
#define DS3231_WDAY_REG       0x03
#define DS3231_MDAY_REG       0x04
#define DS3231_MONTH_REG      0x05
#define DS3231_YEAR_REG       0x06

#define DS3231_AL1SEC_REG     0x07
#define DS3231_AL1MIN_REG     0x08
#define DS3231_AL1HOUR_REG    0x09
#define DS3231_AL1WDAY_REG    0x0A

#define DS3231_AL2MIN_REG     0x0B
#define DS3231_AL2HOUR_REG    0x0C
#define DS3231_AL2WDAY_REG    0x0D

#define DS3231_CONTROL_REG          0x0E
#define DS3231_STATUS_REG           0x0F
#define DS3231_AGING_OFFSET_REG     0x0F
#define DS3231_TMP_UP_REG           0x11
#define DS3231_TMP_LOW_REG          0x12

#define SECONDS_PER_DAY 86400L

////////////////////////////////////////////////////////////////////////////////
// utility code, some of this could be exposed in the DateTime API if needed

static const uint8_t daysInMonth [] PROGMEM = { 31,28,31,30,31,30,31,31,30,31,30,31 };

// number of days since 2000/01/01, valid for 2001..2099 (y is 01 to 99)
static uint16_t date2days(uint8_t y, uint8_t m, uint8_t d) {
    uint16_t days = d;
    for (uint8_t i = 1; i < m; ++i)
        days += pgm_read_byte(daysInMonth + i - 1);
    if (m > 2 && y % 4 == 0)
        ++days;
    return days + 365 * y + (y + 3) / 4 - 1;
}

static long time2long(uint16_t days, uint8_t h, uint8_t m, uint8_t s) {
    return ((days * 24L + h) * 60 + m) * 60 + s;
}

static uint8_t conv2d(const char* p) {
    uint8_t v = 0;
    if ('0' <= *p && *p <= '9')
        v = *p - '0';
    return 10 * v + *++p - '0';
}

////////////////////////////////////////////////////////////////////////////////
// DateTime implementation - ignores time zones and DST changes
// NOTE: also ignores leap seconds, see http://en.wikipedia.org/wiki/Leap_second

DateTime::DateTime(long t) {
    ss = t % 60;
    t /= 60;
    mm = t % 60;
    t /= 60;
    hh = t % 24;
    uint16_t days = t / 24;
    uint8_t leap;
    for (y = 0; ; ++y) {
        leap = y % 4 == 0;
        if (days < 365 + leap)
            break;
        days -= 365 + leap;
    }
    for (m = 1; ; ++m) {
        uint8_t daysPerMonth = pgm_read_byte(daysInMonth + m - 1);
        if (leap && m == 2)
            ++daysPerMonth;
        if (days < daysPerMonth)
            break;
        days -= daysPerMonth;
    }
    d = days + 1;
}

DateTime::DateTime (uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t min, uint8_t sec) {
    y = year;
    m = month;
    d = date;
    hh = hour;
    mm = min;
    ss = sec;
}

// A convenient constructor for using "the compiler's time":
//   DateTime now (__DATE__, __TIME__);
// NOTE: using PSTR would further reduce the RAM footprint
DateTime::DateTime(const char* date, const char* time) {
    // sample input: date = "Dec 26 2009", time = "12:34:56"
    y = conv2d(date + 9);
    // Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec 
    switch (date[0]) {
        case 'J': m = date[1] == 'a' ? 1 : m = date[2] == 'n' ? 6 : 7; break;
        case 'F': m = 2; break;
        case 'A': m = date[2] == 'r' ? 4 : 8; break;
        case 'M': m = date[2] == 'r' ? 3 : 5; break;
        case 'S': m = 9; break;
        case 'O': m = 10; break;
        case 'N': m = 11; break;
        case 'D': m = 12; break;
    }
    d = conv2d(date + 4);
    hh = conv2d(time);
    mm = conv2d(time + 3);
    ss = conv2d(time + 6);
}

long DateTime::get() const {
    uint16_t days = date2days(y, m, d);
    return time2long(days, hh, mm, ss);
}

DateTime::Buf::Buf(const DateTime& dt) {
  formatDecimal(dt.year(), &_buf[0], 2, FMT_ZERO);
  _buf[2] = '-';
  formatDecimal(dt.month(), &_buf[3], 2, FMT_ZERO);
  _buf[5] = '-';
  formatDecimal(dt.date(), &_buf[6], 2, FMT_ZERO);
  _buf[8] = ' ';
  formatDecimal(dt.hour(), &_buf[9], 2, FMT_ZERO);
  _buf[11] = ':';
  formatDecimal(dt.minute(), &_buf[12], 2, FMT_ZERO);
  _buf[14] = ':';
  formatDecimal(dt.second(), &_buf[15], 2, FMT_ZERO);
  _buf[17] = 0;
}

////////////////////////////////////////////////////////////////////////////////
// Utility methods

static uint8_t bcd2bin (uint8_t val) { return val - 6 * (val >> 4); }
static uint8_t bin2bcd (uint8_t val) { return val + 6 * (val / 10); }

////////////////////////////////////////////////////////////////////////////////
// RTC DS3231 implementation

// returns zero on error
uint8_t DS3231::readRegister(uint8_t regaddress) {
  uint8_t result = 0;
  _last_error = TWIMaster.transmitReceive(DS3231_ADDRESS, regaddress, result);
  return result;
}

// returns false on error
bool DS3231::writeRegister(uint8_t regaddress, uint8_t value) {
  uint8_t buf[2];
  buf[0] = regaddress;
  buf[1] = value;
  _last_error = TWIMaster.transmit(DS3231_ADDRESS, buf);
  return _last_error == 0;
}

//Adjust the time-date specified in DateTime format
//writing any non-existent time-data may interfere with normal operation of the RTC
bool DS3231::adjust(const DateTime& dt) {
  uint8_t buf[8];
  buf[0] = DS3231_SEC_REG;  //beginning from SEC Register address
  buf[1] = bin2bcd(dt.second());
  buf[2] = bin2bcd(dt.minute());
  buf[3] = bin2bcd((dt.hour()) & 0b10111111);
  buf[4] = 0; // day of week is not supported
  buf[5] = bin2bcd(dt.date());
  buf[6] = bin2bcd(dt.month());
  buf[7] = bin2bcd(dt.year());
  _last_error = TWIMaster.transmit(DS3231_ADDRESS, buf);
  return _last_error == 0;
}

//Read the current time-date and return it in DateTime format
DateTime DS3231::now() {
  uint8_t regaddress = DS3231_SEC_REG;
  uint8_t buf[7];
  _last_error = TWIMaster.transmitReceive(DS3231_ADDRESS, regaddress, buf);
  if (_last_error != 0)
    return DateTime();
  uint8_t ss = bcd2bin(buf[0]);
  uint8_t mm = bcd2bin(buf[1]);
  uint8_t hh = bcd2bin((buf[2] & (uint8_t)(~0b11000000))); //Ignore 24 Hour bit
  // skip day of week at buf[3]
  uint8_t d = bcd2bin(buf[4]);
  uint8_t m = bcd2bin(buf[5]);
  uint8_t y = bcd2bin(buf[6]);
  return DateTime(y, m, d, hh, mm, ss);
}

//Enable periodic interrupt at /INT pin. Supports only the level interrupt
//for consistency with other /INT interrupts. All interrupts works like single-shot counter
//Use refreshINTA() to re-enable interrupt.
bool DS3231::enableInterrupts(uint8_t periodicity) {
  bool ok = writeRegister(DS3231_CONTROL_REG, 0b00011101);
  switch(periodicity) {
  case EverySecond:
    ok = ok && writeRegister(DS3231_AL1SEC_REG,  0b10000000 ); //set AM1
    ok = ok && writeRegister(DS3231_AL1MIN_REG,  0b10000000 ); //set AM2
    ok = ok && writeRegister(DS3231_AL1HOUR_REG, 0b10000000 ); //set AM3
    ok = ok && writeRegister(DS3231_AL1WDAY_REG, 0b10000000 ); //set AM4
    break;
  case EveryMinute:
    ok = ok && writeRegister(DS3231_AL1SEC_REG,  0b00000000 ); //Clr AM1
    ok = ok && writeRegister(DS3231_AL1MIN_REG,  0b10000000 ); //set AM2
    ok = ok && writeRegister(DS3231_AL1HOUR_REG, 0b10000000 ); //set AM3
    ok = ok && writeRegister(DS3231_AL1WDAY_REG, 0b10000000 ); //set AM4
    break;
  case EveryHour:
    ok = ok && writeRegister(DS3231_AL1SEC_REG,  0b00000000 ); //Clr AM1
    ok = ok && writeRegister(DS3231_AL1MIN_REG,  0b00000000 ); //Clr AM2
    ok = ok && writeRegister(DS3231_AL1HOUR_REG, 0b10000000 ); //Set AM3
    ok = ok && writeRegister(DS3231_AL1WDAY_REG, 0b10000000 ); //set AM4
    break;
  default:
    ok = false;
  }
  return ok;
}

//Enable HH/MM/SS interrupt on /INTA pin. All interrupts works like single-shot counter
bool DS3231::enableInterrupts(uint8_t hh24, uint8_t mm, uint8_t ss) {
  bool ok = writeRegister(DS3231_CONTROL_REG, 0b00011101);
  ok = ok && writeRegister(DS3231_AL1SEC_REG,  0b00000000 | bin2bcd(ss)); //Clr AM1
  ok = ok && writeRegister(DS3231_AL1MIN_REG,  0b00000000 | bin2bcd(mm)); //Clr AM2
  ok = ok && writeRegister(DS3231_AL1HOUR_REG, (0b00000000 | (bin2bcd(hh24) & 0b10111111))); //Clr AM3
  ok = ok && writeRegister(DS3231_AL1WDAY_REG, 0b10000000 ); //set AM4
  return ok;
}

//Disable Interrupts. This is equivalent to begin() method.
bool DS3231::disableInterrupts() {
  return writeRegister(DS3231_CONTROL_REG, 0b00011100); // set default value of control register
}

//Clears the interrrupt flag in status register. 
//This is equivalent to preparing the DS3231 /INT pin to high for MCU to get ready for recognizing the next INT0 interrupt
bool DS3231::clearINTStatus() {
  // Clear interrupt flag 
  uint8_t statusReg = readRegister(DS3231_STATUS_REG);
  if (_last_error != 0) 
    return false;
  statusReg &= 0b11111110;
  return writeRegister(DS3231_STATUS_REG, statusReg);
}

// ========================================================================
// DS3231Temp class for oversampled temperature

const long TEMP_INTERVAL = 1000; // take a reading every second
const uint8_t FAIL_LIMIT = 3;

DS3231Temp::DS3231Temp() :
  _timeout(TEMP_INTERVAL)
{
  clear();
  startConversion();
}

void DS3231Temp::clear() {
  for (uint8_t i = 0; i < QUEUE_SIZE; i++)
    _queue[i] = NO_VAL;
  _size = 0;
  _temp.setInvalid();
}

//force temperature sampling and converting to registers. If this function is not used the temperature is sampled once 64 Sec.
bool DS3231Temp::startConversion() {
  // Set CONV 
  uint8_t ctReg = readRegister(DS3231_CONTROL_REG);
  if (_last_error != 0)
    return fail();
  ctReg |= 0b00100000; 
  if (!writeRegister(DS3231_CONTROL_REG, ctReg))
    return fail();
  return false;
}

bool DS3231Temp::fail() {
  if (_fail_count >= FAIL_LIMIT) {
    if (_size == 0)
      return false; // already cleared
    clear();
    return true;
  } else {
    _fail_count++;
    return false;
  }
}

bool DS3231Temp::check() {
  if (!_timeout.check())
    return false;
  _timeout.reset(TEMP_INTERVAL);
  int16_t val = readTemp();
  bool result;
  if (val == NO_VAL) {
    result = fail();
  } else {
    _fail_count = 0;
    // enqueue new value
    _queue[_index++] = val;
    if (_index == QUEUE_SIZE)
      _index = 0;
    if (_size < QUEUE_SIZE)
      _size++;
    // reset computed value
    _temp.setInvalid();
    result = true;
  }
  result |= startConversion();
  return result;
}

DS3231Temp::temp_t DS3231Temp::getTemp() {
  if (!_temp && _size > 0)
    computeTemp();
  return _temp;
}

void DS3231Temp::computeTemp() {
  int hi = INT_MIN;
  int lo = INT_MAX;
  int sum = 0;
  int count = 0;
  for (uint8_t i = 0; i < QUEUE_SIZE; i++)
    if (_queue[i] != NO_VAL) {
      sum += _queue[i];
      hi = max(hi, _queue[i]);
      lo = min(lo, _queue[i]);
      count++;
    }
  if (count > 2) {
    // drop outliers for even higher precision
    sum -= hi;
    sum -= lo;
    count -= 2;
  }
  _temp = temp_t(((long)sum * temp_t::multiplier) / (count << 2));
}

int16_t DS3231Temp::readTemp() {
  uint8_t regaddr = DS3231_TMP_UP_REG;
  int8_t buf[2];  // note: signed
  _last_error = TWIMaster.transmitReceive(DS3231_ADDRESS, regaddr, buf);
  if (_last_error != 0)
    return NO_VAL;
  return (int16_t(buf[0]) << 2) | (uint8_t(buf[1]) >> 6);
}
