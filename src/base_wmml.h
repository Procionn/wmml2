#pragma once
#ifdef WMML_STATIC
    #define WMML_API
#else
    #ifdef _WIN32
        #ifdef BUILDING_WMML_LIB
            #define WMML_API __declspec(dllexport)
        #else
            #define WMML_API __declspec(dllimport)
        #endif
    #else
        #define WMML_API __attribute__((visibility("default")))
    #endif
#endif

#include <variant>
#include <filesystem>
#include <fstream>
#include <vector>
#ifndef NDEBUG
    #include <iostream>
#endif

class wmml;
class wmml_marker;
class base_wmml;

struct wmml_archive_struct {
    wmml_marker* open();
    wmml_archive_struct(unsigned long long start, unsigned long long end, base_wmml* parent);
private:
    unsigned long long start;
    unsigned long long end;
    base_wmml* parent;
};





class WMML_API base_wmml
{
protected:
enum formats {
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
    STRING1KK,              // 15
                            //
    FLOAT,                  // 16
    DOUBLE,                 // 17
    LONG_DOUBLE,            // 18 
                            //
    BOOL,                   // 19
    S_WMML, // start wmml   // 20
    E_WMML  //  end  wmml   // 21
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
    bool
>;

protected:

    std::filesystem::path filePath;
    std::fstream          targetFile;
    short unsigned int    width_;
    unsigned int          height_;
    std::size_t           end_;
    char                  error_ = 0;

    std::vector<wmml_archive_struct*> archived_files;
    unsigned char         version = 2;
    unsigned int          archivedCount;
    unsigned int          localArchiveCount;
    unsigned long long    start;
    static constexpr int  divisor = 4096;
    static constexpr int  dataSize = sizeof(char)/*version*/ + sizeof(unsigned short)/*width_*/ + sizeof(unsigned) /*height_*/ + sizeof(unsigned) /*archivedCount*/;

    base_wmml() = default;
    virtual ~base_wmml();

public:

    unsigned         height() {
        return height_;
    }
    // Returns the number of objects in the file.

    unsigned short   width() {
        return width_;
    }
    // Returns the number of fields in the object.

    void         reset() {
        seek(start);
        localArchiveCount = archivedCount;
    }
    // Sets the object's state to its original state.

private:
    void move        (char* buffer, std::size_t& f_mark, std::size_t& s_mark, const int& size);
    void reverse_move(char* buffer, std::size_t& f_mark, std::size_t& s_mark, const int& size);

protected:
    bool     skip();
    void     seek(const std::size_t t);
    void     shift_data(const int& size, const std::size_t& f_mark);

    void     minimize_shift_data(std::size_t f_mark, std::size_t s_mark);
    void     maximize_shift_data(const std::size_t& start, std::size_t&& end);

    void     wmml_get();
    std::size_t size();
    void     relative_move(const std::size_t t);
    size_t   sector_size();
    void    resize(const long long& value);

#define read_sector_caseString(type) {                              \
    type size;                                                      \
    targetFile.read(reinterpret_cast<char*>(&size), sizeof(size));  \
    std::string out(size, '\0');                                    \
    targetFile.read(out.data(), size);                              \
    return out;}

