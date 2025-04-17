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
   wmml(const std::filesystem::path& path, short unsigned size);
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
        if (size < 256)                                             return STRING;
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
// это можно попытаться разбить, сделав специализацию частную для строки
variant caseTemplate (char type) {
    switch (type) {
    case 0: {
       T out;
       targetFile.read(reinterpret_cast<char*>(&out), sizeof(out));
       return out;
       }
    case 1: {
       unsigned char size;
       targetFile.read(reinterpret_cast<char*>(&size), sizeof(size));
       std::string out(size, '\0');
       targetFile.read(out.data(), size);
       return out;
       }
    case 2: {
       unsigned short int size;
       targetFile.read(reinterpret_cast<char*>(&size), sizeof(size));
       std::string out(size, '\0');
       targetFile.read(out.data(), size);
       return out;
       }
    case 3: {
       unsigned int size;
       targetFile.read(reinterpret_cast<char*>(&size), sizeof(size));
       std::string out(size, '\0');
       targetFile.read(out.data(), size);
       return out;
       }
    }
    return NULL;
}


variant read_sector() {
   unsigned char id;
   targetFile.read(reinterpret_cast<char*>(&id), sizeof(id));
   switch (id) {
    case INT:                       return caseTemplate<int>(0);
    case UNSIGNED_INT:              return caseTemplate<unsigned int>(0);
    case LONG_INT:                  return caseTemplate<long int>(0);
    case UNSIGNED_LONG_INT:         return caseTemplate<unsigned long int>(0);
    case LONG_LONG_INT:             return caseTemplate<long long int>(0);
	case UNSIGNED_LONG_LONG_INT:	return caseTemplate<unsigned long long int>(0);
    case SHORT_INT:                 return caseTemplate<short int>(0);
    case UNSIGNED_SHORT_INT:        return caseTemplate<unsigned short int>(0);

    case CHAR:                      return caseTemplate<char>(0);
    case SIGNED_CHAR:               return caseTemplate<signed char>(0);
    case UNSIGNED_CHAR:             return caseTemplate<unsigned char>(0);
    case WCHAR_T:                   return caseTemplate<wchar_t>(0);
    case STRING:                    return caseTemplate<std::string>(1);
    case STRING64K:                 return caseTemplate<std::string>(2);
    case STRING1KK:                 return caseTemplate<std::string>(3);

    case FLOAT:                     return caseTemplate<float>(0);
    case DOUBLE:                    return caseTemplate<double>(0);
    case LONG_DOUBLE:               return caseTemplate<long double>(0);
    case WMML:                      return caseTemplate<wmml_marker>(0);

    case BOOL:                      return caseTemplate<bool>(0);
	default : throw "WMML ERROR (in reader): unknown type id";
   }
}


template <typename T>
void write_sector (T& t) {
   char hash = this_type(t);
   targetFile.write((&hash), sizeof(char));

   if constexpr (std::is_same<T, std::string>::value) {
        switch (hash) {
         case STRING: {
            char size = t.size();
            targetFile.write(reinterpret_cast<char*>(&size), sizeof(size));
            targetFile.write(reinterpret_cast<char*>(t.data()), size);
            }break;
         case STRING64K: {
            unsigned short int size = t.size();
            targetFile.write(reinterpret_cast<char*>(&size), sizeof(size));
            targetFile.write(reinterpret_cast<char*>(t.data()), size);
            }break;
         case STRING1KK: {
            unsigned int size = t.size();
            targetFile.write(reinterpret_cast<char*>(&size), sizeof(size));
            targetFile.write(reinterpret_cast<char*>(t.data()), size);
            }break;
        }
    }
    else {
        targetFile.write(reinterpret_cast<char*>(&t), sizeof(t));
    }
}


};   // wmml

// }    // stc
