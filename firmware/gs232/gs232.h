#ifndef __GS232_H__
#define __GS232_H__

#define AZ_SERVER_PORT		4001
#define EL_SERVER_PORT		4002

#define AZ_SERVER_STACK_SIZE	2048
#define EL_SERVER_STACK_SIZE	2048

typedef struct gs232_server_t
{
    char *name;
    uint16_t port;
    uint32_t led;
    bool connected;
} gs232_server_t;

extern gs232_server_t tcp_gs232_az;
extern gs232_server_t tcp_gs232_el;

void gs232_init(void);

#endif /* __GS232_H__ */