
    var Ajax = {
    	// private property
    	_keyStr : "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=",
    	// public method for encoding
    	toBase64win1251 : function (input) { //?s??????N??????∞?????µ Utf8 ?? Base64 win1251
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

	array_toString : function(str) {
	     var output = "";
	     for (var i=0; i < str.length; i++)
		output += String.fromCharCode(str[i]);
	     return output;
	},

	getXmlHttp : function (){
	 var xmlhttp;
	  try  { xmlhttp = new ActiveXObject("Msxml2.XMLHTTP"); }
	  catch (e) {
	    try   { xmlhttp = new ActiveXObject("Microsoft.XMLHTTP"); }
	    catch (E)  { xmlhttp = false; }
	  }
	  if (!xmlhttp && typeof XMLHttpRequest!='undefined')xmlhttp = new XMLHttpRequest();
	  return xmlhttp;
	}, 

   run : function(url,e)  {
   var xmlhttp=Ajax.getXmlHttp();
   var IE_HACK = (/msie/i.test(navigator.userAgent) && 
                 !/opera/i.test(navigator.userAgent));   
   if(IE_HACK) {
//        alert(document.innerHTML);
//document.write("123");
document.write("<script type='text\/vbscript'>\n"+
			"Function BinaryToArray(Binary)\n"+
"Dim i\n"+
         "ReDim byteArray(LenB(Binary))\n"+
        " For i = 1 To LenB(Binary)\n"+
        "     byteArray(i-1) = AscB(MidB(Binary, i, 1))\n"+
        " Next\n"+
        " BinaryToArray = byteArray\n"+
    "End Function\n"+"<\/script>");
//document.write("123");
}


   else xmlhttp.overrideMimeType('text/html;charset=windows-1251');
   if(typeof(e)=='undefined') {
     xmlhttp.open('GET',url,false);
     xmlhttp.send(null);
   }
   else {
     xmlhttp.open('POST',url,false);
     xmlhttp.send(e);
   }
   if(xmlhttp.status!= 200)return(NaN);
   if(IE_HACK) return Ajax.win1251_utf8(Ajax.array_toString(BinaryToArray(xmlhttp.responseBody).toArray()));
   else return(xmlhttp.responseText);
  },


  runEval : function(url,e) {
	var s=String(Ajax.run(url,e));
	if(s.length==0)return eval(s);
	var IE_HACK = (/msie/i.test(navigator.userAgent) && 
                 !/opera/i.test(navigator.userAgent));   
	if(s.charAt(0)!='(' && !IE_HACK)return JSON.parse(s);
	return eval((s.charAt(0)=='{')?s:"("+s+")");
  }

 }



function TABLE_SORT(e) {

    function swap(a,b) {
	a.parentNode.insertBefore(b,a);    
    }

    function compare1(a,b) {
        var _a=a.innerHTML;
        var _b=b.innerHTML;
//alert(_a+"_"+_b);
        if(parseFloat(_a) && parseFloat(_b)) return _a-_b;
        if(_a==_b)return 0;
        if(_a>_b)return 1;
        if(_a<_b)return -1;
    }

    function compare(a,b) {
        var _a= "";
        var _b= "";
        
        
        if (typeof a == "undefined")
        {
        	_a = "";
        }
        else
        {
        	_a = String(a.innerHTML);
        }

        
        if (typeof b == "undefined")
        {
        	_b = "";
        }
        else
        {
        	_b = String(b.innerHTML);
        }


	//alert(typeof(a.innerHTML) + "  " + a.innerHTML);

        
	var reDate = /^(\d\d\d\d)-(\d\d)\-(\d\d)$/;
	var resultReDate1 = _a;
	var resultReDate2 = _b;
//alert(_a);


//alert(1);
	/*if (!resultReDate1 && !resultReDate2)
	{
		alert(2);
		if(_a==_b)return 0;
	        if(_a>_b)return 1;
	        if(_a<_b)return -1;
	}
	else
	{
		//alert("date");
		var date1 = _a;
		date1 = date1.substr(0,10);
		var date2 = _b;
		date2 = date2.substr(0,10);
		var date = new Date(date2);


		if (date1.valueOf() == date2.valueOf()) {return 0;}
		if (date1.valueOf() > date2.valueOf()) {return 1;}
		if (date1.valueOf() < date2.valueOf()) {return -1;}
	}
	
	if ((resultReDate1 && !resultReDate2) || (!resultReDate1 && resultReDate2)) {return 1;}*/
	
//?YN??????µN????∞ ???∞ NЗ??N??ї??
	if (!isNaN(_a) && !isNaN(_b))
	{
		if(eval(_a)==eval(_b))return 0;
	        if(eval(_a)>eval(_b))return 1;
	        if(eval(_a)<eval(_b))return -1;
	}
	else
	{
		if(_a==_b)return 0;
	        if(_a>_b)return 1;
	        if(_a<_b)return -1;
	}



/*if(_a==_b)return 0;
if(_a>_b)return 1;
if(_a<_b)return -1;*/


}
    
    function sort(e) {
      var el = window.event ? window.event.srcElement : e.currentTarget;
        while (el.tagName.toLowerCase() != "td" && el.tagName.toLowerCase() != "th") el = el.parentNode;
        var m=el.getAttribute('m');
        if(m==null)m=1;
        el.setAttribute('m',-m);
        var z=el.cellIndex;

        var dad = el.parentNode;
        var table = dad.parentNode.parentNode;
        var i;
        var j;
        var q;
        var n=table.rows[0].cells.length;
        for(i=0;i<n;i++)table.rows[0].cells[i].removeAttribute('background');
        for(i=(table.rows[0].cells[0].hasAttribute('rowspan'))?table.rows[0].cells[0].getAttribute('rowspan'):1;i<table.rows.length;i++)
            for(j=i+1;j<table.rows.length;j++)
                if(m*compare(table.rows[i].cells[z],table.rows[j].cells[z])>0)swap(table.rows[i],table.rows[j]);

	    var kk=1;
        for(i=(table.rows[0].cells[0].hasAttribute('rowspan'))?table.rows[0].cells[0].getAttribute('rowspan'):1;i<table.rows.length;i++) {
        	table.rows[i].cells[0].innerHTML=kk;
        	kk++;
        }
                
    }
    
//    var k=(e.rows[0].cells[0].hasAttribute('rowspan'))?e.rows[0].cells[0].getAttribute('rowspan'):1;
    var k=1;
    var i,j;
    for(j=0;j<k;j++)
	    for(i=0;i<e.rows[j].cells.length;i++) {
	        e.rows[j].cells[i].title = "ўелкните по названию столбца, чтобы упор€дочить данные";
	        if (e.rows[j].cells[i].addEventListener) e.rows[j].cells[i].addEventListener("click", sort, false);
	        else if (e.rows[j].cells[i].attachEvent) e.rows[j].cells[i].attachEvent("onclick", sort);
        
	    }
    
}


function XXX(){}
