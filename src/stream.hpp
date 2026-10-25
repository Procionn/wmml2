#pragma once

#ifdef DYNAMIC_MEMORY_ONLY
    #define STREAM_SIZE 0
#endif
#ifndef STREAM_SIZE
    #ifdef _WIN64
        #define STREAM_SIZE (32 * 1024)
    #else
        #define STREAM_SIZE (64 * 1024)
    #endif
#endif

#include <array>
#include <cstdint>

class stream
{
    std::array<std::byte, STREAM_SIZE> stack;
    std::byte* lot = nullptr;

    std::size_t size = 0;
    std::size_t headPos = 0;
    std::size_t allocated_memory = 0;

private:
    void realloc();
    void alloc ();
    std::byte* get_buff();
    void alloc_expanding();

public:
    stream () {}
    stream (const std::size_t& allocSize);
    stream (const char* src, const std::size_t& allocSize);
    ~stream ();

    std::size_t get_pos() { return headPos; }
    std::size_t get_size() { return size; }

    void open (const std::size_t& allocSize);
    bool read (std::byte* s, const std::size_t count);
    void write (std::byte* s, const std::size_t count); // записывает объект, начиная с headPos, смещая указатель на конец новых данных
    bool seek (const uint64_t pos);
    void rel_seek (const int64_t shift);
    std::byte* get_ptr (const std::size_t count);
    void move(const std::size_t from, const std::size_t to);
};