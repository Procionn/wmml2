#include "wmml2.hpp"


void base_wmml::write_header () {
    s.seek(0);
    s.write(_p(version));
    s.write(_p(width_));
    s.write(_p(height_));
    s.write(_p(archivedCount));
    s.write(const_cast<std::byte*>(pack_ptr), pack_size);
}


bool base_wmml::width_is_valide () {
    std::size_t padding = (sizeof(version));
    s.seek(padding);
    return (*reinterpret_cast<uint16_t*>(s.get_ptr(sizeof(width_))) == width_);
}


bool base_wmml::version_is_valide () {
    s.seek(0);
    return (*reinterpret_cast<uint8_t*>(s.get_ptr(sizeof(version))) == version);
}


bool base_wmml::hash_is_valide () {
    std::size_t padding = (sizeof(version));
    s.seek(padding);
    decltype(width_) old_width;
    s.read(_p(old_width));
    s.rel_seek(sizeof(old_width) + sizeof(height_) + sizeof(archivedCount));
    if (old_width != width_)
        return false;
    std::byte old_pack[old_width];
    s.read(_p(old_pack));
    start = s.get_pos() + old_width;
    auto temp = memcmp(old_pack, pack_ptr, width_) == 0;
    return temp;
}


void base_wmml::open_file (const bool createFile) {
    descript.rdbuf()->pubsetbuf(nullptr, 0); // buffer in library has been turned off
    std::ios_base::openmode flags = createFile ? std::ios::in | std::ios::out | std::ios::binary |
                                                     std::ios::noreplace | std::ios::trunc
                                               : std::ios::in | std::ios::out | std::ios::binary;
    descript.open(filePath, flags);
    if (!descript.is_open()) {
        if (descript.bad())
            throw std::runtime_error("Fatal error when opening file \"" + filePath.string() +
                                     "\".");
        throw std::runtime_error("Unknown error in file \"" + filePath.string() + "\".");
    }
    isOpen = true;
}


void base_wmml::open (
    const std::filesystem::path& pathToOpenedFile) { // TODO // it should set readOnly in false
    create_file();
    write_stream_to_file();
}


void base_wmml::reset () { s.seek(start); }


void base_wmml::save () {
    if (isOpen)
        if (!readOnly)
            write_stream_to_file();
}


bool base_wmml::open_existed_file () {
    if (std::filesystem::exists(filePath)) {
        if (std::filesystem::file_size(filePath) == 0) { // File doesn't has a metadata
            if (autoDeletingFilesWithoutMetadata) {
                std::filesystem::remove(filePath);
                return false;
            }
            else
                throw std::runtime_error("The opened file \"" + filePath.string() +
                                         "\" is not has metadata.");
        }
        else {
            open_file();
            return true;
        }
    }
    else
        return false;
}


void base_wmml::create_file () {
    if (std::filesystem::exists(filePath))
        throw std::invalid_argument("File \"" + filePath.string() + "\" is already exist.");
    open_file(true);
    isOpen = true;
    archivedCount = height_ = 0;
    width_ = pack_size;
    write_header();
    start = s.get_size();
}






template <> bool base_wmml::read_one<std::string>(std::string& t) {
    int32_t size;
    auto result = false;
    if (s.read(_p(size))) {
        s.rel_seek(sizeof(size));
        result = s.read(reinterpret_cast<std::byte*>(t.data()), size);
        s.rel_seek(size);
    }
    return result;
}


template <> void base_wmml::write_one<std::string>(const std::string& t) {
    int32_t size = t.size();
    s.write(_p(size));
    s.write(reinterpret_cast<std::byte*>(const_cast<char*>(t.data())), size);
}


base_wmml::base_wmml (const std::byte* pack, std::size_t size, bool readonly, const std::filesystem::path& path)
    : pack_ptr(const_cast<std::byte*>(pack)), pack_size(std::move(size)), readOnly(readonly), filePath(path) {}


void base_wmml::read_file_in_stream () {
    assert(isOpen);
    // reading file into stream without unnecessary copying
    std::size_t fileSize = std::filesystem::file_size(filePath);
    s.open(fileSize);
    descript.read(reinterpret_cast<char*>(s.get_ptr(fileSize)), fileSize);
}


void base_wmml::write_stream_to_file () {
    assert(!readOnly);
    descript.clear();
    if (std::filesystem::file_size(filePath) > s.get_size())
        resize(s.get_size());
    descript.seekg(0);
    descript.seekp(0);
    s.seek(0);
    descript.write(reinterpret_cast<char*>(s.get_ptr(s.get_size())), s.get_size());
    descript.flush();
}


void base_wmml::resize (const std::size_t& size) {
    assert(!readOnly);
    descript.close();
    descript.clear();
    std::filesystem::resize_file(filePath, size);
    open_file();
}




