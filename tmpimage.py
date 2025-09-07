import numpy as np
import skimage


img  = skimage.io.imread("debug_mega_image3.png")
img2 = skimage.io.imread("debug_mega_image3.png")

c_white = np.array([255,255,255,255])
c_blue  = np.array([0,0,255,255])
for i in range(img.shape[0] * img.shape[1]):
    x,y = i // img.shape[1], i % img.shape[1]
    xl = x-1 if x > 0 else x
    xh = x+1 if x < img.shape[0]-1 else x
    yl = y-1 if y > 0 else y
    yh = y+1 if y < img.shape[1]-1 else y
    # if not (img[x,y] == c_white).all():
    #     print(img2[x,y])
    if not (img[x,y] == c_blue).all():
        if (img[xl,yl] == c_blue).all() or\
           (img[x ,yl] == c_blue).all() or\
           (img[xh,yl] == c_blue).all() or\
           (img[xl,y ] == c_blue).all() or\
           (img[xh,y ] == c_blue).all() or\
           (img[xl,yh] == c_blue).all() or\
           (img[x ,yh] == c_blue).all() or\
           (img[xh,yh] == c_blue).all():
            img2[x,y] = c_blue

skimage.io.imsave("debug_mega_image4.png", img2)