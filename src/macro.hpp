#pragma once
#include <stdexcept>


#include <iostream>



#define _p(name) reinterpret_cast<std::byte*>(&name), sizeof(name)

#define assert_r(expression, error) if (!(expression)) throw std::runtime_error(error) // Runtime assert
// #define assert_r(expression, error) if (!(expression)) std::cerr << error << std::endl; // Runtime assert