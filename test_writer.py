#!/usr/bin/python

import sys
import matrixbuilder7 as mb

#WRITE
m = mb.writer(sys.argv[1], sys.argv[2], int(sys.argv[3]))
for line in sys.stdin:
    v = line.strip().split('\t')
    uid = int(v[0])
    value = v[2]
    m.append(uid, value, len(value))
del m



