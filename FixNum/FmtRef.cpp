#include <FmtRef.h>

void FmtRef::init(char* pos) {
  char c;
  while (true) {
    c = *pos;
    if (c == 0 || c == '.' || c == '+' || c == '-' || c == '?' || (c >= '0' && c <= '9')) 
      break;
    pos++;
  }
  _pos = pos;
  _size = 0;
  _fmt = 0;
  bool dot = false;
  if (c == '+' || c == '-') {
    _fmt |= FMT_SIGN;
    _size++;
    c = *(++pos);
  }
  while (c != 0) {
    if (c == '.' && !dot)
      dot = true;
    else if (c == '?' || (c >= '0' && c <= '9')) {
      // ok - go next char
      if (dot)
        _fmt++;
    } else
      break; // done
    _size++;
    c = *(++pos);
  }
}

char* FmtRef::findTag(char* pos, char tag) {
  while (true) {
    char c = *pos;
    if (c == 0)
      return pos;
    if (c == tag)
      return pos + 1;
    pos++;
  } 
}
