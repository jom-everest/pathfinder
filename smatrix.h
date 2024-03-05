#ifndef SIMPLE_MATRIX_H
#define SIMPLE_MATRIX_H

#include <stdint.h>

struct Coord {
    int x;
    int y;
    bool operator ==(const Coord& c) {
        return (c.x == x && c.y == y);
    }
};

template<typename T> class SMatrix {

public:
    SMatrix() = default;
    SMatrix(uint32_t _dimx, uint32_t _dimy) : dimx(_dimx), dimy(_dimy) {
        ptr = new T[dimx*dimy];
    }

    SMatrix(uint32_t _dimx, uint32_t _dimy, T value) : dimx(_dimx), dimy(_dimy) {
        ptr = new T[dimx*dimy];
        std::fill_n(ptr, dimx*dimy, value);
    }

    void setSize(uint32_t _dimx, uint32_t _dimy) {
        deletePtr();
        dimx = _dimx;
        dimy = _dimy;
        ptr = new T[dimx*dimy];
    }

    SMatrix(const SMatrix& m) = delete;
    const SMatrix& operator =(const SMatrix& m) = delete;
    SMatrix& operator =(SMatrix& m) = delete;

    SMatrix& operator =(SMatrix&& m) {
        std::swap(dimx, m.dimx);
        std::swap(dimy, m.dimy);
        std::swap(ptr, m.ptr);
        return *this;
    }

    SMatrix(SMatrix&& m) {
        std::swap(dimx, m.dimx);
        std::swap(dimy, m.dimy);
        ptr = m.ptr;
        m.ptr = nullptr;
    }

    ~SMatrix() {
        deletePtr();
    }

    T& data(uint32_t x, uint32_t y) {
        return ptr[x+y*dimx];
    }

    T& data(Coord c) {
        return ptr[c.x+c.y*dimx];
    }

    uint32_t getDimx() {
        return dimx;
    }

    uint32_t getDimy() {
        return dimy;
    }

private:
    void deletePtr() {
        if (ptr){
            delete[] ptr;
            dimx = dimy = 0;
            ptr = nullptr;
        }
    }

    uint32_t dimx{};
    uint32_t dimy{};
    T* ptr {nullptr};
};

#endif // SMATRIX_H
