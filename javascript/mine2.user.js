// ==UserScript==
// @name          Gmail Amir 2
// @namespace     http://www.norcimo.com/fun/greasmonkey/
// @description   Reminds you to attach a file to your email if it appears that you have not.
// @include       http://mail.google.com/*
// @include       https://mail.google.com/*
// ==/UserScript==

// December 2007
// Version 3.0 beta
// Updated for the new Gmail interface


var SHOW_DETAILS_CLASS = "iD";

var gmail = null;
alert("1");
window.addEventListener('load', function() {
  if (unsafeWindow.gmonkey) {
    unsafeWindow.gmonkey.load('1.0', function(g) {
      gmail = g;
      window.setTimeout(registerCallback, 500);
    });
  }
}, true);

function registerCallback() {
    gmail.registerViewChangeCallback(showDetails);
}

function showDetails() {
  var nodes = 
    getNodesByTagNameAndClass(gmail.getActiveViewElement(), "span", SHOW_DETAILS_CLASS);
  if (!nodes) return;
  var show = nodes[0]; //only shows details of the first available message
  if (!show) return;
  if (show.innerHTML == "show details"){
    //simulateClick(show, "click");
    alert("the value found");
  }else{
    alert("not found");
  }
}