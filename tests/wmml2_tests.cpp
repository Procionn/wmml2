#include "../src/wmml2.hpp"
#include <string>
#include <iostream>
using namespace std::literals;

int main () {
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
    wmml<std::string, int> file("2.wmml"s);
    file.write("Hello", 10);
    file.reset();
    std::string s;
    int i;
    file.read(s, i);
    std::cout << s << std::endl;
}