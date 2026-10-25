#include "stream.hpp"
#include <cstring>
#include <cassert>


#ifdef NDEBUG
#include <stdexcept>
#include <string>
#define STREAM_DEBUG_EXCEPTION(err) throw std::runtime_error((std::string(err) + ". in function: ") + __func__);
#else
#define STREAM_DEBUG_EXCEPTION(err)
#endif


void stream::alloc_expanding() {
    // allocated_memory = size * 1.2;
    allocated_memory = size + size / 5; // This expression is optimized better than the above
}


void stream::realloc() {
    if (size > STREAM_SIZE) {
        alloc_expanding();
        if (lot) {
            auto new_lot = new std::byte[allocated_memory];
            std::memcpy(lot, new_lot, STREAM_SIZE);
            delete[] lot;
            lot = new_lot;
        }
        else {
            lot = new std::byte[allocated_memory];
            std::memcpy(lot, stack.data(), STREAM_SIZE);
        }
    }
}


void stream::alloc () {
    if (size > STREAM_SIZE) {
        alloc_expanding();
        lot = new std::byte[allocated_memory];
    }
}


std::byte* stream::get_buff() {
    if (lot)
        return lot;
    return stack.data();
}


stream::stream (const std::size_t& allocSize) : size(allocSize) {
    alloc();
}

stream::stream (const char* src, const std::size_t& allocSize) : size(allocSize) {
    alloc();
    std::memcpy(get_buff(), src, size);
}

stream::~stream () {
    delete[] lot;
}

void stream::open (const std::size_t& allocSize) {
    size = allocSize;
    alloc();
}


bool stream::read (std::byte* s, const std::size_t count) {
    if (headPos + count > size) {
        // STREAM_DEBUG_EXCEPTION("Attempt to carry on a shift abroad")
        return false;
    }
    std::memcpy(s, get_buff() + headPos, count);
    return true;
}

void stream::write (std::byte* s, const std::size_t count) {
    if (headPos + count > size)
        size = headPos + count;
    if (size > STREAM_SIZE && size > allocated_memory) {
        realloc();
    }
    std::memcpy(get_buff() + headPos, s, count);
    rel_seek(count);
}

bool stream::seek (const uint64_t pos) {
    if (pos <= size) {
        headPos = pos;
        return true;
    }
    STREAM_DEBUG_EXCEPTION("Attempt to carry on a shift abroad")
    return false;
}

void stream::rel_seek (const int64_t shift) {
    auto newPos = headPos + shift;
    if (0 < newPos)
        seek(newPos);
}

std::byte* stream::get_ptr (const std::size_t count) {
    if (headPos + count > size) {
        STREAM_DEBUG_EXCEPTION("Attempt to carry on a shift abroad")
        return nullptr;
    }
    return &get_buff()[headPos];
}

void stream::move (const std::size_t from, const std::size_t to) {
    assert(from < size); // Unable to start copying abroad stream
    std::size_t moved_size = size - from;
    size = to + moved_size;
    if (size > STREAM_SIZE && size > allocated_memory)
        realloc();
    assert(to < size);
    assert(to + moved_size >= size);
    memmove(static_cast<void*>(&get_buff()[from]),
            static_cast<void*>(&get_buff()[to]),
            moved_size);
}