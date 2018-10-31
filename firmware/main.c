#include "main.h"

int main(void)
{
  halInit();
  chSysInit();
  lwipInit(NULL);

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
