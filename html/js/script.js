function setCurrentUser() {
	var currentUser = "";
	var xhr = new XMLHttpRequest();		
	xhr.open("GET", "/script/login.exe?user", true);
	xhr.send();
	xhr.onreadystatechange = function() {
		if (xhr.readyState != 4) return;
		if (xhr.status != 200) {
			alert("Error");
		} 
		else {
			currentUser = JSON.parse(xhr.responseText);			
			if (eval(currentUser.userStatusCode)) {
				document.getElementById("currentUser").innerHTML = "НОП ВЗЛЕТ 2016/2017 уч. год<br>Вы вошли как " + currentUser.userFIO + ", " + currentUser.userStatusName;
			}
			else {
				document.getElementById("currentUser").innerHTML = "";
			}
		}
	}
}

function getCurrentDate() {
	var currentDate = "";
	var xhr = new XMLHttpRequest();		
	xhr.open("GET", "/script/statistics.exe?getServerCurrentDate", true);
	xhr.send();
	xhr.onreadystatechange = function() {
		if (xhr.readyState != 4) return;
		if (xhr.status != 200) {
			alert("Error");
		} 
		else {
			currentDate = JSON.parse(xhr.responseText);	
			document.getElementById("currentDate").innerHTML = "Текущая дата - " + currentDate.currentDate;		
		}
	}
}