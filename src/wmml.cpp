#include "wmml.h"

wmml::wmml (const std::filesystem::path& path) {
    targetFile.open(path, std::ios::binary | std::ios::in | std::ios::out | std::ios::ate);
    filePath = path;

    if (!targetFile.is_open())
        throw std::runtime_error(filePath.string() + " WMML ERROR: the opened file does not exists!");
    else {
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
}

wmml::wmml (const std::filesystem::path& path, short unsigned width) {
    if (std::filesystem::exists(path))
        throw std::runtime_error(filePath.string() + " WMML ERROR: you are trying to create an existing file");
    std::ofstream created_file(path, std::ios::binary);
    created_file.close();

    targetFile.open(path, std::ios::binary | std::ios::in | std::ios::out);
    filePath = path;
    if (!targetFile.is_open())
       throw std::runtime_error(filePath.string() + "  WMML ERROR: file is not open");

    height_ = 0;
    width_ = width;
    archivedCount = 0;
    targetFile.write(reinterpret_cast<char*>(&version), sizeof(version));
    targetFile.write(reinterpret_cast<char*>(&width_), sizeof(width_));
    targetFile.write(reinterpret_cast<char*>(&height_), sizeof(height_));
    targetFile.write(reinterpret_cast<char*>(&archivedCount), sizeof(archivedCount));
    end_ = targetFile.tellp();
    start = targetFile.tellg();
}

wmml::~wmml () {
    targetFile.seekp(sizeof(version) + sizeof(width_));
    targetFile.write(reinterpret_cast<char*>(&height_), sizeof(height_));
    targetFile.write(reinterpret_cast<char*>(&archivedCount), sizeof(archivedCount));
    targetFile.close();
}





void wmml::write (std::vector<variant>& writeble) {
    if (writeble.size() != width_)
        throw std::runtime_error(filePath.string() + " WMML ERROR: The size of the recorded object must be equal to the file width_");
    if (end_ != targetFile.tellp())
        targetFile.seekp(end_);
    for (auto& entry : writeble)
        std::visit([&](auto& parameter) {
            write_sector(parameter);
        }, entry);
    ++height_;
    end_ = targetFile.tellp();
}


bool wmml::read(std::vector<variant>& output) {
    if (width_ != output.size())
        throw std::runtime_error(filePath.string() + " WMML ERROR: The size of the container does not match the file width_");
    for (int i = 0; i != width_; ++i) {
        output[i] = read_sector();
        switch(error_) {
            case 0: continue;
            case 1: return false;
            case 2:
            case 3: throw std::runtime_error(filePath.string() + " WMML debug error: reader came across the markup of the wmml archive sector");
        }
    }
    return true;
}





void wmml::remove_object (const size_t& object_index) {
    seek(start);
    std::size_t index = (object_index * width_);
    for (;index != 0; --index)
        if (!skip())
            throw std::runtime_error(filePath.string() + " WMML ERROR: the specified object is outside the file boundary");
    std::size_t f_mark = targetFile.tellg();
    for (int i = width_; i != 0; --i)
        if (!skip())
            throw std::runtime_error(filePath.string() + " WMML ERROR: the specified object is outside the file boundary");
    std::size_t this_pos = targetFile.tellg();
    std::size_t deleted_size = end_ - (this_pos - f_mark);

    std::size_t iterationsCount = (end_ - this_pos) / divisor;

    for (; iterationsCount != 0; --iterationsCount)
        shift_data(divisor, f_mark);
    int surplus_size = end_ - static_cast<std::size_t>(targetFile.tellg());
    shift_data(surplus_size, f_mark);

    std::filesystem::resize_file(filePath, deleted_size);
    end_ = deleted_size;
    --height_;
}


void wmml::reset () {
    seek(start);
    localArchiveCount = archivedCount;
}


void wmml::flush () {
    std::size_t mark = targetFile.tellg();
#if WIN32
    std::size_t mark_p = targetFile.tellp();
#endif
    targetFile.seekp(sizeof(version) + sizeof(width_));
    targetFile.write(reinterpret_cast<char*>(&height_), sizeof(height_));
    targetFile.write(reinterpret_cast<char*>(&archivedCount), sizeof(archivedCount));

    targetFile.seekg(mark);
#if WIN32
    targetFile.seekp(mark_p);
#endif
    targetFile.flush();
}


void wmml::set_wmml (wmml_marker* target) {
    if (!target)
        throw std::runtime_error(filePath.string() + " WMML ERROR: the target wmml_marker is equal to nullptr.");
    if (target->filePath == this->filePath)
        throw std::runtime_error(filePath.string() + " WMML ERROR: it is impossible to archive the file to itself (the path of the main and archived file are the same)");
    std::size_t newFileSize = target->size();
    std::size_t newEnd = end_ + newFileSize + 2; // 2 - the size of the markers located on the borders of the archived file
    std::filesystem::resize_file(filePath, newEnd);

    std::size_t fileDataSize = end_ - start;
    std::size_t iterationsCount = fileDataSize / divisor;
    unsigned int surplusSize = fileDataSize % divisor;

    std::size_t f_mark = newEnd;
    std::size_t s_mark = end_;
    if (iterationsCount > 0 ) {
        for (; iterationsCount != 0; --iterationsCount) {
            f_mark -= divisor;
            s_mark -= divisor;
            seek(s_mark);
            shift_data(divisor, f_mark);
        }
    }
    f_mark -=surplusSize;
    s_mark -=surplusSize;
    seek(s_mark);
    shift_data(surplusSize, f_mark);


    unsigned char id = S_WMML;
    seek(start);
    targetFile.write(reinterpret_cast<char*>(&id), sizeof(id));

    iterationsCount = newFileSize / divisor;
    surplusSize = newFileSize % divisor;

    char buffer[divisor];
    target->seek(0);
    for  (;iterationsCount != 0; --iterationsCount) {
        target->targetFile.read(buffer, divisor);
        targetFile.write(buffer, divisor);
    }
    target->targetFile.read(buffer, surplusSize);
    targetFile.write(buffer, surplusSize);

    id = E_WMML;
    targetFile.write(reinterpret_cast<char*>(&id), sizeof(id));

    ++archivedCount;
    delete target;
}



wmml_marker* wmml::get_wmml () {
    if (localArchiveCount == 0) 
        return nullptr; 
    --localArchiveCount;
    auto marker = archived_files[localArchiveCount]->open();
    return marker;
}
