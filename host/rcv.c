#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <arpa/inet.h>
#include <stddef.h>

#define MAX_NAME_LEN 64

static struct termios stdin_settings;
static FILE *img = 0;

enum DataState {
    STATE_RESET,
    STATE_HAVE_J,
    STATE_HAVE_P,
    STATE_GET_LEN,
    STATE_DOWNLOADING,
    STATE_LOCAL
};

static void restore(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &stdin_settings);
    if (img)
        fclose(img);
}

static bool open_image(const char *fname) {
    if (img)
        fclose(img);
    if (!(img = fopen(fname, "w"))) {
        perror("fopen");
        return false;
    }
    return true;
}

void print_progress(size_t count, size_t max)
{
	const char prefix[] = "Progress: [";
	const char suffix[] = "]";
	const size_t prefix_length = sizeof(prefix) - 1;
	const size_t suffix_length = sizeof(suffix) - 1;
	char *buffer = calloc(max + prefix_length + suffix_length + 1, 1); // +1 for \0
	size_t i = 0;

	strcpy(buffer, prefix);
	for (; i < max; ++i)
	{
		buffer[prefix_length + i] = i < count ? '#' : ' ';
	}

	strcpy(&buffer[prefix_length + i], suffix);
	//printf("\b%c[2K\r%s\n", 27, buffer);
	printf("\b\r%c[2K\r%s", 27, buffer);
	fflush(stdout);
	free(buffer);
}

int main(int argc, char **argv) {
    char imgfile[MAX_NAME_LEN];
    int imgcnt = 1;
    size_t iter = 0;
    size_t maxcount = 0;
    strncpy(imgfile, "img.jpg", MAX_NAME_LEN);
    for (int i=0; i<argc; i++) {
        if (argv[i][0] == '-' && argv[i][1] == 'o') {
            i++;
            strncpy(imgfile, argv[i], MAX_NAME_LEN); 
        }
    }
    fprintf(stderr, "output file: %s\n", imgfile);

    int ttyfd = open("/dev/ttyUSB0", O_RDWR | O_NDELAY);
    if (ttyfd == -1) {
        perror("open");
        return 1;
    }

    /* Set the UART to RAW mode */
    struct termios options;
    tcgetattr(ttyfd, &options);
    cfsetspeed(&options, B115200);
    cfmakeraw(&options);
    tcsetattr(ttyfd, TCSANOW, &options);

    /* Set STDIN to char-at-a-time */
    tcgetattr(STDIN_FILENO, &stdin_settings);
    options = stdin_settings;
    options.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &options);

    atexit(restore); // reset STDIN behavior on exit
    
    enum DataState state = STATE_RESET;
    int val, pos = 0;
    //uint32_t netlen, count = 0;
    uint32_t netlen = 0;
    size_t count = 0;
    while(1) {
        fd_set rfds;
        struct timeval tv = { 0 };

        FD_ZERO(&rfds);
        FD_SET(0, &rfds);
        FD_SET(ttyfd, &rfds);
        int nfds = select(ttyfd+1, &rfds, NULL, NULL, &tv);
        if (nfds == -1) {
            perror("select");
            return 4;
        }
        if (nfds>0 && FD_ISSET(0, &rfds)) {
            char line[8];
            ssize_t cnt;
            if ((cnt = read(STDIN_FILENO, line, sizeof(line))) == -1) {
                perror("stdin");
                break;
            }
            if (cnt == 0) {
                fprintf(stderr, "stdin closed?\n");
                break;
            }
            switch(*line) {
            case 0x4: // ^D
                exit(0);
                break;
            case 0x6: // ^F
                if (state != STATE_DOWNLOADING) {
                    state = STATE_LOCAL;
                    pos = 0;
                    fprintf(stderr, "enter file name: ");
                }
                break;
            default:
                if (state == STATE_LOCAL) {
                    if (line[0] == 0x7f)
                        line[0] = '\b'; // echoing backspace doesn't seem to work

                    if (write(1, line, cnt) == -1) {
                        perror("local echo");
                    }

                    if (line[0] == '\n' || (pos + cnt) >= MAX_NAME_LEN-1) {
                        imgfile[pos] = 0;
                        fprintf(stderr, "new file is: %s\n", imgfile);
                        state = STATE_RESET;
                        pos = 0;
                    }
                    else if (line[0] == 0x7f) { // handle backspace
                        if (pos > 0)
                            pos--;
                    }
                    else {
                        memcpy(imgfile + pos, line, cnt);
                        pos += cnt;
                    }
                    continue;
                }

                if (write(ttyfd, line, cnt) != cnt) {
                    perror("write tty");
                }
                break;
            }

            nfds--;
        }

        if (nfds==0 || !FD_ISSET(ttyfd, &rfds)) {
            continue;
        }

        char c;
        ssize_t rc = read(ttyfd, &c, 1);
        if (rc == -1) {
            perror("read");
            return 2;
        }

        switch(state) {
        case STATE_RESET:
            if (c == 'J')
                state = STATE_HAVE_J;
            break;
        case STATE_HAVE_J:
            state = (c == 'P') ? STATE_HAVE_P : STATE_RESET;
            break;
        case STATE_HAVE_P:
            if (c == 'G') {
                state = STATE_GET_LEN;
                pos = 3;
                netlen = 0;
                if (!open_image(imgfile)) {
                    fprintf(stderr, "can't save to image '%s'\n", imgfile);
                    state = STATE_RESET;
                }
             }
            else
                state = STATE_RESET;
            break;
        case STATE_GET_LEN:
            netlen |= ((c & 0x0ff) << 8*pos);
            if (pos == 0) {
                count = ntohl(netlen);
                fprintf(stderr, " downloading %d bytes\n", count);
                maxcount = count / 1024;
                state = STATE_DOWNLOADING;
            }
            else
                --pos;
            continue;
        default:
            break;
        }
        if (state != STATE_DOWNLOADING) {
            if (c == 0x7f)
                c = '\b';
            if (write(1, &c, 1) == -1) {
                perror("echo");
            }
        }
        else {
            if ((rc = fwrite(&c, 1, 1, img)) == -1) {
                perror("write");
                break;
            }
            count--;

            if (count <= 0) {
                /* Finished receiving the current image. Close it, set the
                 * state back to "looking for image marker", and prepare a
                 * new image filename in case we want to dump another one.
                 */
                fputs("\r\nCapture Complete!\r\n", stderr);
                fclose(img);
                img = 0;
                sprintf(imgfile, "%s(%d)", imgfile, imgcnt);
                state = STATE_RESET; // reset state machine
                iter = 0;
                continue;
            }

            if ((count % 1024) == 0) {
            	 iter++;
                print_progress(iter, maxcount);
            }
        }
    }

    close(ttyfd);
    if (img)
        fclose(img);
    return 0;
}
