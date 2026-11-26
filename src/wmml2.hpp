#pragma once
/**
 * @file The main library header
 */
#include <filesystem>
#include <fstream>
#include "stream.hpp"
#include "macro.hpp"
#include "enum.hpp"

#include <cstring>
#include <cassert>
#include <tuple>
#include <type_traits>
#include <utility>


/**
 * @brief \~russian Базовый класс библиотеки WMML: бинарный поток-зеркало файла,
 *             файловый дескриптор и работа с заголовком.
 * @brief \~english Base class of the WMML library: a binary stream mirroring the
 *             file, the file descriptor and header handling.
 *
 * \~russian Класс хранит внутренний бинарный поток `s` (полная копия содержимого
 *      файла), файловый дескриптор `descript` и метаданные заголовка: версию,
 *      ширину, высоту, количество архивов и пакет хэшей типов. При ошибках
 *      открытия или чтения файла бросает `std::runtime_error` или
 *      `std::invalid_argument`.
 *
 * \~english The class keeps an in-memory binary stream `s` (a full copy of the
 *      file contents), the file descriptor `descript`, and the header
 *      metadata: version, width, height, archive count and the type-hash
 *      pack. On file open or read failure it throws `std::runtime_error` or
 *      `std::invalid_argument`.
 *
 * @note \~russian Методы класса предназначены исключительно для производных
 *            (шаблонный класс `wmml`); публичный API ограничен методами
 *            `open()`, `reset()` и `save()`.
 * @note \~english Class methods are intended for derived classes only (the `wmml`
 *            template); the public API is limited to `open()`, `reset()`
 *            and `save()`.
 */
class base_wmml
{
protected:
    /** \~russian Внутренний бинарный поток — отражение содержимого файла.
     *  \~english Internal binary stream — a mirror of the file contents. */
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

    /**
     * @brief \~russian Создаёт объект WMML, привязанный к пакету типов, режиму
     *            доступа и пути к файлу.
     * @brief \~english Creates a WMML object bound to a type pack, an access mode
     *             and a file path.
     *
     * \~russian Файл в момент конструирования НЕ открывается — сохраняются только
     *      параметры, необходимые для последующей инициализации
     *      (см. производный класс `wmml`).
     *
     * \~english The file is NOT opened at construction time — only the parameters
     *      required for later initialization are stored (see the `wmml`
     *      derived class).
     *
     * @param pack_ptr \~russian указатель на пакет хэшей типов объекта;
     *                 \~english pointer to the object's type-hash pack;
     * @param size     \~russian размер пакета в байтах;
     *                 \~english pack size in bytes;
     * @param readonly \~russian `true` — файл доступен только для чтения;
     *                 \~english `true` — the file is read-only;
     * @param path     \~russian путь к WMML-файлу (пустая строка при создании из
     *                 текстового потока);
     *                 \~english path to the WMML file (empty string when
     *                 constructing from a text stream).
     */
    base_wmml(const std::byte* pack_ptr, std::size_t pack_size, bool readOnly, const std::filesystem::path& path);

    /**
     * @brief \~russian Деструктор. Закрывает внутренний поток и файловый дескриптор.
     * @brief \~english Destructor. Closes the internal stream and the file
     *             descriptor.
     */
    ~base_wmml() = default;

    /**
     * @brief \~russian Перезаписывает заголовок файла (первую секцию потока).
     * @brief \~english Rewrites the file header (the first section of the stream).
     *
     * \~russian Устанавливает позицию потока в начало и последовательно записывает:
     *      версию, ширину, высоту, количество архивов и пакет хэшей типов.
     *
     * \~english Seeks the stream to the beginning and writes in order: version,
     *      width, height, archive count and the type-hash pack.
     */
    void write_header();

    /**
     * @brief \~russian Проверяет, совпадает ли ширина, сохранённая в заголовке,
     *            с ожидаемой для данного набора шаблонных типов.
     * @brief \~english Checks that the width stored in the header matches the
     *             expected value for the current template type set.
     *
     * \~russian Читает поле ширины из заголовка и сравнивает его с `width_`.
     *
     * \~english Reads the width field from the header and compares it with
     *      `width_`.
     *
     * @return \~russian `true`, если ширина совпадает, иначе `false`.
     *         \~english `true` if the widths match, `false` otherwise.
     */
    bool width_is_valide();

    /**
     * @brief \~russian Проверяет, что версия файла совпадает с поддерживаемой версией формата.
     * @brief \~english Checks that the file version matches the supported format
     *             version.
     *
     * \~russian Сравнивает байт версии в начале файла с текущим значением
     *      `version`.
     *
     * \~english Compares the version byte at the beginning of the file with the
     *      current value of `version`.
     *
     * @return \~russian `true`, если версии совпадают, иначе `false`.
     *         \~english `true` if the versions match, `false` otherwise.
     */
    bool version_is_valide();

    /**
     * @brief \~russian Проверяет целостность пакета хэшей типов в заголовке.
     * @brief \~english Validates the type-hash pack stored in the header.
     *
     * \~russian Убеждается, что ширина в заголовке соответствует ожидаемой, и что
     *      байты пакета типов в заголовке идентичны пакету текущих шаблонных
     *      типов. При совпадении устанавливает позицию потока `start` сразу
     *      после пакета типов.
     *
     * \~english Ensures that the header width matches the expected value and that
     *      the type-pack bytes in the header are identical to the current
     *      template type pack. On success the stream position `start` is set
     *      right after the type pack.
     *
     * @return \~russian `true`, если пакет типов идентичен, иначе `false`.
     *         \~english `true` if the type packs are identical, `false` otherwise.
     */
    bool hash_is_valide();

