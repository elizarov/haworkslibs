#include "DS18B20.h"

// Conversion period, 750 ms per spec
const int TEMP_INTERVAL = 1000;

// Scratch Pad Size with CRC
const uint8_t DS18B20_SPS = 9;

const uint8_t FAIL_LIMIT = 3;

DS18B20::DS18B20(uint8_t pin) :
  _wire(pin),
  _timeout(TEMP_INTERVAL)
{
  clear();
  startConversion();
}

void DS18B20::clear() {
  for (uint8_t i = 0; i < QUEUE_SIZE; i++)
    _queue[i] = NO_VAL;
  _size = 0;
  _value.setInvalid();
}

bool DS18B20::fail() {
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

bool DS18B20::check() {
  if (!_timeout.check())
    return false;
  _timeout.reset(TEMP_INTERVAL);
  int16_t val = readScratchPad();
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
    _value.setInvalid();
    // reset error
    _last_error = 0;
    result = true;
  }
  result |= startConversion();
  return result;
}

DS18B20::temp_t DS18B20::getTemp() {
  if (!_value && _size > 0)
    computeValue();
  return _value;
}

uint8_t DS18B20::getLastError() {
  return _last_error;
}

int16_t DS18B20::readScratchPad() {
  if (!_wire.reset()) {
    _last_error = 1;
    return NO_VAL;
  }
  _wire.skip();
  _wire.write(0xBE); // Read Scratchpad
  byte data[DS18B20_SPS];
  for (uint8_t i = 0; i < DS18B20_SPS; i++) // we need it with CRC
    data[i] = _wire.read();
  if (OneWire::crc8(&data[0], DS18B20_SPS - 1) != data[DS18B20_SPS - 1]) {
    _last_error = 2;
    return NO_VAL;
  }
  return (data[1] << 8) + data[0]; // take the two bytes from the response relating to temperature
}

bool DS18B20::startConversion() {
  if (!_wire.reset()) {
    _last_error = 3;
    return fail();
  }
  _wire.skip();
  _wire.write(0x44, 0); // start conversion
  return false;
}

void DS18B20::computeValue() {
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
  _value = temp_t(((long)sum * temp_t::multiplier) / (count << 4));
}

