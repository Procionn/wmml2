#include <iostream>

#include "wmml.h"

int main()
{
    try {

#define MODE (0)
#if MODE
        // wmml* lib = new wmml("file.wmml", 5);
        wmml* lib = new wmml("file.wmml");
        // print(lib->width());
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

        // signed int a        = 436454743;
        // float b             = 12.3;
        // unsigned short c    = 55405;
        // long long int d     = 5654654654164585;
        // v[0] = a;
        // v[1] = b;
        // v[2] = c;
        // v[3] = d;


        lib->write(v);
        delete lib;
#elif 1
        // wmml* lb = new wmml("file.wmml");
        // lb->remove_object(2);
        // int t = 213509;
        // lb->overwriting_sector(1, 1, t);
        // wmml_marker* nwm = new wmml_marker("file2.wmml");

        // lb->set_wmml(nwm);
        // delete lb;

        wmml readed("file.wmml");
        std::vector<wmml::variant> r(readed.width());
        int tst = 0;
        while (readed.read(r)) {
            std::cout << "\n" << "reader cycle: " << tst << "\n" << std::endl;
            for (auto& entry : r) {
                std::visit([&](auto& parameter) {
                    use(parameter);
                }, entry);
            }
            ++tst;
        }
        std::filesystem::remove("unarchived.wmml");
        auto archived = readed.get_wmml();
        std::vector<wmml::variant> v(archived->width());
        // archived->read(v);
        // for (auto& entry : r) {
        //     std::visit([&](auto& parameter) {
        //         use(parameter);
        //     }, entry);
        // }
        // archived->unarchiving("unarchived.wmml");
        delete archived;
#endif
    } catch (const char *error_code) {
        std::cerr << error_code << std::endl;
    }
    return 0;
}
