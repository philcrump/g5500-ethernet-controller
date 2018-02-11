#include <stdio.h>
#include <string.h>

#include "ch.h"
#include "hal.h"

#include "lwipthread.h"
#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/sys.h"

#include "gs232.h"
#include "../main.h"
#include "../tracking/tracking.h"

#define COMMAND_BUFFER_LENGTH  32
#define RESPONSE_BUFFER_LENGTH  32

typedef struct gs232_server_t
{
    char *name;
    uint16_t port;
    uint32_t led;
} gs232_server_t;

gs232_server_t az_server = { "gs232_az", AZ_SERVER_PORT, LINE_LED2 };
gs232_server_t el_server = { "gs232_el", EL_SERVER_PORT, LINE_LED3 };

static void gs232_parse(char* command_buffer, uint32_t command_length, char *response_buffer, uint16_t *response_length, uint16_t response_maxlength)
{
    if(command_length >= 2 && 0==strncmp("S\r", command_buffer, 2))
    {
        /* S - Stop */
        tracking_state.desired_az_ddeg = tracking_state.az_ddeg;
        tracking_state.desired_az_deg = tracking_state.az_deg;
        tracking_state.desired_el_ddeg = tracking_state.el_ddeg;
        tracking_state.desired_el_deg = tracking_state.el_deg;

        *response_length = 0;
        return;
    }
    else if(command_length >= 2 && 0==strncmp("C\r", command_buffer, 2))
    {
        /* C - Query Azimuth position */
        *response_length = snprintf(response_buffer, response_maxlength, "AZ=%03d\r",
            tracking_state.az_deg
        );
        return;
    }
    else if(command_length >= 2 && 0==strncmp("B\r", command_buffer, 2))
    {
        /* B - Query Elevation position */
        *response_length = snprintf(response_buffer, response_maxlength, "EL=%03d\r",
            tracking_state.el_deg
        );
        return;
    }
    else if(command_length >= 3 && 0==strncmp("C2\r", command_buffer, 3))
    {
        /* C2 - Query Azimuth & Elevation position */
        *response_length = snprintf(response_buffer, response_maxlength, "AZ=%03d EL=%03d\r",
            tracking_state.az_deg,
            tracking_state.el_deg
        );
        return;
    }
    else if(command_length >= 5 && 0==strncmp("M", command_buffer, 1))
    {
        /* M - Set Azimuth position */
        sscanf(command_buffer, "M%03hd\r", &tracking_state.desired_az_deg);
        tracking_state.desired_az_ddeg = tracking_state.desired_az_deg * 10;

        *response_length = 0;
        return;
    }
    else if(command_length >= 9 && 0==strncmp("W", command_buffer, 1))
    {
        /* W - Set Azimuth & Elevation */
        sscanf(command_buffer, "W%03hd %03hd\r", &tracking_state.desired_az_deg, &tracking_state.desired_el_deg);
        tracking_state.desired_az_ddeg = tracking_state.desired_az_deg * 10;
        tracking_state.desired_el_ddeg = tracking_state.desired_el_deg * 10;

        *response_length = 0;
        return;
    }

    /* Error Handler */
    //udp_debug_msg("Parser: Unknown:");
    //udp_debug_msg(command_buffer);
    //udp_debug_msg("");
    *response_length = 0;
}

THD_FUNCTION(gs232_server, server_vars)
{
    chRegSetThreadName(((gs232_server_t*)server_vars)->name);

    struct netconn *conn, *newconn;
    err_t err;

    void *fragment_buffer;
    uint16_t fragment_length;
    char command_buffer[COMMAND_BUFFER_LENGTH];
    uint16_t command_length;
    char response_buffer[RESPONSE_BUFFER_LENGTH];
    uint16_t response_length;

    conn = netconn_new(NETCONN_TCP);

    netconn_bind(conn, NULL, ((gs232_server_t*)server_vars)->port);

    netconn_listen(conn);

    while (1)
    {
        palClearLine(((gs232_server_t*)server_vars)->led);
        /* Blocks here for new connection */
        err = netconn_accept(conn, &newconn);

        palSetLine(((gs232_server_t*)server_vars)->led);

        if (err == ERR_OK)
        {
            struct netbuf *net_buf;

            while ((err = netconn_recv(newconn, &net_buf)) == ERR_OK)
            {
                command_length = 0;
                /* defragment packet */
                do
                {
                    /* Get data pointer and length */
                    netbuf_data(net_buf, &fragment_buffer, &fragment_length);

                    /* Crudely handle overflow by dropping end bytes */
                    if(command_length + fragment_length > COMMAND_BUFFER_LENGTH)
                    {
                        fragment_length = COMMAND_BUFFER_LENGTH - command_length;
                    }

                    memcpy(&command_buffer[command_length], fragment_buffer, fragment_length);
                    command_length += fragment_length;
                }
                while (netbuf_next(net_buf) >= 0);

                gs232_parse((char*)command_buffer, command_length, response_buffer, &response_length, RESPONSE_BUFFER_LENGTH);

                /* Respond if required */
                if(response_length>0)
                {
                    err = netconn_write(newconn, response_buffer, response_length, NETCONN_COPY);
                }

                netbuf_delete(net_buf);
            }
            /* Close connection */
            netconn_close(newconn);
            netconn_delete(newconn);
        }
    }
}

THD_WORKING_AREA(wa_az_server, AZ_SERVER_STACK_SIZE);
THD_WORKING_AREA(wa_el_server, EL_SERVER_STACK_SIZE);

void gs232_init(void)
{
    chThdCreateStatic(wa_az_server, sizeof(wa_az_server), NORMALPRIO, gs232_server, (void*)&az_server);
    chThdCreateStatic(wa_el_server, sizeof(wa_el_server), NORMALPRIO, gs232_server, (void*)&el_server);
}