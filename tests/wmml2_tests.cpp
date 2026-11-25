#include "../src/wmml2.hpp"
#include <string>
#include <iostream>
#include <filesystem>
using namespace std::literals;

int main () {
    // Пересоздаём core.wmml из неизменяемой копии, чтобы каждый запуск
    // начинался со «свежего» файла в старом (1.4) формате
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
    wmml<std::string, std::string, bool, uint64_t, bool, signed char> file("core.wmml"s);
    std::string a, b;
    bool c, e;
    uint64_t d;
    int8_t f;
    while(file.read(a, b, c, d, e, f))
        std::cout << a << " | " << b  << " | " << c << " | " << d << " | " << e << " | " << int(f) << std::endl;
}