    /**
     * @brief \~russian Открывает файл, описываемый `filePath`, в бинарном режиме
     *            без буферизации.
     * @brief \~english Opens the file described by `filePath` in unbuffered binary
     *             mode.
     *
     * \~russian При `createFile == true` файл создаётся (с усечением),
     *      при `false` — открывается существующий. Если открыть файл
     *      не удалось, бросается `std::runtime_error`.
     *
     * \~english When `createFile == true` the file is created (truncated),
     *      otherwise an existing file is opened. If the file cannot be
     *      opened, a `std::runtime_error` is thrown.
     *
     * @param createFile \~russian флаг создания нового файла;
     *                   \~english flag creating a new file.
     */
    void open_file(const bool createFile = false);

    /**
     * @brief \~russian Загружает всё содержимое файла во внутренний поток без
     *            промежуточных копирований.
     * @brief \~english Loads the entire file contents into the internal stream
     *             without intermediate copies.
     *
     * \~russian Требует, чтобы файл уже был открыт (`isOpen == true`,
     *      контролируется `assert`). Размер потока выделяется под текущий
     *      размер файла на диске, затем данные читаются напрямую в буфер
     *      потока.
     *
     * \~english Requires the file to be already open (`isOpen == true`, enforced
     *      by `assert`). The stream is sized to the on-disk file size, then
     *      the data are read directly into the stream buffer.
     */
    void read_file_in_stream();

    /**
     * @brief \~russian Сохраняет содержимое внутреннего потока в файл.
     * @brief \~english Persists the internal stream contents to the file.
     *
     * \~russian Если файл на диске крупнее потока, сначала выполняется
     *      `resize()`. Далее весь поток записывается в файл с начала и
     *      буфер принудительно сбрасывается на диск. Требует режима записи
     *      (`readOnly == false`, контролируется `assert`).
     *
     * \~english If the on-disk file is larger than the stream, `resize()` is
     *      performed first. Then the whole stream is written to the file from
     *      the beginning and the buffer is flushed. Requires write access
     *      (`readOnly == false`, enforced by `assert`).
     */
    void write_stream_to_file();

    /**
     * @brief \~russian Изменяет размер файла на диске.
     * @brief \~english Changes the on-disk file size.
     *
     * \~russian Закрывает файловый дескриптор, изменяет размер файла через
     *      `std::filesystem::resize_file` и заново открывает его. Требует
     *      режима записи (`readOnly == false`).
     *
     * \~english Closes the file descriptor, resizes the file via
     *      `std::filesystem::resize_file` and reopens it. Requires write
     *      access (`readOnly == false`).
     *
     * @param size \~russian новый размер файла в байтах;
     *             \~english new file size in bytes.
     */
    void resize(const std::size_t& size);

    /**
     * @brief \~russian Открывает существующий файл, если он есть.
     * @brief \~english Opens the file if it exists.
     *
     * \~russian Если файл существует, но не имеет метаданных (размер 0 байт):
     *      - при `autoDeletingFilesWithoutMetadata == true` файл удаляется и
     *        возвращается `false`;
     *      - иначе бросается `std::runtime_error`.
     *      Если файл содержит данные — он открывается и возвращается `true`.
     *
     * \~english If the file exists but has no metadata (0 bytes in size):
     *      - when `autoDeletingFilesWithoutMetadata == true` the file is
     *        removed and `false` is returned;
     *      - otherwise a `std::runtime_error` is thrown.
     *      If the file contains data — it is opened and `true` is returned.
     *
     * @return \~russian `true`, если файл открыт, `false`, если его не существует
     *         или он был удалён;
     *         \~english `true` if the file was opened, `false` if it does not
     *         exist or was removed.
     */
    bool open_existed_file();

    /**
     * @brief \~russian Создаёт новый пустой WMML-файл с заполненным заголовком.
     * @brief \~english Creates a new empty WMML file with a populated header.
     *
     * \~russian Если файл с таким путём уже существует, бросается
     *      `std::invalid_argument`. В противном случае файл создаётся,
     *      количество объектов (`archivedCount`) и `height_` обнуляются,
     *      `width_` устанавливается равным размеру пакета типов, записывается
     *      заголовок, а `start` — смещением начала данных после заголовка.
     *
     * \~english If a file with the given path already exists,
     *      `std::invalid_argument` is thrown. Otherwise the file is created,
     *      `archivedCount` and `height_` are zeroed, `width_` is set to the
     *      type-pack size, the header is written and `start` is set to the
     *      data offset immediately after the header.
     */
    void create_file();

    /**
     * @brief \~russian Читает из заголовка количество сохранённых объектов.
     * @brief \~english Reads the stored object count from the header.
     *
     * \~russian Позиция устанавливается сразу после полей версии, ширины и
     *      высоты; значение записывается в `archivedCount`.
     *
     * \~english The position is set right after the version, width and height
     *      fields; the value is written into `archivedCount`.
     */
    void read_archive_count();

    /**
     * @brief \~russian Читает из заголовка количество записанных объектов (`height_`).
     * @brief \~english Reads the number of written objects (`height_`) from the
     *             header.
     *
     * \~russian Позиция устанавливается сразу после полей версии и ширины.
     *
     * \~english The position is set right after the version and width fields.
     */
    void read_height();

    /**
     * @brief \~russian Формирует текст сообщения об ошибке выхода за границы файла.
     * @brief \~english Builds the error message for an out-of-range seek.
     *
     * \~russian Возвращает строку с описанием ошибки и путём к текущему файлу.
     *      Используется в связке с макросом `assert_r` для генерации
     *      `std::runtime_error`.
     *
     * \~english Returns a message string describing the error and the current
     *      file path. Used together with the `assert_r` macro to raise a
     *      `std::runtime_error`.
     *
     * @return \~russian текст сообщения об ошибке;
     *         \~english the error message text.
     */
    std::string out_range_error() {
        return "Seek to out of range in file \"" + filePath.string() + "\"";
    }


