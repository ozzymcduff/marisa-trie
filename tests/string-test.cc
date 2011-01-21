#include <cassert>
#include <string>

#include <marisa/string.h>

int main() {
  marisa::String str;
  assert(str.ptr() == NULL);
  assert(str.length() == 0);

  marisa::RString rstr;
  assert(rstr.ptr() == NULL);
  assert(rstr.length() == 0);

  const char *s = "abc";
  str = marisa::String(s);
  assert(str.ptr() == s);
  assert(str.length() == 3);
  assert(str[0] == s[0]);
  assert(str[1] == s[1]);
  assert(str[2] == s[2]);

  rstr = marisa::RString(str);
  assert(rstr.ptr() == s);
  assert(rstr.length() == 3);
  assert(rstr[0] == s[2]);
  assert(rstr[1] == s[1]);
  assert(rstr[2] == s[0]);

  std::string s2 = "defg";
  str = marisa::String(s2.c_str(), (marisa::UInt32)s2.length());
  assert(str.ptr() == s2.c_str());
  assert(str.length() == 4);
  assert(str[0] == s2[0]);
  assert(str[1] == s2[1]);
  assert(str[2] == s2[2]);
  assert(str[3] == s2[3]);

  rstr = marisa::RString(str);
  assert(rstr.ptr() == s2.c_str());
  assert(rstr.length() == 4);
  assert(rstr[0] == s2[3]);
  assert(rstr[1] == s2[2]);
  assert(rstr[2] == s2[1]);
  assert(rstr[3] == s2[0]);

  assert(rstr.substr(1, 2).length() == 2);
  assert(rstr.substr(1, 2)[0] == 'f');
  assert(rstr.substr(1, 2)[1] == 'e');

  assert(marisa::String("abc") == marisa::String("abc"));
  assert(marisa::String("abc") != marisa::String("bcd"));
  assert(marisa::String("abc") < marisa::String("bcd"));
  assert(marisa::String("ab") < marisa::String("abc"));
  assert(marisa::String("bcd") > marisa::String("abc"));
  assert(marisa::String("abc") > marisa::String("ab"));

  assert(marisa::String("abcde").substr(2, 2) == marisa::String("cd"));

  return 0;
}
