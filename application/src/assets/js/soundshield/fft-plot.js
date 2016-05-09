/*var socket_lm;
var amp_data = [];
var msgBuffer;

  if (typeof MozWebSocket != "undefined") {
    socket_lm = new MozWebSocket(get_appropriate_ws_url(),
           "lws-mirror-protocol");
  } else {
    socket_lm = new WebSocket('ws://127.0.0.1:7681',
           "lws-mirror-protocol");
  }

  socket_lm.onerror = function() {
    socket_lm = new WebSocket('ws://10.0.1.10:7681',
            "lws-mirror-protocol");
  }

  socket_lm.onopen = function() {
      console.log("FFT Socket Opened");
    } 

  socket_lm.onmessage = function got_packet(msg) {
        msgBuffer = msg.data.split(":"); 

        for(var i = 0; i < 256; i++) {
            amp_data[i] = parseFloat(msgBuffer[i]);
        }
      
  }

var data = [[],[]];
var time = [[],[]];
totalPoints = 200;

var updateInterval = 120;
var now = new Date().getTime();
var now2 = new Date().getTime();

var data2 = [[1,1,1,1,1,1,1],[1,1,1,1,1,1,1]];
var time2 = [[],[]];

var temp = [[],[]];

function getAmplitudeData() {
    var res = [[],[]];
    data[0].shift();
    data[1].shift();
    time[0].shift();
    time[1].shift();

    while (data[0].length < totalPoints) {
        now += updateInterval;
        var prev = data[0].length > 0 ? data[0][data[0].length - 1] : 0,
        //y = prev + amp_data;
        y = amp_data;

        if (y < -1) {
            y = -1;
        } else if (y > 1) {
            y = 1;
        }

        //console.log(y);

        data[0].push(y);
        data[1].push(y);

        time[0].push(now);
        time[1].push(now);
    }

    for (var i = 0; i < data[0].length; ++i) {
        res[0].push([time[0][i], data[0][i]]);
        res[1].push([time[1][i], data[1][i]]);
    }

    return res;
}

var options = {
    series: {
        shadowSize: 0   // Drawing is faster without shadows
    },
    yaxis: {
        min: -1.0,
        max: 1.0
    },
    xaxis: {
        mode: "time",
        tickSize: [2, "second"],
        tickFormatter: function (v, axis) {
            var date = new Date(v);

            if (date.getSeconds() % 20 === 0) {
                var hours = date.getHours() < 10 ? "0" + date.getHours() : date.getHours();
                var minutes = date.getMinutes() < 10 ? "0" + date.getMinutes() : date.getMinutes();
                var seconds = date.getSeconds() < 10 ? "0" + date.getSeconds() : date.getSeconds();

                return hours + ":" + minutes + ":" + seconds;
            } else {
                return "";
            }
        },
        axisLabel: "Time",
        axisLabelUseCanvas: true,
        axisLabelFontSizePixels: 12,
        axisLabelFontFamily: 'Verdana, Arial',
        axisLabelPadding: 10          
    }
}; 


$(document).ready(function() {
    var plot1 = $.plot("#flot-line-chart-moving",  getAmplitudeData(), options);

    //temp = getAmplitudeData();
    var plot2 = $.plot("#psd-chart-moving", [{
            data: temp[0],
            bars: {show: true }
        }, {
            data: temp[1],
            bars: {show: true }
        }]);
        

    function update() {      
        
        plot1.setupGrid();
        plot2.setupGrid();
        plot1 = $.plot("#flot-line-chart-moving",  getAmplitudeData(), options);
        // Since the axes don't change, we don't need to call 
        
        //temp = getAmplitudeData();
        plot2 = $.plot("#psd-chart-moving", [{
            data: temp[0],
            bars: { show: true,
                    align: "center",
                    fill: true
                    }
        }, {
            data: temp[1],
            bars: { show: true,
                    align: "center",
                    fill: true
                    }
        }]);

        setTimeout(update, updateInterval);
    }

    update();

    // Add the Flot version string to the footer

    $("#footer").prepend("Flot " + $.plot.version + " &ndash; ");
});
*/
    
    