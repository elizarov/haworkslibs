#ifndef FIXNUM_H_
#define FIXNUM_H_

/*
  Defines class for efficient manipulation of decimal number with fixed decimal point. The following base types <T> are supported:

    uint8_t -- valid mantissa from 1 to 254
    int8_t  -- valid mantissa from -127 to 127
    int16_t -- valid mantissa from -32767 to 32767
    int32_t -- valid mantiass from -2147483647 to 2147483647

  Supports precision values <prec> of 0, 1, 2, 3, 4, 5, 6 (large values only make sense for int32_t)

  There is also an efficient state-machine driven parser.
*/

#include "utility/FmtUtil.h"
#include "utility/FixNumUtil.h"

//============================ Public API for FixNum library ============================

// FixNum class
template<typename T, uint8_t prec> class FixNum {
private:
  T _mantissa;
public:

  // constants
  static const T invalid = FixNumUtil::Limits<T>::maxValue;
  static const T multiplier = FixNumUtil::Multiplier<prec>::multiplier;

  // constructors
  inline FixNum() : _mantissa(invalid) {}
  inline FixNum(T mantissa) : _mantissa(mantissa) {}

  // basic conversions and valid/invalid
  inline T mantissa()       { return _mantissa; }
  inline bool valid()       { return _mantissa < FixNumUtil::Limits<T>::maxValue && _mantissa > FixNumUtil::Limits<T>::minValue; }
  inline void setInvalid()  { _mantissa = invalid; }
  inline explicit operator bool() { return valid(); }

  // Buffer class with temporary char array
  class Buf {
  private:
    char _buf[FixNumUtil::Limits<T>::bufSize];
  public:
    Buf(FixNum<T, prec> num, uint8_t size, fmt_t fmt);
    inline operator char*() { return _buf; }
  };

  // formatting
  uint8_t format(char* pos, uint8_t size, fmt_t fmt = (fmt_t)prec);   

  // formats to temporary buffer, with Ardino you can use Serial.println(num.format());
  Buf format(uint8_t size = FixNumUtil::Limits<T>::bufSize - 1, fmt_t fmt = (fmt_t)prec); 

  // casts between various fixnum types
  template<typename T2, prec_t prec2> operator FixNum<T2, prec2>();

  // Comparisons
  template<typename T2, prec_t prec2> bool operator ==(FixNum<T2, prec2> other);
  template<typename T2, prec_t prec2> bool operator !=(FixNum<T2, prec2> other);
  template<typename T2, prec_t prec2> bool operator < (FixNum<T2, prec2> other);
  template<typename T2, prec_t prec2> bool operator <=(FixNum<T2, prec2> other);
  template<typename T2, prec_t prec2> bool operator > (FixNum<T2, prec2> other);
  template<typename T2, prec_t prec2> bool operator >=(FixNum<T2, prec2> other);
};

// Basic fixnum instances
typedef FixNum<uint8_t,0> ufixnum8_0;
typedef FixNum<uint8_t,1> ufixnum8_1;
typedef FixNum<uint8_t,2> ufixnum8_2;

typedef FixNum<int8_t,0> fixnum8_0;
typedef FixNum<int8_t,1> fixnum8_1;
typedef FixNum<int8_t,2> fixnum8_2;

typedef FixNum<int16_t,0> fixnum16_0;
typedef FixNum<int16_t,1> fixnum16_1;
typedef FixNum<int16_t,2> fixnum16_2;
typedef FixNum<int16_t,3> fixnum16_3;
typedef FixNum<int16_t,4> fixnum16_4;

typedef FixNum<int32_t,0> fixnum32_0;
typedef FixNum<int32_t,1> fixnum32_1;
typedef FixNum<int32_t,2> fixnum32_2;
typedef FixNum<int32_t,3> fixnum32_3;
typedef FixNum<int32_t,4> fixnum32_4;
typedef FixNum<int32_t,5> fixnum32_5;
typedef FixNum<int32_t,6> fixnum32_6;

// Parser for FixNum
template<typename T> class FixNumParser {
  private:
    enum State {
      START,
      INT_PART,
      FRAC_PART
    };
    
    bool _neg;
    bool _ok;
    T _mantissa;
    prec_t _prec;
    State _state;
  public:
    enum Result {
      BAD,
      NUM,
      DONE
    };
    static const prec_t MAX_PREC = 6;

    void reset();    
    Result parse(char ch);
    template<typename T2, prec_t prec2> operator FixNum<T2, prec2>();
};

// ----------- class FixNum implementation -----------

template<typename T, prec_t prec> inline uint8_t FixNum<T, prec>::format(char* pos, prec_t size, fmt_t fmt) {
  return valid() ? formatDecimal(FixNumUtil::scale(_mantissa, prec, fmt & FMT_PREC), pos, size, fmt) : formatInvalid(pos, size, fmt);
}

template<typename T, prec_t prec> inline typename FixNum<T, prec>::Buf FixNum<T, prec>::format(uint8_t size, fmt_t fmt) {
  return Buf(*this, size, fmt);
}

template<typename T, prec_t prec> FixNum<T, prec>::Buf::Buf(FixNum<T, prec> num, uint8_t size, fmt_t fmt) {
  size = FixNumUtil::min(size, sizeof(_buf) - 1); // trim to buf size
  uint8_t actualSize = num.format(_buf, size, fmt);
  if (fmt & FMT_RIGHT)
    _buf[size] = 0;
  else
    _buf[actualSize] = 0;
}

