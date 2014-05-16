// pebble-js-app.js
var urlRunname = '{production server}/das/runname';
var urlResults = '{production server}/das/results';
var urlAdd = '{production server}/das/add';

var flag = '-';

var msgKeys = {
	CODE_KEY : 0,
	INIT_KEY : 101,
	RUNNAME_KEY : 256,
	FLAG_KEY : 257,
	LAPS_KEY : 258,
	TOD_KEY : 259, // time of day
	TR_KEY : 260 //time remaining in session
};

function requestData(url, success, failure) {
	var xhr = new XMLHttpRequest();
	
	xhr.onreadystatechange = function() {
		if (xhr.readyState == 4) {
			if (xhr.status == 200)
			{
				try {
					success(JSON.parse(xhr.responseText));
				} catch(err) {
					failure("Code : " + xhr.status);
				}
			} else {
				failure("Code : " + xhr.status);
			}                        
		}
	};
	
	xhr.open('GET', url, true);
	xhr.timeout = 4000;
	xhr.send(null);
}

function getRunname() {
	requestData(urlRunname,
		function(data) {
			if(flag == "N" & data[2].toString() != "N") {				
				setTimeout(getResults, 1000);
				setTimeout(getAdd, 2000);
			}
			
			if(data[2].toString() == "Y" && flag != "Y" && flag != "N") {
				Pebble.showSimpleNotificationOnPebble("NASCAR", "Caution is out!");
			}
			
			var res = {
				"256" : data[1].toString(),
				"257" : data[2].toString(),
				"258" : data[3].toString()
			};
			
			if(res[msgKeys.FLAG_KEY] == "N") {
				res[msgKeys.FLAG_KEY] = " ";
			}
			
			flag = data[2].toString();
			
			Pebble.sendAppMessage(res);
		},
		function(data) {
			// set flag to 'N' to stop requesting results
			// if we can't even get the run name
			flag = "N";
			Pebble.sendAppMessage({"256" : "--lost connection--"});
		});
	
	if(flag == "N") {
		setTimeout(getRunname, 15000);
	}else {
		setTimeout(getRunname, 10000);
	}
}

function getResults() {
	if(flag == "N") {
		return;
	}
	
	requestData(urlResults,
		function(data) {
			Pebble.sendAppMessage(data);
			setTimeout(getResults, 7500);
		},
		function(data) {
			console.log("Error getResults() - " + data);
			setTimeout(getResults, 15000);
		});
}

function getAdd() {
	if(flag == "N") {
		return;
	}
	
	requestData(urlAdd,
		function(data) {
			var i = 1;
			for(i=1;i<6;i++){
				if(data[i] == '59:59.999'){
					data[i] = '---';
				}
			}
			
			Pebble.sendAppMessage(data);
			setTimeout(getAdd, 7500);
		},
		function(data) {
			setTimeout(getAdd, 15000);
		});
}

// Called when JS is ready
Pebble.addEventListener("ready",
	function(e) {
		setTimeout(getResults, 2000);
		setTimeout(getAdd, 4000);
		setTimeout(getRunname, 6000);
});
	
// Called when incoming message from the Pebble is received
Pebble.addEventListener("appmessage",
	function(e) {
		console.log("Received Status: " + e.payload[0]);
		console.log("Received Message: " + e.payload[101]);
		//sendMessage();
	});