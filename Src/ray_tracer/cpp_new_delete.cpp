#include <cstddef>

extern "C" {

// Unsized delete
void operator delete(void* ptr) noexcept {
    (void)ptr;
}

// Sized delete (C++14+)
void operator delete(void* ptr, std::size_t) noexcept {
    (void)ptr;
}

// Unsized delete[]
void operator delete[](void* ptr) noexcept {
    (void)ptr;
}

// Sized delete[]
void operator delete[](void* ptr, std::size_t) noexcept {
    (void)ptr;
}

}

