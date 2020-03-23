// ==UserScript==
// @include        http://mail.google.com/*
// @include        https://mail.google.com/*
// ==/UserScript==
var gmail = null;
var emails = [];
var read_content_global;
(function() {
var callbacks = [];
var callback_counter = 0;

function dispatch_global(id, name, value) {
  var msg_data = {
      'type': 'read_content_global',
      'callback_id': id,
      'name': name,
      'data': value,
      };
  var msg = JSON.stringify(msg_data);
  window.postMessage(msg, '*');
}
location.href = 'javascript:'+dispatch_global.toString();

function receive_global(event) {
  try {
    var result = JSON.parse(event.data);
    if ('read_content_global' != result.type) return;
    if (!callbacks[result.callback_id]) return;
    callbacks[result.callback_id](result.name, result.data);
    del(callbacks[result.callback_id]);
  } catch (e) {

  }
}
window.addEventListener('message', receive_global, false);

read_content_global = function(name, callback) {
  var id = (callback_counter++);
  callbacks[id] = callback;

  location.href = 'javascript:dispatch_global('
      + id + ', "'
      + name.replace(/\"/g, '\\"') + '", '
      + name 
      + ');void(0);';
}
})();

function got(name, value) {
	console.log('got global named', name, 'with value', value);
	var vd = value;

	for (var i = 0; i < 4 ; i++){
		if (vd[i][0] == "tb") {
	        	var da = vd[i][2];
			for (var j = 0; j < 1; j++) {
	                	var r = da[j];
				//location.assign('https://mail.google.com/mail/?shva=1#inbox/'+r[0]);
                window.open('https://mail.google.com/mail/?ui=2&ik=1c74bed0af&view=att&th='+r[0]+'&attid=0.1&disp');
			}	            
	        }
	}	
}

read_content_global('VIEW_DATA', got);


