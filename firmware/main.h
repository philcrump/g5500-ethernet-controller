#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdio.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "shell.h"
#include "chprintf.h"

#include "lwipthread.h"
#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/sys.h"
#include "lwip/netif.h"

#include "gs232/gs232.h"
#include "tracking/tracking.h"
#include "usb/usb_shell.h"
#include "web/web.h"

typedef struct device_status_t {
	bool lan_up;
} device_status_t;

extern device_status_t device_status;

#endif /* __MAIN_H__ */