    /**
     * @brief \~russian Читает одно значение типа `T` из потока в формате версии 1.4
     *            (legacy) во временный буфер.
     * @brief \~english Reads a single value of type `T` from a version 1.4
     *            (legacy) format stream into a temporary buffer.
     *
     * \~russian Считывает идентификатор типа и проверяет его соответствие хэшу
     *      типа `T` (учитывая совместимые legacy-идентификаторы `LONG_INT`
     *      и `UNSIGNED_LONG_INT` для 64-битных целых). При совпадении данные
     *      переписываются в `buff` без идентификатора.
     *
     * \~english Reads the type identifier and checks it against the hash of type
     *      `T` (accounting for the compatible legacy identifiers
     *      `LONG_INT` and `UNSIGNED_LONG_INT` for 64-bit integers). On match
     *      the data are copied into `buff` without the identifier.
     *
     * @tparam T \~russian тип читаемого значения;
     *           \~english type of the value being read.
     * @param buff \~russian буфер, в который записываются данные;
     *             \~english buffer the data are written into.
     * @return \~russian `true`, если тип совпал и данные прочитаны, иначе `false`.
     *         \~english `true` if the type matched and the data were read,
     *         `false` otherwise.
     * @throws std::runtime_error \~russian при выходе за границы потока.
     *                          \~english on seeking beyond the stream range.
     */
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

    /**
     * @brief \~russian Преобразует одно значение типа `T` из legacy-формата
     *            (версия 1.4) в текущий формат (версия 1.5) в place.
     * @brief \~english Converts a single value of type `T` from the legacy format
     *            (version 1.4) to the current format (version 1.5) in place.
     *
     * \~russian Считывает идентификатор типа и само значение из текущей позиции
     *      потока, затем записывает значение в позицию `writePos` (в начало
     *      нового формата, без идентификатора) и увеличивает `writePos` на
     *      sizeof(T). Позиция чтения восстанавливается после записи.
     *
     * \~english Reads the type identifier and the value from the current stream
     *      position, then writes the value at position `writePos` (in the
     *      new format, without an identifier) and advances `writePos` by
     *      sizeof(T). The read position is restored after the write.
     *
     * @tparam T \~russian тип преобразуемого значения;
     *           \~english type of the value being converted.
     * @param[in,out] writePos \~russian позиция записи в новом формате;
     *                        \~english write position in the new format.
     * @return \~russian `true`, если тип совпал и значение преобразовано,
     *         иначе `false`.
     *         \~english `true` if the type matched and the value was converted,
     *         `false` otherwise.
     * @throws std::runtime_error \~russian при неизвестном идентификаторе типа
     *                          или выходе за границы потока.
     *                          \~english on an unknown type identifier or on
     *                          seeking beyond the stream range.
     */
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


    /**
     * @brief \~russian Читает одно значение типа `T` из текущей позиции потока
     *            (формат версии 1.5, без идентификатора типа).
     * @brief \~english Reads a single value of type `T` from the current stream
     *            position (version 1.5 format, no type identifier).
     *
     * \~russian После чтения позиция потока сдвигается вперёд на `sizeof(T)`.
     *
     * \~english After reading, the stream position is advanced by `sizeof(T)`.
     *
     * @tparam T \~russian тип читаемого значения;
     *           \~english type of the value being read.
     * @param[out] t \~russian переменная, в которую записывается значение;
     *             \~english variable the value is written into.
     * @return \~russian `true` при успешном чтении, `false` при выходе за
     *         границы данных.
     *         \~english `true` on successful read, `false` when reading beyond
     *         the data range.
     */
    template <typename T>
    bool read_one(T& t) {
        auto result = s.read(_p(t));
        s.rel_seek(sizeof(t));
        return result;
    }

    /**
     * @brief \~russian Записывает одно значение типа `T` в текущую позицию потока
     *            (формат версии 1.5, без идентификатора типа).
     * @brief \~english Writes a single value of type `T` to the current stream
     *            position (version 1.5 format, no type identifier).
     *
     * \~russian Значение записывается побайтово; позиция потока увеличивается на
     *      `sizeof(T)`.
     *
     * \~english The value is written byte by byte; the stream position advances by
     *      `sizeof(T)`.
     *
     * @tparam T \~russian тип записываемого значения;
     *           \~english type of the value being written.
     * @param t \~russian значение для записи;
     *          \~english the value to write.
     */
    template <typename T>
    void write_one(const T& t) {
        s.write(reinterpret_cast<std::byte*>(const_cast<T*>(&t)), sizeof(t));
    }

public:
    /**
     * @brief \~russian Открывает WMML-файл для чтения/записи.
     * @brief \~english Opens a WMML file for reading/writing.
     *
     * \~russian Публичная точка входа базового класса: создаёт файл и записывает
     *      во внутренний поток его содержимое. Фактическая инициализация
     *      (проверка заголовка, конвертация версий) выполняется в
     *      производном классе `wmml`.
     *
     * \~english The base class public entry point: creates the file and writes
     *      its contents into the internal stream. Actual initialization
     *      (header validation, version conversion) is performed in the
     *      derived `wmml` class.
     *
     * @param pathToOpenedFile \~russian путь к WMML-файлу;
     *                         \~english path to the WMML file.
     */
    void open(const std::filesystem::path& pathToOpenedFile);

