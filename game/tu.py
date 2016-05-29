#coding=utf-8
# 256色
from PIL import Image
from numpy import *
from scipy.misc import *

#需要修改！
picw = 320
pich = 200
picr = 1
picc = 1
im = array(Image.open('title2.png'))
#imshow(im)
pim = Image.fromarray(uint8(im))
#print im.shape
im = array(pim.resize((picw * picc,pich * picr)))
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

fout = open("title.asm",'w')

def Draw(x,y):
    for i in range(y * pich, y * pich+ pich): # row
        fout.write("db ")
        count = 0
        for j in range(x*picw,x*picw+picw): #col
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
            #if color[3] < Imed:
            #    color = (255,0,255,255)
            res = int((int(color[0])&0xe0) | ((int(color[1])>>3)&0x1c) | ((int(color[2])>>6)&0x03))
            fout.write(str(hex(res)))
            if j != x * picw + picw:
                fout.write(', ')

        fout.write('\n')

for y in range(picr):
    for x in range(picc):
        Draw(x,y)
