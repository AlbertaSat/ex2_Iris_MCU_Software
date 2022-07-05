#ifndef DEBUG_DEFH
#define DEBUG_DEFH

void DBG_PUT(const char *format, ...);

static inline char hex_2_ascii(uint8_t hex) { return (hex < 10) ? '0' + hex : 'a' + (hex - 10); }

#endif // DEBUG_DEFH
