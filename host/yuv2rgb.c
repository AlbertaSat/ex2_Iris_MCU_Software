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
    ARG_ALL_MASK = 7,
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

static inline uint8_t floor_ceiling(double val) {
    if (val < 0.0) return 0;
    return(val < 255.0) ? (uint8_t) (val + 0.5) : 255;
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

    if (needed_args & ARG_ALL_MASK) usage(argv[0]);

    fprintf(stderr, "size: %dx%d: raw input: %s, interlaced output: %s\n",
            width, height, rawfile, rgbfile);

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

    raw = malloc(width*2);
    rgb = malloc(width*3);
    if (!raw || !rgb) {
        fprintf(stderr, "malloc failed\n");
        return 4;
    }

    int count = 0;
    int row = 0;
    while(row < height) {
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
         * Y0 U01 Y1 V01 Y2 U23 Y3 V23 ...
         */
        int rcol, pxl;
        double gmin = 100.0;
        double gmax = -100.0;
	double rmin = 100.0;
	double rmax = -100.0;
	double bmin = 100.0;
	double bmax = -100.0;
        for (rcol=0, pxl=0; rcol<width*2; rcol+=4, pxl+=6) {
            double y = raw[rcol + 0];
            double u = raw[rcol + 1];
            double v = raw[rcol + 3];
            double r, g, b;

            u -= 128.0; // U = Cb?
            v -= 128.0; // V = Cr?
            r = y + 0.0*u + 1.371*v;
            g = y - 0.336*u - 0.698*v;
            b = y + 1.732*u + 0.0*v;

            rgb[pxl + 0] = floor_ceiling(r);
            rgb[pxl + 1] = floor_ceiling(g);
            rgb[pxl + 2] = floor_ceiling(b);

            if (r < rmin) rmin = r;
            if (r > rmax) rmax = r;
            if (g < gmin) gmin = g;
            if (g > gmax) gmax = g;
            if (b < bmin) bmin = b;
            if (b > bmax) bmax = b;

            printf("[%d]: Y=%d U=%d V=%d r=%lf g=%lf b=%lf [%d %d %d]\n",
                   rcol, raw[rcol], raw[rcol+1], raw[rcol+3], r, g, b,
                   rgb[pxl], rgb[pxl+1], rgb[pxl+2]);
 
            y = raw[rcol + 2];

            r = y + 0.0*u + 1.371*v;
            g = y - 0.336*u - 0.698*v;
            b = y + 1.732*u - 0.0*v;

            rgb[pxl + 3] = floor_ceiling(r);
            rgb[pxl + 4] = floor_ceiling(g);
            rgb[pxl + 5] = floor_ceiling(b);
        }

        printf("row %d: rmin/max [%f,%f] gmin/max [%f,%f] bmin/max [%f,%f]\n", 
	        row, rmin, rmax, gmin, gmax, bmin, bmax);

        /* Once we have RGB for this row we can write it out */
        if ((rc = write(rgbfd, rgb, width*3)) == -1) {
            perror("write");
            break;
        }
        if (rc < width*3) {
            fprintf(stderr, "short write? %d\n", (int) rc);
        }
        row++;
    }

    close(rawfd);
    close(rgbfd);
    return 0;
}
