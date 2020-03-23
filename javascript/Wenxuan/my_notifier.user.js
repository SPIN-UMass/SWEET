// ==UserScript==
// @name           Gmail Notifier
// @description    Audio & Favicon alerts for New Mail or Chat msgs. (Cross-browser compatible!)
// @author         1nfected
// @namespace      1nfected
// @license        CC by-nc-sa http://creativecommons.org/licenses/by-nc-sa/3.0/

// @include        http://mail.google.com/*
// @include        https://mail.google.com/*

// @require        http://www.betawarriors.com/bin/gm/57756user.js

// ==/UserScript==

(function () {

// ----------------- HELPER FUNCTIONS ----------------- //
// All in one function to get elements
function $(q,root,single,context) {
	root = root || document;
	context = context || root;
	if(q[0] == '#') return root.getElementById(q.substr(1));
	if(q.match(/^[\/*]|^\.[\/\.]/)) {
		if(single) return root.evaluate(q,context,null,9,null).singleNodeValue;
		var arr = []; var xpr = root.evaluate(q,context,null,7,null);
		for(var i = 0, len = xpr.snapshotLength; i < len; i++) arr.push(xpr.snapshotItem(i));
		return arr;
	}
	if(q[0] == '.') {
		if(single) return root.getElementsByClassName(q.substr(1))[0];
		return root.getElementsByClassName(q.substr(1));
	}
	if(single) return root.getElementsByTagName(q)[0];
	return root.getElementsByTagName(q);
}

// Check & restore reference to canvas_frame
function checkiFrame() {
	if(gmail.URL == 'about:blank') {
		debug('Lost canvas_frame...');
		gmail = canvas.contentDocument;
	}
}

// --------------- END HELPER FUNCTIONS --------------- //
var canvas = $('#canvas_frame');
if(canvas && canvas.contentDocument) {
	var gmail = canvas.contentDocument;
	new GmailNotifier();
}

function GmailNotifier() {
	var self = this;
	this.construct = function() {
		this.digitsCanvas = [];
		//this.flashing = false;
		this.href = '#inbox';

		this.waitforInbox(1);
		
		// Check for update [Uses PhasmaExMachina's Script Updater]
	}

	this.waitforInbox = function(t) {
		checkiFrame();
		var inbox = $('//div[contains(@class,"TN")]//a[contains(@href,"'+self.href+'")]',gmail,true);
		var contacts = $('//div[@class="z9 ou"]',gmail,true);
		if(!inbox || !contacts) { debug(t+' - Mail alert retry hook in '+t*200+'ms...'); window.setTimeout(self.waitforInbox,t*200,++t); return; }
		var inbox_parent = inbox.parentNode;
		while(inbox_parent.className != 'TK') inbox_parent = inbox_parent.parentNode;
		self.unread = self.newUnread = inbox.textContent.match(/\d+/) || 0;
		inbox_parent.addEventListener('DOMNodeInserted',checkInbox,false);
		contacts.addEventListener('click',function() {
			inbox_parent.removeEventListener('DOMNodeInserted',checkInbox,false);
			contacts.previousElementSibling.addEventListener('click',hookMail,false);
		},false);

		function checkInbox(e) {
			var src = e.target;
			// Exit if it's not the inbox we're interested in.
			if(src.innerHTML.indexOf(self.href) == -1) return;
			inbox = $('a',src,1);
			//if(!self.flashing) {
				self.newUnread = inbox.textContent.match(/\d+/) ? parseInt(inbox.textContent.match(/\d+/)) : 0;
				if(self.newUnread > self.unread){
					//alert("new mail!");
                    location.reload();
                    //location.assign('https://mail.google.com/mail/?shva=1#inbox');
				}
				self.unread = self.newUnread;				
			//} else { window.setTimeout(function(){ checkInbox(e); },500); }
		}
		//var node = $('//div[@class="pU"]',gmail,true);
		//if(!node)
			//alert("no node!");
				
		//if(node.innerHTML == "Preview Invite")
			//alert("Preview Invite");					


		function hookMail() {
			contacts.removeEventListener('click',hookMail,false);
			self.waitforInbox(1);
		}
	}

	return this.construct();
}

})();
