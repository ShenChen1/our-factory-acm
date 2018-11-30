function getDataFromServerByJQery(url, params, isAsyns){
	var getDatas;
	$.ajax({   
		url:url,   
		type:'get',   
		data: params, 
		dataType:'json', 
		async : isAsyns, //default is true
		error:function(XMLHttpRequest, textStatus, errorThrown){   
			getDatas = "error";   
		},   
		success:function(data){   
//			alert("data from:" + data);
			getDatas = data;  
		}
	});
	
	return getDatas;
}
function postDataToServerByJQery(url, params, isAsyns){
	var getDatas;
	
	$.ajax({   
		url:url,   
		type:'post',   
		data: params, 
		dataType:'json', 
		async : isAsyns, //default is true
		error:function(XMLHttpRequest, textStatus, errorThrown){   
			getDatas = "error";  
		},   
		success:function(data){   
//			alert("success");
			getDatas = data;  
		}
	});
	
	return getDatas;
}
var xmlHttpRequest = null;

function getDataFromServer(url, parameters, isAsyns){
	
	return getDataFromServerByJQery(url, parameters, isAsyns);
		
}

function postDataToServer(url, parameters, isAsyns){
	
	return postDataToServerByJQery(url, parameters, isAsyns);
}