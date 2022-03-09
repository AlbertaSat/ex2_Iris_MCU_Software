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
    char rgbfile[MAX_NAME_LEN];
    char dimensions[MAX_NAME_LEN];
    int width = 0, height = 0;
    int i = 1;
    uint8_t *raw, *rgb;
    
    while (i < argc) {
        i = process_arg(argv, i, 'i', ARG_IN_MASK, rawfile);
        i = process_arg(argv, i, 'o', ARG_OUT_MASK, rgbfile);
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
            dimensions, rawfile, rgbfile);
    
    int rawfd = open(rawfile, O_RDONLY);
    if (rawfd == -1) {
        perror(rawfile);
        return 2;
    }

    int rgbfd = open(rgbfile, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR | S_IWUSR);
    if (rgbfd == -1) {
        perror(rgbfile);
        return 3;
    }

    raw = malloc(width);
    rgb = malloc(width*3);
    if (!raw || !rgb) {
        fprintf(stderr, "malloc failed\n");
        return 4;
    }
    
    int count = 0;
    int layer = 0;
    int row = 0;
    while(1) {
        /* Read what we think is a row of red or green or blue intensities */
        ssize_t rc;
        if ((rc = read(rawfd, raw, width)) == -1) {
            perror("read");
            return 2;
        }
        count += rc;
        if (rc < width) {
            fprintf(stderr, "EOF? read %d, total %d\n", (int) rc, count);
            break;
        }

        /* Spread the layer across the rgb image. Layer is red/green/blue. */
        int zero = 1;
        for (int col=0; col<width; col++) {
            rgb[col*3 + layer] = raw[col];
            if (raw[col]) zero = 0;
        }
        if (zero) {
            fprintf(stderr, "row %d layer %d all zero\n", row, layer);
        }
        layer++;
        if (layer >= 3) {
            /* Once we have RGB for this row we can write it out */
            if ((rc = write(rgbfd, rgb, width*3)) == -1) {
                perror("write");
                break;
            }
            if (rc < width*3) {
                fprintf(stderr, "short write? %d\n", (int) rc);
            }
            row ++;
            layer = 0;
        }
    }

    close(rawfd);
    close(rgbfd);
    return 0;
}
