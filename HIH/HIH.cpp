
#include "HIH.h"

static const uint8_t STATE_MEASURE = 0;
static const uint8_t STATE_RECEIVE = 1;

static const uint8_t ADDR = 0x27;
static const uint8_t BYTES = 4;
static const uint8_t RETRY_LIMIT = 3;
static const unsigned long MEASURE = 50; 
static const unsigned long RETRY = Timeout::SECOND; 
static const unsigned long WAIT = 5 * Timeout::SECOND; 

struct Data {
  uint16_t t; // temp 
  uint16_t h; // rh
  
  uint8_t set(uint8_t (&b)[BYTES]);
};

inline uint8_t Data::set(uint8_t (&b)[BYTES]) {
  h = (((uint16_t)b[0] & 0x3f) << 8) | b[1];
  t = ((uint16_t)b[2] << 6) | (b[3] >> 2);
  return b[0] >> 6;
}

HIH::HIH() : 
  _timeout(0)
{}

inline uint8_t HIH::receive() {
  // start twi master transaction
  uint8_t status;
  uint8_t b[BYTES];
  // there is no CRC, so keepBus to do second transfer without releasing twi bus to verify correctness 
  status = TWIMaster.receive(ADDR, b, BYTES, true);
  if (status != 0)
    return status;
  Data data1;
  if (data1.set(b) != 0) {
    TWIMaster.stop();
    return 1; // stale data fetched
  }
  // second transfer
  status = TWIMaster.receive(ADDR, b, BYTES);
  if (status != 0)
    return status;
  Data data2; // verify data
  if (data2.set(b) != 1)
    return 2; // should have been stale
  if (data1.h != data1.h || data2.t != data2.t)
    return 3; // different data second time
  // update temp and rh
  _temp = fixnum32_1::scale(data1.t) * 165 / ((1 << 14) - 2) - 40;
  if (!_temp)
    return 4; // invalid temp reading
  _rh = fixnum32_1::scale(data1.h) * 100 / ((1 << 14) - 2);
  if (!_rh)
    return 5; // invalid RH reading
  return 0;
}

bool HIH::retry() {
  _timeout.reset(RETRY);
  _state = STATE_MEASURE;
  if (_temp) {
    if (_retry_count++ >= RETRY_LIMIT) {
      _temp.clear();
      _rh.clear();
      return true;
    }
  }
  return false;
}

bool HIH::check() {
  if (!_timeout.check())
    return false;
  uint8_t status;
  switch (_state) {
  case STATE_MEASURE:
    status = TWIMaster.transmit(ADDR, 0, 0);
    if (status != 0) {
      _last_error = status;
      if (retry())
        return true;
    } else {
      _state = STATE_RECEIVE;
      _timeout.reset(MEASURE);
    }
    break;
  default: // STATE_RECEIVE
    status = receive();
    if (status != 0) {
      _last_error = status;
      if (retry())
        return true;
    } else {
      _retry_count = 0;
      _last_error = 0;
      _state = STATE_MEASURE;
      _timeout.reset(WAIT);
      return true;
    }
  }
  return false;
}
