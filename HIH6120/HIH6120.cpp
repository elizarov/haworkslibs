
#include "HIH6120.h"

uint8_t HIH6120::Data::set(uint8_t (&b)[BYTES]) {
  h = (((uint16_t)b[0] & 0x3f) << 8) | b[1];
  t = ((uint16_t)b[2] << 6) | (b[3] >> 2);
  return b[0] >> 6;
}

HIH6120::HIH6120(twi_speed_t twi_speed) : 
  _twi_speed(twi_speed),
  _timeout(0),
  _measure(true)
{}

uint8_t HIH6120::receive() {
  // start twi master transaction
  TWIMaster twiMaster(_twi_speed); 
  uint8_t status;
  uint8_t b[BYTES];
  status = twiMaster.receive(ADDR, b, BYTES);
  if (status != 0)
    return status;
  if (_data.set(b) != 0)
    return 1; // stale data fetched
  // there is no CRC, so do second transfer without releasing twi bus to verify correctness 
  status = twiMaster.receive(ADDR, b, BYTES);
  if (status != 0)
    return status;
  Data vdata; // verify data
  if (vdata.set(b) != 1)
    return 1; // should have been stale
  if (vdata.h != _data.h || vdata.t != _data.t)
    return 1; // different data second time
  return 0;
}

void HIH6120::retry() {
  _timeout.reset(RETRY);
  _measure = true;
  if (_valid) {
    if (_retry_count++ >= RETRY_LIMIT)
      _valid = false;
  }
}

bool HIH6120::check() {
  if (!_timeout.check())
    return false;
  if (_measure) {
    uint8_t status = TWIMaster(_twi_speed).transmit(ADDR, 0, 0);
    if (status != 0) {
      retry();
    } else {
      _measure = false;
      _timeout.reset(MEASURE);
    }
  } else {
    uint8_t status = receive();
    if (status != 0) {
      retry();
    } else {
      _valid = true;
      _retry_count = 0;
      _measure = true;
      _timeout.reset(WAIT);
      return true;
    }
  }
  return false;
}

HIH6120::rh_t HIH6120::getRH() {
  if (!_valid)
     return rh_t::invalid;
  return fixnum32_1::scale(_data.h) * 100 / ((1 << 14) - 1);
}

HIH6120::temp_t HIH6120::getTemp() {
  if (!_valid)
     return temp_t::invalid;
  return fixnum32_1::scale(_data.t) * 165 / ((1 << 14) - 1) - 40;
}
