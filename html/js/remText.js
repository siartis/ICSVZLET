function simple(x){   return(String(x).replace(/\r|\n|/g, ''));  };
function text(x) { return(String(x).replace(/<\/?[^>]+>/gi, '')); };

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