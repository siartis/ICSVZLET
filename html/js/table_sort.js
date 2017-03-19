function TABLE_SORT(e) {

    function swap(a,b) {
	a.parentNode.insertBefore(b,a);    
    }

    function compare1(a,b) {
        var _a=a.innerHTML;
        var _b=b.innerHTML;
        
        
        _a = _a.replace(/<\/?[^>]+>/g, '');
        _b = _b.replace(/<\/?[^>]+>/g, '');
        
        alert(_a);
        
        
        
//alert(_a+"_"+_b);
        if(parseFloat(_a) && parseFloat(_b)) return _a-_b;
        if(_a==_b)return 0;
        if(_a>_b)return 1;
        if(_a<_b)return -1;
    }

    function compare(a,b) {
        var _a= "";
        var _b= "";
        
        
        
        if(parseFloat(a) && parseFloat(b)) return a-b;
        if(a==b)return 0;
        if(a>b)return 1;
        if(a<b)return -1;

        
        
        
        
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

		
		_a = _a.replace(/<\/?[^>]+>/g, '');
        _b = _b.replace(/<\/?[^>]+>/g, '');



	//alert(typeof(a.innerHTML) + "  " + a.innerHTML);

        
	var reDate = /^(\d\d\d\d)-(\d\d)\-(\d\d)$/;
	var resultReDate1 = _a;
	var resultReDate2 = _b;
//alert(_a);


//alert(1);
	if (!resultReDate1 && !resultReDate2)
	{
		//alert(2);
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
	
	if ((resultReDate1 && !resultReDate2) || (!resultReDate1 && resultReDate2)) {return 1;}
	
//I?iaa?ea ia ?enei
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
//alert("qweqwe");  
      var el = window.event ? window.event.srcElement : e.currentTarget;
        while (el.tagName.toLowerCase() != "td" && el.tagName.toLowerCase() != "th") el = el.parentNode;
        var m=el.getAttribute('m');
        if(m==null)m=1;
        el.setAttribute('m',-m);
        var z=el.cellIndex;
//alert(z);
        var dad = el.parentNode;
        var table = dad.parentNode.parentNode;
        var i;
        var j;
        var q;
        var n=table.rows[0].cells.length;
        for(i=0;i<n;i++) {
        	table.rows[0].cells[i].removeAttribute('background');
        	//table.rows[0].cells[i].removeAttribute('style');
        }
        for(i=(table.rows[0].cells[0].hasAttribute('rowspan'))?table.rows[0].cells[0].getAttribute('rowspan'):1;i<table.rows+1.length;i++)
            for(j=i+1;j<table.rows.length-1;j++)
                if(m*compare(table.rows[i].cells[z],table.rows[j].cells[z])>0)swap(table.rows[i],table.rows[j]);
//alert(123);	    
	    var kk=1;
        for(i=(table.rows[0].cells[0].hasAttribute('rowspan'))?table.rows[0].cells[0].getAttribute('rowspan'):1;i<table.rows+1.length;i++) {
        	table.rows[i].cells[0].innerHTML=kk;
        	kk++;
        }
//alert(567);        
                
    }
    
//    var k=(e.rows[0].cells[0].hasAttribute('rowspan'))?e.rows[0].cells[0].getAttribute('rowspan'):1;
    var k=1;
    var i,j;
    for(j=0;j<k;j++)
	    for(i=0;i<e.rows[j].cells.length;i++) {
	        e.rows[j].cells[i].title = "ўелкните по заголовку столбца, чтобы упор€дочить данные";
	        if (e.rows[j].cells[i].addEventListener) e.rows[j].cells[i].addEventListener("click", sort, false);
	        else if (e.rows[j].cells[i].attachEvent) e.rows[j].cells[i].attachEvent("onclick", sort);
        
	    }
    
}



function T(x) { 
		var s=String(x);	
		while(s.indexOf("\n")!=-1)s=s.replace("\n"," ");
		while(s.indexOf("\r")!=-1)s=s.replace("\r"," ");
		while(s.indexOf("&nbsp")!=-1)s=s.replace("&nbsp"," ");
		s=s.replace(/<!-- .*?>/g,"");
                s=String(s).replace(/<xml.*?>(.*?)<\/xml.*?>/gm,"");
                s=String(s).replace(/<style.*?>(.*?)<\/style.*?>/gm,"");
		s=text(simple(s));
		return(s);
}