    /**
     * @brief \~russian Возвращает позицию чтения в начало данных и сбрасывает
     *            счётчик локально прочитанных архивов.
     * @brief \~english Resets the read position to the beginning of the data and
     *            clears the locally read archive counter.
     *
     * \~russian После вызова можно перечитывать объект с начала файла
     *      (см. `wmml::read`).
     *
     * \~english After calling, the file can be re-read from the beginning (see
     *      `wmml::read`).
     */
    void reset();

    /**
     * @brief \~russian Сохраняет изменения во внутренней копии в файл.
     * @brief \~english Persists changes made in the in-memory copy to the file.
     *
     * \~russian Выполняется только если файл открыт и доступен на запись.
     *      Метод не бросает исключений (`noexcept`).
     *
     * \~english Only runs when the file is open and writable. This method does
     *      not throw (`noexcept`).
     */
    void save() noexcept;
};

/**
 * @brief \~russian Специализация: читает строку формата 1.5 (16-битная длина +
 *            байты).
 * @brief \~english Specialization: reads a version 1.5 string (16-bit length +
 *            bytes).
 *
 * @param[out] t \~russian строка, в которую записывается результат;
 *             \~english string the result is written into.
 * @return \~russian `true` при успешном чтении, иначе `false`.
 *         \~english `true` on successful read, `false` otherwise.
 */
template <> bool base_wmml::read_one<std::string>(std::string& t);

/**
 * @brief \~russian Специализация: записывает строку формата 1.5 (16-битная
 *            длина + байты).
 * @brief \~english Specialization: writes a version 1.5 string (16-bit length +
 *            bytes).
 *
 * @param t \~russian строка для записи.
 *          \~english the string to write.
 */
template <> void base_wmml::write_one<std::string>(const std::string& t);

/**
 * @brief \~russian Специализация: читает строку legacy-формата (идентификатор +
 *            8/16/32-битная длина + байты) во временный буфер.
 * @brief \~english Specialization: reads a legacy-format string (identifier +
 *            8/16/32-bit length + bytes) into a temporary buffer.
 *
 * @param buff \~russian буфер, в который записываются данные (16-битная длина +
 *             байты, без legacy-идентификатора);
 *             \~english buffer the data are written into (16-bit length + bytes,
 *             without the legacy identifier).
 * @return \~russian `true`, если идентификатор строки распознан и данные
 *         прочитаны, иначе `false`.
 *         \~english `true` if the string identifier is recognized and the data
 *         were read, `false` otherwise.
 */
template <> bool base_wmml::legacy_read_one<std::string>(details::SmallStream& buff);

/**
 * @brief \~russian Специализация: преобразует строку legacy-формата в формат 1.5
 *            in place.
 * @brief \~english Specialization: converts a legacy-format string to version 1.5
 *            in place.
 *
 * @param[in,out] writePos \~russian позиция записи в новом формате;
 *                        \~english write position in the new format.
 * @return \~russian `true`, если идентификатор строки распознан и значение
 *         преобразовано, иначе `false`.
 *         \~english `true` if the string identifier is recognized and the value
 *         was converted, `false` otherwise.
 */
template <> bool base_wmml::legacy_convert_one<std::string>(std::size_t& writePos);


namespace details {

/**
 * @brief \~russian Возвращает размер (в байтах) поля типа `T`, записанного по
 *            позиции `pos` потока.
 * @brief \~english Returns the size (in bytes) of a field of type `T` stored at
 *            stream position `pos`.
 *
 * \~russian Поля с плавающей точкой и целые имеют фиксированный размер `sizeof(T)`.
 *      Строки имеют переменную длину: 16-битная длина + байты строки.
 *      Позиция чтения (головка) восстанавливается после измерения.
 *
 * \~english Floating point and integer fields have a fixed size of `sizeof(T)`.
 *      Strings have a variable length: a 16-bit length plus the string bytes.
 *      The stream head is restored after measuring.
 *
 * @tparam T \~russian тип поля; \~english the field type.
 * @param s \~russian поток; \~english the stream.
 * @param pos \~russian начало поля в потоке; \~english the field start position.
 * @return \~russian размер поля в байтах; \~english the field size in bytes.
 * @throws std::runtime_error \~russian при выходе за границы потока.
 *                          \~english on seeking beyond the stream range.
 */
template <typename T>
inline std::size_t field_size_at(stream& s, const std::size_t pos) {
    if constexpr (std::is_same_v<T, std::string>) {
        const std::size_t saved = s.get_pos();
        assert_r(s.seek(pos), "Attempt to measure field size out of stream range");
        int16_t len;
        assert_r(s.read(_p(len)), "Attempt to read field length out of stream range");
        s.seek(saved);
        return sizeof(len) + static_cast<std::size_t>(len);
    }
    else {
        return sizeof(T);
    }
}

/**
 * @brief \~russian Пропускает одно поле типа `T`: сдвигает головку потока вперёд
 *            на размер поля, не копируя данные.
 * @brief \~english Skips a single field of type `T`: advances the stream head by
 *            the field size without copying the data.
 *
 * \~russian Для строк сначала считывается 16-битная длина, затем головка
 *      сдвигается на длину + sizeof(длины).
 *
 * \~english For strings the 16-bit length is read first, then the head is
 *      advanced by the length plus the size of the length field.
 *
 * @tparam T \~russian тип поля; \~english the field type.
 * @param s \~russian поток; \~english the stream.
 * @return \~russian `true` при успехе, `false` при выходе за границы данных;
 *         \~english `true` on success, `false` when reading beyond the data range.
 */
template <typename T>
inline bool skip_field(stream& s) {
    if constexpr (std::is_same_v<T, std::string>) {
        int16_t len;
        if (!s.rel_read(_p(len)))
            return false;
        return s.rel_seek(len);
    }
    else {
        return s.rel_seek(sizeof(T));
    }
}

} // namespace details


