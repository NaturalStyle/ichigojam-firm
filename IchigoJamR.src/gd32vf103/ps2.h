typedef struct{
    uint8_t  bit;
    uint16_t data;
} kb_frame;
extern kb_frame g_frames[32];

typedef struct{
    uint8_t r;
    uint8_t w;
    uint8_t waiting_for_r;
} kb_status;
extern kb_status g_rw ;

#define RW_INC(rw) (rw = (rw+1)&0b11111)
#define WFR_NO   0
#define WFR_FULL 1
#define WFR_ERR  2
