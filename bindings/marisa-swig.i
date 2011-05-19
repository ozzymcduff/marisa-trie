%module marisa

%include "cstring.i"
%include "exception.i"

%{
#include "marisa-swig.h"
%}

%apply (char *STRING, int LENGTH) { (const char *ptr, std::size_t length) };

%cstring_output_allocate_size(const char **ptr, std::size_t *length, );

%exception {
  try {
    $action
  } catch (const marisa::Exception &ex) {
    SWIG_exception(SWIG_RuntimeError, ex.what());
  } catch (...) {
    SWIG_exception(SWIG_UnknownError,"Unknown exception");
  }
}

%include "marisa-swig.h"
