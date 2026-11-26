#include "stream.hpp"
#include "macro.hpp"
#include <cstring>
#include <cassert>


#ifdef NDEBUG
#include <stdexcept>
#include <string>
#define WMML_STREAM_DEBUG_EXCEPTION(err) throw std::runtime_error((std::string(err) + ". in function: ") + __func__);
#else
#define WMML_STREAM_DEBUG_EXCEPTION(err)
#endif


template <std::size_t stackSize>
void details::BaseStream<stackSize>::alloc_expanding() {
    // allocated_memory = size * 1.2;
    allocated_memory = size + size / 5; // This expression is optimized better than the above
}


template<std::size_t stackSize>
void details::BaseStream<stackSize>::realloc(const std::size_t old_size) {
    if (size > STREAM_SIZE) {
        alloc_expanding();
        const std::byte* old_buff = lot ? lot : stack.data();
        auto new_lot = new std::byte[allocated_memory];
        std::memcpy(new_lot, old_buff, old_size);
        if (lot)
            delete[] lot;
        lot = new_lot;
    }
}


template<std::size_t stackSize>
void details::BaseStream<stackSize>::alloc () {
    if (size > STREAM_SIZE) {
        alloc_expanding();
        lot = new std::byte[allocated_memory];
    }
}


template<std::size_t stackSize>
std::byte* details::BaseStream<stackSize>::get_buff() {
    if (lot)
        return lot;
    return stack.data();
}


template<std::size_t stackSize>
details::BaseStream<stackSize>::BaseStream (const std::size_t& allocSize) : size(allocSize) {
    alloc();
}

template<std::size_t stackSize>
details::BaseStream<stackSize>::BaseStream (const char* src, const std::size_t& allocSize) : size(allocSize) {
    alloc();
    std::memcpy(get_buff(), src, size);
}

template<std::size_t stackSize>
details::BaseStream<stackSize>::~BaseStream () {
    delete[] lot;
}

template<std::size_t stackSize>
void details::BaseStream<stackSize>::open (const std::size_t& allocSize) {
    size = allocSize;
    alloc();
}


template<std::size_t stackSize>
bool details::BaseStream<stackSize>::read (std::byte* s, const std::size_t count) {
    if (headPos + count > size) {
        // WMML_STREAM_DEBUG_EXCEPTION("Attempt to carry on a shift abroad")
        return false;
    }
    std::memcpy(s, get_buff() + headPos, count);
    return true;
}

template <std::size_t stackSize>
bool details::BaseStream<stackSize>::rel_read (std::byte* s, const std::size_t count) {
    auto res = read(s, count);
    return res && rel_seek(count);
}

template<std::size_t stackSize>
void details::BaseStream<stackSize>::write (std::byte* s, const std::size_t count) {
    const std::size_t old_size = size;
    if (headPos + count > size)
        size = headPos + count;
    if (size > STREAM_SIZE && size > allocated_memory)
        realloc(old_size);
    std::memcpy(get_buff() + headPos, s, count);
    rel_seek(count);
}

template<std::size_t stackSize>
bool details::BaseStream<stackSize>::seek (const uint64_t pos) {
    if (pos <= size) {
        headPos = pos;
        return true;
    }
    WMML_STREAM_DEBUG_EXCEPTION("Attempt to carry on a shift abroad")
    return false;
}

template<std::size_t stackSize>
bool details::BaseStream<stackSize>::rel_seek (const int64_t shift) {
    auto newPos = headPos + shift;
    if (0 < newPos)
        if (seek(newPos))
            return true;
    return false;
}

template<std::size_t stackSize>
std::byte* details::BaseStream<stackSize>::get_ptr (const std::size_t count) {
    if (headPos + count > size) {
        WMML_STREAM_DEBUG_EXCEPTION("Attempt to carry on a shift abroad")
        return nullptr;
    }
    return &get_buff()[headPos];
}

template<std::size_t stackSize>
void details::BaseStream<stackSize>::resize(const std::size_t newSize) {
    const std::size_t old_size = size;
    if (headPos > newSize)
        headPos = newSize;
    size = newSize;
    if (size > STREAM_SIZE && size > allocated_memory)
        realloc(old_size);
}

template<std::size_t stackSize>
void details::BaseStream<stackSize>::move (const std::size_t from, const std::size_t to) {
    assert(from <= size); // Unable to start copying abroad stream
    const std::size_t old_size = size;
    const std::size_t moved_size = size - from;
    size = to + moved_size;
    if (size > STREAM_SIZE && size > allocated_memory)
        realloc(old_size);
    if (moved_size != 0)
        memmove(static_cast<void*>(&get_buff()[to]),
                static_cast<void*>(&get_buff()[from]),
                moved_size);
}

template class details::BaseStream<STREAM_SIZE>;
template class details::BaseStream<1024>;
