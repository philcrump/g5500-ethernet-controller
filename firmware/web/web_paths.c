#include "../main.h"

static char http_response[4096];

/* 403 - Forbidden */
static const char http_403_json_hdr[] = "HTTP/1.0 HTTP/1.0 403 Forbidden\r\nContent-type: application/javascript\r\n\r\n";
//static const char http_403_hdr[] = "HTTP/1.0 HTTP/1.0 403 Forbidden\r\nContent-type: text/html\r\n\r\n";
//static const char http_403_body[] = "<html><body><h4>Forbidden</h4></body></html>";

/* 404 - File Not Found */
static const char http_404_hdr[] = "HTTP/1.0 404 Not Found\r\nContent-type: text/html\r\n\r\n";
static const char http_404_body[] = "<html><body><h4>Path not found</h4></body></html>";
static void web_path_404(struct netconn *conn);

/* HTML */
//static const char http_html_hdr[] = "HTTP/1.0 200 OK\r\nContent-type: text/html\r\n\r\n";
static const char http_html_gz_hdr[] = "HTTP/1.0 200 OK\r\nContent-Encoding: gzip\r\nContent-type: text/html\r\n\r\n";
#include "dist/index_html_gz.h"
static void web_path_index_html(struct netconn *conn);

/* CSS */
//static const char http_css_hdr[] = "HTTP/1.0 200 OK\r\nContent-type: text/css\r\n\r\n";
//static const char http_css_gz_hdr[] = "HTTP/1.0 200 OK\r\nContent-Encoding: gzip\r\nContent-type: text/css\r\n\r\n";

/* Javascript */
//static const char http_javascript_hdr[] = "HTTP/1.0 200 OK\r\nContent-type: application/javascript\r\n\r\n";
static const char http_javascript_gz_hdr[] = "HTTP/1.0 200 OK\r\nContent-Encoding: gzip\r\nContent-type: application/javascript\r\n\r\n";
#include "dist/index_js_gz.h"
static void web_path_index_js(struct netconn *conn);

/* JSON API */
static const char http_json_hdr[] = "HTTP/1.0 200 OK\r\nContent-type: application/json\r\n\r\n";
static void web_path_api_status(struct netconn *conn);

/* PNG Image */
static const char http_png_gz_hdr[] = "HTTP/1.0 200 OK\r\nContent-Encoding: gzip\r\nContent-type: image/png\r\n\r\n";
#include "dist/favicon_png_gz.h"
static void web_path_favicon_png(struct netconn *conn);

/* Binary Files */
//static const char http_binary_hdr[] = "HTTP/1.0 200 OK\r\nContent-Type:application/octet-stream\r\n\r\n";
//static const char http_binary_gz_hdr[] = "HTTP/1.0 200 OK\r\nContent-Encoding: gzip\r\nContent-Type:application/octet-stream\r\n\r\n";

void web_paths_get(struct netconn *conn, char *url_buffer)
{
  if(strcmp("/", url_buffer) == 0 || strcmp("/index.html", url_buffer) == 0)
  {
    web_path_index_html(conn);
  }
  else if(strcmp("/index.js", url_buffer) == 0)
  {
    web_path_index_js(conn);
  }
  else if(strcmp("/favicon.png", url_buffer) == 0)
  {
    web_path_favicon_png(conn);
  }
  else if(strcmp("/api/status", url_buffer) == 0)
  {
    web_path_api_status(conn);
  }
  else
  {
    web_path_404(conn);
  }
}

static void web_path_404(struct netconn *conn)
{
  netconn_write(conn, http_404_hdr, sizeof(http_404_hdr)-1, NETCONN_NOCOPY);
  netconn_write(conn, http_404_body, sizeof(http_404_body), NETCONN_NOCOPY);
}

static void web_path_favicon_png(struct netconn *conn)
{
  netconn_write(conn, http_png_gz_hdr, sizeof(http_png_gz_hdr)-1, NETCONN_NOCOPY);
  netconn_write(conn, favicon_png_gz, favicon_png_gz_len, NETCONN_NOCOPY);
}

static void web_path_index_html(struct netconn *conn)
{
  netconn_write(conn, http_html_gz_hdr, sizeof(http_html_gz_hdr)-1, NETCONN_NOCOPY);
  netconn_write(conn, index_html_gz, index_html_gz_len, NETCONN_NOCOPY);
}

