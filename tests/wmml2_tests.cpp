#include "../src/wmml2.hpp"
#include <string>
#include <iostream>
#include <filesystem>
using namespace std::literals;

int main () {
#if 0
    namespace fs = std::filesystem;
    if (fs::exists("core_original.wmml"))
        fs::copy_file("core_original.wmml", "core.wmml",
                      fs::copy_options::overwrite_existing);
    /*
    wmml<int8_t, uint64_t, int> file("1.wmml"s);
    file.write(64, 9185156315636120321, -100); // TODO добавить возможноть сброса указателя на начало потока
    file.write(-30, 14591856120321, -15800);
    int8_t a, a1 = 1;
    uint64_t b, b1 = 2;
    int c, c1 = 3;
    file.reset();
    while(file.read(a, b, c))
        std::cout << int(a)  << " " << b  << " " << c  << std::endl;
    */
    // wmml<std::string, int> file("2.wmml"s);
    // file.write("Hello", 10);
    // file.reset();
    // std::string s;
    // int i;
    // file.read(s, i);
    // std::cout << s << std::endl;
    // std::string | std::string | bool | unsigned long int | bool | signed char |
    /*
    ===== Тест новых методов: get_stream_ptr / skip / remove_object / overwrite_sector =====
    */
    {
        const fs::path tmp("tmp_new_api.wmml");
        const fs::path tmp2("tmp_new_api_copy.wmml");
        if (fs::exists(tmp))
            fs::remove(tmp);
        if (fs::exists(tmp2))
            fs::remove(tmp2);

        wmml<std::string, int, uint64_t> test(tmp);
        test.write("hello", 1, 42);
        test.write("world", 2, 43);
        test.write("final", 3, 44);

        std::string a;
        int c;
        uint64_t d;

        // skip: пропускаем первый блок — читаем второй
        test.reset();
        test.skip();
        test.read(a, c, d);
        std::cout << "skip:        " << a << " " << c << " " << d << std::endl; // world 2 43

        // overwrite_sector: удлиняем строку и меняем числовое поле объекта №1
        // (полное позиционирование: головка не важна, после операции — в start)
        test.overwrite_sector<0, std::string>(1, "worldly");
        test.overwrite_sector<1, int>(1, 99);
        test.skip();
        test.read(a, c, d);
        std::cout << "overwrite:   " << a << " " << c << " " << d << std::endl; // worldly 99 43

        // overwrite_sector: укорачиваем строку
        test.overwrite_sector<0, std::string>(1, "wd");
        test.skip();
        test.read(a, c, d);
        std::cout << "overwrite:   " << a << " " << c << " " << d << std::endl; // wd 99 43

        // remove_object: удаляем объект №0
        test.remove_object(0);
        test.read(a, c, d);
        std::cout << "remove:      " << a << " " << c << " " << d << std::endl; // wd 99 43

        test.save();

        // get_stream_ptr: выписываем поток напрямую в отдельный файл
        // (минуя write_stream_to_file) и проверяем идентичность содержимого
        {
            std::ofstream out(tmp2, std::ios::binary);
            const std::size_t size = test.get_stream_size();
            out.write(test.get_stream_ptr(0, size), size);
        }
        wmml<std::string, int, uint64_t> verify(tmp2);
        verify.reset();
        while (verify.read(a, c, d))
            std::cout << "verify:      " << a << " " << c << " " << d << std::endl; // wd 99 43 / final 3 44
    }




    wmml<std::string, std::string, bool, uint64_t, bool, signed char> file("core.wmml"s);
    std::string a, b;
    bool c, e;
    uint64_t d;
    int8_t f;
    while(file.read(a, b, c, d, e, f))
        std::cout << a << " | " << b  << " | " << c << " | " << d << " | " << e << " | " << int(f) << std::endl;
#endif
}