#pragma once
#include <cstdint>
#include <stdfloat>
#include <string>
#include "stream.hpp"

#if __STDCPP_FLOAT128_T__ && __STDCPP_FLOAT64_T__ && __STDCPP_FLOAT32_T__ && __STDCPP_FLOAT16_T__
#define SUPPORTED_TYPES(X)            \
    X(INT,      1,  int32_t)          \
    X(UINT,     2,  uint32_t)         \
    X(LINT,     5,  int64_t)          \
    X(LUINT,    6,  uint64_t)         \
    X(SINT,     7,  int16_t)          \
    X(SUINT,    8,  uint16_t)         \
    X(CHAR,     9,  char)             \
    X(SCHAR,    10, int8_t)           \
    X(UCHAR,    11, uint8_t)          \
    X(WCHAR,    12, wchar_t)          \
    X(STRING8,  13, std::string)      \
    X(BOOL,     19, bool)             \
    X(S_WMML,   20, details::s_wmml)  \
    X(E_WMML,   21, details::e_wmml)  \
    X(SFLOAT,   22, std::float16_t)   \
    X(FLOAT,    23, std::float32_t)   \
    X(LFLOAT,   24, std::float64_t)   \
    X(LLFLOAT,  25, std::float128_t)  \
    X(CSTRTING, 26, detailds::string) \

#else
#warning Used compilator is not support fixed-size floating variables. Supporting variables with floating point has been turned off
#define SUPPORTED_TYPES(X)            \
    X(INT,      1,  int32_t)          \
    X(UINT,     2,  uint32_t)         \
    X(LINT,     5,  int64_t)          \
    X(LUINT,    6,  uint64_t)         \
    X(SINT,     7,  int16_t)          \
    X(SUINT,    8,  uint16_t)         \
    X(CHAR,     9,  char)             \
    X(SCHAR,    10, int8_t)           \
    X(UCHAR,    11, uint8_t)          \
    X(WCHAR,    12, wchar_t)          \
    X(STRING8,  13, std::string)      \
    X(BOOL,     19, bool)             \
    X(S_WMML,   20, details::s_wmml)  \
    X(E_WMML,   21, details::e_wmml)  \
    X(CSTRTING, 23, details::string) \

#endif


namespace details {
struct s_wmml {};
struct e_wmml {};
struct string {}; /// TODO: make the realization of compressed string to cheap writing to file
}





enum class WmmlFormats : uint8_t{
    ERROR = 0
#define X(name, num, type) ,name = num
    SUPPORTED_TYPES(X)
#undef X
};





template <typename T>
constexpr std::byte type_to_hash();

#define X(name, num, type) \
template<> constexpr std::byte type_to_hash<type> () { return static_cast<std::byte>(num); }
    SUPPORTED_TYPES(X)
#undef X





enum class LegacyWmmlFormats : uint8_t{
    // ERROR = 0,              // 0
    INT = 1,                // 1
    UNSIGNED_INT,           // 2
    LONG_INT,               // 3
    UNSIGNED_LONG_INT,      // 4
    LONG_LONG_INT,          // 5
    UNSIGNED_LONG_LONG_INT, // 6
    SHORT_INT,              // 7
    UNSIGNED_SHORT_INT,     // 8
                            //
    CHAR,                   // 9
    SIGNED_CHAR,            // 10
    UNSIGNED_CHAR,          // 11
    WCHAR_T,                // 12
    STRING,                 // 13
    STRING64K,              // 14
    STRING1KK,//STRING4KKK* // 15
                            //
    FLOAT,                  // 16
    DOUBLE,                 // 17
    LONG_DOUBLE,            // 18
                            //
    BOOL,                   // 19
    S_WMML, // start wmml   // 20
    E_WMML  //  end  wmml   // 21
};