static void web_path_index_js(struct netconn *conn)
{
  netconn_write(conn, http_javascript_gz_hdr, sizeof(http_javascript_gz_hdr)-1, NETCONN_NOCOPY);
  netconn_write(conn, index_js_gz, index_js_gz_len, NETCONN_NOCOPY);
}

static void web_path_api_status(struct netconn *conn)
{
  int n;

  netconn_write(conn, http_json_hdr, sizeof(http_json_hdr)-1, NETCONN_NOCOPY);

  n = snprintf(http_response, 4096,
    "{"
      "\"az_raw\":%d,"
      "\"az_ddeg\":%d,"
      "\"cw\":%d,"
      "\"ccw\":%d,"
      "\"desired_az_ddeg\":%d"
#ifdef ELEVATION_ENABLED
      ",\"el_raw\":%d,"
      "\"el_ddeg\":%d,"
      "\"up\":%d,"
      "\"down\":%d,"
      "\"desired_el_ddeg\":%d"
#endif
    "}",
    tracking_state.az_raw,
    tracking_state.az_ddeg,
    tracking_state.cw,
    tracking_state.ccw,
    tracking_state.desired_az_ddeg
#ifdef ELEVATION_ENABLED
    ,tracking_state.el_raw,
    tracking_state.el_ddeg,
    tracking_state.up,
    tracking_state.down,
    tracking_state.desired_el_ddeg
#endif
  );

  netconn_write(conn, http_response, n, NETCONN_NOFLAG);
}

#ifndef WEBPASSWORD
 #error "Must define WEBPASSWORD"
 #define WEBPASSWORD  ""
#endif

static void web_path_new_bearing(struct netconn *conn, char *postbody_buffer)
{
  int n;
  char *bearing_ptr;
  char *password_ptr;
  int desired_bearing;

  /*** Check Password ***/

  password_ptr = strstr(postbody_buffer, "password=");
  if(password_ptr == NULL)
  {
    n = snprintf(http_response, 4096,
      "{"
        "\"error\":\"Failed to parse request (password not found)\""
      "}"
    );
    netconn_write(conn, http_json_hdr, sizeof(http_json_hdr)-1, NETCONN_NOCOPY);
    netconn_write(conn, http_response, n, NETCONN_NOFLAG);
    return;
  }

  if(0 != strncmp(password_ptr + strlen("password="), WEBPASSWORD, strlen(WEBPASSWORD)))
  {
    n = snprintf(http_response, 4096,
      "{"
        "\"error\":\"Incorrect password!\""
      "}"
    );
    netconn_write(conn, http_403_json_hdr, sizeof(http_403_json_hdr)-1, NETCONN_NOCOPY);
    netconn_write(conn, http_response, n, NETCONN_NOFLAG);
    return;
  }

  /*** Check bearing ***/

  bearing_ptr = strstr(postbody_buffer, "bearing=");
  if(bearing_ptr == NULL)
  {
    n = snprintf(http_response, 4096,
      "{"
        "\"error\":\"Failed to parse request (bearing not found)\""
      "}"
    );
    netconn_write(conn, http_json_hdr, sizeof(http_json_hdr)-1, NETCONN_NOCOPY);
    netconn_write(conn, http_response, n, NETCONN_NOFLAG);
    return;
  }

  desired_bearing = strtol(bearing_ptr + strlen("bearing="), NULL, 10);

  if(desired_bearing < 0 || desired_bearing > 360)
  {
    n = snprintf(http_response, 4096,
      "{"
        "\"error\":\"Bearing out of range\","
        "\"new_bearing\":%d"
      "}"
      , desired_bearing
    );
    netconn_write(conn, http_json_hdr, sizeof(http_json_hdr)-1, NETCONN_NOCOPY);
    netconn_write(conn, http_response, n, NETCONN_NOFLAG);
    return;
  }

  /*** Success! Set bearing ***/

  tracking_state.desired_az_deg = desired_bearing;
  tracking_state.desired_az_ddeg = tracking_state.desired_az_deg * 10;
  
  n = snprintf(http_response, 4096,
    "{"
      "\"new_bearing\":%d"
    "}",
    tracking_state.desired_az_deg
  );

  netconn_write(conn, http_json_hdr, sizeof(http_json_hdr)-1, NETCONN_NOCOPY);
  netconn_write(conn, http_response, n, NETCONN_NOFLAG);
}

void web_paths_post(struct netconn *conn, char *url_buffer, char *postbody_buffer)
{
  if(strcmp("/new_bearing",url_buffer) == 0)
  {
    web_path_new_bearing(conn, postbody_buffer);
  }
  else
  {
    web_path_404(conn);
  }
}