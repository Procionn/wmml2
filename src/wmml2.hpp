#pragma once
#include <filesystem>
#include <fstream>
#include "stream.hpp"
#include "macro.hpp"

#include <iostream>
#include "enum.hpp"
#include <cstring>
#include <cassert>


/**
 * @brief (RU) Базовый класс библиотеки WMML: бинарный поток-зеркало файла,
 *             файловый дескриптор и работа с заголовком.
 * @brief (EN) Base class of the WMML library: a binary stream mirroring the
 *             file, the file descriptor and header handling.
 *
 * (RU) Класс хранит внутренний бинарный поток `s` (полная копия содержимого
 *      файла), файловый дескриптор `descript` и метаданные заголовка: версию,
 *      ширину, высоту, количество архивов и пакет хэшей типов. При ошибках
 *      открытия или чтения файла бросает `std::runtime_error` или
 *      `std::invalid_argument`.
 *
 * (EN) The class keeps an in-memory binary stream `s` (a full copy of the
 *      file contents), the file descriptor `descript`, and the header
 *      metadata: version, width, height, archive count and the type-hash
 *      pack. On file open or read failure it throws `std::runtime_error` or
 *      `std::invalid_argument`.
 *
 * @note (RU) Методы класса предназначены исключительно для производных
 *            (шаблонный класс `wmml`); публичный API ограничен методами
 *            `open()`, `reset()` и `save()`.
 * @note (EN) Class methods are intended for derived classes only (the `wmml`
 *            template); the public API is limited to `open()`, `reset()`
 *            and `save()`.
 */
class base_wmml
{
protected:
    /** (RU) Внутренний бинарный поток — зеркало содержимого файла.
     *  (EN) Internal binary stream — a mirror of the file contents. */
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
    /**
     * @brief (RU) Создаёт объект WMML, привязанный к пакету типов, режиму
     *            доступа и пути к файлу.
     * @brief (EN) Creates a WMML object bound to a type pack, an access mode
     *             and a file path.
     *
     * (RU) Файл в момент конструирования НЕ открывается — сохраняются только
     *      параметры, необходимые для последующей инициализации
     *      (см. производный класс `wmml`).
     *
     * (EN) The file is NOT opened at construction time — only the parameters
     *      required for later initialization are stored (see the `wmml`
     *      derived class).
     *
     * @param pack_ptr (RU) указатель на пакет хэшей типов объекта;
     *                 (EN) pointer to the object's type-hash pack;
     * @param size     (RU) размер пакета в байтах;
     *                 (EN) pack size in bytes;
     * @param readonly (RU) `true` — файл доступен только для чтения;
     *                 (EN) `true` — the file is read-only;
     * @param path     (RU) путь к WMML-файлу (пустая строка при создании из
     *                 текстового потока);
     *                 (EN) path to the WMML file (empty string when
     *                 constructing from a text stream).
     */
    base_wmml(const std::byte* pack_ptr, std::size_t pack_size, bool readOnly, const std::filesystem::path& path);

    /**
     * @brief (RU) Деструктор. Закрывает внутренний поток и файловый дескриптор.
     * @brief (EN) Destructor. Closes the internal stream and the file
     *             descriptor.
     */
    ~base_wmml() = default;

    /**
     * @brief (RU) Перезаписывает заголовок файла (первую секцию потока).
     * @brief (EN) Rewrites the file header (the first section of the stream).
     *
     * (RU) Устанавливает позицию потока в начало и последовательно записывает:
     *      версию, ширину, высоту, количество архивов и пакет хэшей типов.
     *
     * (EN) Seeks the stream to the beginning and writes in order: version,
     *      width, height, archive count and the type-hash pack.
     */
    void write_header();

    /**
     * @brief (RU) Проверяет, совпадает ли ширина, сохранённая в заголовке,
     *            с ожидаемой для данного набора шаблонных типов.
     * @brief (EN) Checks that the width stored in the header matches the
     *             expected value for the current template type set.
     *
     * (RU) Читает поле ширины из заголовка и сравнивает его с `width_`.
     *
     * (EN) Reads the width field from the header and compares it with
     *      `width_`.
     *
     * @return (RU) `true`, если ширина совпадает, иначе `false`.
     *         (EN) `true` if the widths match, `false` otherwise.
     */
    bool width_is_valide();

