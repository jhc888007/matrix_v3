#ifndef _MATRIX_READER_H_
#define _MATRIX_READER_H_


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <exception>
#include <fcntl.h>  
#include <vector>

using namespace std;

#define MAX_SIZE 1024

class MatrixReader {
public:
    MatrixReader() {
        _max_idx = 0;
        _max_size = 0;
        _vec.reserve(MAX_SIZE);
    }
    void Open(const char *idx_file, const char *data_file) {
        _idx_fp = open(idx_file, O_RDONLY);
        if (_idx_fp < 0) {
            printf("!!!matrixbuilder lib error!!! open idx fail\n");
            return ;
        }
        _data_fp = open(data_file, O_RDONLY);
        if (_data_fp < 0) {
            printf("!!!matrixbuilder lib error!!! open data fail\n");
            return ;
        }
        if (pread(_idx_fp, (void *)&_max_idx, sizeof(IndexHeader), 0) < 0) {
            printf("!!!matrixbuilder lib error!!! read idx header fail\n");
            close(_idx_fp);
            _idx_fp = -1;
            return ;
        }
        if (pread(_data_fp, (void *)&_max_size, sizeof(MatrixHeader), 0) < 0) {
            printf("!!!matrixbuilder lib error!!! read data header fail\n");
            close(_data_fp);
            _data_fp = -1;
            return ;
        }
    }
    void Close() {
        if (_idx_fp >= 0) {
            if (close(_idx_fp) < 0) {
                printf("!!!matrixbuilder lib error!!! close fail\n");
            }
        }
        if (_data_fp >= 0) {
            if (close(_data_fp) < 0) {
                printf("!!!matrixbuilder lib error!!! close fail\n");
            }
        }
    }
    IndexBody GetIndex(uint32_t uid) {
        IndexBody idx;
        if (uid > _max_idx || _idx_fp < 0) {
            return idx;
        }
        if (pread(_idx_fp, (void *)&idx, sizeof(IndexBody), 
            sizeof(IndexHeader)+uid*sizeof(IndexBody)) < 0) {
            idx.Clear();
        }
        return idx;
    }
    vector<MatrixBody> *GetData(uint64_t offset, uint64_t size) {
        _vec.resize(0);
        if (offset + size > _max_size || _data_fp < 0) {
            return &_vec;
        }
        if (size > MAX_SIZE) {
            size = MAX_SIZE;
        }
        _vec.resize(size);
        if (pread(_data_fp, (void *)&_vec[0], size*sizeof(MatrixBody),
            sizeof(MatrixHeader)+offset*sizeof(MatrixBody)) < 0) {
            _vec.resize(0);
        }
        return &_vec;
    }
    vector<MatrixBody> *GetDataRev(uint64_t offset, uint64_t size, uint64_t head_size, uint64_t tail_size) {
        _vec.resize(0);
        if (offset + head_size + tail_size > _max_size || _data_fp < 0) {
            return &_vec;
        }
        if (head_size + tail_size > MAX_SIZE) {
            head_size = MAX_SIZE/2;
            tail_size = MAX_SIZE/2;
        }
        if (head_size + tail_size > size) {
            head_size = size/2;
            tail_size = size/2;
        }
        _vec.resize(head_size + tail_size);
        if (pread(_data_fp, (void *)&_vec[0], head_size*sizeof(MatrixBody),
            sizeof(MatrixHeader)+offset*sizeof(MatrixBody)) < 0) {
            _vec.resize(0);
        }
        if (pread(_data_fp, (void *)&_vec[head_size], tail_size*sizeof(MatrixBody),
            sizeof(MatrixHeader)+(offset+size-tail_size)*sizeof(MatrixBody)) < 0) {
            _vec.resize(0);
        }
        return &_vec;
    }
    uint64_t GetMaxIndex() {
        return _max_idx;
    }
    uint64_t GetMaxData() {
        return _max_size;
    }
private:
    int _idx_fp, _data_fp;
    uint64_t _max_size;
    uint64_t _max_idx;
    vector<MatrixBody> _vec;
};

#endif

