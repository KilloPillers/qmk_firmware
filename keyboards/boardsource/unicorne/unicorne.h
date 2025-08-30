#include <stdint.h>

typedef struct _master_to_slave_t {
    uint8_t m2s_data[512];
} master_to_slave_t;

typedef struct _slave_to_master_t {
    uint8_t s2m_data[512];
} slave_to_master_t;
