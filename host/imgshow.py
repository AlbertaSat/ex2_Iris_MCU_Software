#!/usr/bin/env python3

import sys
import matplotlib.pyplot as plt
import matplotlib.image as mpimg
from parse import *
import argparse

def read_image(file: str, width: int, height: int, depth: int): 
    img = []
    try:
        with open(file, "rb") as f:
            for row in range(height):
                line = []
                for col in range(width):
                    pixel = []
                    # depth bytes per pixel
                    for p in range(depth):
                        p = f.read(1)
                        pixel.append(int.from_bytes(p, byteorder='big'))
                    line.append(pixel)
                img.append(line)
    except IOError:
        print("Error opening file: " + file)

    return img

def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("size", type=str, help="<width>x<height>")
    parser.add_argument("file", type=str, help="file containing a raw image") 
    parser.add_argument("--rgb888", action="store_true", help="3 bytes per pixel")
    parser.add_argument("--gray", action="store_true", help="1 byte per pixel")
    parser.add_argument("--yuv422", action="store_true", help="2 bytes per pixel")
    args = parser.parse_args()

    (width, height) = parse("{:d}x{:d}", args.size)

    print("file: %s, size: %dx%d" % (args.file, width, height))

    img = None
    if args.rgb888:
        img = read_image(args.file, width, height, 3)
    elif args.gray:
        img = read_image(args.file, width, height, 1)
    elif args.yuv422:
        img = read_yuv422_image(args.file, width, height)
    else:
        print("missing image type")
        return 1

    #print(img)
    
    plt.imshow(img)
    plt.show()
    gmi = img[::-1]
    plt.imshow(gmi)
    plt.show()

    return 0

if __name__ == '__main__':
    sys.exit(main())
