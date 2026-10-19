#pragma once
#include <string>
#include <cstdint>
#include "stream.hpp"

#if 0

namespace wmml_legacy {
#define read_sector_caseString(type) {                              \
    type size;                                                      \
    targetFile.read(reinterpret_cast<char*>(&size), sizeof(size));  \
    std::string out(size, '\0');                                    \
    targetFile.read(out.data(), size);                              \
    return out;}


template <typename T>
auto read_sector_caseTemplate (stream& s) {
    T out;
    s.read(reinterpret_cast<std::byte*>(&out), sizeof(out));
    return out;
}


// variant read_sector_hash(stream& s) {
//     error_ = 0;
//     unsigned char id = 0;
//     targetFile.read(reinterpret_cast<char*>(&id), sizeof(id));
//     switch (id) {
//     case 0:                         error_ = 1; break;
//     case INT:                       return read_sector_caseTemplate<int32_t>();
//     case UNSIGNED_INT:              return read_sector_caseTemplate<uint32_t>();
//     case LONG_INT:                  return read_sector_caseTemplate<long>();
//     case UNSIGNED_LONG_INT:         return read_sector_caseTemplate<unsigned long>();
//     case LONG_LONG_INT:             return read_sector_caseTemplate<long long>();
//     case UNSIGNED_LONG_LONG_INT:    return read_sector_caseTemplate<unsigned long long>();
//     case SHORT_INT:                 return read_sector_caseTemplate<int16_t>();
//     case UNSIGNED_SHORT_INT:        return read_sector_caseTemplate<uint16_t>();

//     case CHAR:                      return read_sector_caseTemplate<char>();
//     case SIGNED_CHAR:               return read_sector_caseTemplate<signed char>();
//     case UNSIGNED_CHAR:             return read_sector_caseTemplate<unsigned char>();
//     case WCHAR_T:                   return read_sector_caseTemplate<wchar_t>();
//     case STRING:                    read_sector_caseString(unsigned char);
//     case STRING64K:                 read_sector_caseString(uint16_t);
//     case STRING1KK:                 read_sector_caseString(uint32_t);

//     case FLOAT:                     return read_sector_caseTemplate<float>();
//     case DOUBLE:                    return read_sector_caseTemplate<double>();
//     case LONG_DOUBLE:               return read_sector_caseTemplate<long double>();

//     case BOOL:                      return read_sector_caseTemplate<bool>();
//     case E_WMML:                    error_ = 3; break;
//     case S_WMML:                    error_ = 2; break;
//     default: throw std::runtime_error(std::string("WMML ERROR (in reader): unknown type id ") + std::to_string(id) + " " + std::to_string(targetFile.tellg()));
//     }
//     return NULL;
// }



} // namespace wmml_legacy

#endif