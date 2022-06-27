"use strict";

var liveView;
var hostname="127.0.0.1";
var port=7111;

function saveImage() {
    saveBase64Image(liveView, "test.jpg");
}

function saveImageStereo() {
    saveBase64ImageStereo(liveView, "sbs", "test.jpg");
}

function main() {
	loadFile("./js/hostname", function(evt) {
	    hostname = evt.replace(/^\n|\n$/g, ''); // remove line breaks
        
        liveView = document.getElementById("live_view");
		liveView.src = "http://" + hostname + ".local:" + port;
	});
}

window.onload = main;
