#include "../main.h"
#include "usb/usbcfg.h"

/**** USB SHELL ****/
bool network_dynamic = true;

#define SHELL_WA_SIZE   THD_WORKING_AREA_SIZE(2048)

static void cmd_version(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;

  if (argc > 0)
  {
    chprintf(chp, "Usage: version\r\n");
    return;
  }

  chprintf(chp, "Phil M0DNY Ethernet Rotator Controller\r\n");
  #ifdef __DATE__
  #ifdef __TIME__
    chprintf(chp, "Build time:  %s%s%s"SHELL_NEWLINE_STR, __DATE__, " ", __TIME__);
  #endif
  #endif
  #ifdef PORT_COMPILER_NAME
    chprintf(chp, "Compiler:    %s"SHELL_NEWLINE_STR, PORT_COMPILER_NAME);
  #endif
  #ifdef CH_KERNEL_VERSION
    chprintf(chp, "Kernel:      ChibiOS/RT %s"SHELL_NEWLINE_STR, CH_KERNEL_VERSION);
  #endif
}

static void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[]) {
  size_t n, size, largest;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: mem\r\n");
    return;
  }
  n = chHeapStatus(NULL, &size, &largest);
  chprintf(chp, "core free memory : %u bytes\r\n", chCoreGetStatusX());
  chprintf(chp, "heap fragments   : %u\r\n", n);
  chprintf(chp, "heap free total  : %u bytes\r\n", size);
  chprintf(chp, "heap free largest: %u bytes\r\n", largest);

}

static void cmd_top(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;
  static const char *states[] = {CH_STATE_NAMES};
  #if CH_DBG_THREADS_PROFILING
    uint64_t busy = 0, total = 0;
  #endif
  thread_t *tp;

  if (argc > 0) {
    chprintf(chp, "Usage: top\r\n");
    return;
  }

  chprintf(
    chp, "name        |addr    |stack   |free|prio|refs|state    |time\r\n");
  chprintf(
    chp, "------------|--------|--------|----|----|----|---------|--------\r\n");
  tp = chRegFirstThread();
  do {
    chprintf(chp, "%12s|%.8lx|%.8lx|%4lu|%4lu|%4lu|%9s|%lu\r\n",
            chRegGetThreadNameX(tp),
            (uint32_t)tp,
            (uint32_t)tp->ctx.sp,
            (uint32_t)tp->ctx.sp - (uint32_t)tp->wabase,
            (uint32_t)tp->prio, (uint32_t)(tp->refs - 1),
            states[tp->state],
            #if CH_DBG_THREADS_PROFILING
              (uint32_t)tp->time
            #else
              0
            #endif
            );
    #if CH_DBG_THREADS_PROFILING
      if(tp->prio != 1) {
          busy += tp->time;
      }
      total += tp->time;
    #endif
    tp = chRegNextThread(tp);
  } while (tp != NULL);
  #if CH_DBG_THREADS_PROFILING
    chprintf(chp, "CPU Usage: %ld%%\r\n", busy*100/total);
  #endif
}