    /**
     * @brief (RU) Проверяет, что версия файла совпадает с поддерживаемой версией формата.
     * @brief (EN) Checks that the file version matches the supported format
     *             version.
     *
     * (RU) Сравнивает байт версии в начале файла с текущим значением
     *      `version`.
     *
     * (EN) Compares the version byte at the beginning of the file with the
     *      current value of `version`.
     *
     * @return (RU) `true`, если версии совпадают, иначе `false`.
     *         (EN) `true` if the versions match, `false` otherwise.
     */
    bool version_is_valide();

    /**
     * @brief (RU) Проверяет целостность пакета хэшей типов в заголовке.
     * @brief (EN) Validates the type-hash pack stored in the header.
     *
     * (RU) Убеждается, что ширина в заголовке соответствует ожидаемой, и что
     *      байты пакета типов в заголовке идентичны пакету текущих шаблонных
     *      типов. При совпадении устанавливает позицию потока `start` сразу
     *      после пакета типов.
     *
     * (EN) Ensures that the header width matches the expected value and that
     *      the type-pack bytes in the header are identical to the current
     *      template type pack. On success the stream position `start` is set
     *      right after the type pack.
     *
     * @return (RU) `true`, если пакет типов идентичен, иначе `false`.
     *         (EN) `true` if the type packs are identical, `false` otherwise.
     */
    bool hash_is_valide();

    /**
     * @brief (RU) Открывает файл, описываемый `filePath`, в бинарном режиме
     *            без буферизации.
     * @brief (EN) Opens the file described by `filePath` in unbuffered binary
     *             mode.
     *
     * (RU) При `createFile == true` файл создаётся (с усечением),
     *      при `false` — открывается существующий. Если открыть файл
     *      не удалось, бросается `std::runtime_error`.
     *
     * (EN) When `createFile == true` the file is created (truncated),
     *      otherwise an existing file is opened. If the file cannot be
     *      opened, a `std::runtime_error` is thrown.
     *
     * @param createFile (RU) флаг создания нового файла;
     *                   (EN) flag creating a new file.
     */
    void open_file(const bool createFile = false);

    /**
     * @brief (RU) Загружает всё содержимое файла во внутренний поток без
     *            промежуточных копирований.
     * @brief (EN) Loads the entire file contents into the internal stream
     *             without intermediate copies.
     *
     * (RU) Требует, чтобы файл уже был открыт (`isOpen == true`,
     *      контролируется `assert`). Размер потока выделяется под текущий
     *      размер файла на диске, затем данные читаются напрямую в буфер
     *      потока.
     *
     * (EN) Requires the file to be already open (`isOpen == true`, enforced
     *      by `assert`). The stream is sized to the on-disk file size, then
     *      the data are read directly into the stream buffer.
     */
    void read_file_in_stream();

    /**
     * @brief (RU) Сохраняет содержимое внутреннего потока в файл.
     * @brief (EN) Persists the internal stream contents to the file.
     *
     * (RU) Если файл на диске крупнее потока, сначала выполняется
     *      `resize()`. Далее весь поток записывается в файл с начала и
     *      буфер принудительно сбрасывается на диск. Требует режима записи
     *      (`readOnly == false`, контролируется `assert`).
     *
     * (EN) If the on-disk file is larger than the stream, `resize()` is
     *      performed first. Then the whole stream is written to the file from
     *      the beginning and the buffer is flushed. Requires write access
     *      (`readOnly == false`, enforced by `assert`).
     */
    void write_stream_to_file();

    /**
     * @brief (RU) Изменяет размер файла на диске.
     * @brief (EN) Changes the on-disk file size.
     *
     * (RU) Закрывает файловый дескриптор, изменяет размер файла через
     *      `std::filesystem::resize_file` и заново открывает его. Требует
     *      режима записи (`readOnly == false`).
     *
     * (EN) Closes the file descriptor, resizes the file via
     *      `std::filesystem::resize_file` and reopens it. Requires write
     *      access (`readOnly == false`).
     *
     * @param size (RU) новый размер файла в байтах;
     *             (EN) new file size in bytes.
     */
    void resize(const std::size_t& size);

