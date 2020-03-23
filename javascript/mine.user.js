// ==UserScript==
// @name          Gmail Amir
// @namespace     http://www.norcimo.com/fun/greasmonkey/
// @description   Reminds you to attach a file to your email if it appears that you have not.
// @include       http://mail.google.com/*
// @include       https://mail.google.com/*
// ==/UserScript==

// December 2007
// Version 3.0 beta
// Updated for the new Gmail interface

var wordTrigger = 'attach';

if (!GM_getValue("attach_trigger")) {
	GM_setValue("attach_trigger", wordTrigger);
}



var str= "alex@sd.cs"

//document.write(str.search("W3SCHOOLS"));

alert("the value "+ str.search("W3SCHOOLS"))

//var loadScript = document.getElementById('ogb-settings')

// this asks for a URL
/*
GM_xmlhttpRequest({
    method: 'GET',
    url: 'http://greaseblog.blogspot.com/atom.xml',
    headers: {
        'User-agent': 'Mozilla/4.0 (compatible) Greasemonkey',
        'Accept': 'application/atom+xml,application/xml,text/xml',
    },
    onload: function(responseDetails) {
        alert('Request for Atom feed returned ' + responseDetails.status +
              ' ' + responseDetails.statusText + '\n\n' +
              'Feed data:\n' + responseDetails.responseText);
    }
});
*/