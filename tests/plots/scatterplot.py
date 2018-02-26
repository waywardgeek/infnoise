#!/usr/bin/env python

import matplotlib.pyplot as plt
import numpy as np
import sys

if sys.argv[1]:
        filename=sys.argv[1]
else:
        filename='infnoise.bin'


bp = np.dtype([('byte1',np.uint8),('byte2',np.uint8)]) # 'struct' byte pairs

Z = np.fromfile(filename, dtype=bp, count=2000) # read 2000 byte pairs from binary file

x, y = zip(*Z) # unpack Z pairs into lists

plt.grid(True)

plt.xlim(0, 255)
plt.ylim(0, 255)

plt.xlabel('bytes')
plt.ylabel('bytes')
plt.title('TRNG ' + filename)

plt.scatter(x[:1000],y[:1000])

plt.savefig(filename + '-scatter.png')
plt.show()


