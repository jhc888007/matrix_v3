#ifndef _MATRIXWRITER_H_
#define _MATRIXWRITER_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <exception>
#include <fcntl.h>  
#include "matrix.h"

using namespace std;


class MatrixWriter {
public:
    void Open(const char *idx_file, const char *data_file, int max_id)throw(exception) {
        _idx_fp = fopen(idx_file, "w");
        if (_idx_fp == NULL) {
            printf("open idx fail\n");
            throw exception();
        }
        setvbuf(_idx_fp, _idx_buffer, _IOFBF, _buffer_size);
        _data_fp = fopen(data_file, "w");
        if (_data_fp == NULL) {
            printf("open data fail\n");
            throw exception();
        }
        setvbuf(_data_fp, _data_buffer, _IOFBF, _buffer_size);
        _offset = 0;
        if (fseek(_data_fp, sizeof(MatrixHeader), SEEK_SET) < 0) {
            printf("seek fail\n");
            throw exception();
        }
        _max_idx = max_id;
        _real_max_idx = 0;
        _idx_vec.resize(_max_idx);
    }
    void Close()throw(exception) {
        char beacon[50] = "1:1|2:2|3:3|4:4|5:5|6:6|";
        Append(9, beacon, strlen(beacon));

        if (fseek(_idx_fp, 0, SEEK_SET) < 0) {
            printf("seek idx fail\n");
            throw exception();
        }
        uint64_t max = _real_max_idx;
        if (fwrite(&max, sizeof(IndexHeader), 1, _idx_fp) <= 0) {
            printf("write idx header fail\n");
            throw exception();
        }
        _idx_vec.resize(_real_max_idx+1);
        for (vector<IndexBody>::iterator itor = _idx_vec.begin(); itor != _idx_vec.end(); itor++) {
            if (fwrite(&(*itor), sizeof(IndexBody), 1, _idx_fp) <= 0) {
                printf("write idx fail\n");
                throw exception();
            }
        }

        if (fflush(_idx_fp) != 0) {
            printf("flush idx fail\n");
            throw exception();
        }
        if (fclose(_idx_fp) != 0) {
            printf("close idx fail\n");
            throw exception();
        }

        if (fseek(_data_fp, 0, SEEK_SET) < 0) {
            printf("seek data fail\n");
            throw exception();
        }
        if (fwrite(&_offset, sizeof(MatrixHeader), 1, _data_fp) <= 0) {
            printf("write data header fail\n");
            throw exception();
        }
        if (fflush(_data_fp) != 0) {
            printf("flush data fail\n");
            throw exception();
        }
        if (fclose(_data_fp) != 0) {
            printf("close data fail\n");
            throw exception();
        }
    }
    void Append(uint32_t uid, char *data, int len) {
        char *start = data;
        char *end;
        int ret;

        if (uid > _max_idx) {
            return ;
        }

        _data_vec.resize(0);
        MatrixBody body;
        while (start < data + len) {
            end = strchr(start, ':');
            if (NULL == end) {
                break;
            }
            *end = '\0';
            body.rid = atol(start);
            start = end + 1;
            end = strchr(start, '|');
            if (NULL == end) {
                break;
            }
            *end = '\0';
            body.value = atoi(start);
            start = end + 1;

            _data_vec.push_back(body);
        }

        if (_data_vec.size() == 0) {
            return ;
        }

        sort(_data_vec.begin(), _data_vec.end(), matrix_comp_func);

        uint64_t count = 0;
        for (vector<MatrixBody>::iterator iter = _data_vec.begin(); iter != _data_vec.end(); iter++) {
            ret = fwrite(&(*iter), sizeof(MatrixBody), 1, _data_fp);
            if (ret <= 0) {
                printf("write error\n");
            } else {
                count++; 
            }
        }

        while (uid > _idx_vec.size()) {
            _idx_vec.resize(_idx_vec.size()+50000000);
        }
        if (_real_max_idx < uid) {
            _real_max_idx = uid;
        }

        _idx_vec[uid].offset = _offset;
        _idx_vec[uid].count = count;

        /*
        ret = sprintf(_buffer, "%u\t%lu\t%lu\n", uid, _offset, count);
        if (ret < 0) {
            printf("sprintf error\n");
        } else {
            ret = fwrite(_buffer, ret, 1, _idx_fp);
            if (ret <= 0) {
                printf("write idx error\n");
            }
        }
        */

        _offset += count;
    }

private:
    FILE *_idx_fp, *_data_fp;
    static const int _buffer_size = 32 * 1024 * 1024;
    char _idx_buffer[_buffer_size], _data_buffer[_buffer_size];
    uint64_t _offset;
    char _buffer[512];
    int _max_idx;
    int _real_max_idx;
    vector<IndexBody> _idx_vec;
    vector<MatrixBody> _data_vec;
};

#endif