static void cmd_ip(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;

  if (argc == 0)
  {
    chprintf(chp, "Usage: ip\r\n");
    chprintf(chp, "       ip [static/dynamic]\r\n");
    chprintf(chp, "       ip address <x.x.x.x>\r\n");
    chprintf(chp, "Link is %s\r\n", (netif_default->flags & NETIF_FLAG_LINK_UP)?"UP":"DOWN");
    chprintf(chp, "Mode is %s\r\n", (network_dynamic)?"DYNAMIC":"STATIC");
    chprintf(chp, "IP: %d.%d.%d.%d\r\n",
        netif_default->ip_addr.addr & 0xFF,
        (netif_default->ip_addr.addr >> 8) & 0xFF,
        (netif_default->ip_addr.addr >> 16) & 0xFF,
        (netif_default->ip_addr.addr >> 24) & 0xFF);
  }
  else if (argc == 1)
  {
    if(0==strcmp(argv[0], "static"))
    {
      network_dynamic = false;
      chprintf(chp, "Link is %s\r\n", (netif_default->flags & NETIF_FLAG_LINK_UP)?"UP":"DOWN");
      chprintf(chp, "Mode is %s\r\n", (network_dynamic)?"DYNAMIC":"STATIC");
      chprintf(chp, "IP: %d.%d.%d.%d\r\n",
          netif_default->ip_addr.addr & 0xFF,
          (netif_default->ip_addr.addr >> 8) & 0xFF,
          (netif_default->ip_addr.addr >> 16) & 0xFF,
          (netif_default->ip_addr.addr >> 24) & 0xFF);
    }
    else if(0==strcmp(argv[0], "dynamic"))
    {
      network_dynamic = true;
      chprintf(chp, "Link is %s\r\n", (netif_default->flags & NETIF_FLAG_LINK_UP)?"UP":"DOWN");
      chprintf(chp, "Mode is %s\r\n", (network_dynamic)?"DYNAMIC":"STATIC");
      chprintf(chp, "IP: %d.%d.%d.%d\r\n",
          netif_default->ip_addr.addr & 0xFF,
          (netif_default->ip_addr.addr >> 8) & 0xFF,
          (netif_default->ip_addr.addr >> 16) & 0xFF,
          (netif_default->ip_addr.addr >> 24) & 0xFF);
    }
    else
    {
      chprintf(chp, "Usage: ip\r\n");
      chprintf(chp, "       ip [static/dynamic]\r\n");
      chprintf(chp, "       ip address <x.x.x.x>\r\n");
    }
  }
  else if (argc == 2 && 0==strcmp(argv[0], "address"))
  {
    ip_addr_t new_ip;
    if(ipaddr_aton(argv[1], &new_ip))
    {
      netif_set_ipaddr(netif_default, &new_ip);
      chprintf(chp, "IP set!\r\n");
      chprintf(chp, "Link is %s\r\n", (netif_default->flags & NETIF_FLAG_LINK_UP)?"UP":"DOWN");
      chprintf(chp, "Mode is %s\r\n", (network_dynamic)?"DYNAMIC":"STATIC");
      chprintf(chp, "IP: %d.%d.%d.%d\r\n",
          netif_default->ip_addr.addr & 0xFF,
          (netif_default->ip_addr.addr >> 8) & 0xFF,
          (netif_default->ip_addr.addr >> 16) & 0xFF,
          (netif_default->ip_addr.addr >> 24) & 0xFF);
    }
    else
    {
      chprintf(chp, "IP not recognised, try again.\r\n");
      chprintf(chp, "       ip address <x.x.x.x>\r\n");
    }
  }
  else
  {
    chprintf(chp, "Usage: ip\r\n");
    chprintf(chp, "       ip [static/dynamic]\r\n");
    chprintf(chp, "       ip address <x.x.x.x>\r\n");
  }
}

static void cmd_info(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;
  if (argc > 0)
  {
    chprintf(chp, "Usage: info\r\n");
    return;
  }
  chprintf(chp, "Setup Information:\r\n");
  chprintf(chp, "  pstrotator\r\n");
  chprintf(chp, "    * Setup -> AZ Controller -> GS232 Yaesu 360 deg - Az\r\n");
  chprintf(chp, "    * Setup -> EL / AZ+EL Controller -> GS232 Yaesu 360 deg\r\n");
  chprintf(chp, "    * Communication -> TCP Client\r\n");
  chprintf(chp, "      - IP:            [controller ip address]\r\n");
  chprintf(chp, "      - Port Az:       4001\r\n");
  chprintf(chp, "      - Port El/Az+El: 4002\r\n");
}

static const ShellCommand commands[] = {
  {"version", cmd_version},
  {"top", cmd_top},
  {"mem", cmd_mem},
  {"ip", cmd_ip},
  {"info", cmd_info},
  {NULL, NULL}
};

static const ShellConfig shell_cfg1 = {
  (BaseSequentialStream *)&SDU2,
  commands
};

static thread_t *shelltp = NULL;

THD_FUNCTION(usb_thread, arg)
{
  (void)arg;
  chRegSetThreadName("usb_shell");

  sduObjectInit(&SDU2);
  sduStart(&SDU2, &serusbcfg);

  /*
   * Activates the USB driver and then the USB bus pull-up on D+.
   * Note, a delay is inserted in order to not have to disconnect the cable
   * after a reset.
   */
  usbDisconnectBus(serusbcfg.usbp);
  chThdSleepMilliseconds(1500);
  usbStart(serusbcfg.usbp, &usbcfg);
  usbConnectBus(serusbcfg.usbp);

  /*
   * Shell manager initialization.
   */
  shellInit();

  while(1)
  {

    if (!shelltp && (SDU2.config->usbp->state == USB_ACTIVE)) {
      shelltp = chThdCreateFromHeap(NULL, SHELL_WA_SIZE,
                                    "shell", NORMALPRIO + 1,
                                    shellThread, (void *)&shell_cfg1);
    }
    chThdSleepMilliseconds(10);
  }
}

THD_WORKING_AREA(wa_usb_shell, USB_SHELL_STACK_SIZE);
void usb_shell_init(void)
{
  chThdCreateStatic(wa_usb_shell, sizeof(wa_usb_shell), NORMALPRIO, usb_thread, NULL);
}