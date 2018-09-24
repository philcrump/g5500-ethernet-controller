#include "../main.h"

static char http_response[4096];

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

/* Binary Files */
//static const char http_binary_hdr[] = "HTTP/1.0 200 OK\r\nContent-Type:application/octet-stream\r\n\r\n";
//static const char http_binary_gz_hdr[] = "HTTP/1.0 200 OK\r\nContent-Encoding: gzip\r\nContent-Type:application/octet-stream\r\n\r\n";

void web_paths(struct netconn *conn, char *url_buffer)
{
  if(strcmp("/",url_buffer) == 0 || strcmp("/index.html",url_buffer) == 0)
  {
    web_path_index_html(conn);
  }
  else if(strcmp("/index.js",url_buffer) == 0)
  {
    web_path_index_js(conn);
  }
  else if(strcmp("/api/status",url_buffer) == 0)
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

static void web_path_index_html(struct netconn *conn)
{
  netconn_write(conn, http_html_gz_hdr, sizeof(http_html_gz_hdr)-1, NETCONN_NOCOPY);
  netconn_write(conn, index_html_gz,  index_html_gz_len, NETCONN_NOCOPY);
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
      "\"el_raw\":%d,"
      "\"az_ddeg\":%d,"
      "\"el_ddeg\":%d,"
      "\"up\":%d,"
      "\"down\":%d,"
      "\"cw\":%d,"
      "\"ccw\":%d,"
      "\"desired_az_ddeg\":%d,"
      "\"desired_el_ddeg\":%d"
    "}",
    tracking_state.az_raw,
    tracking_state.el_raw,
    tracking_state.az_ddeg,
    tracking_state.el_ddeg,
    tracking_state.up,
    tracking_state.down,
    tracking_state.cw,
    tracking_state.ccw,
    tracking_state.desired_az_ddeg,
    tracking_state.desired_el_ddeg
  );

  netconn_write(conn, http_response, n, NETCONN_NOFLAG);
}