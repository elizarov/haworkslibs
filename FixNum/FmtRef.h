#ifndef FMT_REF_H_
#define FMT_REF_H_

#include <FixNum.h>

class FmtRef {
public:
  inline FmtRef(char* buf)           { init(buf); }                    // first formatted number
  inline FmtRef(FmtRef& prev)        { init(prev._pos + prev._size); } // next formatted number
  inline FmtRef(char* buf, char tag) { init(findTag(buf, tag)); }      // first formatted number taged with a given tag

  template<typename T, prec_t prec> inline void operator =(FixNum<T, prec> x) {
    x.format(_pos, _size, _fmt); 
  }

  template<typename T> inline void operator =(T x) { 
    ((FixNum<T,0>)x).format(_pos, _size, _fmt); 
  }

private:
  char* _pos;
  uint8_t _size;    
  fmt_t _fmt;

  void init(char* pos);
  static char* findTag(char* pos, char tag);
};

#endif
