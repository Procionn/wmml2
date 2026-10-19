#pragma once
#include <filesystem>
#include <fstream>
#include "stream.hpp"

#include "enum.hpp"
#include <cstring>
#include <cassert>

#ifndef NDEBUG
#include <iostream>
#endif


class base_wmml
{
protected:

    stream       s;
    std::fstream descript;
    uint32_t     archivedCount;
    uint32_t     localArchiveCount = 0;
    uint32_t     height_;
    uint16_t     width_;
    uint16_t     start;
    uint8_t      version = 3;
    int8_t       error_ = 0;
    /*
    ...
    */

    template <typename T>
    bool read_one(T& t) {
        auto result = s.read(_p(t));
        s.rel_seek(sizeof(t));
        return result;
    }

    template <typename T>
    void write_one(const T& t) {
        s.write(reinterpret_cast<std::byte*>(const_cast<T*>(&t)), sizeof(t));
    }

    base_wmml() {}
    ~base_wmml() {}
};


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



template <typename... Args>
class wmml : public base_wmml
{
    const std::filesystem::path filePath;
    inline static constexpr std::array<std::byte, sizeof...(Args)> pack = {
        type_to_hash<Args>()...
    };
    bool readOnly = false;
    bool isOpen = false;
    bool convertAutomatically = true; // TODO: add changing it flag
    bool autoDeletingFilesWithoutMetadata = true; // TODO: add changing it flag

protected:
    void write_header () {
        s.seek(0);
        s.write(_p(version));
        s.write(_p(width_));
        s.write(_p(height_));
        s.write(_p(archivedCount));
        s.write(const_cast<std::byte*>(pack.data()), sizeof(pack));
    }


    bool width_is_valide () {
        std::size_t padding = (sizeof(version));
        s.seek(padding);
        return (*reinterpret_cast<uint16_t*>(s.get_ptr(sizeof(width_))) == width_);
    }

    bool version_is_valide () {
        s.seek(0);
        return (*reinterpret_cast<uint8_t*>(s.get_ptr(sizeof(version))) == version);
    }

    bool hash_is_valide () {
        std::size_t padding = (sizeof(version));
        s.seek(padding);
        decltype(width_) old_width;
        s.read(_p(old_width));
        s.rel_seek(sizeof(old_width) + sizeof(height_) + sizeof(archivedCount));
        if (old_width != width_) return false;
        std::byte old_pack[old_width];
        s.read(_p(old_pack));
        start = s.get_pos() + old_width;
        auto temp = memcmp(old_pack, pack.data(), width_) == 0;
        return temp;
    }


    std::array<std::byte, sizeof...(Args)> get_hash_pack_from_legacy_file() {
        /*std::array<std::byte, sizeof...(Args)> pack;
        // need be realizate it */
    }


    bool legacy_hash_is_valide () {
        return pack == get_hash_pack_from_legacy_file();
    }



    void convert_stream_to_latest_version() {
        // TODO
    }


    void open_file(const bool createFile = false) {
        descript.rdbuf()->pubsetbuf(nullptr, 0); // buffer in library has been turned off
        std::ios_base::openmode flags = createFile
                                            ? std::ios::in | std::ios::out | std::ios::binary |
                                                  std::ios::noreplace | std::ios::trunc
                                            : std::ios::in | std::ios::out | std::ios::binary;
        descript.open(filePath, flags);
        if (!descript.is_open()) {
            if (descript.bad())
                throw std::runtime_error("Fatal error when opening file \"" + filePath.string() + "\".");
            throw std::runtime_error("Unknown error in file \"" + filePath.string() + "\".");
        }
        isOpen = true;
    }





    void init() {
        width_ = sizeof(pack);
        if (!width_is_valide())
            throw std::runtime_error("The width of opened file \"" + filePath.string() +
                                     "\" is not comply with tool's width.");
        if (!version_is_valide()) {
            if (legacy_hash_is_valide()) {
                convert_stream_to_latest_version();
                if (!readOnly && convertAutomatically)
                    save();
            }
            else
                throw std::runtime_error(
                    "Template instruction options do not correspond to the legacy-version file \"" +
                    filePath.string() + "\"  parameters");
        }
        else if (!hash_is_valide())
            throw std::runtime_error(
                "Template instruction options do not correspond to the file \"" +
                filePath.string() + "\"  parameters");
        s.seek(start);
    }


    bool open_existed_file () {
        if (std::filesystem::exists(filePath)) {
            if (std::filesystem::file_size(filePath) == 0) { // File doesn't has a metadata
                if (autoDeletingFilesWithoutMetadata) {
                    std::filesystem::remove(filePath);
                    return false;
                }
                else throw std::runtime_error("The opened file \"" + filePath.string() +
                                             "\" is not has metadata.");
            }
            else {
                open_file();
                return true;
            }
        }
        else return false;
    }


    void create_file() {
        if (std::filesystem::exists(filePath))
            throw std::invalid_argument("File \"" + filePath.string() + "\" is already exist.");
        open_file(true);
        isOpen = true;
        archivedCount = height_ = 0;
        width_ = sizeof(pack);
        write_header();
        start = s.get_size();
    }

    void read_file_in_stream() {
        assert(isOpen);
        // reading file into stream without unnecessary copying
        std::size_t fileSize = std::filesystem::file_size(filePath);
        s.open(fileSize);
        descript.read(reinterpret_cast<char*>(s.get_ptr(fileSize)), fileSize);
    }

    void write_stream_to_file() {
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

    void resize (const std::size_t& size) {
        assert(!readOnly);
        descript.close();
        descript.clear();
        std::filesystem::resize_file(filePath, size);
        open_file();
    }


public:
    wmml() = default;

    wmml(const char* file_stream) : readOnly(true) {
        std::size_t streamSize = strlen(file_stream);
        s.open(streamSize);
        s.write(reinterpret_cast<std::byte*>(const_cast<char*>(file_stream)), streamSize);
        init();
    }

    wmml (const std::filesystem::path& pathToOpenedFile) : filePath(pathToOpenedFile) {
        if (open_existed_file()) {
            read_file_in_stream();
            init();
        }
        else { // if file not exist
            create_file();
            write_stream_to_file();
        }
    }

    ~wmml () {
        save();
    }

    bool read(Args&... args) {
        if (s.get_pos() < start)
            s.seek(start);
        return (read_one(args) && ...);
    }

    void write(const Args&... args) {
        s.seek(s.get_size());
        (write_one(args), ...);
        height_++;
    }

    void open(const std::filesystem::path& pathToOpenedFile) { // TODO // it should set readOnly in false
        create_file();
        write_stream_to_file();
    }
    // const char* get_stream_ptr(const std::size_t& pos, const std::size_t& size);
    // а нахуя это здесь?
    // Это требуется для низкоуровневого чтения данных напрямую из стрима, без копирований
    // В целом, это на вряд ли даст весомый выигрыш. А вот потенциальных ошибок в пользовательском коде может наплодить уйму

    // remove object // TODO
    // overwrite sector // TODO


    void reset () {
        s.seek(start);
    }

    void save () {
        if (isOpen)
            if (!readOnly)
                write_stream_to_file();
    }
};
