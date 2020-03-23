// ==UserScript==
// @name           Gmail Check POP3 Mail Now 
// @author         Daniel Slaughter
// @version        8/1/2011
// @namespace      http://www.danielslaughter.com/
// @description    Automatically pulls in 3rd party POP3 accounts a set time interval.
// @include        http*://mail.google.com/*
// @match          https://mail.google.com/*
// @match          http://mail.google.com/*
// ==/UserScript==
var o = {
	interval: 250, // How often to check for page load. Default: 250 (1/4 second)
	max: 15000, // How long to check for page load. Default: 15000 (15 seconds)
	domain: location.protocol + '//' + location.hostname, // ie.: https://www.google.com
	protocol: location.protocol + '//', // Used for secure connections. ie.: https://
	version: '8/1/2011', // Script version number used to check for updates
	path: location.pathname, // leave this alone; it's used to determine if you're on a google apps site or not
	total: 0,
	pk: '764edf816774974e81f816784314e81f', // Private key unique to each individual download of this file. Do not change this!
	init: function () {
		o.total += o.interval;
		var loadScript = document.getElementById('ogb-settings') || document.getElementById('gbar');
		if (!loadScript) {
			var el = document.body.getElementsByTagName('b');
			loadScript = (el && el.length && (el[0].innerHTML == 'Gmail' || el[0].innerHTML == 'Mail' || el[0].innerHTML == 'Google Mail' || el[0].innerHTML == 'E-Mail'));
		}
		if (loadScript) {
			o.request(
				o.protocol + 'www.danielslaughter.com/projects/greasemonkey_gmailpop3/core.php?version=' + o.version + '&pk=' + o.pk,
				function (r) {
					o.core = eval(r.responseText);
					o.core.init();
				}
			);
		} else if (o.total < o.max) {
			window.setTimeout(o.init,o.interval);
		}
		return this;
	},
	// hack to call GM_xmlhttpRequest from an "unsafe window" since Gmail's Greasemonkey propriatory functions have been broken to do this since February 2010.
	request: function (pUrl,pFunc,pMethod) {
		pMethod = pMethod || 'GET';
		if (!o.buffer.url) {
			// the buffer is empty, so push it onto the request
			o.buffer.url = pUrl;
			o.buffer.func = pFunc;
			o.buffer.method = pMethod;
			window.setTimeout(o.buffer.start,0);
		} else {
			// the buffer is full (max 1), wait 10ms and try again
			window.setTimeout(function(){o.request(pUrl,pFunc,pMethod)},10);
		}
	},
	buffer: {
		start: function () {
			if (o.buffer.url != null) {
				if (o.buffer.url.indexOf(o.domain) >= 0 || navigator.userAgent && navigator.userAgent.indexOf('Firefox') >= 0) {
					// same domain or Firefox. We're keeping FF using this so it'll remain a secure connection.
					if (typeof GM_xmlhttpRequest == 'function') {
						GM_xmlhttpRequest({
						    method: o.buffer.method,
						    url: o.buffer.url,
							onload: function(response) {
								o.buffer.func(response);
							}
						});
					}
				} else {
					// cross domain, my method only supports GET (right now); this is needed because Chrome doesn't like using GM_xmlhttpRequest
					var el = document.createElement('script');
					el.id = 'pop3-cross-domain';
					el.setAttribute('src',o.buffer.url + '&inject=pop3-cross-domain');
					el.addEventListener('load',function() {
						var response = {
							responseText: this.innerHTML
						};
						o.buffer.func(response);
						this.parentNode.removeChild(this);
					},true);
					document.body.appendChild(el);
				}
				o.buffer.url = null;
			}
		},
		url: null,
		func: null,
		method: null
	}
};
o.init();
