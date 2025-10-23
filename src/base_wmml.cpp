#include "base_wmml.h"


base_wmml::~base_wmml () {
    for (const auto* pointer : archived_files)
        delete pointer;
}



bool base_wmml::skip () {
    auto mark = targetFile.tellp();
    size_t size = sector_size();
    if (error_ == 1)
        return false;
    else if (error_ == 2 | error_ == 3)
        std::runtime_error("WMML ERROR (in skip): the method detected the boundaries of the attached file when crossing the sector.");
    seek(mark);
    relative_move(size);
    if (targetFile.eof())
        return false;
    else return true;
}


void base_wmml::seek(std::size_t t) {
#ifdef WIN32
    targetFile.seekp(t);
    targetFile.seekg(t);
#elif __linux__
    targetFile.seekp(t);
#endif
}


void base_wmml::relative_move(std::size_t t) {
#ifdef WIN32
//    targetFile.seekp(t, std::ios::cur);
    targetFile.seekg(t, std::ios::cur);
#elif __linux__
    targetFile.seekp(t, std::ios::cur);
#endif
}



template <>
void base_wmml::write_sector<std::string> (const std::string& t) {
    char hash = this_type(t);
    targetFile.write((&hash), sizeof(char));
    switch (hash) {
        case STRING: {
            unsigned char size = t.size();
            targetFile.write(reinterpret_cast<char*>(&size), sizeof(size));
            targetFile.write(reinterpret_cast<char*>(const_cast<char*>(t.data())), size);
        }break;
        case STRING64K: {
            uint16_t size = t.size();
            targetFile.write(reinterpret_cast<char*>(&size), sizeof(size));
            targetFile.write(reinterpret_cast<char*>(const_cast<char*>(t.data())), size);
        }break;
        case STRING1KK: {
            uint32_t size = t.size();
            targetFile.write(reinterpret_cast<char*>(&size), sizeof(size));
            targetFile.write(reinterpret_cast<char*>(const_cast<char*>(t.data())), size);
        }break;
    }
}



void base_wmml::resize(const long long& value) {
    end_ += value;
    std::filesystem::resize_file(filePath, end_);
}



void base_wmml::move (char* buffer, std::size_t& f_mark, std::size_t& s_mark, const int& size) {
    seek(f_mark);
    targetFile.read(buffer, size);
    f_mark+= size;
    seek(s_mark);
    targetFile.write(buffer, size);
    s_mark+= size;
}



void base_wmml::reverse_move (char* buffer, std::size_t& f_mark, std::size_t& s_mark, const int& size) {
    f_mark-= size;
    seek(f_mark);
    targetFile.read(buffer, size);
    s_mark-= size;
    seek(s_mark);
    targetFile.write(buffer, size);
}



void base_wmml::minimize_shift_data (std::size_t f_mark, std::size_t s_mark) {
    char buffer[divisor];
    auto div = std::lldiv((end_ - f_mark), divisor);

    for (auto i = div.quot; i != 0; --i)
        move(buffer, f_mark, s_mark, divisor);
    move(buffer, f_mark, s_mark, div.rem);
}



void base_wmml::maximize_shift_data (const std::size_t& start, std::size_t end) {
    char buffer[divisor];
    std::size_t s_mark = end_;
    auto div = std::lldiv((end - start), divisor);

    for (auto i = div.quot; i != 0; --i)
        reverse_move(buffer, end, s_mark, divisor);
    reverse_move(buffer, end, s_mark, div.rem);
}



void base_wmml::wmml_get () {
    int openedArchiveCount = 0;
    unsigned long long f_mark;
    skip();
    switch (error_) {
        case 1: throw std::runtime_error(filePath.string() + " WMML ERROR: file is end");
        case 2: {
            f_mark = targetFile.tellg();
            relative_move(dataSize);
            ++openedArchiveCount;
            while (openedArchiveCount != 0) {
                skip();
                switch (error_) {
                    case 0: continue;
                    case 1: throw std::runtime_error(filePath.string() + " WMML ERROR: file is end");
                    case 2: ++openedArchiveCount;
                         relative_move(dataSize); break;
                    case 3: --openedArchiveCount; break;
                }
            }
        } break;
        default: {
#ifndef NDEBUG
            std::cerr << this->filePath << std::endl;
#endif
            throw std::runtime_error(filePath.string() + " WMML ERROR: not found wmml!");
        } 
    }
    archived_files.emplace_back(new wmml_archive_struct(f_mark, targetFile.tellg(), this));
    --localArchiveCount;
    if (localArchiveCount != 0)
        wmml_get();
}



