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
    ARG_SZ_MASK = 4,
    ARG_ALL_MASK = 7
};

static enum argmasks needed_args = ARG_ALL_MASK;

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

    if (needed_args & ARG_ALL_MASK) usage(argv[0]);

    fprintf(stderr, "size: %dx%d: raw input: %s, Y output: %s\n", width, height, rawfile, yuvfile);

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

    raw = malloc(2*width);
    yuv = malloc(width);
    if (!raw || !yuv) {
        fprintf(stderr, "malloc failed\n");
        return 4;
    }

    int count = 0;
    int row = 0;
    while(row < height) { 
        ssize_t rc;
        if ((rc = read(rawfd, raw, 2*width)) == -1) {
            perror("read");
            return 2;
        }
        count += rc;
        if (rc < 2*width) {
            fprintf(stderr, "EOF? read %d, total %d\n", (int) rc, count);
            break;
        }

	int col, yx;
        for (col=0, yx=0; col<width*2; col+=2, yx++) {
            yuv[yx] = raw[col]; // just take the Y part
        }

        /* Once we have RGB for this row we can write it out */
        if ((rc = write(yuvfd, yuv, width)) == -1) {
            perror("write");
            break;
        }
        if (rc < width) {
            fprintf(stderr, "short write? %d\n", (int) rc);
        }
        row++;
    }

    close(rawfd);
    close(yuvfd);
    return 0;
}
