#include "wmml.h"

wmml::wmml (const std::filesystem::path& path) {
   targetFile.open(path, std::ios::binary | std::ios::in | std::ios::out | std::ios::ate);
   file_path = path;

   if (!targetFile.is_open())
	throw "WMML ERROR: the opened file does not exists!";
   else {
    end_ = targetFile.tellp();
    targetFile.seekp(0);
    targetFile.read(reinterpret_cast<char*>(&version), sizeof(version));
    targetFile.read(reinterpret_cast<char*>(&width_), sizeof(short unsigned int));
    targetFile.read(reinterpret_cast<char*>(&height_), sizeof(unsigned int));
    targetFile.read(reinterpret_cast<char*>(&archived_count), sizeof(archived_count));
    if (archived_count != 0)
        wmml_get();
   }
}

wmml::wmml (const std::filesystem::path& path, short unsigned width) {
   if (std::filesystem::exists(path))
	throw "WMML ERROR: you are trying to create an existing file";
   std::ofstream created_file(path, std::ios::binary);
   created_file.close();

   targetFile.open(path, std::ios::binary | std::ios::in | std::ios::out);
   file_path = path;
   if (!targetFile.is_open())
       throw " WMML ERROR: file is not open";

   height_ = 0;
   width_ = width;
   archived_count = 0;
   targetFile.write(reinterpret_cast<char*>(&version), sizeof(version));
   targetFile.write(reinterpret_cast<char*>(&width_), sizeof(unsigned short int));
   targetFile.write(reinterpret_cast<char*>(&height_), sizeof(unsigned int));
   targetFile.write(reinterpret_cast<char*>(&archived_count), sizeof(archived_count));
   end_ = targetFile.tellp();
}

wmml::~wmml () {
   targetFile.seekp(sizeof(unsigned short int));
   targetFile.write(reinterpret_cast<char*>(&height_), sizeof(unsigned int));
   targetFile.close();
}


unsigned wmml::height () {
   return height_;
}


unsigned short wmml::width () {
   return width_;
}





void wmml::write (std::vector<variant>& writeble) {
   if (writeble.size() != width_)
        throw "WMML ERROR: The size of the recorded object must be equal to the file width_";
   if (end_ != targetFile.tellp())
       targetFile.seekp(end_);
   for (auto& entry : writeble)
        std::visit([&](auto& parameter) {
            write_sector(parameter);
        }, entry);
   ++height_;
}


bool wmml::read(std::vector<variant>& output) {
   if (width_ != output.size())
        throw "WMML ERROR: The size of the container does not match the file width_";
   for (int i = 0, size = width_; i != size; ++i) {
        output[i] = read_sector();
        switch(error_) {
         case 0: continue;
         case 1: return false;
         case 2:
         case 3: throw "WMML debug error: reader came across the markup of the wmml archive sector";
        }
   }
   return true;
}




bool wmml::skip () {
   error_ = 0;
   char id = 0;
   std::size_t size = 0;
   targetFile.read(&id, sizeof(id));
   switch (id) {
#ifndef NDEBUG
        case 0:                         std::cout << false << std::endl; return false;
#elif 1
        case 0:                         return false;
#endif
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
        case S_WMML:             error_ = 2; break;
        case E_WMML:             error_ = 3; break;
        default : throw "WMML ERROR (in reader): unknown type id";
   }
   seek(static_cast<std::size_t>(targetFile.tellg()) + size);
   if (targetFile.eof())
        return false;
   else return true;
}


void wmml::seek(std::size_t t) {
#ifdef WIN32
    targetFile.seekp(t);
    targetFile.seekg(t);
#elif __linux__
    targetFile.seekp(t);
#endif
}




std::string wmml::read_sector_caseString (char type) {
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
void wmml::write_sector<std::string> (std::string& t) {
    char hash = this_type(t);
    targetFile.write((&hash), sizeof(char));
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



void wmml::shift_data (const int& size, std::size_t& f_mark) {
    std::size_t s_mark;
    char* buffer = new char[size];
    targetFile.read(buffer, size);
    s_mark = targetFile.tellg();
    seek(f_mark);
    targetFile.write(buffer, size);
    seek(s_mark);
    delete[] buffer;
}



void wmml::remove_object (int object_index) {
    std::size_t index = (object_index * width_);
    for (;index != 0; --index)
        if (!skip())
            throw "WMML ERROR: the specified object is outside the file boundary";
    std::size_t f_mark = targetFile.tellg();
    for (int i = width_; i != 0; --i)
        if (!skip())
            throw "WMML ERROR: the specified object is outside the file boundary";
    std::size_t this_pos = targetFile.tellg();
    std::size_t deleted_size = end_ - (this_pos - f_mark);

    const int divisor = 1024;
    std::size_t iterations_count = (end_ - this_pos) / divisor;

    for (; iterations_count != 0; --iterations_count)
        shift_data(divisor, f_mark);
    int surplus_size = end_ - static_cast<std::size_t>(targetFile.tellg());
    shift_data(surplus_size, f_mark);

    std::filesystem::resize_file(file_path, deleted_size);
    --height_;
}



void wmml::set_wmml (wmml_marker* target) {


    delete target;
}



wmml_marker* wmml::get_wmml () {
    auto marker = archived_files[archived_count]->open();
    --archived_count;
    return marker;
}



void wmml::wmml_get () {
    int opened_archive_count = 0;
    unsigned long long f_mark;
    bool triger = true;
    do {
        skip();
        switch (error_) {
            case 0: throw "WMML ERROR: not found wmml!";
            case 1: throw "WMML ERROR: file is end";
            case 2:
                ++opened_archive_count;
                if (triger) {
                    triger = false;
                    f_mark = targetFile.tellg();
                }
                break;
            case 3: --opened_archive_count;    break;
        }
    } while(opened_archive_count != 0);
    archived_files.emplace_back(new wmml_archive_struct(f_mark, targetFile.tellg(), this));
    wmml_get();
}




#ifndef NDEBUG
template<>
void use<wmml_marker> (wmml_marker t) {
    std::cout << "it's a wmml!" << std::endl;
}
#endif
