#include <iostream>
#include "../src/stream.hpp"
#include <cstring>

using namespace details;


const char* data1 = "abcdefghijklmnopqrstuvwxyz123456";
const char* data2 = "abcdefghijklmnopqrstuvwxyz1234567890";

std::string to_str(std::byte* src, int size) {
    std::string str;
    for (int i = 0; i<size; ++i) {
        str.push_back(char(src[i]));
    }
    return str;
}

int main() {
    stream s1(data2, strlen(data2));
    stream s2(data2, strlen(data2));

    s1.seek(36);
    s1.rel_seek(-10);
    s1.rel_seek(4);
    std::byte arr[5];
    s1.read(arr, sizeof(arr));
    std::cout << to_str(arr, sizeof(arr)) << std::endl;

    s2.seek(26);
    std::byte arr2[7];
    s2.read(arr2, sizeof(arr2));
    std::cout << to_str(arr2, sizeof(arr2)) << std::endl;
    std::byte arr3[4] = {std::byte('0'), std::byte('1'), std::byte('1'), std::byte('0')};
    s2.write(arr3, 4);
    std::byte arr4[4];
    s2.rel_seek(-4);
    s2.read(arr4, 4);
    std::cout << to_str(arr4, sizeof(arr4)) << std::endl;

    stream s4(data1, strlen(data1));
    s4.seek(30);
    std::byte arr6[5] = {std::byte('2'), std::byte('1'), std::byte('1'), std::byte('3'), std::byte('5')};
    s4.write(arr6, sizeof(arr6));
    std::byte arr7[33];
    // std::cout << s4.get_pos() << std::endl;
    s4.seek(0);
    s4.read(arr7, 33);
    std::cout << to_str(arr7, sizeof(arr7)) << std::endl;



    stream s3(20);
    std::byte arr5[20];
    std::string tdata = "abcdefghijklmnopqrst";
    std::memcpy(arr5, tdata.data(), 20);

    s3.write(arr5, 20);
    s3.seek(0);
    std::byte arr8[20];
    s3.read(arr8, 20);
    std::cout << to_str(arr5, sizeof(arr5)) << std::endl;
    std::cout << to_str(arr8, sizeof(arr8)) << std::endl;
    char* test = static_cast<char*>(static_cast<void*>(arr8));
    std::cout << test << std::endl;


    stream s5(data2, strlen(data2));
    s5.seek(29);
    std::byte* ptr = s5.get_ptr(7);
    if (ptr)
        std::cout << to_str(ptr, 7) << std::endl;
    else {
        std::cout << "Null" << std::endl;
    }
}
