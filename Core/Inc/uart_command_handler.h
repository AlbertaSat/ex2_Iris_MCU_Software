#ifndef CLI_DEFH
#define CLI_DEFH

void uart_reset_sensors(void);
extern int format;
void uart_scan_i2c();
void uart_reset_sensors(void);
void sensor_togglepower(int i);
void uart_handle_capture_cmd(const char *cmd);
void uart_handle_transfer_cmd(const char *cmd);
void uart_handle_format_cmd(const char *cmd);
void uart_handle_width_cmd(const char *cmd);
void uart_handle_saturation_cmd(const char *cmd, uint8_t sensor);
void uart_handle_xfer_cmd(const char *cmd);
void uart_handle_read_file_cmd(const char *cmd);
void uart_handle_list_files_cmd(const char *cmd);

#endif // CLI_DEFH
