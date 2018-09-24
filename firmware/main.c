#include "main.h"

device_status_t device_status = { .lan_up = false };

static THD_WORKING_AREA(wa_led_blinker, 128);
static THD_FUNCTION(led_blinker, arg)
{
  (void) arg;

  chRegSetThreadName("blinker");

  while (true)
  {
    /* Green LED */
    palToggleLine(LINE_LED1);

    device_status.lan_up = (bool)netif_is_link_up(netif_default);

    /* Blue LED */
    if(device_status.lan_up)
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

  uint8_t macaddr[6] = { 0xC2, 0xAF, 0x51, 0x03, 0xCF, 0x46 };
  lwipthread_opts_t netopts = {
    .macaddress = macaddr,
    .address = IP4_ADDR_VALUE(192,168,100,80),
    .netmask = IP4_ADDR_VALUE(255,255,255,0),
    .gateway = IP4_ADDR_VALUE(192,168,100,1),
    .addrMode = NET_ADDRESS_DHCP,
    .ourHostName = "g5500"
  };
  lwipInit(&netopts);

  chThdCreateStatic(wa_led_blinker, sizeof(wa_led_blinker), NORMALPRIO - 1, led_blinker, NULL);

  tracking_init();

  gs232_init();

  usb_shell_init();

  /*
  * Creates the HTTP thread (it changes priority internally).
  */
  chThdCreateStatic(wa_http_server, sizeof(wa_http_server), NORMALPRIO + 1,
                    http_server, NULL);
      
  while (true)
  {
    chThdSleepMilliseconds(200);
  }

  return 0;
}