/**
 * @brief \~russian Шаблонный класс для работы с бинарными WMML-файлами фиксированной
 *            схемы.
 * @brief \~english Template class for working with fixed-schema binary WMML files.
 *
 * \~russian Набор шаблонных аргументов `Args...` определяет схему объекта
 *      (полевые типы в порядке следования) и фиксирует «ширину» файла.
 *      Класс автоматически открывает существующий файл либо создаёт новый,
 *      при необходимости конвертирует данные из версии формата 1.4 в 1.5
 *      и сохраняет изменения при уничтожении объекта.
 *
 * \~english The set of template arguments `Args...` defines the object schema
 *      (field types in order) and fixes the file "width". The class
 *      automatically opens an existing file or creates a new one, converts
 *      data from format version 1.4 to 1.5 when needed, and persists changes
 *      on destruction.
 *
 * @tparam Args \~russian типы полей объекта, поддерживаемые WMML (см.
 *            `WmmlFormats`);
 *            \~english object field types supported by WMML (see `WmmlFormats`).
 *
 * @throws std::runtime_error \~russian при несовпадении ширины/пакета типов файла
 *                          с ожидаемыми, ошибках чтения и конвертации.
 *                          \~english on mismatch between the file width/type pack
 *                          and the expected ones, on read and conversion
 *                          errors.
 * @throws std::invalid_argument \~russian при попытке создать уже существующий
 *                          файл.
 *                          \~english when trying to create an already existing
 *                          file.
 *
 * @note \~russian TODO: добавить флаг isValide — при ошибке операции не должна
 *            записываться битая копия; либо lock-guard, перехватывающий флаг
 *            `readOnly` на время записи, чтобы деструктор, вызванный из
 *            исключения, не перезаписал файл.
 * @note \~english TODO: add an isValide flag — a failed operation must not write a
 *            corrupted copy; or a lock-guard taking over the `readOnly` flag
 *            during writes so that a destructor invoked from an exception
 *            cannot overwrite the file.
 */
template <typename... Args>
class wmml : public base_wmml
{
    inline static constexpr std::array<std::byte, sizeof...(Args)> pack = {
        type_to_hash<Args>()...
    };

    /**
     * @brief \~russian Инициализирует объект: проверяет заголовок, при необходимости
     *            конвертирует legacy-файл (1.4 → 1.5).
     * @brief \~english Initializes the object: validates the header and, when
     *             needed, converts a legacy file (1.4 → 1.5).
     *
     * \~russian Последовательность действий:
     *      1. ширина файла проверяется на соответствие `width_`;
     *      2. читаются количество архивов и высота;
     *      3. если версия файла старее текущей — выполняется конвертация
     *         `convert_stream_from_1_4_to_1_5_version()` и (если файл доступен
     *         на запись и включена автоконвертация) результат сохраняется;
     *      4. иначе проверяется пакет хэшей типов — при несовпадении
     *         бросается `std::runtime_error`;
     *      5. позиция устанавливается в начало данных (`start`).
     *
     * \~english Steps:
     *      1. the file width is checked against `width_`;
     *      2. the archive count and height are read;
     *      3. if the file version is older than the current one,
     *         `convert_stream_from_1_4_to_1_5_version()` runs and (when the
     *         file is writable and auto-conversion is enabled) the result is
     *         saved;
     *      4. otherwise the type-hash pack is validated — on mismatch a
     *         `std::runtime_error` is thrown;
     *      5. the position is set to the start of the data (`start`).
     *
     * @throws std::runtime_error \~russian при несовпадении ширины, несовпадении
     *                          пакета типов или ошибке конвертации.
     *                          \~english on width mismatch, type-pack mismatch or
     *                          conversion error.
     */
    void init () {
        width_ = pack_size;
        if (!width_is_valide())
            throw std::runtime_error("The width of opened file \"" + filePath.string() +
                                     "\" is not comply with tool's width.");
        read_archive_count();
        read_height();
        if (!version_is_valide()) {
            convert_stream_from_1_4_to_1_5_version();
            if (!readOnly && convertAutomatically)
                save();
        }
        else if (!hash_is_valide())
            throw std::runtime_error(
                "Template instruction options do not correspond to the file \"" +
                filePath.string() + "\"  parameters");
        s.seek(start);
    }


    /**
     * @brief \~russian Преобразует поток из версии формата 1.4 в версию 1.5.
     * @brief \~english Converts the stream from format version 1.4 to version 1.5.
     *
     * \~russian Алгоритм:
     *      1. новый заголовок (версия, ширина, высота, количество архивов,
     *         пакет хэшей типов) записывается во временный буфер;
     *      2. из legacy-потока (с идентификаторами типов) читаются первые
     *         два объекта в тот же временный буфер;
     *      3. временный буфер переписывается в начало потока, вытесняя
     *         старое содержимое;
     *      4. оставшиеся объекты читаются из legacy-данных и дописываются
     *         в конец нового потока;
     *      5. поток усекается до размера записанных данных.
     *      Конвертация выполняется in place: старая копия данных не
     *      сохраняется.
     *
     * \~english Algorithm:
     *      1. a new header (version, width, height, archive count, type-hash
     *         pack) is written into a temporary buffer;
     *      2. the first two objects are read from the legacy stream (with
     *         type identifiers) into the same temporary buffer;
     *      3. the temporary buffer is written back to the beginning of the
     *         stream, displacing the old contents;
     *      4. the remaining objects are read from the legacy data and
     *         appended to the end of the new stream;
     *      5. the stream is truncated to the size of the written data.
     *      The conversion is performed in place: no copy of the old data is
     *      kept.
     *
     * @throws std::runtime_error \~russian при несовпадении типов при чтении или
     *                          конвертации, либо выходе за границы потока.
     *                          \~english on type mismatch during read or
     *                          conversion, or on seeking beyond the stream
     *                          range.
     */
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
    /**
     * @brief \~russian Пустой конструктор: создаёт неинициализированный объект без
     *            привязки к файлу.
     * @brief \~english Default constructor: creates an uninitialized object not
     *             bound to any file.
     *
     * @warning \~russian Объект, созданный таким образом, нельзя использовать
     *              для чтения/записи — файл не открыт.
     * @warning \~english An object created this way cannot be used for
     *              reading/writing — no file is open.
     */
    wmml() = default;

