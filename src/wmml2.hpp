#pragma once
#include <filesystem>
#include <fstream>
#include "stream.hpp"
#include "macro.hpp"

#include <iostream>
#include "enum.hpp"
#include <cstring>
#include <cassert>


class base_wmml
{
protected:

    details::stream s;
    std::fstream descript;
    std::byte*   pack_ptr;
    std::size_t  pack_size;
    uint32_t     archivedCount;
    uint32_t     localArchiveCount = 0;
    uint32_t     height_;
    uint16_t     width_;
    uint16_t     start;
    uint8_t      version = 3;

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
    void read_archive_count();
    void read_height();




    std::string out_range_error() {
        return "Seek to out of range in file \"" + filePath.string() + "\"";
    }


    template <typename T>
    bool legacy_read_one(details::SmallStream& buff) {
        std::byte id;
        assert_r(s.rel_read(_p(id)), out_range_error());
        bool legacyFormat =
            (type_to_hash<T>() == static_cast<std::byte>(WmmlFormats::LINT) &&
             id == static_cast<std::byte>(LegacyWmmlFormats::LONG_INT) ) ||
            (type_to_hash<T>() == static_cast<std::byte>(WmmlFormats::LUINT) &&
             id == static_cast<std::byte>(LegacyWmmlFormats::UNSIGNED_LONG_INT) );
        if (type_to_hash<T>() == id || legacyFormat) {
            std::size_t varSize = sizeof(T);
            buff.write(s.get_ptr(varSize), varSize);
            s.rel_seek(varSize), out_range_error();
            return true;
        }
        return false;
    }

    template <typename T>
    bool legacy_convert_one(std::size_t& writePos) {
        std::byte id;
        assert_r(s.rel_read(_p(id)), "Error when reading legacy stream. Unknown ID");
        bool legacyFormat =
            (type_to_hash<T>() == static_cast<std::byte>(WmmlFormats::LINT) &&
             id == static_cast<std::byte>(LegacyWmmlFormats::LONG_INT) ) ||
            (type_to_hash<T>() == static_cast<std::byte>(WmmlFormats::LUINT) &&
             id == static_cast<std::byte>(LegacyWmmlFormats::UNSIGNED_LONG_INT) );
        if (type_to_hash<T>() == id || legacyFormat) {
            T var;
            assert_r(s.rel_read(_p(var)), "Error when reading legacy stream. Reading in OOM zone");
            std::size_t readPos = s.get_pos();
            s.seek(writePos);
            s.write(_p(var));
            writePos += sizeof(var);
            s.seek(readPos);
            return true;
        }
        return false;
    }


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
    void save() noexcept;
};
template <> bool base_wmml::legacy_read_one<std::string>(details::SmallStream& buff);
template <> bool base_wmml::legacy_convert_one<std::string>(std::size_t& writePos);
template <> bool base_wmml::read_one<std::string>(std::string& t);
template <> void base_wmml::write_one<std::string>(const std::string& t);





template <typename... Args>
class wmml : public base_wmml
{
    // TODO: добавить флаг isValide. В случае, если операция завершается с ошибкой - она не даёт записать в файл битые данные.
    // Либо, реализовать лок-гвард, захватывающий readOnly флаг на время проведение операций записи. В случае возникновения
    // ошибки - поток переключается в режим только для чтения, из-за чего деструктор, вызванные из летящего исключения не
    // сможет перезаписать файл

    inline static constexpr std::array<std::byte, sizeof...(Args)> pack = {
        type_to_hash<Args>()...
    };

    void init () {
        width_ = pack_size;
        if (!width_is_valide())
            throw std::runtime_error("The width of opened file \"" + filePath.string() +
                                     "\" is not comply with tool's width.");
        read_archive_count();
        read_height();
        if (!version_is_valide()) {
            //try {
                convert_stream_from_1_4_to_1_5_version();
                if (!readOnly && convertAutomatically)
                    save();
            //} catch (const std::exception& e) {
            //    throw std::runtime_error(std::string("WMML Error in converting: ") + e.what());
            //}
        }
        else if (!hash_is_valide())
            throw std::runtime_error(
                "Template instruction options do not correspond to the file \"" +
                filePath.string() + "\"  parameters");
        s.seek(start);
    }


    void convert_stream_from_1_4_to_1_5_version () {
        std::size_t readedBlocks = 0;
        details::SmallStream tempBuffer(1);
        { // read header
            tempBuffer.write(_p(version));
            tempBuffer.write(_p(width_));
            tempBuffer.write(_p(height_));
            tempBuffer.write(_p(archivedCount));
            tempBuffer.write(const_cast<std::byte*>(pack.data()), sizeof(pack));
            start = tempBuffer.get_size();
            std::size_t padding = sizeof(version) + sizeof(width_) + sizeof(height_) + sizeof(archivedCount);
            s.seek(padding);
        }
        static constexpr size_t sobr = 2; // steps_of_buffered_read
        for (int i = (height_ <= sobr ? height_ : sobr); i != 0; --i) {
            readedBlocks++;
            assert_r((legacy_read_one<Args>(tempBuffer) && ...),
                     "Error when converting from 1.4 to 1.5 format<legacy_read_one>");
        }

        std::size_t tempMarker = s.get_pos();
        tempBuffer.seek(0);
        s.seek(0);
        s.write(tempBuffer.get_ptr(tempBuffer.get_size()), tempBuffer.get_size());
        std::size_t writedPos = s.get_pos();
        assert_r(s.seek(tempMarker), out_range_error());

        if (height_ > 2) {
            for (; readedBlocks < height_; readedBlocks++) {
                assert_r((legacy_convert_one<Args>(writedPos) && ...),
                         "Error when converting from 1.4 to 1.5 format<legacy_convert_one>");
            }
        }
        s.resize(writedPos);
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
            assert_r(s.seek(start), out_range_error());
        return (read_one(args) && ...);
    }

    void write(const Args&... args) {
        assert_r(s.seek(s.get_size()), out_range_error());
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