
    var AjaxAsynx = {
    	// private property
    	_keyStr : "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=",
    	// public method for encoding
    	toBase64win1251 : function (input) { //Кодирование Utf8 в Base64 win1251
    		var output = "";
    		var chr1, chr2, chr3, enc1, enc2, enc3, enc4;
    		var i = 0;
    		input = Ajax.utf8_win1251(input);
    		while (i < input.length) {
    			chr1 = input.charCodeAt(i++);
    			chr2 = input.charCodeAt(i++);
    			chr3 = input.charCodeAt(i++);
    			enc1 = chr1 >> 2;
    			enc2 = ((chr1 & 3) << 4) | (chr2 >> 4);
    			enc3 = ((chr2 & 15) << 2) | (chr3 >> 6);
    			enc4 = chr3 & 63;
    			if (isNaN(chr2)) enc3 = enc4 = 64;
    			else if (isNaN(chr3)) enc4 = 64;
    			output = output +
    			this._keyStr.charAt(enc1) + this._keyStr.charAt(enc2) +
    			this._keyStr.charAt(enc3) + this._keyStr.charAt(enc4);
    		}
    		return output;
    	},

  runEval : function(t, url, e) {
	//var s=String(Ajax.run(url,e));
	var s = "";

//alert("t = " + t + " url = " + url);
		var xhr = new XMLHttpRequest();		
		xhr.open(t, url, true);
		xhr.send();
		xhr.onreadystatechange = function() {
			if (xhr.readyState != 4) return;
			if (xhr.status != 200) {
				alert("Error");
			} 
			else {
				//alert("qqq")
				s = xhr.responseText;
				alert("s=   " + s);
			}
		
		}
	return s;
  }

 }


function XXX(){}
