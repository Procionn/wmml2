#include "wmml.h"

wmml_marker* wmml_archive_struct::open () {
    return new wmml_marker(static_cast<wmml*>(parent), start, end);
}



wmml_archive_struct::wmml_archive_struct(unsigned long long start, unsigned long long end, base_wmml* parent) {
    this->start = start;
    this->end = end;
    this->parent = parent;
}
