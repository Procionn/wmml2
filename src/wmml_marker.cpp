#include "wmml.h"

wmml_marker::wmml_marker (wmml* parent, unsigned long long& f_mark, unsigned long long& s_mark) {
    targetFile.open(parent->filePath, std::ios::binary | std::ios::in | std::ios::out | std::ios::ate);
    filePath = parent->filePath;

    this->parent = parent;
    this->f_mark = f_mark;
    this->s_mark = s_mark;

    seek(f_mark);

    targetFile.read(reinterpret_cast<char*>(&version), sizeof(version));
    targetFile.read(reinterpret_cast<char*>(&width_), sizeof(width_));
    targetFile.read(reinterpret_cast<char*>(&height_), sizeof(height_));
    targetFile.read(reinterpret_cast<char*>(&archivedCount), sizeof(archivedCount));
    localArchiveCount = archivedCount;
    if (archivedCount != 0)
        wmml_get();
    start = targetFile.tellg();
}

wmml_marker::wmml_marker (const std::filesystem::path& path) {
    parent = nullptr;
    f_mark = 0;
    s_mark = 0;
    filePath = path;

    targetFile.open(path, std::ios::binary | std::ios::in | std::ios::out | std::ios::ate);
    if (!targetFile.is_open())
        throw "WMML ERROR: the opened file does not exists!";

    end_ = targetFile.tellp();
    seek(0);
    targetFile.read(reinterpret_cast<char*>(&version), sizeof(version));
    targetFile.read(reinterpret_cast<char*>(&width_), sizeof(width_));
    targetFile.read(reinterpret_cast<char*>(&height_), sizeof(height_));
    targetFile.read(reinterpret_cast<char*>(&archivedCount), sizeof(archivedCount));
#ifdef WIN32
    targetFile.seekp(targetFile.tellg());
#endif
    localArchiveCount = archivedCount;
    if (archivedCount != 0)
        wmml_get();
    start = targetFile.tellg();
    localArchiveCount = archivedCount;
}



void wmml_marker::unarchiving (const std::filesystem::path& unarchivedFilePath) {
    if (std::filesystem::exists(unarchivedFilePath))
        throw "WMML ERROR: you are trying to create an existing file";
    if (!parent)
        throw "WMML ERROR: you trying unurchiving to not archived file";
    std::ofstream new_file(unarchivedFilePath, std::ios::binary | std::ios::out);
    filePath = unarchivedFilePath;

    std::size_t fileDataSize = s_mark - f_mark - 1;
    std::size_t iterationsCount = fileDataSize / divisor;
    unsigned int surplusSize = fileDataSize % divisor;

    char buffer[divisor];
    parent->targetFile.clear();
    parent->seek(f_mark);

    for (; iterationsCount != 0; --iterationsCount) {
        parent->targetFile.read(buffer, divisor);
        new_file.write(buffer, divisor);
    }

    parent->targetFile.read(buffer, surplusSize);
    new_file.write(buffer, surplusSize);
    new_file.close();
}



bool wmml_marker::read (std::vector<variant>& output) {
    if (width_ != output.size())
        throw "WMML ERROR: The size of the container does not match the file width_";
    for (int i = 0; i != width_; ++i) {
        if (targetFile.tellg() >= s_mark)
            return false;
        output[i] = read_sector();
        switch(error_) {
            case 0: continue;
            case 1: return false;
            case 2: throw "WMML debug error: reader came across the markup of the wmml archive sector";
            case 3: return false;
        }
    }
   return true;
}
