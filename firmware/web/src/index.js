document.getElementById("compass-needle").style.display = 'none';
update_status();

function update_status()
{
    json_req('/api/status',
        function(data)
        {
            document.getElementById("tracking-az-deg-desired-span").textContent = (data.desired_az_ddeg / 10).toFixed(1);
            document.getElementById("tracking-az-deg-span").textContent = (data.az_ddeg / 10).toFixed(1);
            if(data.cw == 1)
            {
                document.getElementById("tracking-cw-span").style.color = '#000';
            }
            else
            {
                document.getElementById("tracking-cw-span").style.color = '#888';
            }
            if(data.ccw == 1)
            {
                document.getElementById("tracking-ccw-span").style.color = '#000';
            }
            else
            {
                document.getElementById("tracking-ccw-span").style.color = '#888';
            }
            
            document.getElementById("compass-needle").setAttribute('transform','rotate('+(data.az_ddeg / 10)+' 50 50)');
            document.getElementById("compass-needle").style.display = '';

            console.log("Raw: AZ: "+data.az_raw);

            if('el_ddeg' in data)
            {
                document.getElementById("tracking-el-deg-span").textContent = (data.el_ddeg / 10).toFixed(1);
                document.getElementById("tracking-up-span").textContent = data.up;
                document.getElementById("tracking-down-span").textContent = data.down;
                document.getElementById("tracking-el-deg-desired-span").textContent = (data.desired_el_ddeg / 10).toFixed(1);

                console.log("Raw: EL: "+data.el_raw);
            }

            setTimeout(update_status, 500);
        },
        function(error_status)
        {
            document.getElementById("tracking-az-raw-span").textContent = "Error in request";

            setTimeout(update_status, 500);
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


function submit_bearing(desired_bearing)
{
    var password = document.getElementById("password-input").value;

    // Check for empty password input
    if(password == "")
    {
        document.getElementById("submit-status-span").textContent = "Error: Password input empty.";
        return;
    }

    var request = new XMLHttpRequest();
    request.open('POST', '/new_bearing', true);
    request.onload = function()
    {
      if (this.status == 403)
      {
        document.getElementById("submit-status-span").textContent = "Error: Password incorrect.";
      }
      else if(this.status != 200)
      {
        console.log("submit_bearing: Unknown response: "+this.status);
      }
    };
    request.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded; charset=UTF-8');
    request.send(encodeURI('bearing='+String(desired_bearing)+'&password='+String(password)));

    document.getElementById("submit-status-span").textContent = "Command sent.";
}

document.getElementById("submit-button").onclick = function()
{
    var desired_bearing = parseInt(document.getElementById("bearing-input").value);

    // Check for bearing input validity
    if(desired_bearing < 0 || desired_bearing > 360)
    {
        document.getElementById("submit-status-span").textContent = "Error: Bearing input invalid (0=<x<=360.";
        return;
    }

    submit_bearing(desired_bearing);
};