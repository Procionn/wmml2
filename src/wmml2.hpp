#pragma once
#include <filesystem>
#include <fstream>
#include "stream.hpp"

#include "enum.hpp"
#include <cstring>
#include <cassert>


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
    std::byte*   pack_ptr;
    std::size_t  pack_size;

    const std::filesystem::path filePath;
    bool readOnly = false;
    bool isOpen = false;
    bool convertAutomatically = true; // TODO: add changing it flag
    bool autoDeletingFilesWithoutMetadata = true; // TODO: add changing it flag
    /*
    ...
    */
    base_wmml(const std::byte* pack_ptr, std::size_t pack_size, bool readOnly, const std::filesystem::path& path);
    ~base_wmml() = default;
    void write_header();
    bool width_is_valide();
    bool version_is_valide();
    bool hash_is_valide();
    void open_file(const bool createFile = false);
    void read_file_in_stream();
    void write_stream_to_file();
    void resize(const std::size_t& size);
    bool open_existed_file();
    void create_file();


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

public:
    void open(const std::filesystem::path& pathToOpenedFile);
    void reset();
    void save();
};
template <> bool base_wmml::read_one<std::string>(std::string& t);
template <> void base_wmml::write_one<std::string>(const std::string& t);





template <typename... Args>
class wmml : public base_wmml
{

    inline static constexpr std::array<std::byte, sizeof...(Args)> pack = {
        type_to_hash<Args>()...
    };

    void init () {
        width_ = pack_size;
        if (!width_is_valide())
            throw std::runtime_error("The width of opened file \"" + filePath.string() +
                                     "\" is not comply with tool's width.");
        if (!version_is_valide()) {
            if (convert_stream_to_latest_version()) {
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


    bool convert_stream_to_latest_version () {
        // TODO
        s.seek(start);


        return true;
    }

public:
    wmml() = default;

    wmml (const char* file_stream) : base_wmml(pack.data(), sizeof(pack), true, "") {
        std::size_t streamSize = strlen(file_stream);
        s.open(streamSize);
        s.write(reinterpret_cast<std::byte*>(const_cast<char*>(file_stream)), streamSize);
        init();
    }

    wmml (const std::filesystem::path& pathToOpenedFile)
        : base_wmml(pack.data(), sizeof(pack), false, pathToOpenedFile){
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


    // const char* get_stream_ptr(const std::size_t& pos, const std::size_t& size);
    // а нахуя это здесь?
    // Это требуется для низкоуровневого чтения данных напрямую из стрима, без копирований
    // В целом, это на вряд ли даст весомый выигрыш. А вот потенциальных ошибок в пользовательском коде может наплодить уйму
    // skip // TODO

    // remove object // TODO
    // overwrite sector // TODO



};
