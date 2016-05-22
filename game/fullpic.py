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

fout = open("res.asm",'w')

def Draw(x,y):
    for i in range(32*4): # row
        fout.write("db ")
        count = 0
        for j in range(32*4): #col
            color = im[i][j]
            for k in range(4):
                if color[k] > Imed:
                    color[k] = Ihigh
                else:
                    if k != 3:
                        color[k] = Ilow
                    else:
                        color[k] = 0
            im[i][j] = color
            count += 1
            if count == 2:
                count = 0
                fout.write('0')
                fout.write(ToHex(im[i][j-1]))
                fout.write(ToHex(im[i][j]))
                fout.write("h")
                if (j != 32*4-1):
                    #if it is not end
                    fout.write(", ")
        fout.write('\n')

Draw(0,0)