    /**
     * @brief \~russian Создаёт объект в режиме только для чтения из буфера
     *            WMML-данных (C-строка).
     * @brief \~english Creates a read-only object from a buffer of WMML data
     *             (C-string).
     *
     * \~russian Содержимое `file_stream` копируется во внутренний поток, после
     *      чего выполняется `init()`. Файл на диске не используется
     *      (`filePath` — пустая строка).
     *
     * \~english The contents of `file_stream` are copied into the internal stream,
     *      then `init()` is performed. No on-disk file is used
     *      (`filePath` is an empty string).
     *
     * @param file_stream \~russian буфер с байтами WMML (включая заголовок);
     *                   \~english buffer with WMML bytes (including the header).
     * @param streamSize \~russian размер буфера;
     *                   \~english buffer size.
     * @throws std::runtime_error \~russian при несовпадении заголовка с ожидаемой
     *                          схемой или ошибке конвертации.
     *                          \~english on header mismatch with the expected
     *                          schema or on conversion error.
     */
    wmml (const char* file_stream, const std::size_t streamSize) : base_wmml(pack.data(), sizeof(pack), true, "") {
        s.open(streamSize);
        s.write(reinterpret_cast<std::byte*>(const_cast<char*>(file_stream)), streamSize);
        init();
    }

    /**
     * @brief \~russian Создаёт объект, привязанный к WMML-файлу на диске.
     * @brief \~english Creates an object bound to an on-disk WMML file.
     *
     * \~russian Если файл существует, он открывается, загружается во внутренний
     *      поток и инициализируется. Если файл не существует — создаётся
     *      новый пустой файл с заполненным заголовком. Объект доступен на
     *      чтение и запись.
     *
     * \~english If the file exists, it is opened, loaded into the internal stream
     *      and initialized. If it does not exist, a new empty file with a
     *      populated header is created. The object is readable and writable.
     *
     * @param pathToOpenedFile \~russian путь к WMML-файлу;
     *                         \~english path to the WMML file.
     * @throws std::runtime_error \~russian при ошибке открытия, несовпадении
     *                          заголовка или ошибке конвертации.
     *                          \~english on open failure, header mismatch or
     *                          conversion error.
     * @throws std::invalid_argument \~russian если при создании файла файл с таким
     *                          путём уже существует.
     *                          \~english if, when creating the file, a file with
     *                          that path already exists.
     */
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

    /**
     * @brief \~russian Деструктор: сохраняет все несохранённые изменения в файл.
     * @brief \~english Destructor: saves all unsaved changes to the file.
     *
     * \~russian Вызывает `save()` — изменения пишутся только если файл открыт и
     *      доступен на запись.
     *
     * \~english Calls `save()` — changes are written only if the file is open and
     *      writable.
     */
    ~wmml () {
        save();
    }

    /**
     * @brief \~russian Читает один объект (набор полей схемы) из текущей позиции.
     * @brief \~english Reads one object (a full set of schema fields) from the
     *             current position.
     *
     * \~russian Аргументы должны передаваться в том же порядке и с теми же
     *      типами, что и шаблонные аргументы класса. Если позиция находится
     *      в заголовке, она возвращается в начало данных (`start`).
     *
     * \~english The arguments must be passed in the same order and with the same
     *      types as the class template arguments. If the position is in the
     *      header, it is moved back to the start of the data (`start`).
     *
     * @tparam \~russian типы берутся из `Args...`.
     * @param args \~russian переменные, в которые записываются поля объекта;
     *             \~english variables the object fields are written into.
     * @return \~russian `true`, если все поля прочитаны успешно, иначе `false`.
     *         \~english `true` if all fields were read successfully, `false`
     *         otherwise.
     * @throws std::runtime_error \~russian при выходе позиции за границы файла.
     *                          \~english if the position seeks beyond the file
     *                          range.
     */
    bool read(Args&... args) {
        if (s.get_pos() < start)
            assert_r(s.seek(start), out_range_error());
        return (read_one(args) && ...);
    }

    /**
     * @brief \~russian Добавляет один объект (набор полей схемы) в конец файла.
     * @brief \~english Appends one object (a full set of schema fields) to the end
     *             of the file.
     *
     * \~russian Позиция устанавливается в конец потока, затем все аргументы
     *      записываются последовательно; счётчик объектов `height_`
     *      увеличивается на 1. Аргументы должны соответствовать схеме
     *      `Args...` по типам и порядку.
     *
     * \~english The position is set to the end of the stream, then all arguments
     *      are written in order; the object counter `height_` is incremented
     *      by 1. The arguments must match the `Args...` schema by type and
     *      order.
     *
     * @param args \~russian значения полей нового объекта;
     *             \~english the field values of the new object.
     * @throws std::runtime_error \~russian при выходе позиции за границы файла.
     *                          \~english if the position seeks beyond the file
     *                          range.
     */
    void write(const Args&... args) {
        assert_r(s.seek(s.get_size()), out_range_error());
        (write_one(args), ...);
        height_++;
    }


