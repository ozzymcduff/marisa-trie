#ifndef MARISA_BASE_H_
#define MARISA_BASE_H_

#ifndef _MSC_VER
#include <stdint.h>
#endif  // _MSC_VER

#include <cstddef>

namespace marisa {

#ifdef _MSC_VER
typedef unsigned __int8 UInt8;
typedef unsigned __int16 UInt16;
typedef unsigned __int32 UInt32;
typedef unsigned __int64 UInt64;
#else  // _MSC_VER
typedef ::uint8_t UInt8;
typedef ::uint16_t UInt16;
typedef ::uint32_t UInt32;
typedef ::uint64_t UInt64;
#endif  // _MSC_VER

}  // namespace marisa

#endif  // MARISA_BASE_H_
