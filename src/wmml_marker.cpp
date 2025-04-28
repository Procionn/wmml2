#include "wmml.h"

wmml_marker::wmml_marker (wmml* parent, unsigned long long& f_mark, unsigned long long& s_mark) {
   targetFile.open(parent->file_path, std::ios::binary | std::ios::in | std::ios::out | std::ios::ate);
   file_path = parent->file_path;

   this->f_mark = f_mark;
   this->s_mark = s_mark;
}

wmml_marker::wmml_marker (const std::filesystem::path& path) : wmml(path){
    f_mark = 0;
    s_mark = 0;
}


void wmml_marker::archiving (wmml* base) {

}


std::size_t wmml_marker::size () {
   return end_;
}