    /**
     * @brief \~russian Возвращает текущий размер внутреннего потока в байтах.
     * @brief \~english Returns the current size of the internal stream in bytes.
     *
     * \~russian Удобно использовать вместе с `get_stream_ptr` для получения
     *      полного содержимого потока.
     * \~english Handy to use together with `get_stream_ptr` to obtain the whole
     *      stream contents.
     */
    std::size_t get_stream_size() const { return s.get_size(); }

    /**
     * @brief \~russian Возвращает указатель на участок внутреннего потока,
     *            начинающийся с позиции `pos` и имеющий длину `size` байт.
     * @brief \~english Returns a pointer to a region of the internal stream,
     *            starting at position `pos` and `size` bytes long.
     *
     * \~russian Позволяет записывать WMML-файл напрямую, например, в архив,
     *      минуя запись в файл на диске. Позиция чтения (головка) при этом
     *      устанавливается в `pos`.
     *
     * \~english Allows writing the WMML file directly (e.g. into an archive)
     *      without saving it to a file on disk. The stream head is set to
     *      `pos` as a side effect.
     *
     * @param pos \~russian начало участка в потоке; \~english the region start
     *                 position in the stream;
     * @param size \~russian длина участка в байтах; \~english the region length in bytes.
     * @return \~russian указатель на байты участка (действителен до следующей
     *              модификации потока);
     *         \~english pointer to the region bytes (valid until the next stream
     *                 modification).
     * @throws std::runtime_error \~russian при выходе за границы потока.
     *                          \~english on seeking beyond the stream range.
     */
    const char* get_stream_ptr(const std::size_t& pos, const std::size_t& size) {
        assert_r(s.seek(pos), out_range_error());
        const std::byte* ptr = s.get_ptr(size);
        assert_r(ptr != nullptr, out_range_error());
        return reinterpret_cast<const char*>(ptr);
    }

    /**
     * @brief \~russian Пропускает целый блок (объект схемы) без копирования данных
     *            в переменные.
     * @brief \~english Skips a whole block (one object of the schema) without
     *            copying the data into variables.
     *
     * \~russian Аналогичен вызову `read`, но не читает значения: головка потока
     *      сдвигается вперёд на суммарный размер всех полей блока. Для строк
     *      учитывается их переменная длина (16-битная длина + байты). Если
     *      позиция находится в заголовке, она возвращается в начало данных
     *      (`start`).
     *
     * \~english Similar to a `read` call, but does not fetch values: the stream
     *      head is advanced by the total size of all fields in the block. For
     *      strings their variable length (16-bit length + bytes) is accounted
     *      for. If the position is in the header, it is moved back to the start
     *      of the data (`start`).
     *
     * @return \~russian `true`, если блок целиком находится в данных,
     *         `false`, если при пропуске головка вышла за конец потока;
     *         \~english `true` if the whole block is within the data,
     *         `false` if the head seeks beyond the end of the stream while skipping;
     * @throws std::runtime_error \~russian при выходе позиции за границы файла.
     *                          \~english if the position seeks beyond the file range.
     */
    bool skip() {
        if (s.get_pos() < start)
            assert_r(s.seek(start), out_range_error());
        return (details::skip_field<Args>(s) && ...);
    }

    /**
     * @brief \~russian Удаляет объект схемы с номером `objectNumber`, сдвигая
     *            остаток потока влево.
     * @brief \~english Removes the schema object at index `objectNumber`, shifting
     *            the rest of the stream to the left.
     *
     * \~russian Позиция объекта вычисляется по полному позиционированию:
     *      отсчёт ведётся от `start`, суммируя размеры предыдущих объектов
     *      (для строк — с учётом их переменной длины). Данные, следующе за
     *      удалённым объектом, перемещаются на его место, размер потока
     *      уменьшается, а головка восстанавливается в `start`.
     *      Счётчик объектов `height_` уменьшается на 1.
     *
     * \~english The object position is computed by full positioning: counting
     *      from `start` and summing the sizes of the preceding objects (for
     *      strings — accounting for their variable length). The data following
     *      the removed object are moved to its place, the stream is truncated,
     *      and the head is restored to `start`. The object counter `height_`
     *      is decremented by 1.
     *
     * @param objectNumber \~russian порядковый номер объекта (отсчёт от 0);
     *                    \~english zero-based object index.
     * @throws std::runtime_error \~russian при выходе номера объекта за границы
     *                          данных или позиции за границы потока.
     *                          \~english when the object index is out of the data
     *                          range or the position seeks beyond the stream range.
     */
    void remove_object(const std::size_t objectNumber) {
        assert_r(objectNumber < height_, "Object index is out of range");
        const std::size_t objStart = object_start(objectNumber);
        const std::size_t objEnd =
            objStart + block_size_at(objStart, std::index_sequence_for<Args...>{});
        s.move(objEnd, objStart);
        s.seek(start);
        --height_;
    }

