#pragma once
#include <variant>
#include <filesystem>
#include <fstream>
#include <vector>

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





class base_wmml
{
protected:
enum formats {
    INT = 1,
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
    S_WMML,      // start wmml
    E_WMML       //  end  wmml
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

    std::filesystem::path file_path;
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

public:

    unsigned         height();
    // Returns the number of objects in the file.

    unsigned short   width();
    // Returns the number of fields in the object.
protected:
    bool     skip();
    void     seek(std::size_t t);
    void     shift_data(const int& size, const std::size_t& f_mark);
    void     wmml_get();
    // char     this_type(T& t);
    // auto     read_sector_caseTemplate(); / std::string read_sector_caseString (char type);
    // variant  read_sector();
    // void     write_sector (T& t);        / write_sector<std::string> (std::string& t)

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
        else if (size < 4294967295)                                 return STRING1KK;
        else throw "WMML ERROR: string is a very big!";
    }
    if constexpr (std::is_same<T, float>::value)                     return FLOAT;
    if constexpr (std::is_same<T, double>::value)                    return DOUBLE;
    if constexpr (std::is_same<T, long double>::value)               return LONG_DOUBLE;

    if constexpr (std::is_same<T, bool>::value)                      return BOOL;
}



template <typename T>
auto read_sector_caseTemplate () {
    T out;
    targetFile.read(reinterpret_cast<char*>(&out), sizeof(out));
    return out;
}

std::string read_sector_caseString (char type);


variant read_sector() {
    error_ = 0;
    unsigned char id = 0;
    targetFile.read(reinterpret_cast<char*>(&id), sizeof(id));
    switch (id) {
    case 0:                         error_ = 1; break;
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

    case BOOL:                      return read_sector_caseTemplate<bool>();
    case E_WMML:                    error_ = 3; break;
    case S_WMML:                    error_ = 2; break;
    default : throw "WMML ERROR (in reader): unknown type id";
    }
    return NULL;
}



template <typename T>
void write_sector (T& t) {
    char hash = this_type(t);
    targetFile.write((&hash), sizeof(char));
    targetFile.write(reinterpret_cast<char*>(&t), sizeof(t));
}




template<typename T>
void overwriting(int& object_index, int& sector_index, T& new_data) {
    if (sector_index > width_)
        throw "WMML ERROR: the field does not belong to the object";
    targetFile.seekp(start);
    std::size_t index = (object_index * width_) + sector_index;
    for (; index != 0; --index)
        if (!skip())
            throw "WMML ERROR: the selected field does not belong to the file";

    char new_id = this_type(new_data);
    unsigned char id = 0;
    targetFile.read(reinterpret_cast<char*>(&id), sizeof(id));
    targetFile.seekp(targetFile.tellg());
    if (new_id != id)
        throw "WMML ERROR (in overwriting): the type of the old object does not match the new one";

    targetFile.write(reinterpret_cast<char*>(&new_data), sizeof(new_data));
}

}; // base_wmml

template <>
void base_wmml::write_sector<std::string> (std::string& t);
