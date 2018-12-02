/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/*
 * This file is a modified version of the lwIP web server demo. The original
 * author is unknown because the file didn't contain any license information.
 */

/**
 * @file web.c
 * @brief HTTP server wrapper thread code.
 * @addtogroup WEB_THREAD
 * @{
 */

#include "../main.h"

#if LWIP_NETCONN

static char url_buffer[WEB_MAX_PATH_SIZE];
static char postbody_buffer[WEB_MAX_POSTBODY_SIZE];
/**
 * @brief   Decodes an URL sting.
 * @note    The string is terminated by a zero or a separator.
 *
 * @param[in] url       encoded URL string
 * @param[out] buf      buffer for the processed string
 * @param[in] max       max number of chars to copy into the buffer
 * @return              The conversion status.
 * @retval false        string converted.
 * @retval true         the string was not valid or the buffer overflowed
 *
 * @notapi
 */
#define HEXTOI(x) (isdigit(x) ? (x) - '0' : (x) - 'a' + 10)
static bool decode_url(const char *url, char *buf, size_t max)
{
  while (true) {
    int h, l;
    unsigned char c = *url++;

    switch (c) {
    case 0:
    case '\r':
    case '\n':
    case '\t':
    case ' ':
    case '?':
      *buf = 0;
      return true;
    case '.':
      if (max <= 1)
        return false;

      h = *(url + 1);
      if (h == '.')
        return false;

      break;
    case '%':
      if (max <= 1)
        return false;

      h = tolower((int)*url++);
      if (h == 0)
        return false;
      if (!isxdigit(h))
        return false;

      l = tolower((int)*url++);
      if (l == 0)
        return false;
      if (!isxdigit(l))
        return false;

      c = (char)((HEXTOI(h) << 4) | HEXTOI(l));
      break;
    default:
      if (max <= 1)
        return false;

      if (!isalnum(c) && (c != '_') && (c != '-') && (c != '+') &&
          (c != '/'))
        return false;

      break;
    }

    *buf++ = c;
    max--;
  }
}


static bool decode_body(const char *buf, uint16_t buflen, char *outbuf, size_t max)
{
  uint16_t i, j;

  // Sanity-check length
  if(buflen < 4)
  {
    return false;
  }

  i = 4;

  while(i < buflen)
  {
    if(buf[i-1] == '\n' && buf[i-2] == '\r' && buf[i-3] == '\n' && buf[i-4] == '\r')
    {
      j = 0;
      while((i + j) < buflen && j < (max-1) && buf[i+j] != '\0')
      {
        outbuf[j] = buf[i+j];
        j++;
      }
      outbuf[j] = '\0';
      return true;
    }
    i++;
  }
  return false;
}

static void http_server_serve(struct netconn *conn) {
  struct netbuf *inbuf;
  char *buf;
  u16_t buflen;
  err_t err;

  /* Read the data from the port, blocking if nothing yet there.
   We assume the request (the part we care about) is in one netbuf */
  err = netconn_recv(conn, &inbuf);

  if (err == ERR_OK) {
    netbuf_data(inbuf, (void **)&buf, &buflen);

    /* Is this an HTTP GET command? (only check the first 5 chars, since
    there are other formats for GET, and we're keeping it very simple )*/
    if (buflen>=5 &&
        buf[0]=='G' &&
        buf[1]=='E' &&
        buf[2]=='T' &&
        buf[3]==' ' &&
        buf[4]=='/' ) {

      if (!decode_url(buf + 4, url_buffer, WEB_MAX_PATH_SIZE)) {
        /* Invalid URL handling.*/
        return;
      }

      web_paths_get(conn, url_buffer);
    }
    else if(buflen>=6 &&
        buf[0]=='P' &&
        buf[1]=='O' &&
        buf[2]=='S' &&
        buf[3]=='T' &&
        buf[4]==' ' &&
        buf[5]=='/' ) {

      if (!decode_url(buf + 5, url_buffer, WEB_MAX_PATH_SIZE))
      {
        /* Invalid URL handling.*/
        return;
      }

      if (!decode_body(buf, buflen, postbody_buffer, WEB_MAX_POSTBODY_SIZE))
      {

        return;
      }

      web_paths_post(conn, url_buffer, postbody_buffer);
    }
  }
  /* Close the connection (server closes in HTTP) */
  netconn_close(conn);

  /* Delete the buffer (netconn_recv gives us ownership,
   so we have to make sure to deallocate the buffer) */
  netbuf_delete(inbuf);
}

/**
 * Stack area for the http thread.
 */
THD_WORKING_AREA(wa_http_server, WEB_THREAD_STACK_SIZE);

/**
 * HTTP server thread.
 */
THD_FUNCTION(http_server, p) {
  struct netconn *conn, *newconn;
  err_t err;

  (void)p;
  chRegSetThreadName("http");

  /* Create a new TCP connection handle */
  conn = netconn_new(NETCONN_TCP);
  LWIP_ERROR("http_server: invalid conn", (conn != NULL), chThdExit(MSG_RESET););

  /* Bind to port 80 (HTTP) with default IP address */
  netconn_bind(conn, NULL, WEB_THREAD_PORT);

  /* Put the connection into LISTEN state */
  netconn_listen(conn);

  /* Goes to the final priority after initialization.*/
  chThdSetPriority(WEB_THREAD_PRIORITY);

  while (true) {
    err = netconn_accept(conn, &newconn);
    if (err != ERR_OK)
      continue;
    http_server_serve(newconn);
    netconn_delete(newconn);
  }
}

void web_init(void)
{
  chThdCreateStatic(wa_http_server, sizeof(wa_http_server), NORMALPRIO + 1, http_server, NULL);
}

#endif /* LWIP_NETCONN */

/** @} */
