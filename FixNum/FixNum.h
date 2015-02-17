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

#include "FmtUtil.h"
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

  // factory method to create from integer with appropriate scaling
  inline static FixNum<T,prec> scale(T x) { return x * multiplier; }

  // basic conversions and valid/invalid
  inline T mantissa()       { return _mantissa; }
  inline bool valid()       { return _mantissa < FixNumUtil::Limits<T>::maxValue && _mantissa > FixNumUtil::Limits<T>::minValue; }
  inline void setInvalid()  { _mantissa = invalid; }
  inline explicit operator bool() { return valid(); }
  inline bool operator!() { return !valid(); }

  // additional math
  T floor();

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

template<typename T, prec_t prec> T FixNum<T, prec>::floor() {
  if (_mantissa >= FixNumUtil::Limits<T>::maxValue)
    return FixNumUtil::Limits<T>::maxValue;
  if (_mantissa <= FixNumUtil::Limits<T>::minValue + multiplier - 1)
    return FixNumUtil::Limits<T>::minValue;
  return _mantissa >= 0 ? _mantissa / multiplier : (_mantissa - multiplier + 1) / multiplier;
}

template<typename T, prec_t prec> inline uint8_t FixNum<T, prec>::format(char* pos, prec_t size, fmt_t fmt) {
  return valid() ? formatDecimal(FixNumUtil::scale(_mantissa, prec, fmt & FMT_PREC), pos, size, fmt) : formatInvalid(pos, size, fmt);
}

template<typename T, prec_t prec> inline typename FixNum<T, prec>::Buf FixNum<T, prec>::format(uint8_t size, fmt_t fmt) {
  return Buf(*this, size, fmt);
}

template<typename T, prec_t prec> FixNum<T, prec>::Buf::Buf(FixNum<T, prec> num, uint8_t size, fmt_t fmt) {
  if (size > sizeof(_buf) - 1)
    size = sizeof(_buf) - 1; // trim to buf size
  uint8_t actualSize = num.format(_buf, size, fmt);
  if (fmt & FMT_RIGHT)
    _buf[size] = 0;
  else
    _buf[actualSize] = 0;
}