    /**
     * @brief (RU) Открывает существующий файл, если он есть.
     * @brief (EN) Opens the file if it exists.
     *
     * (RU) Если файл существует, но не имеет метаданных (размер 0 байт):
     *      - при `autoDeletingFilesWithoutMetadata == true` файл удаляется и
     *        возвращается `false`;
     *      - иначе бросается `std::runtime_error`.
     *      Если файл содержит данные — он открывается и возвращается `true`.
     *
     * (EN) If the file exists but has no metadata (0 bytes in size):
     *      - when `autoDeletingFilesWithoutMetadata == true` the file is
     *        removed and `false` is returned;
     *      - otherwise a `std::runtime_error` is thrown.
     *      If the file contains data — it is opened and `true` is returned.
     *
     * @return (RU) `true`, если файл открыт, `false`, если его не существует
     *         или он был удалён;
     *         (EN) `true` if the file was opened, `false` if it does not
     *         exist or was removed.
     */
    bool open_existed_file();

    /**
     * @brief (RU) Создаёт новый пустой WMML-файл с заполненным заголовком.
     * @brief (EN) Creates a new empty WMML file with a populated header.
     *
     * (RU) Если файл с таким путём уже существует, бросается
     *      `std::invalid_argument`. В противном случае файл создаётся,
     *      количество объектов (`archivedCount`) и `height_` обнуляются,
     *      `width_` устанавливается равным размеру пакета типов, записывается
     *      заголовок, а `start` — смещением начала данных после заголовка.
     *
     * (EN) If a file with the given path already exists,
     *      `std::invalid_argument` is thrown. Otherwise the file is created,
     *      `archivedCount` and `height_` are zeroed, `width_` is set to the
     *      type-pack size, the header is written and `start` is set to the
     *      data offset immediately after the header.
     */
    void create_file();

    /**
     * @brief (RU) Читает из заголовка количество сохранённых объектов.
     * @brief (EN) Reads the stored object count from the header.
     *
     * (RU) Позиция устанавливается сразу после полей версии, ширины и
     *      высоты; значение записывается в `archivedCount`.
     *
     * (EN) The position is set right after the version, width and height
     *      fields; the value is written into `archivedCount`.
     */
    void read_archive_count();

    /**
     * @brief (RU) Читает из заголовка количество записанных объектов (`height_`).
     * @brief (EN) Reads the number of written objects (`height_`) from the
     *             header.
     *
     * (RU) Позиция устанавливается сразу после полей версии и ширины.
     *
     * (EN) The position is set right after the version and width fields.
     */
    void read_height();




    /**
     * @brief (RU) Формирует текст сообщения об ошибке выхода за границы файла.
     * @brief (EN) Builds the error message for an out-of-range seek.
     *
     * (RU) Возвращает строку с описанием ошибки и путём к текущему файлу.
     *      Используется в связке с макросом `assert_r` для генерации
     *      `std::runtime_error`.
     *
     * (EN) Returns a message string describing the error and the current
     *      file path. Used together with the `assert_r` macro to raise a
     *      `std::runtime_error`.
     *
     * @return (RU) текст сообщения об ошибке;
     *         (EN) the error message text.
     */
    std::string out_range_error() {
        return "Seek to out of range in file \"" + filePath.string() + "\"";
    }


