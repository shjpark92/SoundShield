var socket_lm;
var amp_data = 0;

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
      amp_data = msg;
      console.log("AMP Value = " + msg);
      console.log("FFT socket OPENED!!");
  }

var data = [[],[]];
var time = [[],[]];
totalPoints = 300;

var updateInterval = 60;
var now = new Date().getTime();
var now2 = new Date().getTime();

var data2 = [[1,1,1,1,1,1,1],[1,1,1,1,1,1,1]];
var time2 = [[],[]];

var temp = [[],[]];


function getRealData() {

    var res = [[],[]];
    data[0].shift();
    data[1].shift();
    time[0].shift();
    time[1].shift();

    while (data[0].length < totalPoints) {

        now += updateInterval;
        var prev = data[0].length > 0 ? data[0][data[0].length - 1] : 50,
        y = prev + Math.random() * 10 - 5;

        if (y < 0) {
            y = 0;
        } else if (y > 100) {
            y = 100;
        }

        data[0].push(y);
        data[1].push(y/2);

        time[0].push(now);
        time[1].push(now);
    }

    for (var i = 0; i < data[0].length; ++i) {
        res[0].push([time[0][i], data[0][i]]);
        res[1].push([time[1][i], data[1][i]]);
    }

    return res;
}

function getRandomData() {

    var res = [[],[]];
    data[0].shift();
    data[1].shift();
    time[0].shift();
    time[1].shift();

    while (data[0].length < totalPoints) {

        now += updateInterval;
        var prev = data[0].length > 0 ? data[0][data[0].length - 1] : 50,
        y = prev + Math.random() * 10 - 5;

        if (y < 0) {
            y = 0;
        } else if (y > 100) {
            y = 100;
        }

        data[0].push(y);
        data[1].push(y/2);

        time[0].push(now);
        time[1].push(now);
    }

    for (var i = 0; i < data[0].length; ++i) {
        res[0].push([time[0][i], data[0][i]]);
        res[1].push([time[1][i], data[1][i]]);
    }

    return res;
}

function getRandomData2() {

    var res2 = [[],[]];

    for (var i = 0; i < 10; i++) {

        data2[0][i] = Math.random();
        data2[1][i] = Math.random();

    }

    // Zip the generated y values with the x values

    for (var i = 0; i < 10; ++i) {
        res2[0].push([i, data2[0][i]]);
        res2[1].push([i, data2[1][i]]);
    }

    return res2;
}

var options = {
    series: {
        shadowSize: 0   // Drawing is faster without shadows
    },
    yaxis: {
        min: 100,
        max: 0
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

    var plot1 = $.plot("#flot-line-chart-moving",  getRealData(), options);

    temp = getRandomData2();
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
        plot1 = $.plot("#flot-line-chart-moving",  getRealData(), options);
        // Since the axes don't change, we don't need to call 
        
        temp = getRandomData2();
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

    
    