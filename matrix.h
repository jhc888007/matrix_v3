#ifndef _MATRIX_H_
#define _MATRIX_H_

#include <sys/types.h>

typedef uint64_t IndexHeader;

struct IndexBody{
    uint64_t offset:40;
    uint64_t count:24;
    IndexBody() {
        offset = 0;
        count = 0;
    }
    inline void Clear() {
        offset = 0;
        count = 0;
    }
};

typedef uint64_t MatrixHeader;

struct MatrixBody{
    int64_t rid:40;
    int64_t value:24;
    MatrixBody() {
        rid = 0;
        value = 0;
    }
};

bool matrix_comp_func(const MatrixBody &m1, const MatrixBody &m2) {
    return m1.value > m2.value;
}

#endif