    /**
     * @brief (RU) Читает одно значение типа `T` из потока в формате версии 1.4
     *            (legacy) во временный буфер.
     * @brief (EN) Reads a single value of type `T` from a version 1.4
     *            (legacy) format stream into a temporary buffer.
     *
     * (RU) Считывает идентификатор типа и проверяет его соответствие хэшу
     *      типа `T` (учитывая совместимые legacy-идентификаторы `LONG_INT`
     *      и `UNSIGNED_LONG_INT` для 64-битных целых). При совпадении данные
     *      переписываются в `buff` без идентификатора.
     *
     * (EN) Reads the type identifier and checks it against the hash of type
     *      `T` (accounting for the compatible legacy identifiers
     *      `LONG_INT` and `UNSIGNED_LONG_INT` for 64-bit integers). On match
     *      the data are copied into `buff` without the identifier.
     *
     * @tparam T (RU) тип читаемого значения;
     *           (EN) type of the value being read.
     * @param buff (RU) буфер, в который записываются данные;
     *             (EN) buffer the data are written into.
     * @return (RU) `true`, если тип совпал и данные прочитаны, иначе `false`.
     *         (EN) `true` if the type matched and the data were read,
     *         `false` otherwise.
     * @throws std::runtime_error (RU) при выходе за границы потока.
     *                          (EN) on seeking beyond the stream range.
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
     * @brief (RU) Преобразует одно значение типа `T` из legacy-формата
     *            (версия 1.4) в текущий формат (версия 1.5) в place.
     * @brief (EN) Converts a single value of type `T` from the legacy format
     *            (version 1.4) to the current format (version 1.5) in place.
     *
     * (RU) Считывает идентификатор типа и само значение из текущей позиции
     *      потока, затем записывает значение в позицию `writePos` (в начало
     *      нового формата, без идентификатора) и увеличивает `writePos` на
     *      sizeof(T). Позиция чтения восстанавливается после записи.
     *
     * (EN) Reads the type identifier and the value from the current stream
     *      position, then writes the value at position `writePos` (in the
     *      new format, without an identifier) and advances `writePos` by
     *      sizeof(T). The read position is restored after the write.
     *
     * @tparam T (RU) тип преобразуемого значения;
     *           (EN) type of the value being converted.
     * @param[in,out] writePos (RU) позиция записи в новом формате;
     *                        (EN) write position in the new format.
     * @return (RU) `true`, если тип совпал и значение преобразовано,
     *         иначе `false`.
     *         (EN) `true` if the type matched and the value was converted,
     *         `false` otherwise.
     * @throws std::runtime_error (RU) при неизвестном идентификаторе типа
     *                          или выходе за границы потока.
     *                          (EN) on an unknown type identifier or on
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
     * @brief (RU) Читает одно значение типа `T` из текущей позиции потока
     *            (формат версии 1.5, без идентификатора типа).
     * @brief (EN) Reads a single value of type `T` from the current stream
     *            position (version 1.5 format, no type identifier).
     *
     * (RU) После чтения позиция потока сдвигается вперёд на `sizeof(T)`.
     *
     * (EN) After reading, the stream position is advanced by `sizeof(T)`.
     *
     * @tparam T (RU) тип читаемого значения;
     *           (EN) type of the value being read.
     * @param[out] t (RU) переменная, в которую записывается значение;
     *             (EN) variable the value is written into.
     * @return (RU) `true` при успешном чтении, `false` при выходе за
     *         границы данных.
     *         (EN) `true` on successful read, `false` when reading beyond
     *         the data range.
     */
    template <typename T>
    bool read_one(T& t) {
        auto result = s.read(_p(t));
        s.rel_seek(sizeof(t));
        return result;
    }

    /**
     * @brief (RU) Записывает одно значение типа `T` в текущую позицию потока
     *            (формат версии 1.5, без идентификатора типа).
     * @brief (EN) Writes a single value of type `T` to the current stream
     *            position (version 1.5 format, no type identifier).
     *
     * (RU) Значение записывается побайтово; позиция потока увеличивается на
     *      `sizeof(T)`.
     *
     * (EN) The value is written byte by byte; the stream position advances by
     *      `sizeof(T)`.
     *
     * @tparam T (RU) тип записываемого значения;
     *           (EN) type of the value being written.
     * @param t (RU) значение для записи;
     *          (EN) the value to write.
     */
    template <typename T>
    void write_one(const T& t) {
        s.write(reinterpret_cast<std::byte*>(const_cast<T*>(&t)), sizeof(t));
    }

public:
    /**
     * @brief (RU) Открывает WMML-файл для чтения/записи.
     * @brief (EN) Opens a WMML file for reading/writing.
     *
     * (RU) Публичная точка входа базового класса: создаёт файл и записывает
     *      во внутренний поток его содержимое. Фактическая инициализация
     *      (проверка заголовка, конвертация версий) выполняется в
     *      производном классе `wmml`.
     *
     * (EN) The base class public entry point: creates the file and writes
     *      its contents into the internal stream. Actual initialization
     *      (header validation, version conversion) is performed in the
     *      derived `wmml` class.
     *
     * @param pathToOpenedFile (RU) путь к WMML-файлу;
     *                         (EN) path to the WMML file.
     */
    void open(const std::filesystem::path& pathToOpenedFile);