std::size_t base_wmml::size () {
    return end_;
}



size_t base_wmml::sector_size () {
    error_ = 0;
    char id = 0;
    std::size_t size = 0;
    targetFile.read(&id, sizeof(id));
    switch (id) {
    case 0:                         error_ = 1; return 0;
    case INT:                       size = sizeof(int32_t);                 break;
    case UNSIGNED_INT:              size = sizeof(uint32_t);                break;
    case LONG_INT:                  size = sizeof(int64_t);                 break;
    case UNSIGNED_LONG_INT:         size = sizeof(uint64_t);                break;
    case LONG_LONG_INT:             size = sizeof(int64_t);                 break;
    case UNSIGNED_LONG_LONG_INT:    size = sizeof(uint64_t);                break;
    case SHORT_INT:                 size = sizeof(int16_t);                 break;
    case UNSIGNED_SHORT_INT:        size = sizeof(uint16_t);                break;

    case CHAR:                      size = sizeof(char);                    break;
    case SIGNED_CHAR:               size = sizeof(signed char);             break;
    case UNSIGNED_CHAR:             size = sizeof(unsigned char);           break;
    case WCHAR_T:                   size = sizeof(wchar_t);                 break;
    case STRING:{                   unsigned char temp_size;
                                    targetFile.read(reinterpret_cast<char*>
                                    (&temp_size),sizeof(temp_size));
                                    size = temp_size + sizeof(temp_size);  }break;
    case STRING64K:{                uint16_t temp_size;
                                    targetFile.read(reinterpret_cast<char*>
                                    (&temp_size),sizeof(temp_size));
                                    size = temp_size + sizeof(temp_size);  }break;
    case STRING1KK:{                uint32_t temp_size;
                                    targetFile.read(reinterpret_cast<char*>
                                    (&temp_size),sizeof(temp_size));
                                    size = temp_size + sizeof(temp_size);  }break;

    case FLOAT:                     size = sizeof(float);                   break;
    case DOUBLE:                    size = sizeof(double);                  break;
    case LONG_DOUBLE:               size = sizeof(long double);             break;

    case BOOL:                      size = sizeof(bool);                    break;
    case S_WMML:                    error_ = 2;                             break;
    case E_WMML:                    error_ = 3;                             break;
    default: throw std::runtime_error(filePath.string() + " WMML ERROR (in reader): unknown type id");
    }
    return (size + sizeof(id));
}



template <>
void base_wmml::overwriting<std::string> (const int& object_index, const int& sector_index, const std::string& newData) {
    if (sector_index > width_)
        throw std::runtime_error("WMML ERROR: the field does not belong to the object");
    seek(start);
    std::size_t index = (object_index * width_) + sector_index;
    for (; index != 0; --index)
        if (!skip())
            throw std::runtime_error("WMML ERROR: the selected field does not belong to the file");

    const size_t f_mark = targetFile.tellg();
    const size_t freeSpaces = sector_size();

    size_t newSectorSize;
    switch (this_type(newData)) {
    case STRING:    newSectorSize = newData.size() + (sizeof(char)     + sizeof(char));  break;
    case STRING64K: newSectorSize = newData.size() + (sizeof(uint16_t) + sizeof(char));  break;
    case STRING1KK: newSectorSize = newData.size() + (sizeof(uint32_t) + sizeof(char));  break;
    }
    long long shiftSize = newSectorSize - freeSpaces;

    if (shiftSize < 0) {
        minimize_shift_data(f_mark + freeSpaces, f_mark + newSectorSize);
        resize(shiftSize);
    }
    else if (shiftSize > 0) {
        auto oldEnd = end_;
        resize(shiftSize);
        maximize_shift_data(f_mark, std::move(oldEnd));
    }

    seek(f_mark);
    write_sector(newData);
}
