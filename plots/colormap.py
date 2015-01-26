#!/usr/bin/env python

import numpy as np
import matplotlib.pyplot as plt
from matplotlib import cm

filename='infnoise.bin'

nx = 1000
ny = 1000

data = np.fromfile(open(filename,'rb'), dtype=np.uint8, count=nx*nx)
data.resize(nx,ny)

plt.xlim(0, nx)
plt.ylim(0, ny)

plt.xlabel('samples')
plt.ylabel('samples')
plt.title('TRNG ' + filename)

#cax = plt.imshow(data, interpolation='nearest', cmap=cm.coolwarm)
cax = plt.imshow(data, interpolation='nearest', cmap=cm.afmhot)
cbar = plt.colorbar(cax, ticks=[255, 127, 0])
cbar.ax.set_yticklabels(['255', '127', '0'])

plt.savefig(filename + '-colormap.png')
plt.show() 