    // char     this_type(T& t);
    // auto     read_sector_caseTemplate(); / define read_sector_caseString (type);
    // variant  read_sector();
    // void     write_sector (T& t);
    // size_t   object_size(const T& t);



template <typename T>
char this_type (const T& t) {
    if constexpr (std::is_same<T, int32_t>::value)                  return INT;
    if constexpr (std::is_same<T, uint32_t>::value)                 return UNSIGNED_INT;
    if constexpr (std::is_same<T, int64_t>::value)                  return LONG_INT;
    if constexpr (std::is_same<T, uint64_t>::value)                 return UNSIGNED_LONG_INT;
    if constexpr (std::is_same<T, long long>::value) {
        static_assert(sizeof(unsigned long long) == sizeof(uint64_t),
                      "FATAL COMPILE ERROR. Not supported architecture");
        return LONG_LONG_INT;
    }
    if constexpr (std::is_same<T, unsigned long long>::value) {
        static_assert(sizeof(unsigned long long) == sizeof(uint64_t),
                      "FATAL COMPILE ERROR. Not supported architecture");
        return UNSIGNED_LONG_LONG_INT;
    }
    if constexpr (std::is_same<T, int16_t>::value)                  return SHORT_INT;
    if constexpr (std::is_same<T, uint16_t>::value)                 return UNSIGNED_SHORT_INT;

    if constexpr (std::is_same<T, char>::value)                     return CHAR;
    if constexpr (std::is_same<T, signed char>::value)              return SIGNED_CHAR;
    if constexpr (std::is_same<T, unsigned char>::value)            return UNSIGNED_CHAR;
    if constexpr (std::is_same<T, wchar_t>::value)                  return WCHAR_T;
    if constexpr (std::is_same<T, std::string>::value) {
        std::size_t size = t.size();
        if      (size < 256)                                        return STRING;
        else if (size < 65535)                                      return STRING64K;
        else if (size < 4294967295)                                 return STRING1KK;
        else throw std::runtime_error("WMML ERROR: string is a very big!");
    }
    if constexpr (std::is_same<T, float>::value)                    return FLOAT;
    if constexpr (std::is_same<T, double>::value)                   return DOUBLE;
    if constexpr (std::is_same<T, long double>::value)              return LONG_DOUBLE;

    if constexpr (std::is_same<T, bool>::value)                     return BOOL;

    else throw std::runtime_error("WMML ERROR (in this_type): the method could not determine the hash for the object. " + std::string(__PRETTY_FUNCTION__));
}



template <typename T>
auto read_sector_caseTemplate () {
    T out;
    targetFile.read(reinterpret_cast<char*>(&out), sizeof(out));
    return out;
}


variant read_sector() {
    error_ = 0;
    unsigned char id = 0;
    targetFile.read(reinterpret_cast<char*>(&id), sizeof(id));
    switch (id) {
    case 0:                         error_ = 1; break;
    case INT:                       return read_sector_caseTemplate<int32_t>();
    case UNSIGNED_INT:              return read_sector_caseTemplate<uint32_t>();
    case LONG_INT:                  return read_sector_caseTemplate<int64_t>();
    case UNSIGNED_LONG_INT:         return read_sector_caseTemplate<uint64_t>();
    case LONG_LONG_INT:             return read_sector_caseTemplate<long long>();
    case UNSIGNED_LONG_LONG_INT:    return read_sector_caseTemplate<unsigned long long>();
    case SHORT_INT:                 return read_sector_caseTemplate<int16_t>();
    case UNSIGNED_SHORT_INT:        return read_sector_caseTemplate<uint16_t>();

    case CHAR:                      return read_sector_caseTemplate<char>();
    case SIGNED_CHAR:               return read_sector_caseTemplate<signed char>();
    case UNSIGNED_CHAR:             return read_sector_caseTemplate<unsigned char>();
    case WCHAR_T:                   return read_sector_caseTemplate<wchar_t>();
    case STRING:                    read_sector_caseString(unsigned char);
    case STRING64K:                 read_sector_caseString(uint16_t);
    case STRING1KK:                 read_sector_caseString(uint32_t);

    case FLOAT:                     return read_sector_caseTemplate<float>();
    case DOUBLE:                    return read_sector_caseTemplate<double>();
    case LONG_DOUBLE:               return read_sector_caseTemplate<long double>();

    case BOOL:                      return read_sector_caseTemplate<bool>();
    case E_WMML:                    error_ = 3; break;
    case S_WMML:                    error_ = 2; break;
    default: throw std::runtime_error(std::string("WMML ERROR (in reader): unknown type id ") + std::to_string(id) + " " + std::to_string(targetFile.tellg()));
    }
    return NULL;
}



template <typename T>
void write_sector (const T& t) {
    char hash = this_type(t);
    targetFile.write((&hash), sizeof(char));
    targetFile.write(reinterpret_cast<char*>(const_cast<T*>(&t)), sizeof(t));
}




template <typename T>
size_t object_size(const T& t) {
    switch (this_type(t)) {
    case INT:                       return sizeof(int32_t);
    case UNSIGNED_INT:              return sizeof(uint32_t);
    case LONG_INT:                  return sizeof(int64_t);
    case UNSIGNED_LONG_INT:         return sizeof(uint64_t);
    case LONG_LONG_INT:             return sizeof(int64_t);
    case UNSIGNED_LONG_LONG_INT:    return sizeof(uint64_t);
    case SHORT_INT:                 return sizeof(int16_t);
    case UNSIGNED_SHORT_INT:        return sizeof(uint16_t);

    case CHAR:                      return sizeof(char);
    case SIGNED_CHAR:               return sizeof(signed char);
    case UNSIGNED_CHAR:             return sizeof(unsigned char);
    case WCHAR_T:                   return sizeof(wchar_t);
    case STRING:                    return (sizeof(unsigned char) + t.size());
    case STRING64K:                 return (sizeof(uint16_t) + t.size());
    case STRING1KK:                 return (sizeof(uint32_t) + t.size());

    case FLOAT:                     return sizeof(float);
    case DOUBLE:                    return sizeof(double);
    case LONG_DOUBLE:               return sizeof(long double);

    case BOOL:                      return sizeof(bool);
    default: throw std::runtime_error(filePath.string() + " WMML ERROR (in object_size): unknown type id");
    }
}





template<typename T>
void overwriting(const int& object_index, const int& sector_index, const T& newData) {
    if (sector_index > width_)
        throw std::runtime_error("WMML ERROR: the field does not belong to the object");
    seek(start);
    std::size_t index = (object_index * width_) + sector_index;
    for (; index != 0; --index)
        if (!skip())
            throw std::runtime_error("WMML ERROR: the selected field does not belong to the file");

    const size_t f_mark = targetFile.tellg();
    const size_t freeSpaces = sector_size();
    const size_t newSectorSize = sizeof(char) + sizeof(T);

    long long shiftSize = newSectorSize - freeSpaces;
    if (shiftSize < 0) {
        minimize_shift_data(f_mark + freeSpaces, f_mark + newSectorSize);
        resize(shiftSize);
    }
    else if (shiftSize > 0) {
        auto oldEnd = end_;
        resize(shiftSize);
        maximize_shift_data(targetFile.tellg(), std::move(oldEnd));
    }

    seek(f_mark);
    write_sector(newData);
}

}; // base_wmml

template <>
void base_wmml::write_sector<std::string> (const std::string& t);



template <>
void base_wmml::overwriting<std::string> (const int& object_index, const int& sector_index, const std::string& newData);
