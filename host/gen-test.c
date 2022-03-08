#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define MAX_NAME_LEN 64

void usage(const char *pgm) {
    const char *name = (pgm)? pgm : "usage";

    fprintf(stderr, "%s -i <raw> -o <rgb> -s '<w>x<d>'\n", name);
    exit(1);
}

enum argmasks {
    ARG_IN_MASK = 1,
    ARG_OUT_MASK = 2,
    ARG_SZ_MASK = 4
};

#define BYTES_PER_PIXEL 3

static enum argmasks needed_args = ARG_OUT_MASK | ARG_SZ_MASK;

int process_arg(char **argv, char pos, char token, enum argmasks mask, char *val) {
    if (argv[pos][0] == '-' && argv[pos][1] == token) {
        strncpy(val, argv[pos+1], MAX_NAME_LEN);
        pos += 2;

        if (!(needed_args & mask)) {
            fprintf(stderr, "already saw arg %x\n", mask);
            usage(argv[0]);
        }
        needed_args &= ~mask;
    }
    return pos;
}

int main(int argc, char **argv) {
    char outfile[MAX_NAME_LEN];
    char dimensions[MAX_NAME_LEN];
    int width = 0, height = 0;
    int i = 1;
    uint8_t *data;

    dimensions[0] = 0;
    while (i < argc) {
        i = process_arg(argv, i, 'o', ARG_OUT_MASK, outfile);
        i = process_arg(argv, i, 's', ARG_SZ_MASK, dimensions);

        if (dimensions[0] != 0) {
            if(sscanf(dimensions, "%dx%d", &width, &height) != 2) {
                fprintf(stderr, "couldn't parse size: %s\n", dimensions);
                usage(argv[0]);
            }
        }
    }

    if (needed_args & (ARG_OUT_MASK | ARG_SZ_MASK)) usage(argv[0]);

    fprintf(stderr, "size: %dx%d: output: %s\n", width, height, outfile);

    int outfd = open(outfile, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR | S_IWUSR);
    if (outfd == -1) {
        perror(outfile);
        return 3;
    }

    if (!(data = malloc(BYTES_PER_PIXEL*width))) {
        fprintf(stderr, "malloc failed\n");
        return 4;
    }

    int row = 0;
    for (int row=0; row<height; row++) {
        ssize_t rc;

        memset(data, 0, BYTES_PER_PIXEL*width);
        for(int i=0; i<BYTES_PER_PIXEL*width; i+=BYTES_PER_PIXEL) {
#if 0
            int gray = i/BYTES_PER_PIXEL + row;
            if (gray > 255) gray = (255 - (gray - 256));
            for (int p=0; p<BYTES_PER_PIXEL; p++)
                data[i+p] = gray;
#else
            switch(row/16) {
            case 0:
                data[i +0] = i/3;
                break;
            case 1:
                data[i + 1] = i/3;
                break;
            case 2:
                data[i + 2] = i/3;
                break;
            case 3:
                data[i + 0] = i/3;
                data[i + 2] = i/3;
                break;
            case 4:
                data[i + 1] = i/3;
                data[i + 2] = i/3;
                break;
            case 5:
                data[i + 0] = i/3;
                data[i + 1] = i/3;
                break;
            default:
                data[i + 0] = i/3;
                data[i + 1] = i/3;
                data[i + 2] = i/3;
                break;
            }
#endif
        }

        /* Once we have RGB for this row we can write it out */
        if ((rc = write(outfd, data, BYTES_PER_PIXEL*width)) == -1) {
            perror("write");
            break;
        }
        if (rc < BYTES_PER_PIXEL*width) {
            fprintf(stderr, "short write? %d\n", (int) rc);
        }
    }

    close(outfd);
    return 0;
}