    /**
     * @brief (RU) Возвращает позицию чтения в начало данных и сбрасывает
     *            счётчик локально прочитанных архивов.
     * @brief (EN) Resets the read position to the beginning of the data and
     *            clears the locally read archive counter.
     *
     * (RU) После вызова можно перечитывать объект с начала файла
     *      (см. `wmml::read`).
     *
     * (EN) After calling, the file can be re-read from the beginning (see
     *      `wmml::read`).
     */
    void reset();

    /**
     * @brief (RU) Сохраняет изменения во внутренней копии в файл.
     * @brief (EN) Persists changes made in the in-memory copy to the file.
     *
     * (RU) Выполняется только если файл открыт и доступен на запись.
     *      Метод не бросает исключений (`noexcept`).
     *
     * (EN) Only runs when the file is open and writable. This method does
     *      not throw (`noexcept`).
     */
    void save() noexcept;
};

/**
 * @brief (RU) Специализация: читает строку формата 1.5 (16-битная длина +
 *            байты).
 * @brief (EN) Specialization: reads a version 1.5 string (16-bit length +
 *            bytes).
 *
 * @param[out] t (RU) строка, в которую записывается результат;
 *             (EN) string the result is written into.
 * @return (RU) `true` при успешном чтении, иначе `false`.
 *         (EN) `true` on successful read, `false` otherwise.
 */
template <> bool base_wmml::read_one<std::string>(std::string& t);

/**
 * @brief (RU) Специализация: записывает строку формата 1.5 (16-битная
 *            длина + байты).
 * @brief (EN) Specialization: writes a version 1.5 string (16-bit length +
 *            bytes).
 *
 * @param t (RU) строка для записи.
 *          (EN) the string to write.
 */
template <> void base_wmml::write_one<std::string>(const std::string& t);

/**
 * @brief (RU) Специализация: читает строку legacy-формата (идентификатор +
 *            8/16/32-битная длина + байты) во временный буфер.
 * @brief (EN) Specialization: reads a legacy-format string (identifier +
 *            8/16/32-bit length + bytes) into a temporary buffer.
 *
 * @param buff (RU) буфер, в который записываются данные (16-битная длина +
 *             байты, без legacy-идентификатора);
 *             (EN) buffer the data are written into (16-bit length + bytes,
 *             without the legacy identifier).
 * @return (RU) `true`, если идентификатор строки распознан и данные
 *         прочитаны, иначе `false`.
 *         (EN) `true` if the string identifier is recognized and the data
 *         were read, `false` otherwise.
 */
template <> bool base_wmml::legacy_read_one<std::string>(details::SmallStream& buff);

/**
 * @brief (RU) Специализация: преобразует строку legacy-формата в формат 1.5
 *            in place.
 * @brief (EN) Specialization: converts a legacy-format string to version 1.5
 *            in place.
 *
 * @param[in,out] writePos (RU) позиция записи в новом формате;
 *                        (EN) write position in the new format.
 * @return (RU) `true`, если идентификатор строки распознан и значение
 *         преобразовано, иначе `false`.
 *         (EN) `true` if the string identifier is recognized and the value
 *         was converted, `false` otherwise.
 */
template <> bool base_wmml::legacy_convert_one<std::string>(std::size_t& writePos);





