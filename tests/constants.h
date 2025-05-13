#include <wmml.h>

// File 1: width 3, height 2
const std::vector<std::vector<wmml::variant>> FILE_1 = {
    {
        int(42), 
        std::string("hello"), 
        bool(true)
    },
    {
        unsigned(2538652438), 
        double(3.1415), 
        wchar_t(L'A')
    }
};

// File 2: width 5, height 2
const std::vector<std::vector<wmml::variant>> FILE_2 = {
    {
        long(1000000000000000), 
        float(-5.5), 
        int32_t(1744671537), 
        char(132), 
        std::string("")
    },
    {
        long(1003867580000), 
        float(-7.4), 
        int(1849354634), 
        char(48), 
        std::string("kshfkjgbh3nfdjkvbdfskjgfbns3fdjvbfdshfvdsjkfghsdklhrfvjdhvbgbdsjfdjsgkjsfghdskjghfkjdshgkjdfkghdskjgfhdjsghdjkfghjdskdjfhgjjdsfghsfdgkhsdjfgjdfsjghdfdjgksfjhsjdjghsfdjkghdsjfgkdfg")
    }
};

// File 3: width 2, height 3
const std::vector<std::vector<wmml::variant>> FILE_3 = {
    {bool(false),           short(32767)},
    {double(123348.456),    int16_t(65531)},
    {char('x'),             char(255)}
};

// File 4: width 4, height 2
const std::vector<std::vector<wmml::variant>> FILE_4 = {
    {
        int(-2147483648), 
        int(4294967295), 
        int64_t(-9223372036854775807), 
        std::string("special\n\tchars")
    },
    {
        bool(true), 
        double(3487543.563), 
        wchar_t(0), 
        int32_t(728684032832848275)
    }
};

// File 5: width 1, height 4 (минимальная width)
const std::vector<std::vector<wmml::variant>> FILE_5 = {
    {std::string("single")},
    {long(0.896)},
    {float(46.5)},
    {bool(false)}
};

// File 6: width 3, height 5 (смешанные типы и повторяющиеся значения)
const std::vector<std::vector<wmml::variant>> FILE_6 = {
    {int(0),            std::string("repeat"),      bool(true)},
    {unsigned(05654),   std::string("repeat"),      long(-999999)},
    {double(0.0),       std::string("different"),   wchar_t(L'Ω')},
    {short(-32768),     float(99.99),               int64_t(1234567890)},
    {char('\n'),        char(-128),                 std::string("escape\nchars")}
};

// File 7: width 4, height 6 (большие наборы с экстремальными значениями)
const std::vector<std::vector<wmml::variant>> FILE_7 = {
    {
        int(2147483647),
        int32_t(18446741145615),
        double(-43545.5454),
        std::string("max values")
    },
    {
        bool(false),
        float(0.000001),
        int64_t(9223372036854775807),
        wchar_t(65535)
    },
    {
        (unsigned short)(65535),
        double(123456789.123456789),
        char(127),
        std::string("!@#$%^&*()")
    },
    {
        (unsigned char)255,
        int(-2147483648),
        float(45414536356.34),
        bool(true)
    },
    {
        std::string("unicode: привет"),
        short(0),
        (unsigned int)(0xFFFFFFFF),
        double(3.141592653589793)
    },
    {
        char('\t'),
        long(046555414),
        (unsigned long long)(053345634634561),
        std::string("end of data")
    }
};

// File 8: width 2, height 7 (разнообразные комбинации типов)
const std::vector<std::vector<wmml::variant>> FILE_8 = {
    {bool(true),                    int(100)},
    {double(2.71828),               std::string("euler")},
    {wchar_t(L'ß'),                 unsigned(42000)},
    {float(-273.15),                (long long)(4241242142)},
    {std::string("null\0byte", 9),  (unsigned char)(0)},
    {short(32767),                  (signed char)(-1)},
    {(unsigned short)(65535),       (long double)(6.02214076e23)}
};