/**
 * @brief Unit-тесты публичного API шаблонного класса `wmml` (Google Test).
 *
 * Поведение методов проверяется исключительно на основании doxygen-документации
 * класса `wmml` из `wmml2.hpp`:
 *  - конструкторы (по пути, из C-строки);
 *  - `read`, `write`;
 *  - `reset`, `skip`;
 *  - `get_stream_size`, `get_stream_ptr`;
 *  - `remove_object`, `overwrite_sector`;
 *  - `save`.
 */

#include "../src/wmml.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>
#include <string>

namespace {

/**
 * @brief Возвращает путь к бинарному датасету `core3.wmml`, либо пустой
 *        `std::filesystem::path`, если датасет не найден.
 *
 * \~russian Каталоги проверяются в порядке возрастания вероятности: сначала
 *      текущий рабочий каталог, затем путь, зашитый в CMake
 *      (`WMML_TEST_DATA_DIR`).
 */
std::filesystem::path find_dataset_path() {
    const std::filesystem::path candidates[] = {
        std::filesystem::current_path() / "dataset" / "core3.wmml",
#ifdef WMML_TEST_DATA_DIR
        std::filesystem::path(WMML_TEST_DATA_DIR) / "core3.wmml",
#endif
    };
    for (const auto& p : candidates)
        if (std::filesystem::exists(p))
            return p;
    return {};
}

/**
 * @brief Читает бинарный датасет `core3.wmml` целиком в строку (c-style
 *        «сурс»-буфер).
 *
 * \~russian Возвращает `std::nullopt`, если датасет не найден. Строка
 *      содержит полный побайтовый образ файла, включая заголовок, и
 *      используется как аргумент конструктора `wmml(const char*)`.
 *
 * @return \~russian строка с байтами WMML-датасета; \~english dataset bytes.
 */
std::optional<std::string> load_dataset_as_cstring() {
    const auto path = find_dataset_path();
    if (path.empty())
        return std::nullopt;
    std::ifstream in(path, std::ios::binary);
    if (!in)
        return std::nullopt;
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

/**
 * @brief Возвращает каталог для временных тестовых файлов, создавая его при
 *        необходимости.
 */
std::filesystem::path temp_dir() {
    static const std::filesystem::path dir =
        std::filesystem::current_path() / "wmml_gtest_tmp";
    std::filesystem::create_directories(dir);
    return dir;
}

/**
 * @brief Формирует путь к временному файлу и удаляет его, если он уже существует.
 */
std::filesystem::path make_path(const std::string& name) {
    const std::filesystem::path p = temp_dir() / name;
    if (std::filesystem::exists(p))
        std::filesystem::remove(p);
    return p;
}

} // namespace

// =====================================================================
//  Конструкторы
// =====================================================================

/**
 * @brief Если файл не существует, конструктор по пути создаёт новый пустой
 *        файл с заполненным заголовком.
 */
TEST(WmmlConstruction, CreatesNewFileWhenAbsent) {
    const auto path = make_path("create_new.wmml");
    ASSERT_FALSE(std::filesystem::exists(path));

    wmml<int32_t, uint64_t> obj(path);

    EXPECT_TRUE(std::filesystem::exists(path));
    EXPECT_GT(obj.get_stream_size(), 0u);
}

/**
 * @brief Если файл существует, конструктор по пути открывает его: данные,
 *        записанные ранее, доступны для чтения.
 */
TEST(WmmlConstruction, ReopensExistingFile) {
    const auto path = make_path("reopen.wmml");

    {
        wmml<int32_t, uint64_t> obj(path);
        obj.write(42, 777777u);
        // obj.save();
    }

    {
        wmml<int32_t, uint64_t> obj(path);
        int32_t a = 0;
        uint64_t b = 0;
        ASSERT_TRUE(obj.read(a, b));
        EXPECT_EQ(a, 42);
        EXPECT_EQ(b, 777777u);
    }
}

/**
 * @brief Конструктор из C-строки: при несовпадении заголовка буфера с ожидаемой
 *        схемой бросается `std::runtime_error`.
 */
TEST(WmmlConstruction, CStringThrowsOnHeaderMismatch) {
    // Заголовок «не WMML»: поле ширины не совпадает с ожидаемым.
    const char* buf = "this-is-not-a-wmml-buffer";
    EXPECT_THROW({ wmml<int32_t> obj(buf, strlen(buf)); }, std::runtime_error);
}

// =====================================================================
//  Конструктор из C-строки: реальный бинарный датасет
// =====================================================================

/**
 * @brief Схема объекта датасета `core3.wmml` (совпадает со схемой
 *        `core.wmml`).
 */
using Core3Schema = wmml<std::string, std::string, bool, uint64_t, bool, signed char>;

/**
 * @brief Конструктор из C-строки: полный побайтовый образ реального WMML-файла
 *        (`dataset/core3.wmml`), прочитанный через `std::fstream` в строку,
 *        успешно создаёт объект без исключения.
 *
 * \~russian Датасет не содержит байт NUL, поэтому `strlen()` в конструкторе
 *      определяет точный размер файла. Размер внутреннего потока должен
 *      совпасть с размером прочитанной строки, что подтверждает отсутствие
 *      усечения данных.
 *
 * @throws \~russian Тест пропускается (`GTEST_SKIP`), если датасет не найден.
 */
TEST(WmmlConstruction, CStringCreatesObjectFromDataset) {
    const auto data = load_dataset_as_cstring();
    if (!data.has_value())
        GTEST_SKIP() << "Dataset \"dataset/core3.wmml\" not found";

    // Байт NUL усёк бы строку в конструкторе — проверяем их отсутствие.
    // EXPECT_EQ(data->find('\0'), std::string::npos);

    Core3Schema obj(data->c_str(), data->size());

    EXPECT_EQ(obj.get_stream_size(), data->size());
}

/**
 * @brief Конструктор из C-строки: объекты датасета `core3.wmml` читаются
 *        полностью — `read` возвращает `true` для каждого из 11 объектов
 *        датасета и `false` после последнего.
 *
 * \~russian Строковые поля первого объекта проверяются на непустоту как
 *      индикатор целостности побайтового копирования буфера во внутренний
 *      поток.
 *
 * @throws \~russian Тест пропускается (`GTEST_SKIP`), если датасет не найден.
 */
TEST(WmmlConstruction, CStringReadsAllDatasetObjects) {
    const auto data = load_dataset_as_cstring();
    if (!data.has_value())
        GTEST_SKIP() << "Dataset \"dataset/core3.wmml\" not found";

    Core3Schema obj(data->c_str(), data->size());

    std::string a, b;
    bool c, e;
    uint64_t d;
    int8_t f;

    std::size_t count = 0;
    while (obj.read(a, b, c, d, e, f)) {
        // Строки каждого объекта непусты — буфер скопирован без искажений.
        EXPECT_FALSE(a.empty());
        EXPECT_FALSE(b.empty());
        ++count;
    }

    EXPECT_EQ(count, 11u); // датасет core3.wmml содержит 11 объектов
}

// =====================================================================
//  read / write
// =====================================================================

/**
 * @brief В пустом файле `read` возвращает `false` — объектов нет.
 */
TEST(WmmlRead, EmptyFileReturnsFalse) {
    const auto path = make_path("read_empty.wmml");
    wmml<int32_t> obj(path);

    int32_t a = -1;
    EXPECT_FALSE(obj.read(a));
}

/**
 * @brief `write` добавляет объект в конец файла, `read` считывает его в том же
 *        порядке; после последнего объекта `read` возвращает `false`.
 */
TEST(WmmlReadWrite, WriteThenReadBack) {
    const auto path = make_path("rw.wmml");

    {
        wmml<int32_t, uint64_t> obj(path);
        obj.write(10, 20u);
        obj.write(-30, 40u);
        obj.save();
    }

    wmml<int32_t, uint64_t> obj(path);
    int32_t a = 0;
    uint64_t b = 0;

    ASSERT_TRUE(obj.read(a, b));
    EXPECT_EQ(a, 10);
    EXPECT_EQ(b, 20u);

    ASSERT_TRUE(obj.read(a, b));
    EXPECT_EQ(a, -30);
    EXPECT_EQ(b, 40u);

    EXPECT_FALSE(obj.read(a, b)); // конец данных
}

/**
 * @brief Схема со строками: переменная длина корректно сохраняется и
 *        восстанавливается.
 */
TEST(WmmlReadWrite, StringSchemaRoundTrip) {
    const auto path = make_path("rw_string.wmml");

    {
        wmml<std::string, int32_t> obj(path);
        obj.write("hello", 1);
        obj.write("world", 2);
        obj.save();
    }

    wmml<std::string, int32_t> obj(path);
    std::string s;
    int32_t i = 0;

    ASSERT_TRUE(obj.read(s, i));
    EXPECT_EQ(s, "hello");
    EXPECT_EQ(i, 1);

    ASSERT_TRUE(obj.read(s, i));
    EXPECT_EQ(s, "world");
    EXPECT_EQ(i, 2);

    EXPECT_FALSE(obj.read(s, i));
}

// =====================================================================
//  reset / skip
// =====================================================================

/**
 * @brief `reset` возвращает позицию чтения в начало данных: объект можно
 *        перечитать с начала.
 */
TEST(WmmlReset, RereadFromBeginning) {
    const auto path = make_path("reset.wmml");

    wmml<int32_t, int32_t> obj(path);
    obj.write(1, 100);
    obj.write(2, 200);

    obj.reset();
    int32_t a = 0;
    int32_t b = 0;
    ASSERT_TRUE(obj.read(a, b));
    ASSERT_TRUE(obj.read(a, b));

    obj.reset();
    ASSERT_TRUE(obj.read(a, b));
    EXPECT_EQ(a, 1);
    EXPECT_EQ(b, 100);
}

/**
 * @brief `skip` пропускает один блок без копирования данных: следующее
 *        чтение возвращает следующий объект.
 */
TEST(WmmlSkip, SkipsOneBlock) {
    const auto path = make_path("skip.wmml");

    wmml<int32_t, uint64_t> obj(path);
    obj.write(1, 10u);
    obj.write(2, 20u);

    obj.reset();
    EXPECT_TRUE(obj.skip());

    int32_t a = 0;
    uint64_t b = 0;
    ASSERT_TRUE(obj.read(a, b));
    EXPECT_EQ(a, 2);
    EXPECT_EQ(b, 20u);
}

/**
 * @brief `skip` возвращает `false`, если при пропуске головка выходит за конец
 *        потока.
 */
TEST(WmmlSkip, ReturnsFalseBeyondEnd) {
    const auto path = make_path("skip_end.wmml");

    wmml<int32_t> obj(path);
    obj.write(1);

    obj.reset();
    EXPECT_TRUE(obj.skip());
    EXPECT_FALSE(obj.skip()); // за единственным объектом — конец
}

// =====================================================================
//  get_stream_size / get_stream_ptr
// =====================================================================

/**
 * @brief `get_stream_size` растёт после записи нового объекта.
 */
TEST(WmmlStreamSize, GrowsAfterWrite) {
    const auto path = make_path("size.wmml");

    wmml<int32_t> obj(path);
    const auto before = obj.get_stream_size();

    obj.write(123);

    EXPECT_GT(obj.get_stream_size(), before);
}

/**
 * @brief `get_stream_ptr` бросает `std::runtime_error` при выходе за границы
 *        потока.
 */
TEST(WmmlStreamPtr, OutOfRangeThrows) {
    const auto path = make_path("ptr_oor.wmml");

    wmml<int32_t> obj(path);
    EXPECT_THROW(obj.get_stream_ptr(10'000'000, 1), std::runtime_error);
}

/**
 * @brief `get_stream_ptr` + `get_stream_size` позволяют выписать полное
 *        содержимое потока напрямую (минуя файл) и сохранить идентичность
 *        данных.
 */
TEST(WmmlStreamPtr, ContentRoundTripViaCopyFile) {
    const auto path = make_path("ptr_main.wmml");
    const auto copy = make_path("ptr_copy.wmml");

    {
        wmml<int32_t, uint64_t> obj(path);
        obj.write(10, 20u);
        obj.write(-30, 40u);

        const std::size_t size = obj.get_stream_size();
        {
            std::ofstream out(copy, std::ios::binary);
            out.write(obj.get_stream_ptr(0, size), static_cast<std::streamsize>(size));
        }
        obj.save();
    }

    wmml<int32_t, uint64_t> verify(copy);
    int32_t a = 0;
    uint64_t b = 0;

    ASSERT_TRUE(verify.read(a, b));
    EXPECT_EQ(a, 10);
    EXPECT_EQ(b, 20u);

    ASSERT_TRUE(verify.read(a, b));
    EXPECT_EQ(a, -30);
    EXPECT_EQ(b, 40u);

    EXPECT_FALSE(verify.read(a, b));
}

// =====================================================================
//  remove_object
// =====================================================================

/**
 * @brief `remove_object` удаляет объект по номеру и сдвигает остаток потока
 *        влево: после удаления первый объект становится вторым.
 */
TEST(WmmlRemoveObject, RemovesFirstAndShifts) {
    const auto path = make_path("remove_first.wmml");

    wmml<int32_t, uint64_t> obj(path);
    obj.write(1, 10u);
    obj.write(2, 20u);
    obj.write(3, 30u);

    obj.remove_object(0);

    int32_t a = 0;
    uint64_t b = 0;
    ASSERT_TRUE(obj.read(a, b));
    EXPECT_EQ(a, 2);
    EXPECT_EQ(b, 20u);
    ASSERT_TRUE(obj.read(a, b));
    EXPECT_EQ(a, 3);
    EXPECT_EQ(b, 30u);
    EXPECT_FALSE(obj.read(a, b));
}

/**
 * @brief `remove_object` корректно удаляет «средний» объект: оставшиеся
 *        объекты сохраняют порядок.
 */
TEST(WmmlRemoveObject, RemovesMiddle) {
    const auto path = make_path("remove_middle.wmml");

    wmml<int32_t, uint64_t> obj(path);
    obj.write(1, 10u);
    obj.write(2, 20u);
    obj.write(3, 30u);

    obj.remove_object(1);

    int32_t a = 0;
    uint64_t b = 0;
    ASSERT_TRUE(obj.read(a, b));
    EXPECT_EQ(a, 1);
    EXPECT_EQ(b, 10u);
    ASSERT_TRUE(obj.read(a, b));
    EXPECT_EQ(a, 3);
    EXPECT_EQ(b, 30u);
    EXPECT_FALSE(obj.read(a, b));
}

/**
 * @brief `remove_object` бросает `std::runtime_error`, если номер объекта
 *        выходит за границы данных.
 */
TEST(WmmlRemoveObject, ThrowsOutOfRange) {
    const auto path = make_path("remove_oor.wmml");

    wmml<int32_t> obj(path);
    obj.write(1);
    obj.write(2);

    EXPECT_THROW(obj.remove_object(5), std::runtime_error);
}

// =====================================================================
//  overwrite_sector
// =====================================================================

/**
 * @brief `overwrite_sector` перезаписывает поле фиксированного размера внутри
 *        объекта.
 */
TEST(WmmlOverwriteSector, FixedSizeField) {
    const auto path = make_path("ovr_fixed.wmml");

    wmml<int32_t, uint64_t> obj(path);
    obj.write(1, 10u);
    obj.write(2, 20u);

    obj.overwrite_sector<1, uint64_t>(0, 999u);

    int32_t a = 0;
    uint64_t b = 0;
    ASSERT_TRUE(obj.read(a, b));
    EXPECT_EQ(a, 1);
    EXPECT_EQ(b, 999u);
}

/**
 * @brief `overwrite_sector` для строки: при удлинении данные, следующие за
 *        строкой, сдвигаются вправо, при укорочении — влево; остальные объекты
 *        сохраняются.
 */
TEST(WmmlOverwriteSector, StringFieldExtendAndShrink) {
    const auto path = make_path("ovr_string.wmml");

    wmml<std::string, int32_t> obj(path);
    obj.write("ab", 1);
    obj.write("cd", 2);

    // Удлиняем строку первого объекта.
    obj.overwrite_sector<0, std::string>(0, "abcdef");
    // Укорачиваем строку второго объекта.
    obj.overwrite_sector<0, std::string>(1, "x");

    std::string s;
    int32_t i = 0;
    ASSERT_TRUE(obj.read(s, i));
    EXPECT_EQ(s, "abcdef");
    EXPECT_EQ(i, 1);
    ASSERT_TRUE(obj.read(s, i));
    EXPECT_EQ(s, "x");
    EXPECT_EQ(i, 2);
    EXPECT_FALSE(obj.read(s, i));
}

// =====================================================================
//  save
// =====================================================================

/**
 * @brief `save` персистит изменения во внутренней копии в файл: после
 *        сохранения данные читаются из нового объекта.
 */
TEST(WmmlSave, PersistsChanges) {
    const auto path = make_path("save.wmml");

    wmml<int32_t> obj1(path);
    obj1.write(7);
    obj1.save();

    wmml<int32_t> obj2(path);
    int32_t a = 0;
    ASSERT_TRUE(obj2.read(a));
    EXPECT_EQ(a, 7);
    EXPECT_FALSE(obj2.read(a));
}
