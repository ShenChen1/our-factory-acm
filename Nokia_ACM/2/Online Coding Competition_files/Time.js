/**
 * show time at divname
 * if time less than 10m, show red content.
 */

var interval = 1000;
function ShowCountDown(divname) {
  if (isEnd) {
    return;
  }
  //	alert(servSecond);
  var now = new Date(
    servYear,
    servMonth,
    servDay,
    servHour,
    servMinute,
    servSecond
  );
  var endDate = new Date(
    compYear,
    compMonth,
    compDay,
    compHour,
    compMinute,
    compSecond
  );

  var leftTime = endDate.getTime() - now.getTime();
  var Day_Param = 60 * 60 * 24; //
  var Hour_Param = 60 * 60; //
  var leftsecond = parseInt(leftTime / 1000);
  var day = Math.floor(leftsecond / Day_Param);
  var hour = Math.floor((leftsecond - day * 24 * 60 * 60) / 3600);
  var minute = Math.floor((leftsecond - day * 24 * 60 * 60 - hour * 3600) / 60);
  var second = Math.floor(
    leftsecond - day * 24 * 60 * 60 - hour * 3600 - minute * 60
  );
  var cc = document.getElementById(divname);
  var timeHtml = '<font size="3" color="green">';
  var timeLeft;
  if (hour == 0 && minute <= 10 && day == 0) {
    timeHtml = '<font size="3" color="red">';
  }
  minute = fixNumber(minute, 2);
  second = fixNumber(second, 2);
  var endTime = endDate.getTime() - now.getTime();
  if (endTime <= 0) {
    timeHtml = '<font size="3" color="red">';
    timeLeft = timeHtml + 'Time is end.' + '</font>';
    alert(
      "The Competition '" +
        compTaskName +
        "' is already end at " +
        endDate.toLocaleString()
    );

    $('#compeRunTest').attr('disabled', 'disabled');
    $('#compeSubmitCode').attr('disabled', 'disabled');
    isEnd = true;
    //do not submit code, let user to do that
    //subCode();
  } else {
    timeLeft =
      'End in ' + day + 'd ' + hour + 'h ' + minute + 'm ' + second + 's';
  }
  cc.innerHTML = timeHtml + timeLeft + '</font>';
  servSecond = parseInt(servSecond) + 1;
}
//function recordCometerTime(compeEndDate){
//	compYear = compeEndDate.getFullYear();
//	compMonth = compeEndDate.getMonth();
//	compDay = compeEndDate.getDay();
//	compHour = compeEndDate.getHours();
//	compMinute = compeEndDate.getMinutes();
//	compSecond = compeEndDate.getSeconds();
//
//	isNewUser = false;
//}
function fixNumber(num, length) {
  return ('' + num).length < length
    ? (new Array(length + 1).join('0') + num).slice(-length)
    : '' + num;
}