template<typename T, prec_t prec> template<typename T2, prec_t prec2> inline FixNum<T, prec>::operator FixNum<T2, prec2>() {
  return FixNum<T2, prec2>(FixNumUtil::convert<T, T2>(_mantissa, prec, prec2));
}

template<typename T, prec_t prec> template<typename T2, prec_t prec2> bool FixNum<T, prec>::operator ==(FixNum<T2, prec2> other) {
  if (!valid() || !other.valid())
    return false;
  typedef typename FixNumUtil::Common<T, T2>::type T0;
  prec_t p0 = FixNumUtil::max(prec, prec2);
  T0 x1 = FixNumUtil::scale((T0)_mantissa, prec, p0);
  T0 x2 = FixNumUtil::scale((T0)other.mantissa(), prec2, p0);
  return x1 == x2;  
}

template<typename T, prec_t prec> template<typename T2, prec_t prec2> bool FixNum<T, prec>::operator !=(FixNum<T2, prec2> other) {
  if (!valid() || !other.valid())
    return false;
  typedef typename FixNumUtil::Common<T, T2>::type T0;
  prec_t p0 = FixNumUtil::max(prec, prec2);
  T0 x1 = FixNumUtil::scale((T0)_mantissa, prec, p0);
  T0 x2 = FixNumUtil::scale((T0)other.mantissa(), prec2, p0);
  return x1 != x2;  
}
  
template<typename T, prec_t prec> template<typename T2, prec_t prec2> bool FixNum<T, prec>::operator < (FixNum<T2, prec2> other) {
  if (!valid() || !other.valid())
    return false;
  typedef typename FixNumUtil::Common<T, T2>::type T0;
  prec_t p0 = FixNumUtil::max(prec, prec2);
  T0 x1 = FixNumUtil::scale((T0)_mantissa, prec, p0);
  T0 x2 = FixNumUtil::scale((T0)other.mantissa(), prec2, p0);
  return x1 < x2;  
}
  
template<typename T, prec_t prec> template<typename T2, prec_t prec2> bool FixNum<T, prec>::operator <=(FixNum<T2, prec2> other) {
  if (!valid() || !other.valid())
    return false;
  typedef typename FixNumUtil::Common<T, T2>::type T0;
  prec_t p0 = FixNumUtil::max(prec, prec2);
  T0 x1 = FixNumUtil::scale((T0)_mantissa, prec, p0);
  T0 x2 = FixNumUtil::scale((T0)other.mantissa(), prec2, p0);
  return x1 <= x2;  
}
  
template<typename T, prec_t prec> template<typename T2, prec_t prec2> bool FixNum<T, prec>::operator > (FixNum<T2, prec2> other) {
  if (!valid() || !other.valid())
    return false;
  typedef typename FixNumUtil::Common<T, T2>::type T0;
  prec_t p0 = FixNumUtil::max(prec, prec2);
  T0 x1 = FixNumUtil::scale((T0)_mantissa, prec, p0);
  T0 x2 = FixNumUtil::scale((T0)other.mantissa(), prec2, p0);
  return x1 > x2;  
}
  
template<typename T, prec_t prec> template<typename T2, prec_t prec2> bool FixNum<T, prec>::operator >=(FixNum<T2, prec2> other) {
  if (!valid() || !other.valid())
    return false;
  typedef typename FixNumUtil::Common<T, T2>::type T0;
  prec_t p0 = FixNumUtil::max(prec, prec2);
  T0 x1 = FixNumUtil::scale((T0)_mantissa, prec, p0);
  T0 x2 = FixNumUtil::scale((T0)other.mantissa(), prec2, p0);
  return x1 >= x2;  
}

// ----------- class FixNumParser implementation -----------

template<typename T> inline void FixNumParser<T>::reset() {
  _state = START;
}

template<typename T> typename FixNumParser<T>::Result FixNumParser<T>::parse(char ch) {
  switch (_state) {
    case START:
      if (ch == '+' || ch == '-' || ch == '.' || (ch >= '0' && ch <= '9')) {
        _neg = ch == '-';
        _ok = (ch >= '0' && ch <= '9');
        _mantissa = (ch >= '0' && ch <= '9') ? ch - '0' : 0;
        _prec = 0;
        _state = (ch == '.') ? FRAC_PART : INT_PART;
        return NUM;
      } else
        return BAD;
    case INT_PART:
      if (ch == '.' ) {
          _state = FRAC_PART;
          return NUM;
      }
      // falls through!!!
    case FRAC_PART: 
      if (ch >= '0' && ch <= '9') {
        if (_state == FRAC_PART) {
          if (_prec >= MAX_PREC)
            break; // skip the rest
          _prec++; 
        }
        _ok = true;
        prec_t digit = ch - '0';
        if (_mantissa > (FixNumUtil::Limits<T>::maxValue - 9) / 10)
          _mantissa = FixNumUtil::Limits<T>::maxValue;
        else
          _mantissa = _mantissa * 10 + digit;
        return NUM;
      }
      // fall through to the end of method
  }
  // number is over
  _state = START;
  return _ok ? DONE : BAD;
}

template<typename T> template<typename T2, prec_t prec2> inline FixNumParser<T>::operator FixNum<T2, prec2>() {
  T x = _neg ? -_mantissa : _mantissa;
  return FixNum<T2, prec2>(FixNumUtil::convert<T, T2>(x, _prec, prec2));  
}

#endif
