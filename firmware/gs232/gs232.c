#include "../main.h"

#define COMMAND_BUFFER_LENGTH  32
#define RESPONSE_BUFFER_LENGTH  32

gs232_server_t tcp_gs232_az = { "tcp_gs232_az", AZ_SERVER_PORT, LINE_LED2, false };
gs232_server_t tcp_gs232_el = { "tcp_gs232_el", EL_SERVER_PORT, LINE_LED3, false };

static void gs232_parse(char* command_buffer, uint32_t command_length, char *response_buffer, uint16_t *response_length, uint16_t response_maxlength)
{
    if(command_length >= 2 && 0==strncmp("S\r", command_buffer, 2))
    {
        /* S - Stop */
        tracking_set_stop();

        *response_length = 0;
        return;
    }
    else if(command_length >= 2 && 0==strncmp("C\r", command_buffer, 2))
    {
        /* C - Query Azimuth position */
        int16_t az_ddeg;
        tracking_get_state_ddeg(&az_ddeg, NULL);

        *response_length = snprintf(response_buffer, response_maxlength, "AZ=%03d\r",
            az_ddeg
        );
        return;
    }
    else if(command_length >= 2 && 0==strncmp("B\r", command_buffer, 2))
    {
        /* B - Query Elevation position */
        int16_t el_ddeg;
        tracking_get_state_ddeg(NULL, &el_ddeg);

        *response_length = snprintf(response_buffer, response_maxlength, "EL=%03d\r",
            el_ddeg
        );
        return;
    }
    else if(command_length >= 3 && 0==strncmp("C2\r", command_buffer, 3))
    {
        /* C2 - Query Azimuth & Elevation position */
        int16_t az_ddeg, el_ddeg;
        tracking_get_state_ddeg(&az_ddeg, &el_ddeg);

        *response_length = snprintf(response_buffer, response_maxlength, "AZ=%03d EL=%03d\r",
            az_ddeg,
            el_ddeg
        );
        return;
    }
    else if(command_length >= 5 && 0==strncmp("M", command_buffer, 1))
    {
        /* M - Set Azimuth position */
        int16_t desired_az_ddeg;

        sscanf(command_buffer, "M%03hd\r", &desired_az_ddeg);
        desired_az_ddeg *= 10;

        tracking_set_desired_ddeg(&desired_az_ddeg, NULL);

        *response_length = 0;
        return;
    }
    else if(command_length >= 9 && 0==strncmp("W", command_buffer, 1))
    {
        /* W - Set Azimuth & Elevation */
        int16_t desired_az_ddeg, desired_el_ddeg;

        sscanf(command_buffer, "W%03hd %03hd\r", &desired_az_ddeg, &desired_el_ddeg);
        desired_az_ddeg *= 10;
        desired_el_ddeg *= 10;

        tracking_set_desired_ddeg(&desired_az_ddeg, &desired_el_ddeg);

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
        ((gs232_server_t*)server_vars)->connected = false;
        palClearLine(((gs232_server_t*)server_vars)->led);

        /* Blocks here for new connection */
        err = netconn_accept(conn, &newconn);

        ((gs232_server_t*)server_vars)->connected = true;
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
    chThdCreateStatic(wa_az_server, sizeof(wa_az_server), NORMALPRIO, gs232_server, (void*)&tcp_gs232_az);
    chThdCreateStatic(wa_el_server, sizeof(wa_el_server), NORMALPRIO, gs232_server, (void*)&tcp_gs232_el);
}