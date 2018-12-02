#include "main.h"

int main(void)
{
  halInit();
  chSysInit();

  settings_init();

  lwipInit(NULL);

  tracking_init();

  //gs232_init();

  //usb_shell_init();

  web_init();
      
  while (true)
  {
    chThdSleepMilliseconds(2000);
  }

  return 0;
}