template<typename T, prec_t prec> template<typename T2, prec_t prec2> inline FixNum<T, prec>::operator FixNum<T2, prec2>() {
  return FixNum<T2, prec2>(FixNumUtil::convert<T, T2>(_mantissa, prec, prec2));
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

// ----------- Comparisons between fixnums -----------

template<typename T1, prec_t prec1, typename T2, prec_t prec2> bool operator ==(FixNum<T1, prec1> a, FixNum<T2, prec2> b) {
  if (!a || !b) 
    return false;
  typedef typename FixNumUtil::Common<T1, T2>::type T0;
  static const prec_t p0 = FixNumUtil::Max<prec1, prec2>::max;
  return FixNumUtil::scaleUp((T0)a.mantissa(), prec1, p0) == FixNumUtil::scaleUp((T0)b.mantissa(), prec2, p0);  
}

template<typename T1, prec_t prec1, typename T2, prec_t prec2> bool operator !=(FixNum<T1, prec1> a, FixNum<T2, prec2> b) {
  if (!a || !b) 
    return false;
  typedef typename FixNumUtil::Common<T1, T2>::type T0;
  static const prec_t p0 = FixNumUtil::Max<prec1, prec2>::max;
  return FixNumUtil::scaleUp((T0)a.mantissa(), prec1, p0) != FixNumUtil::scaleUp((T0)b.mantissa(), prec2, p0);  
}

template<typename T1, prec_t prec1, typename T2, prec_t prec2> bool operator < (FixNum<T1, prec1> a, FixNum<T2, prec2> b) {
  if (!a || !b) 
    return false;
  typedef typename FixNumUtil::Common<T1, T2>::type T0;
  static const prec_t p0 = FixNumUtil::Max<prec1, prec2>::max;
  return FixNumUtil::scaleUp((T0)a.mantissa(), prec1, p0) < FixNumUtil::scaleUp((T0)b.mantissa(), prec2, p0);  
}

template<typename T1, prec_t prec1, typename T2, prec_t prec2> bool operator <=(FixNum<T1, prec1> a, FixNum<T2, prec2> b) {
  if (!a || !b) 
    return false;
  typedef typename FixNumUtil::Common<T1, T2>::type T0;
  static const prec_t p0 = FixNumUtil::Max<prec1, prec2>::max;
  return FixNumUtil::scaleUp((T0)a.mantissa(), prec1, p0) <= FixNumUtil::scaleUp((T0)b.mantissa(), prec2, p0);  
}

template<typename T1, prec_t prec1, typename T2, prec_t prec2> bool operator > (FixNum<T1, prec1> a, FixNum<T2, prec2> b) {
  if (!a || !b) 
    return false;
  typedef typename FixNumUtil::Common<T1, T2>::type T0;
  static const prec_t p0 = FixNumUtil::Max<prec1, prec2>::max;
  return FixNumUtil::scaleUp((T0)a.mantissa(), prec1, p0) > FixNumUtil::scaleUp((T0)b.mantissa(), prec2, p0);  
}

template<typename T1, prec_t prec1, typename T2, prec_t prec2> bool operator >=(FixNum<T1, prec1> a, FixNum<T2, prec2> b) {
  if (!a || !b) 
    return false;
  typedef typename FixNumUtil::Common<T1, T2>::type T0;
  static const prec_t p0 = FixNumUtil::Max<prec1, prec2>::max;
  return FixNumUtil::scaleUp((T0)a.mantissa(), prec1, p0) >= FixNumUtil::scaleUp((T0)b.mantissa(), prec2, p0);  
}

// ----------- Arithmetics between fixnums -----------

template<typename T1, prec_t prec1, typename T2, prec_t prec2> FixNum<typename FixNumUtil::Common<T1, T2>::type, FixNumUtil::Max<prec1, prec2>::max> operator +(FixNum<T1, prec1> a, FixNum<T2, prec2> b) {
  typedef typename FixNumUtil::Common<T1, T2>::type T0;
  static const prec_t p0 = FixNumUtil::Max<prec1, prec2>::max;
  if (!a || !b) 
    return FixNum<T0, p0>::invalid;
  return FixNumUtil::scaleUp((T0)a.mantissa(), prec1, p0) + FixNumUtil::scaleUp((T0)b.mantissa(), prec2, p0);  
}

template<typename T1, prec_t prec1, typename T2, prec_t prec2> FixNum<typename FixNumUtil::Common<T1, T2>::type, FixNumUtil::Max<prec1, prec2>::max> operator -(FixNum<T1, prec1> a, FixNum<T2, prec2> b) {
  typedef typename FixNumUtil::Common<T1, T2>::type T0;
  static const prec_t p0 = FixNumUtil::Max<prec1, prec2>::max;
  if (!a || !b) 
    return FixNum<T0, p0>::invalid;
  return FixNumUtil::scaleUp((T0)a.mantissa(), prec1, p0) - FixNumUtil::scaleUp((T0)b.mantissa(), prec2, p0);  
}

template<typename T1, prec_t prec1, typename T2, prec_t prec2> FixNum<typename FixNumUtil::Common<T1, T2>::type, prec1 + prec2> operator *(FixNum<T1, prec1> a, FixNum<T2, prec2> b) {
  typedef typename FixNumUtil::Common<T1, T2>::type T0;
  static const prec_t p0 = prec1 + prec2;
  if (!a || !b) 
    return FixNum<T0, p0>::invalid;
  return (T0)a.mantissa() * (T0)b.mantissa();  
}

template<typename T1, prec_t prec1, typename T2, prec_t prec2> FixNum<typename FixNumUtil::Common<T1, T2>::type, FixNumUtil::Max<prec1, prec2>::max> operator /(FixNum<T1, prec1> a, FixNum<T2, prec2> b) {
  typedef typename FixNumUtil::Common<T1, T2>::type T0;
  static const prec_t p0 = FixNumUtil::Max<prec1, prec2>::max;
  if (!a || !b) 
    return FixNum<T0, p0>::invalid;
  return FixNumUtil::scaleUp((T0)a.mantissa(), prec1, p0 + prec2) / (T0)b.mantissa();  
}

// ----------- Comparisons between fixnums and integers -----------

template<typename T1, prec_t prec1, typename T2> bool operator ==(FixNum<T1, prec1> a, T2 b) { return a == (FixNum<T2,0>)b; }
template<typename T1, prec_t prec1, typename T2> bool operator !=(FixNum<T1, prec1> a, T2 b) { return a != (FixNum<T2,0>)b; }
template<typename T1, prec_t prec1, typename T2> bool operator < (FixNum<T1, prec1> a, T2 b) { return a <  (FixNum<T2,0>)b; }
template<typename T1, prec_t prec1, typename T2> bool operator <=(FixNum<T1, prec1> a, T2 b) { return a <= (FixNum<T2,0>)b; }
template<typename T1, prec_t prec1, typename T2> bool operator > (FixNum<T1, prec1> a, T2 b) { return a >  (FixNum<T2,0>)b; }
template<typename T1, prec_t prec1, typename T2> bool operator >=(FixNum<T1, prec1> a, T2 b) { return a >= (FixNum<T2,0>)b; }

template<typename T1, typename T2, prec_t prec2> bool operator ==(T1 a, FixNum<T2, prec2> b) { return (FixNum<T1,0>)a == b; }
template<typename T1, typename T2, prec_t prec2> bool operator !=(T1 a, FixNum<T2, prec2> b) { return (FixNum<T1,0>)a != b; }
template<typename T1, typename T2, prec_t prec2> bool operator < (T1 a, FixNum<T2, prec2> b) { return (FixNum<T1,0>)a <  b; }
template<typename T1, typename T2, prec_t prec2> bool operator <=(T1 a, FixNum<T2, prec2> b) { return (FixNum<T1,0>)a <= b; }
template<typename T1, typename T2, prec_t prec2> bool operator > (T1 a, FixNum<T2, prec2> b) { return (FixNum<T1,0>)a >  b; }
template<typename T1, typename T2, prec_t prec2> bool operator >=(T1 a, FixNum<T2, prec2> b) { return (FixNum<T1,0>)a >= b; }

// ----------- Arithmetics between fixnums and integers -----------

template<typename T1, prec_t prec1, typename T2> FixNum<typename FixNumUtil::Common<T1, T2>::type, prec1> operator +(FixNum<T1, prec1> a, T2 b) { return a + (FixNum<T2,0>)b; }
template<typename T1, prec_t prec1, typename T2> FixNum<typename FixNumUtil::Common<T1, T2>::type, prec1> operator -(FixNum<T1, prec1> a, T2 b) { return a - (FixNum<T2,0>)b; }
template<typename T1, prec_t prec1, typename T2> FixNum<typename FixNumUtil::Common<T1, T2>::type, prec1> operator *(FixNum<T1, prec1> a, T2 b) { return a * (FixNum<T2,0>)b; }
template<typename T1, prec_t prec1, typename T2> FixNum<typename FixNumUtil::Common<T1, T2>::type, prec1> operator /(FixNum<T1, prec1> a, T2 b) { return a / (FixNum<T2,0>)b; }

template<typename T1, typename T2, prec_t prec2> FixNum<typename FixNumUtil::Common<T1, T2>::type, prec2> operator +(T1 a, FixNum<T2, prec2> b) { return (FixNum<T1,0>)a + b; }
template<typename T1, typename T2, prec_t prec2> FixNum<typename FixNumUtil::Common<T1, T2>::type, prec2> operator -(T1 a, FixNum<T2, prec2> b) { return (FixNum<T1,0>)a - b; }
template<typename T1, typename T2, prec_t prec2> FixNum<typename FixNumUtil::Common<T1, T2>::type, prec2> operator *(T1 a, FixNum<T2, prec2> b) { return (FixNum<T1,0>)a * b; }
template<typename T1, typename T2, prec_t prec2> FixNum<typename FixNumUtil::Common<T1, T2>::type, prec2> operator /(T1 a, FixNum<T2, prec2> b) { return (FixNum<T1,0>)a / b; }

#endif
