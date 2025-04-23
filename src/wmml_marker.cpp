#include "wmml.h"

wmml_marker::wmml_marker (wmml* parent, unsigned long long& f_mark, unsigned long long& s_mark) {
   targetFile.open(parent->file_path, std::ios::binary | std::ios::in | std::ios::out | std::ios::ate);
   file_path = parent->file_path;

   this->f_mark = f_mark;
}

wmml_marker::wmml_marker (std::filesystem::path& path) : wmml(path){

}
