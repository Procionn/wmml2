#include "base_wmml.h"

unsigned base_wmml::height () {
    return height_;
}


unsigned short base_wmml::width () {
    return width_;
}



bool base_wmml::skip () {
    error_ = 0;
    char id = 0;
    std::size_t size = 0;
    targetFile.read(&id, sizeof(id));
    switch (id) {
        case 0:                         error_ = 1; return false;
        case INT:                       size = sizeof(int);                     break;
        case UNSIGNED_INT:              size = sizeof(unsigned int);            break;
        case LONG_INT:                  size = sizeof(long int);                break;
        case UNSIGNED_LONG_INT:         size = sizeof(unsigned long int);       break;
        case LONG_LONG_INT:             size = sizeof(long long int);           break;
        case UNSIGNED_LONG_LONG_INT:    size = sizeof(unsigned long long int);  break;
        case SHORT_INT:                 size = sizeof(short int);               break;
        case UNSIGNED_SHORT_INT:        size = sizeof(unsigned short int);      break;

        case CHAR:                      size = sizeof(char);                    break;
        case SIGNED_CHAR:               size = sizeof(signed char);             break;
        case UNSIGNED_CHAR:             size = sizeof(unsigned char);           break;
        case WCHAR_T:                   size = sizeof(wchar_t);                 break;
        case STRING:{                   unsigned char temp_size;
                                        targetFile.read(reinterpret_cast<char*>
                                        (&temp_size),sizeof(temp_size));
                                        size = temp_size;                      }break;
        case STRING64K:{                unsigned short int temp_size;
                                        targetFile.read(reinterpret_cast<char*>
                                        (&temp_size),sizeof(temp_size));
                                        size = temp_size;                      }break;
        case STRING1KK:{                unsigned int temp_size;
                                        targetFile.read(reinterpret_cast<char*>
                                        (&temp_size),sizeof(temp_size));
                                        size = temp_size;                      }break;

        case FLOAT:                     size = sizeof(float);                   break;
        case DOUBLE:                    size = sizeof(double);                  break;
        case LONG_DOUBLE:               size = sizeof(long double);             break;

        case BOOL:                      size = sizeof(bool);                    break;
        case S_WMML:                    error_ = 2;                             break;
        case E_WMML:                    error_ = 3;                             break;
        default : throw "WMML ERROR (in reader): unknown type id";
    }
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


std::string base_wmml::read_sector_caseString (char type) {
    switch (type) {
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




template <>
void base_wmml::write_sector<std::string> (std::string& t) {
    char hash = this_type(t);
    targetFile.write((&hash), sizeof(char));
    switch (hash) {
        case STRING: {
            unsigned char size = t.size();
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



void base_wmml::shift_data (const int& size, const std::size_t& f_mark) {
    if (size > divisor)
        throw "WMML debug ERROR: the buffer does not fit on the stack";
    char buffer[divisor];
    targetFile.read(buffer, size);
    std::size_t s_mark = targetFile.tellg();
    seek(f_mark);
    targetFile.write(buffer, size);
    seek(s_mark);
}



void base_wmml::wmml_get () {
    int openedArchiveCount = 0;
    unsigned long long f_mark;
    skip();
    switch (error_) {
        case 1: throw "WMML ERROR: file is end";
        case 2: {
            f_mark = targetFile.tellg();
            relative_move(dataSize);
            ++openedArchiveCount;
            while (openedArchiveCount != 0) {
                skip();
                switch (error_) {
                    case 0: continue;
                    case 1: throw "WMML ERROR: file is end";
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
            throw ("WMML ERROR: not found wmml!");
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
