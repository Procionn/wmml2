#include "wmml.h"

wmml_marker* wmml_archive_struct::open () {
    return new wmml_marker(parent, start, end);
}



wmml_archive_struct::wmml_archive_struct(unsigned long long start, unsigned long long end, wmml* parent) {
    this->start = start;
    this->end = end;
    this->parent = parent;
}
