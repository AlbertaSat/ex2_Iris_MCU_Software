#ifndef CLI_DEFH
#define CLI_DEFH

void handle_command(char *cmd);
void reset_sensors(void);
extern int format;
void scan_i2c();
#endif // CLI_DEFH