/**
 * @brief (RU) Шаблонный класс для работы с бинарными WMML-файлами фиксированной
 *            схемы.
 * @brief (EN) Template class for working with fixed-schema binary WMML files.
 *
 * (RU) Набор шаблонных аргументов `Args...` определяет схему объекта
 *      (полевые типы в порядке следования) и фиксирует «ширину» файла.
 *      Класс автоматически открывает существующий файл либо создаёт новый,
 *      при необходимости конвертирует данные из версии формата 1.4 в 1.5
 *      и сохраняет изменения при уничтожении объекта.
 *
 * (EN) The set of template arguments `Args...` defines the object schema
 *      (field types in order) and fixes the file "width". The class
 *      automatically opens an existing file or creates a new one, converts
 *      data from format version 1.4 to 1.5 when needed, and persists changes
 *      on destruction.
 *
 * @tparam Args (RU) типы полей объекта, поддерживаемые WMML (см.
 *            `WmmlFormats`);
 *            (EN) object field types supported by WMML (see `WmmlFormats`).
 *
 * @throws std::runtime_error (RU) при несовпадении ширины/пакета типов файла
 *                          с ожидаемыми, ошибках чтения и конвертации.
 *                          (EN) on mismatch between the file width/type pack
 *                          and the expected ones, on read and conversion
 *                          errors.
 * @throws std::invalid_argument (RU) при попытке создать уже существующий
 *                          файл.
 *                          (EN) when trying to create an already existing
 *                          file.
 *
 * @note (RU) TODO: добавить флаг isValide — при ошибке операции не должна
 *            записываться битая копия; либо lock-guard, перехватывающий флаг
 *            `readOnly` на время записи, чтобы деструктор, вызванный из
 *            исключения, не перезаписал файл.
 * @note (EN) TODO: add an isValide flag — a failed operation must not write a
 *            corrupted copy; or a lock-guard taking over the `readOnly` flag
 *            during writes so that a destructor invoked from an exception
 *            cannot overwrite the file.
 */
template <typename... Args>
class wmml : public base_wmml
{
    /** (RU) Статический пакет хэшей типов схемы объекта;
     *  (EN) Static pack of the object schema's type hashes; */
    inline static constexpr std::array<std::byte, sizeof...(Args)> pack = {
        type_to_hash<Args>()...
    };

    /**
     * @brief (RU) Инициализирует объект: проверяет заголовок, при необходимости
     *            конвертирует legacy-файл (1.4 → 1.5).
     * @brief (EN) Initializes the object: validates the header and, when
     *             needed, converts a legacy file (1.4 → 1.5).
     *
     * (RU) Последовательность действий:
     *      1. ширина файла проверяется на соответствие `width_`;
     *      2. читаются количество архивов и высота;
     *      3. если версия файла старее текущей — выполняется конвертация
     *         `convert_stream_from_1_4_to_1_5_version()` и (если файл доступен
     *         на запись и включена автоконвертация) результат сохраняется;
     *      4. иначе проверяется пакет хэшей типов — при несовпадении
     *         бросается `std::runtime_error`;
     *      5. позиция устанавливается в начало данных (`start`).
     *
     * (EN) Steps:
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
     * @throws std::runtime_error (RU) при несовпадении ширины, несовпадении
     *                          пакета типов или ошибке конвертации.
     *                          (EN) on width mismatch, type-pack mismatch or
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


    /**
     * @brief (RU) Преобразует поток из версии формата 1.4 в версию 1.5.
     * @brief (EN) Converts the stream from format version 1.4 to version 1.5.
     *
     * (RU) Алгоритм:
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
     * (EN) Algorithm:
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
     * @throws std::runtime_error (RU) при несовпадении типов при чтении или
     *                          конвертации, либо выходе за границы потока.
     *                          (EN) on type mismatch during read or
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
     * @brief (RU) Пустой конструктор: создаёт неинициализированный объект без
     *            привязки к файлу.
     * @brief (EN) Default constructor: creates an uninitialized object not
     *             bound to any file.
     *
     * @warning (RU) Объект, созданный таким образом, нельзя использовать
     *              для чтения/записи — файл не открыт.
     * @warning (EN) An object created this way cannot be used for
     *              reading/writing — no file is open.
     */
    wmml() = default;

    /**
     * @brief (RU) Создаёт объект в режиме только для чтения из буфера
     *            WMML-данных (C-строка).
     * @brief (EN) Creates a read-only object from a buffer of WMML data
     *             (C-string).
     *
     * (RU) Содержимое `file_stream` копируется во внутренний поток, после
     *      чего выполняется `init()`. Файл на диске не используется
     *      (`filePath` — пустая строка).
     *
     * (EN) The contents of `file_stream` are copied into the internal stream,
     *      then `init()` is performed. No on-disk file is used
     *      (`filePath` is an empty string).
     *
     * @param file_stream (RU) буфер с байтами WMML (включая заголовок);
     *                   (EN) buffer with WMML bytes (including the header).
     * @throws std::runtime_error (RU) при несовпадении заголовка с ожидаемой
     *                          схемой или ошибке конвертации.
     *                          (EN) on header mismatch with the expected
     *                          schema or on conversion error.
     */
    wmml (const char* file_stream) : base_wmml(pack.data(), sizeof(pack), true, "") {
        std::size_t streamSize = strlen(file_stream);
        s.open(streamSize);
        s.write(reinterpret_cast<std::byte*>(const_cast<char*>(file_stream)), streamSize);
        init();
    }

