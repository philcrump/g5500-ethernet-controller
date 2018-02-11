#include <stdio.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "lwipthread.h"
#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/tcp.h"
#include "lwip/netif.h"
#include "lwip/sys.h"
#include "lwip/udp.h"

#include "main.h"
#include "usb/usb_shell.h"
#include "tracking/tracking.h"
#include "gs232/gs232.h"

/* For debugging only */
void udp_debug_msg(char *message)
{
    struct pbuf *p;
    struct udp_pcb *pcb;
    pcb = udp_new();
    ip_addr_t udp_target;

    IP4_ADDR(&udp_target, 192,168,100,190);

    p = pbuf_alloc(PBUF_TRANSPORT,strlen(message),PBUF_RAM);
    memcpy (p->payload, message, strlen(message));
    udp_sendto(pcb, p, &udp_target, 7777);
    pbuf_free(p);
}

static THD_WORKING_AREA(wa_led_blinker, 128);
static THD_FUNCTION(led_blinker, arg)
{
  (void) arg;

  chRegSetThreadName("blinker");


  while (true)
  {
    /* Green LED */
    palToggleLine(LINE_LED1);

    /* Blue LED */
    if(netif_default->flags & NETIF_FLAG_LINK_UP)
    {
      chThdSleepMilliseconds(100);
    }
    else
    {
      chThdSleepMilliseconds(500);
    }
  }
}

int main(void)
{
  halInit();
  chSysInit();
  palSetLineMode(LINE_LED1, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLineMode(LINE_LED2, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLineMode(LINE_LED3, PAL_MODE_OUTPUT_PUSHPULL);
  lwipInit(NULL);

  chThdCreateStatic(wa_led_blinker, sizeof(wa_led_blinker), NORMALPRIO - 1, led_blinker, NULL);

  tracking_init();

  gs232_init();

  usb_shell_init();
      
  while (true)
  {
    chThdSleepMilliseconds(200);
  }

  return 0;
}
