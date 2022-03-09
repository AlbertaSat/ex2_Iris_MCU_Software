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

static enum argmasks needed_args = ARG_IN_MASK | ARG_OUT_MASK | ARG_SZ_MASK;

int process_arg(char **argv, char pos, char token, enum argmasks mask, char *val) {
    *val = 0;
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
    char rawfile[MAX_NAME_LEN];
    char yuvfile[MAX_NAME_LEN];
    char dimensions[MAX_NAME_LEN];
    int width = 0, height = 0;
    int i = 1;
    uint8_t *raw, *yuv;

    while (i < argc) {
        i = process_arg(argv, i, 'i', ARG_IN_MASK, rawfile);
        i = process_arg(argv, i, 'o', ARG_OUT_MASK, yuvfile);
        i = process_arg(argv, i, 's', ARG_SZ_MASK, dimensions);

        if (dimensions[0]) {
            if(sscanf(dimensions, "%dx%d", &width, &height) != 2) {
                fprintf(stderr, "couldn't parse size: %s\n", dimensions);
                usage(argv[0]);
            }
        }
    }

    if (needed_args) usage(argv[0]);

    fprintf(stderr, "size: %s: raw input: %s, interlaced output: %s\n",
            dimensions, rawfile, yuvfile);

    int rawfd = open(rawfile, O_RDONLY);
    if (rawfd == -1) {
        perror(rawfile);
        return 2;
    }

    int yuvfd = open(yuvfile, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR | S_IWUSR);
    if (yuvfd == -1) {
        perror(yuvfile);
        return 3;
    }

    raw = malloc(width*2);
    yuv = malloc(width*3);
    if (!raw || !yuv) {
        fprintf(stderr, "malloc failed\n");
        return 4;
    }

    int count = 0;
    int row = 0;
    while(1) {
        /* Read what we think is a row of YUV422 intensities */
        ssize_t rc;
        if ((rc = read(rawfd, raw, width*2)) == -1) {
            perror("read");
            return 2;
        }
        count += rc;
        if (rc < width*2) {
            fprintf(stderr, "EOF? read %d, total %d\n", (int) rc, count);
            break;
        }

        /* Spread the YUV422 row across the YUV444 row. The YUV422 should be
         * Y0 U0 Y1 V1 Y2 U2 Y3 V3 ...
         */
        int ycol;
        for (int rcol=0, ycol=0; rcol<width*2; rcol+=4, ycol+=6) {
#if 0
            uint8_t t1 = raw[rcol];
            raw[rcol] = raw[rcol + 1];
            raw[rcol + 1] = t1;
            t1 = raw[rcol + 2];
            raw[rcol + 2] = raw[rcol + 3];
            raw[rcol + 3] = t1;
#endif
            yuv[ycol + 0] = raw[rcol + 0]; // Y0 from pixel 0
            yuv[ycol + 1] = raw[rcol + 1]; // U0 from pixel 0
            yuv[ycol + 2] = raw[rcol + 3]; // V0 from pixel 1
            yuv[ycol + 3] = raw[rcol + 2]; // Y1 from pixel 1
            yuv[ycol + 4] = raw[rcol + 1]; // U1 from pixel 0
            yuv[ycol + 5] = raw[rcol + 3]; // V1 from pixel 1
        }

        /* Once we have RGB for this row we can write it out */
        if ((rc = write(yuvfd, yuv, width*3)) == -1) {
            perror("write");
            break;
        }
        if (rc < width*3) {
            fprintf(stderr, "short write? %d\n", (int) rc);
        }
        row++;
    }

    close(rawfd);
    close(yuvfd);
    return 0;
}
