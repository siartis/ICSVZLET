function TABLE_SORT(e) {

    function swap(a,b) {
	a.parentNode.insertBefore(b,a);    
    }

    function compare1(a,b) {
        var _a=a.innerHTML;
        var _b=b.innerHTML;
        
//        alert(a);
        
        
        
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
	
//Проверка на число
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
//alert(z);
        var dad = el.parentNode;
        var table = dad.parentNode.parentNode;
        var i;
        var j;
        var q;
        var n=table.rows[0].cells.length;
        for(i=0;i<n;i++)table.rows[0].cells[i].removeAttribute('background');
        for(i=(table.rows[0].cells[0].hasAttribute('rowspan'))?table.rows[0].cells[0].getAttribute('rowspan'):1;i<table.rows.length-1;i++)
            for(j=i+1;j<table.rows.length;j++)
                if(m*compare(table.rows[i].cells[z],table.rows[j].cells[z])>0)swap(table.rows[i],table.rows[j]);
    }
    
//    var k=(e.rows[0].cells[0].hasAttribute('rowspan'))?e.rows[0].cells[0].getAttribute('rowspan'):1;
    var k=1;
    var i,j;
    for(j=0;j<k;j++)
	    for(i=0;i<e.rows[j].cells.length;i++) {
	        e.rows[j].cells[i].title = "Нажмите на заголовок, чтобы отсортировать колонку";
	        if (e.rows[j].cells[i].addEventListener) e.rows[j].cells[i].addEventListener("click", sort, false);
	        else if (e.rows[j].cells[i].attachEvent) e.rows[j].cells[i].attachEvent("onclick", sort);
        
	    }
    
}