    /**
     * @brief (RU) Создаёт объект, привязанный к WMML-файлу на диске.
     * @brief (EN) Creates an object bound to an on-disk WMML file.
     *
     * (RU) Если файл существует, он открывается, загружается во внутренний
     *      поток и инициализируется. Если файл не существует — создаётся
     *      новый пустой файл с заполненным заголовком. Объект доступен на
     *      чтение и запись.
     *
     * (EN) If the file exists, it is opened, loaded into the internal stream
     *      and initialized. If it does not exist, a new empty file with a
     *      populated header is created. The object is readable and writable.
     *
     * @param pathToOpenedFile (RU) путь к WMML-файлу;
     *                         (EN) path to the WMML file.
     * @throws std::runtime_error (RU) при ошибке открытия, несовпадении
     *                          заголовка или ошибке конвертации.
     *                          (EN) on open failure, header mismatch or
     *                          conversion error.
     * @throws std::invalid_argument (RU) если при создании файла файл с таким
     *                          путём уже существует.
     *                          (EN) if, when creating the file, a file with
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
     * @brief (RU) Деструктор: сохраняет все несохранённые изменения в файл.
     * @brief (EN) Destructor: saves all unsaved changes to the file.
     *
     * (RU) Вызывает `save()` — изменения пишутся только если файл открыт и
     *      доступен на запись.
     *
     * (EN) Calls `save()` — changes are written only if the file is open and
     *      writable.
     */
    ~wmml () {
        save();
    }

    /**
     * @brief (RU) Читает один объект (набор полей схемы) из текущей позиции.
     * @brief (EN) Reads one object (a full set of schema fields) from the
     *             current position.
     *
     * (RU) Аргументы должны передаваться в том же порядке и с теми же
     *      типами, что и шаблонные аргументы класса. Если позиция находится
     *      в заголовке, она возвращается в начало данных (`start`).
     *
     * (EN) The arguments must be passed in the same order and with the same
     *      types as the class template arguments. If the position is in the
     *      header, it is moved back to the start of the data (`start`).
     *
     * @tparam (RU) типы берутся из `Args...`.
     * @param args (RU) переменные, в которые записываются поля объекта;
     *             (EN) variables the object fields are written into.
     * @return (RU) `true`, если все поля прочитаны успешно, иначе `false`.
     *         (EN) `true` if all fields were read successfully, `false`
     *         otherwise.
     * @throws std::runtime_error (RU) при выходе позиции за границы файла.
     *                          (EN) if the position seeks beyond the file
     *                          range.
     */
    bool read(Args&... args) {
        if (s.get_pos() < start)
            assert_r(s.seek(start), out_range_error());
        return (read_one(args) && ...);
    }

    /**
     * @brief (RU) Добавляет один объект (набор полей схемы) в конец файла.
     * @brief (EN) Appends one object (a full set of schema fields) to the end
     *             of the file.
     *
     * (RU) Позиция устанавливается в конец потока, затем все аргументы
     *      записываются последовательно; счётчик объектов `height_`
     *      увеличивается на 1. Аргументы должны соответствовать схеме
     *      `Args...` по типам и порядку.
     *
     * (EN) The position is set to the end of the stream, then all arguments
     *      are written in order; the object counter `height_` is incremented
     *      by 1. The arguments must match the `Args...` schema by type and
     *      order.
     *
     * @param args (RU) значения полей нового объекта;
     *             (EN) the field values of the new object.
     * @throws std::runtime_error (RU) при выходе позиции за границы файла.
     *                          (EN) if the position seeks beyond the file
     *                          range.
     */
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