var socket_lm;

  if (typeof MozWebSocket != "undefined") {
    socket_lm = new MozWebSocket(get_appropriate_ws_url(),
           "fft-plot-protocol");
  } else {
    socket_lm = new WebSocket('ws://127.0.0.1:7681',
           "fft-plot-protocol");
  }

  socket_lm.onerror = function() {
    socket_lm = new WebSocket('ws://10.0.1.10:7681',
            "fft-plot-protocol");
  }

  socket_lm.onopen = function() {
      console.log("on open called");
  } 

  socket_lm.onmessage = function got_packet(msg) {
      console.log(msg);
      var arrMsgBuffer = msg.data.split("!!");
      msgBuffer = arrMsgBuffer[0].split(":"); 
      var msgBuffer2 = arrMsgBuffer[1].split(":");

      for(var i = 0; i < 2048; i++) {
          amp_data[i] = parseFloat(msgBuffer[i]);
          out_amp_data[i] = parseFloat(msgBuffer2[i]);
      }
  }

$(function() {
   $("#calib").button( "option", "disabled", true);
   $("#reset").button( "option", "disabled", true);
   $("#mute").button( "option", "disabled", true);
   $("#volUp").button( "option", "disabled", true);
   $("#volDown").button( "option", "disabled", true);
   $("#timer").button( "option", "disabled", true);
   $("#smaster, #s0, #s1, #s2, #s3, #s4, #s5, #s6, #s7, #s8, #s9").slider({
      orientation: "vertical",
      range: "min",
	    min: 0,
      max: 0.99,
      value: 0.3,
      step:0.01,
      animate: "slow",
      slide: function( event, ui ) { sliderChange(event.target.id) },
      change: function( event, ui ) { sliderChange(event.target.id) }
    });
    $("#reset")
      .button()
	  .click(function() { resetMod(); });
	  $("#mute")
      .button()
	  .click(function() { toggleMute(event.target.id); });
	  $("#volUp")
      .button()
	  .click(function() { volUp(); });
	  $("#volDown")
      .button()
	  .click(function() { volDown(); });
	  $("#timer")
    	.button()
    	.click(function() { setTimer(); });

  function resetMod() {
      var barNames = ["smaster", "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9"];
      var volumeBar = document.getElementsByClassName("ui-slider-range ui-widget-header ui-corner-all ui-slider-range-min");
      var volumeButton = document.getElementsByClassName("ui-slider-handle ui-state-default ui-corner-all");
      var bar_count = 11;

      for(var bar = 0; bar < bar_count; ++bar) {
        var payload = barNames[bar] + ":" + "30";
        socket_lm.send(payload);
        volumeButton[bar].style.height = "30%";
        volumeButton[bar].style.bottom = "30%";
        volumeBar[bar].style.height = "30%";
      }
  }

  function toggleMute(muteButton) {
      var muteStatus = document.getElementById(muteButton);
      var muteAttribute = muteStatus.attributes;

      if(muteAttribute.value == true) {
        muteAttribute.value = false;

        var volume = document.getElementById("smaster").firstChild.scrollHeight;
        volume = (volume / 118) * 100;

        var payload = "smaster:" + volume;
        socket_lm.send(payload);
      }
      else {
        muteAttribute.value = true;
        var payload = "smaster:0";
        socket_lm.send(payload);
        amp_data = [ 0 ];
      }
  }

    function sliderChange(movedSliderName) {
        var targetSlider = document.getElementById(movedSliderName);
        var targetSliderVolume = targetSlider.firstChild.scrollHeight;
        targetSliderVolume = (targetSliderVolume / 118) * 100;
        var targetSliderID = targetSlider.id;

        console.log(targetSliderID);
        console.log(targetSliderVolume);

        var payload = targetSliderID + ":" + targetSliderVolume;

        console.log(payload);
        socket_lm.send(payload);

        socket_lm.onclose = function(){
            console.log("closed");
        }
    }
});

$(function() {
  $('.actionlink,.blue,.powertip').powerTip({placement: 'nw'});
  $('#reset,#animLabel,#volDown,#volUp,#muteLabel,#timer').powerTip({placement: 'sw'});
  $('#s0,#s1,#s2,#s3,#s4,#s5,#s6,#s7,#s8,#s9').powerTip({placement: 'n'});
  $('.powertipS').powerTip({placement: 'se'});
});

var amp_data = [];
var out_amp_data = [];
var msgBuffer;
var dataIndex = 0;

var data = [[],[]];
var time = [[],[]];
totalPoints = 512;

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
        var prev = data[0].length > 0 ? data[0][data[0].length - 1] : 0;
        y = amp_data[dataIndex];

        var prev = data[1].length > 0 ? data[1][data[1].length - 1] : 0;
        y2 = out_amp_data[dataIndex];

        dataIndex++;
        if(dataIndex > 500) {
          dataIndex = 0;
        }

        if((y < -1) || (y > 1)) {
          y = 0;
        }

        if((y2 < -1) || (y > 1)) {
          y2 = 0;
        }

        data[0].push(y);
        data[1].push(y2);

        time[0].push(now);
        time[1].push(now);
      }

    for (var i = 0; i < data[0].length; ++i) {
        res[0].push([time[0][i], data[0][i]]);
        res[1].push([time[1][i], data[1][i]]);
    }

    var myfatobj = [{ label: "Input", data: res[0] }, { label: "Output", data: res[1] }];

    return myfatobj;
}

/*function getBandData() {
    var prev = data[0].length > 0 ? data[0][data[0].length - 1] : 0;
    temp[0].push(amp_data[dataIndex]);

    var prev = data[1].length > 0 ? data[1][data[1].length - 1] : 0;
    temp[1].push(in_amp_data[dataIndex]);
  }
*/

var options = {
    series: {
        clickable: true,
        hoverable: true,
        shadowSize: 4   // Drawing is faster without shadows
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
            } 
            else {
                return "";
            }
        },
        axisLabel: "Time",
        axisLabelUseCanvas: true,
        axisLabelFontSizePixels: 12,
        axisLabelFontFamily: 'Verdana, Arial',
        axisLabelPadding: 10          
    },
    legend: { position: "sw" }
};

$(document).ready(function() {
    var plot1 = $.plot("#flot-line-chart-moving",  getAmplitudeData(), options);

    //temp = getBandData();
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

        //temp = getBandData();
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

