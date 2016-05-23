#coding=utf-8
# 256色
from PIL import Image
from numpy import *
from scipy.misc import *

im = array(Image.open('g.png'))
#imshow(im)
pim = Image.fromarray(uint8(im))
im = array(pim.resize((32 * 4,32 * 4)))
Ihigh = 255
Ilow = 0
Imed = (Ihigh + Ilow) / 2

def ToHex(c):
    u = 0
    b = 1
    #RGBA -> ARGB
    for i in [2,1,0,3]:
        if c[i] >= Imed:
            u += b
        b*=2
    t = ['0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f']
    return t[u]

fout = open("map256.asm",'w')

def Draw(x,y):
    for i in range(y * 32, y * 32 + 32): # row
        fout.write("db ")
        count = 0
        for j in range(x*32,x*32+32): #col
            color = im[i][j]
            b = 1
            res = 0
            #for y in [2,1,0,3]:
                #256色8 bits
                #各种颜色2bit
                #u = color[y]
                #u = int(u) / (256 / 4)
                #if u >= 4:
                #    u = 3
                #res += u * b
                #b *= 4
            res = int((int(color[0])&0xe0) | ((int(color[1])>>3)&0x1c) | ((int(color[2])>>6)&0x03))
            fout.write(str(hex(res)))
            if j != x * 32 + 32:
                fout.write(', ')

        fout.write('\n')

for y in range(4):
    for x in range(4):
        Draw(x,y)
