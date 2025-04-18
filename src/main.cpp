#include <iostream>

#include "wmml.h"

int main()
{
    try {
// #if 0
        wmml* lib = new wmml("file.wmml", 5);
        std::vector<wmml::variant> v(lib->width());

        signed int a        = 2090345;
        int b               = 123;
        float c             = 0.5;
        long long int d     = 50095958585585;
        std::string e       = "dsagfaf";
        v[0] = a;
        v[1] = b;
        v[2] = c;
        v[3] = d;
        v[4] = e;

        lib->write(v);
        delete lib;
// #endif
#if 0
        wmml readed("file.wmml");
        std::vector<wmml::variant> r(readed.width());
        int tst = 0;
        while (readed.read(r)) {
            std::cout << "tst: " << tst << std::endl;
            // for (auto& entry : r) {
            //     std::visit([&](auto& parameter) {
            //         use(parameter);
            //     }, entry);
            // }
        }
#endif
    } catch (const char *error_code) {
        std::cerr << error_code << std::endl;
    }
    return 0;
}