    /**
     * @brief \~russian Перезаписывает сектор (поле) внутри объекта с номером
     *            `objectNumber`.
     * @brief \~english Overwrites a sector (field) inside the object at index
     *            `objectNumber`.
     *
     * \~russian `Index` — порядковый номер поля в схеме (начиная с 0); тип
     *      значения выводится из аргумента и сверяется с типом поля схемы
     *      через `static_assert`. Позиция объекта вычисляется по полному
     *      позиционированию от `start`. Для полей фиксированного размера
     *      значение записывается поверх старых данных. Для строк при изменении
     *      длины выполняется сдвиг хвоста потока: данные, следующие за строкой,
     *      перемещаются вправо (при удлинении) или влево (при укорочении),
     *      а затем записываются новая 16-битная длина и байты строки.
     *      После операции головка восстанавливается в `start`.
     *
     * \~english `Index` is the zero-based field number in the schema; the value
     *      type is deduced from the argument and checked against the schema
     *      field type via `static_assert`. The object position is computed by
     *      full positioning from `start`. Fixed-size fields are overwritten in
     *      place. For strings whose length changes a stream tail shift is
     *      performed: the data following the string are moved right (on
     *      lengthening) or left (on shortening), then the new 16-bit length and
     *      the string bytes are written. After the operation the head is
     *      restored to `start`.
     *
     * @tparam Index \~russian порядковый номер поля в схеме;
     *              \~english the zero-based field number in the schema;
     * @tparam T \~russian тип значения (выводится из аргумента, должен
     *             совпадать с типом поля `Index`);
     *             \~english the value type (deduced from the argument, must
     *             match the type of field `Index`);
     * @param objectNumber \~russian порядковый номер объекта (отсчёт от 0);
     *                    \~english zero-based object index.
     * @param value \~russian новое значение поля; \~english the new field value.
     * @throws std::runtime_error \~russian при несовпадении типов (компиляция),
     *                          выходе номера объекта за границы данных или
     *                          позиции за границы потока.
     *                          \~english on type mismatch (compile time), object
     *                          index out of range, or position out of stream range.
     */
    template <std::size_t Index, typename T>
    void overwrite_sector(const std::size_t objectNumber, const T& value) {
        using FieldT = typename std::tuple_element<Index, std::tuple<Args...>>::type;
        static_assert(std::is_same_v<T, FieldT>,
                      "The value type does not match the type of the schema field at Index");
        assert_r(objectNumber < height_, "Object index is out of range");
        const std::size_t objStart = object_start(objectNumber);
        const std::size_t fieldPos =
            objStart + field_offset_impl<Index>(objStart, std::index_sequence_for<Args...>{});
        if constexpr (std::is_same_v<T, std::string>) {
            int16_t oldLen = 0;
            assert_r(s.seek(fieldPos), out_range_error());
            assert_r(s.read(_p(oldLen)), out_range_error());
            const std::size_t oldLenSize = static_cast<std::size_t>(oldLen);
            if (value.size() != oldLenSize) {
                const std::size_t tailFrom = fieldPos + sizeof(oldLen) + oldLenSize;
                const std::size_t tailTo = fieldPos + sizeof(oldLen) + value.size();
                s.move(tailFrom, tailTo);
            }
            s.seek(fieldPos);
            int16_t newLen = static_cast<int16_t>(value.size());
            s.write(_p(newLen));
            s.write(reinterpret_cast<std::byte*>(const_cast<char*>(value.data())), value.size());
        }
        else {
            assert_r(s.seek(fieldPos), out_range_error());
            s.write(reinterpret_cast<std::byte*>(const_cast<T*>(&value)), sizeof(value));
        }
        s.seek(start);
    }

private:
    /**
     * @brief \~russian Вычисляет суммарный размер блока, начинающегося с позиции
     *            `pos` (сумма размеров всех полей схемы).
     * @brief \~english Computes the total size of the block starting at position
     *            `pos` (the sum of the sizes of all schema fields).
     *
     * \~russian Для строк размер вычисляется в рантайме по 16-битной длине,
     *      сохранённой в самом блоке.
     *
     * \~english For strings the size is computed at runtime from the 16-bit
     *      length stored in the block itself.
     *
     * @param pos \~russian начало блока; \~english the block start position.
     * @return \~russian размер блока в байтах; \~english the block size in bytes.
     */
    template <std::size_t... Is>
    std::size_t block_size_at(const std::size_t pos, std::index_sequence<Is...>) {
        std::size_t total = 0;
        ((total += details::field_size_at<Args>(s, pos + total)), ...);
        return total;
    }

    /**
     * @brief \~russian Вычисляет абсолютную позицию начала объекта с номером
     *            `objectNumber` (полное позиционирование от `start`).
     * @brief \~english Computes the absolute start position of the object at index
     *            `objectNumber` (full positioning from `start`).
     *
     * \~russian Суммирует размеры всех предыдущих объектов, начиная с `start`.
     *      Позиция головки после вызова не меняется.
     *
     * \~english Sums the sizes of all preceding objects, starting from `start`.
     *      The stream head is not changed by the call.
     *
     * @param objectNumber \~russian порядковый номер объекта (отсчёт от 0);
     *                    \~english zero-based object index.
     * @return \~russian позиция начала объекта в потоке;
     *         \~english the object start position in the stream.
     */
    std::size_t object_start(const std::size_t objectNumber) {
        std::size_t pos = start;
        for (std::size_t i = 0; i < objectNumber; ++i)
            pos += block_size_at(pos, std::index_sequence_for<Args...>{});
        return pos;
    }
    /**
     * @brief \~russian Вычисляет смещение `Index`-го поля внутри блока, начинающегося
     *            с позиции `objStart` (сумма размеров полей, предшествующих ему).
     * @brief \~english Computes the offset of the `Index`-th field inside the block
     *            starting at `objStart` (the sum of the sizes of the fields that
     *            precede it).
     *
     * \~russian Для строк размер вычисляется в рантайме по 16-битной длине,
     *      сохранённой в самом блоке.
     *
     * \~english For strings the size is computed at runtime from the 16-bit
     *      length stored in the block itself.
     *
     * @tparam Index \~russian порядковый номер целевого поля;
     *              \~english the zero-based index of the target field;
     * @param objStart \~russian начало блока; \~english the block start position.
     * @return \~russian смещение поля относительно начала блока;
     *         \~english the field offset relative to the block start.
     */
    template <std::size_t Index, std::size_t... Is>
    std::size_t field_offset_impl(const std::size_t objStart, std::index_sequence<Is...>) {
        std::size_t offset = 0;
        ((offset += (Is < Index ? details::field_size_at<Args>(s, objStart + offset) : 0)), ...);
        return offset;
    }


};