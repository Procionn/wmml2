#include "wmml.h"

wmml::wmml (const std::filesystem::path& path) {
   targetFile.open(path, std::ios::binary | std::ios::in | std::ios::out | std::ios::ate);

   if (!targetFile.is_open())
	throw "WMML ERROR: the opened file does not exists!";
   else {
    end_ = targetFile.tellp();
    targetFile.seekp(0);
	targetFile.read(reinterpret_cast<char*>(&width_), sizeof(short unsigned int));
	targetFile.read(reinterpret_cast<char*>(&height_), sizeof(unsigned int));
   }
}

wmml::wmml (const std::filesystem::path& path, short unsigned width) {
   if (std::filesystem::exists(path))
	throw "WMML ERROR: you are trying to create an existing file";
   std::ofstream created_file(path, std::ios::binary);
   created_file.close();

   targetFile.open(path, std::ios::binary | std::ios::in | std::ios::out | std::ios::ate);
   if (!targetFile.is_open())
       throw " WMML ERROR: file is not open";

   height_ = 0;
   width_ = width;
   end_ = targetFile.tellp();
   targetFile.seekp(0);
   targetFile.write(reinterpret_cast<char*>(&width_), sizeof(short unsigned int));
   targetFile.write(reinterpret_cast<char*>(&height_), sizeof(unsigned int));
}

wmml::~wmml () {
   targetFile.seekp(sizeof(short unsigned int));
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
        }
   }
   return true;
}




bool wmml::skip () {
   char id = 0;
   std::size_t size;
   targetFile.read(&id, sizeof(id));
   switch (id) {
        case 0:                         return false;
        case INT:                       size = sizeof(int);
        case UNSIGNED_INT:              size = sizeof(unsigned int);
        case LONG_INT:                  size = sizeof(long int);
        case UNSIGNED_LONG_INT:         size = sizeof(unsigned long int);
        case LONG_LONG_INT:             size = sizeof(long long int);
        case UNSIGNED_LONG_LONG_INT:    size = sizeof(unsigned long long int);
        case SHORT_INT:                 size = sizeof(short int);
        case UNSIGNED_SHORT_INT:        size = sizeof(unsigned short int);

        case CHAR:                      size = sizeof(char);
        case SIGNED_CHAR:               size = sizeof(signed char);
        case UNSIGNED_CHAR:             size = sizeof(unsigned char);
        case WCHAR_T:                   size = sizeof(wchar_t);

        case FLOAT:                     size = sizeof(float);
        case DOUBLE:                    size = sizeof(double);
        case LONG_DOUBLE:               size = sizeof(long double);

        case BOOL:                      size = sizeof(bool);
        default : throw "WMML ERROR (in reader): unknown type id";
   }
   targetFile.seekp(static_cast<std::size_t>(targetFile.tellg()) + size);
   if (targetFile.eof()) return false;
   else return true;
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




#ifndef NDEBUG
template<>
void use<wmml_marker> (wmml_marker t) {
    std::cout << "it's a wmml!" << std::endl;
}
#endif
