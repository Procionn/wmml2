#pragma once
#include <filesystem>
#include <fstream>
#include <vector>
#include <iostream>
#include <variant>

// namespace stc {



class wmml_marker {
};

#ifndef NDEBUG
template <typename T>
void use(T t) {
    std::cout << t << std::endl;
}

template <>
void use<wmml_marker>(wmml_marker t);
#endif






class wmml
{

enum formats {
   INT,
   UNSIGNED_INT,
   LONG_INT,
   UNSIGNED_LONG_INT,
   LONG_LONG_INT,
   UNSIGNED_LONG_LONG_INT,
   SHORT_INT,
   UNSIGNED_SHORT_INT,

   CHAR,
   SIGNED_CHAR,
   UNSIGNED_CHAR,
   WCHAR_T,
   STRING,
   STRING64K,
   STRING1KK,

   FLOAT,
   DOUBLE,
   LONG_DOUBLE,

   BOOL,
   WMML
};

public:

using variant = std::variant<
   int,
   unsigned int,
   long int,
   unsigned long int,
   long long int,
   unsigned long long int,
   short int,
   unsigned short int,
   char,
   signed char,
   unsigned char,
   wchar_t,
   std::string,
   float,
   double,
   long double,
   bool,
   wmml_marker
>;

private:

   std::fstream targetFile;
   short unsigned int width_;
   unsigned int height_;
public:
   wmml(const std::filesystem::path& path);
   wmml(const std::filesystem::path& path, short unsigned width);
   ~wmml();
   unsigned height();
   unsigned short width();
   void write(std::vector<variant>& writeble);
   bool read(std::vector<variant>& output);
private:
   bool skip();
// char this_type(T& t);
// auto caseTemplate ();
// variant read_sector();
// void write_sector (T& t);

template <typename T>
char this_type (T& t) {
   if constexpr (std::is_same<T, int>::value)                       return INT;
   if constexpr (std::is_same<T, unsigned int>::value)              return UNSIGNED_INT;
   if constexpr (std::is_same<T, long int>::value)                  return LONG_INT;
   if constexpr (std::is_same<T, unsigned long int>::value)         return UNSIGNED_LONG_INT;
   if constexpr (std::is_same<T, long long int>::value)             return LONG_LONG_INT;
   if constexpr (std::is_same<T, unsigned long long int>::value)    return UNSIGNED_LONG_LONG_INT;
   if constexpr (std::is_same<T, short int>::value)                 return SHORT_INT;
   if constexpr (std::is_same<T, unsigned short int>::value)        return UNSIGNED_SHORT_INT;

   if constexpr (std::is_same<T, char>::value)                      return CHAR;
   if constexpr (std::is_same<T, signed char>::value)               return SIGNED_CHAR;
   if constexpr (std::is_same<T, unsigned char>::value)             return UNSIGNED_CHAR;
   if constexpr (std::is_same<T, wchar_t>::value)                   return WCHAR_T;
   if constexpr (std::is_same<T, std::string>::value) {
        std::size_t size = t.size();
        if      (size < 256)                                        return STRING;
        else if (size < 65535)                                      return STRING64K;
        else if (size < 4294967296)                                 return STRING1KK;
        else throw "WMML ERROR: string is a very big!";
   }
   if constexpr (std::is_same<T, float>::value)                     return FLOAT;
   if constexpr (std::is_same<T, double>::value)                    return DOUBLE;
   if constexpr (std::is_same<T, long double>::value)               return LONG_DOUBLE;

   if constexpr (std::is_same<T, bool>::value)                      return BOOL;
   if constexpr (std::is_same<T, wmml_marker>::value)               return WMML;
}



template <typename T>
auto read_sector_caseTemplate () {
   T out;
   targetFile.read(reinterpret_cast<char*>(&out), sizeof(out));
   return out;
}

std::string read_sector_caseString (char type);


variant read_sector() {
   unsigned char id;
   targetFile.read(reinterpret_cast<char*>(&id), sizeof(id));
   switch (id) {
    case INT:                       return read_sector_caseTemplate<int>();
    case UNSIGNED_INT:              return read_sector_caseTemplate<unsigned int>();
    case LONG_INT:                  return read_sector_caseTemplate<long int>();
    case UNSIGNED_LONG_INT:         return read_sector_caseTemplate<unsigned long int>();
    case LONG_LONG_INT:             return read_sector_caseTemplate<long long int>();
    case UNSIGNED_LONG_LONG_INT:	return read_sector_caseTemplate<unsigned long long int>();
    case SHORT_INT:                 return read_sector_caseTemplate<short int>();
    case UNSIGNED_SHORT_INT:        return read_sector_caseTemplate<unsigned short int>();

    case CHAR:                      return read_sector_caseTemplate<char>();
    case SIGNED_CHAR:               return read_sector_caseTemplate<signed char>();
    case UNSIGNED_CHAR:             return read_sector_caseTemplate<unsigned char>();
    case WCHAR_T:                   return read_sector_caseTemplate<wchar_t>();
    case STRING:                    return read_sector_caseString(1);
    case STRING64K:                 return read_sector_caseString(2);
    case STRING1KK:                 return read_sector_caseString(3);

    case FLOAT:                     return read_sector_caseTemplate<float>();
    case DOUBLE:                    return read_sector_caseTemplate<double>();
    case LONG_DOUBLE:               return read_sector_caseTemplate<long double>();

    case WMML:                      return read_sector_caseTemplate<wmml_marker>();
    case BOOL:                      return read_sector_caseTemplate<bool>();
	default : throw "WMML ERROR (in reader): unknown type id";
   }
}



template <typename T>
void write_sector (T& t) {
   char hash = this_type(t);
   targetFile.write((&hash), sizeof(char));
   targetFile.write(reinterpret_cast<char*>(&t), sizeof(t));
}

};   // wmml

template <>
void wmml::write_sector<std::string> (std::string& t);

// }    // stc
