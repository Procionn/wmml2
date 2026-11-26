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

namespace details {

template<std::size_t stackSize>
class BaseStream
{
    std::array<std::byte, stackSize> stack;
    std::byte* lot = nullptr;

    std::size_t size = 0;
    std::size_t headPos = 0;
    std::size_t allocated_memory = 0;

private:
    void realloc(const std::size_t old_size);
    void alloc ();
    std::byte* get_buff();
    void alloc_expanding();

public:
    BaseStream () {}
    BaseStream (const std::size_t& allocSize);
    BaseStream (const char* src, const std::size_t& allocSize);
    ~BaseStream ();

    std::size_t get_pos() const { return headPos; }
    std::size_t get_size() const { return size; }

    void open (const std::size_t& allocSize);
    void move(const std::size_t from, const std::size_t to); // сдвигает хвост потока: данные, начинающиеся с позиции `from`, переносятся в позицию `to`, размер потока корректируется (если to > from — вставка, если to < from — удаление диапазона [to, from))
    void write (std::byte* s, const std::size_t count); // записывает объект, начиная с headPos, смещая указатель на конец новых данных
    bool read (std::byte* s, const std::size_t count);
    bool rel_read(std::byte* s, const std::size_t count);
    bool seek (const uint64_t pos);
    bool rel_seek (const int64_t shift);
    [[nodiscard]] std::byte* get_ptr (const std::size_t count);
    void resize(const std::size_t size);
};

using stream = BaseStream<STREAM_SIZE>;
using SmallStream = BaseStream<1024>;

} // namespace details