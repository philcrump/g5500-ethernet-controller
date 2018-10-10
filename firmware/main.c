#include "main.h"

int main(void)
{
  halInit();
  chSysInit();

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

  tracking_init();

  //gs232_init();

  //usb_shell_init();

  /*
  * Creates the HTTP thread (it changes priority internally).
  */
  chThdCreateStatic(wa_http_server, sizeof(wa_http_server), NORMALPRIO + 1, http_server, NULL);
      
  while (true)
  {
    chThdSleepMilliseconds(2000);
  }

  return 0;
}
