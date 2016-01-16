Pebble.addEventListener("ready",
    function(e) {
        console.log("Hello world! - Sent from your javascript application.");
    }
);



Pebble.addEventListener("showConfiguration", function() 
			{
			    var options = JSON.parse(localStorage.getItem('options'));
			    console.log("read options: " + JSON.stringify(options));
			    console.log("showing configuration");
			    var url='http://mischievous.us/tracker2.html';
			    
			    if (options !== null) {
				url = url + '?';
				if (typeof options.name1 == 'undefined') {
				    url = url + 'n1=';
				} else {
				    url = url + 'n1=' + encodeURIComponent(options.name1);
				}
				url = url + '&';
				if (typeof options.name2 == 'undefined') {
				    url = url + 'n2=';
				} else {
				    url = url + 'n2=' + encodeURIComponent(options.name2);
				}
				url = url + '&';
				if (typeof options.name3 == 'undefined') {
				    url = url + 'n3=';
				} else {
				    url = url + 'n3=' + encodeURIComponent(options.name3);
				}
				url = url + '&';
				if (typeof options.name4 == 'undefined') {
				    url = url + 'n4=';
				} else {
				    url = url + 'n4=' + encodeURIComponent(options.name4);
				}
				url = url + '&';
				if (typeof options.name5 == 'undefined') {
				    url = url + 'n5=';
				} else {
				    url = url + 'n5=' + encodeURIComponent(options.name5);
				}
				url = url + '&';
				if (typeof options.n6 == 'undefined') {
				    url = url + 'n6=';
				} else {
				    url = url + 'n6=' + encodeURIComponent(options.n6);
				}
				url = url + '&';
				if (typeof options.n7 == 'undefined') {
				    url = url + 'n7=';
				} else {
				    url = url + 'n7=' + encodeURIComponent(options.n7);
				}
				url = url + '&';
				if (typeof options.n8 == 'undefined') {
				    url = url + 'n8=';
				} else {
				    url = url + 'n8=' + encodeURIComponent(options.n8);
				}
				url = url + '&';
				if (typeof options.n9 == 'undefined') {
				    url = url + 'n9=';
				} else {
				    url = url + 'n9=' + encodeURIComponent(options.n9);
				}
				url = url + '&';
				if (typeof options.n10 == 'undefined') {
				    url = url + 'n10=';
				} else {
				    url = url + 'n10=' + encodeURIComponent(options.n10);
				}



				url = url + '&';
				if (typeof options.hour1 == 'undefined') {
				    url = url + 'h1=';
				} else {
				    url = url + 'h1=' + encodeURIComponent(options.hour1);
				}
				url = url + '&';
				if (typeof options.hour2 == 'undefined') {
				    url = url + 'h2=';
				} else {
				    url = url + 'h2=' + encodeURIComponent(options.hour2);
				}
				url = url + '&';
				if (typeof options.hour3 == 'undefined') {
				    url = url + 'h3=';
				} else {
				    url = url + 'h3=' + encodeURIComponent(options.hour3);
				}
				url = url + '&';
				if (typeof options.hour4 == 'undefined') {
				    url = url + 'h4=';
				} else {
				    url = url + 'h4=' + encodeURIComponent(options.hour4);
				}
				url = url + '&';
				if (typeof options.hour5 == 'undefined') {
				    url = url + 'h5=';
				} else {
				    url = url + 'h5=' + encodeURIComponent(options.hour5);
				}
				url = url + '&';
				if (typeof options.h6 == 'undefined') {
				    url = url + 'h6=';
				} else {
				    url = url + 'h6=' + encodeURIComponent(options.h6);
				}
				url = url + '&';
				if (typeof options.h7 == 'undefined') {
				    url = url + 'h7=';
				} else {
				    url = url + 'h7=' + encodeURIComponent(options.h7);
				}
				url = url + '&';
				if (typeof options.h8 == 'undefined') {
				    url = url + 'h8=';
				} else {
				    url = url + 'h8=' + encodeURIComponent(options.h8);
				}
				url = url + '&';
				if (typeof options.h9 == 'undefined') {
				    url = url + 'h9=';
				} else {
				    url = url + 'h9=' + encodeURIComponent(options.h9);
				}
				url = url + '&';
				if (typeof options.h10 == 'undefined') {
				    url = url + 'h10=';
				} else {
				    url = url + 'h10=' + encodeURIComponent(options.h10);
				}



				url = url + '&';
				if (typeof options.min1 == 'undefined') {
				    url = url + 'm1=';
				} else {
				    url = url + 'm1=' + encodeURIComponent(options.min1);
				}
				url = url + '&';
				if (typeof options.min2 == 'undefined') {
				    url = url + 'm2=';
				} else {
				    url = url + 'm2=' + encodeURIComponent(options.min2);
				}
				url = url + '&';
				if (typeof options.min3 == 'undefined') {
				    url = url + 'm3=';
				} else {
				    url = url + 'm3=' + encodeURIComponent(options.min3);
				}
				url = url + '&';
				if (typeof options.min4 == 'undefined') {
				    url = url + 'm4=';
				} else {
				    url = url + 'm4=' + encodeURIComponent(options.min4);
				}
				url = url + '&';
				if (typeof options.min5 == 'undefined') {
				    url = url + 'm5=';
				} else {
				    url = url + 'm5=' + encodeURIComponent(options.min5);
				}
				url = url + '&';
				if (typeof options.m6 == 'undefined') {
				    url = url + 'm6=';
				} else {
				    url = url + 'm6=' + encodeURIComponent(options.m6);
				}
				url = url + '&';
				if (typeof options.m7 == 'undefined') {
				    url = url + 'm7=';
				} else {
				    url = url + 'm7=' + encodeURIComponent(options.m7);
				}
				url = url + '&';
				if (typeof options.m8 == 'undefined') {
				    url = url + 'm8=';
				} else {
				    url = url + 'm8=' + encodeURIComponent(options.m8);
				}
				url = url + '&';
				if (typeof options.m9 == 'undefined') {
				    url = url + 'm9=';
				} else {
				    url = url + 'm9=' + encodeURIComponent(options.m9);
				}
				url = url + '&';
				if (typeof options.m10 == 'undefined') {
				    url = url + 'm10=';
				} else {
				    url = url + 'm10=' + encodeURIComponent(options.m10);
				}
			    }
			    console.log("url=" + url);
			    Pebble.openURL(url);
			}
    );


function appMessageAck(e) 
{
    console.log("options sent to Pebble successfully");
}


function appMessageNack(e) 
{
    if (e.error) 
	console.log("options not sent to Pebble: " + e.error.message);
}
    

Pebble.addEventListener("webviewclosed", function(e) 
			{
			    console.log("configuration closed");
			    if (e.response !== '') {
				options = JSON.parse(decodeURIComponent(e.response));
				console.log("Options = " + JSON.stringify(options));
				localStorage.setItem('options', JSON.stringify(options));
				Pebble.sendAppMessage(options, appMessageAck, appMessageNack);
			    }
			}
    );

Pebble.addEventListener("appmessage", function(e)
			{
			    console.log("Received message: " + JSON.stringify(e.payload));
			    localStorage.setItem('options', JSON.stringify(e.payload));
			}
    );
