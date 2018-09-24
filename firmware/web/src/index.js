setInterval(function()
{
    update_status();
},
500);

function update_status()
{
    json_req('/api/status',
        function(data)
        {
            document.getElementById("tracking-az-raw-span").textContent = data.az_raw;
            document.getElementById("tracking-el-raw-span").textContent = data.el_raw;
            document.getElementById("tracking-az-deg-span").textContent = (data.az_ddeg / 10);
            document.getElementById("tracking-el-deg-span").textContent = (data.el_ddeg / 10);
            document.getElementById("tracking-up-span").textContent = data.up;
            document.getElementById("tracking-down-span").textContent = data.down;
            document.getElementById("tracking-cw-span").textContent = data.cw;
            document.getElementById("tracking-ccw-span").textContent = data.ccw;
            document.getElementById("tracking-az-deg-desired-span").textContent = (data.desired_az_ddeg / 10);
            document.getElementById("tracking-el-deg-desired-span").textContent = (data.desired_el_ddeg / 10);
        },
        function(error_status)
        {
            document.getElementById("tracking-az-raw-span").textContent = "Error in request";
        }
    );
}

function json_req(url,success_cb,fail_cb)
{
    var request = new XMLHttpRequest();
    request.open('GET', url, true);
    request.onload = function()
    {
      if (this.status >= 200 && this.status < 400)
      {
        var data = JSON.parse(this.response.replace(/\n/g, "&#10;").replace(/\r/g, ""));
        success_cb(data);
      }
      else
      {
        fail_cb(this.status);
      }
    };
    request.onerror = fail_cb;
    request.send();
}