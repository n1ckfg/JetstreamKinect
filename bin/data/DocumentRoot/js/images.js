"use strict";

function encodeBase64Image(img, format) {
    var c = document.createElement('canvas');
    c.width = img.naturalWidth;
    c.height = img.naturalHeight;
    var ctx = c.getContext('2d');
    ctx.drawImage(img, 0, 0);
    return c.toDataURL('image/' + format);
}

function saveBase64Image(img, fileName) {
    var extensionTemp = fileName.split('.');
    var format = extensionTemp[extensionTemp.length - 1];

    var imgBase64 = encodeBase64Image(img, format);

    var a = document.createElement("a"); //Create <a>
    a.href = imgBase64; //Image Base64 Goes here
    a.download = fileName; //File name Here
    a.click(); //Downloaded file
}

function encodeBase64ImageStereo(img, stereoMode, format) {
    var c = document.createElement('canvas');
    
    if (stereoMode === "ou") {
        c.width = img.naturalWidth;
        c.height = img.naturalHeight / 2.0;
    } else {
        c.width = img.naturalWidth / 2.0;
        c.height = img.naturalHeight;
    } 
    
    var ctx = c.getContext('2d');
    
    var imgL, imgR;

    if (stereoMode === "ou") {
        ctx.drawImage(img, 0, 0);
        imgL = c.toDataURL('image/' + format);
        ctx.drawImage(img, 0, -img.naturalHeight / 2.0);
        imgR = c.toDataURL('image/' + format);
    } else {
        ctx.drawImage(img, 0, 0);
        imgL = c.toDataURL('image/' + format);
        ctx.drawImage(img, -img.naturalWidth / 2.0, 0);
        imgR = c.toDataURL('image/' + format);
    }
    
    return [imgL, imgR];
}

function saveBase64ImageStereo(img, stereoMode, fileName) {
    var fileNameSplit = fileName.split('.');
    var extension = fileNameSplit[fileNameSplit.length - 1];
    var shortFileName = "";
    for (var i=0; i<fileNameSplit.length-1; i++) {
        shortFileName += fileNameSplit[i];
    }

    var imgBase64 = encodeBase64ImageStereo(img, stereoMode, extension);

    var a = document.createElement("a"); //Create <a>
    a.href = imgBase64[0]; //Image Base64 Goes here
    a.download = shortFileName + "_L." + extension; //File name Here
    a.click(); //Downloaded file

    a.href = imgBase64[1]; //Image Base64 Goes here
    a.download = shortFileName + "_R." + extension; //File name Here
    a.click(); //Downloaded file
}

// ~ ~ ~ ~ 

function loadFile(filepath, callback) { 
    // https://codepen.io/KryptoniteDove/post/load-json-file-locally-using-pure-javascript  
    var xobj = new XMLHttpRequest();
    xobj.overrideMimeType("text/plain");
    xobj.open('GET', filepath, true);
    xobj.onreadystatechange = function() {
        if (xobj.readyState == 4 && xobj.status == "200") {
            callback(xobj.responseText);
        }
    };
    xobj.send(null);  
}

function download(filename, url) {
    var element = document.createElement('a');
    //element.setAttribute('href', 'data:text/plain;charset=utf-8,' + encodeURIComponent(text));
    element.setAttribute('href', url);
    element.setAttribute('download', filename);
    element.setAttribute('target', '_blank');
    element.style.display = 'none';
    document.body.appendChild(element);

    element.click();

    document.body.removeChild(element);
}
