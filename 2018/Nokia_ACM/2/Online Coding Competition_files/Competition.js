/**
 * use codemirror to set html and get codemirror value
 */
var oldSelect = 'C';
var editor;
var compeCodeMode = 'text/x-c++src';
// init system
function initSystem() {
  var _t;
  window.onbeforeunload = function() {
    setTimeout(function() {
      setTimeout(onunloadcancel, 0);
    }, 0);
    if (userStatus == true) {
      return 'Are you sure you want to leave? Codes you made may not be saved.';
    }
  };
  window.onunloadcancel = function() {
    clearTimeout(_t);
  };
  initData();
}
function closeWindowCompe() {}
function checkTime() {
  var startDate = new Date(
    comStartTime.substring(0, 4),
    comStartTime.substring(4, 6) - 1,
    comStartTime.substring(6, 8),
    comStartTime.substring(8, 10),
    comStartTime.substring(10, 12),
    comStartTime.substring(12, 14)
  );
  compYear = comEndTime.substring(0, 4);
  compMonth = comEndTime.substring(4, 6) - 1;
  compDay = comEndTime.substring(6, 8);
  compHour = comEndTime.substring(8, 10);
  compMinute = comEndTime.substring(10, 12);
  compSecond = comEndTime.substring(12, 14);
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
  var startTime = startDate.getTime() - now.getTime();

  var endTime = endDate.getTime() - now.getTime();

  if (startTime > 0) {
    alert(
      "The Competition '" +
        compTaskName +
        "' is not start. It will be start at " +
        startDate.toLocaleString()
    );
    return -1;
  } else if (endTime <= 0) {
    alert(
      "The Competition '" +
        compTaskName +
        "' is already end at " +
        endDate.toLocaleString()
    );
    return 1;
  }
  return 0;
}
// set name and problem
function initData() {
  var taskAndName = getCookie('taskAndTask');

  if (taskAndName == null || taskAndName == 'null') {
    alert('Can not find task parameter.');
    return;
  }
  compUserName = taskAndName.split('.')[0];
  compTaskName = taskAndName.split('.')[1];

  var url = './data/subject/getSubject';

  var getParameters = {
    taskName: compTaskName,
    userName: compUserName
  };

  var getDatas = getDataFromServer(url, getParameters, false);
  var nameContainer = document.getElementById('compe-name');
  compeName = getDatas.subjectName;
  nameContainer.innerHTML = compeName;
  sampleCPlus = getDatas.subjectCDesc;
  sampleJava = getDatas.subjectJavaDesc;
  samplePython = getDatas.subjectPythonDesc;
  comStartTime = getDatas.startTime;
  comEndTime = getDatas.endTime;
  isSubmit = getDatas.submit;
  subjectJavaContent = getDatas.subjectJavaContent;
  subjectCPPContent = getDatas.subjectCPPContent;
  subjectPytonContent = getDatas.subjectPythonContent;
  userJavaContent = getDatas.userJavaContent;
  userCPPContent = getDatas.userCPPContent;
  userPytonContent = getDatas.userPythonContent;
  var serverTime = getDatas.serverTime;
  servYear = serverTime.substring(0, 4);
  servMonth = serverTime.substring(4, 6) - 1;
  servDay = serverTime.substring(6, 8);
  servHour = serverTime.substring(8, 10);
  servMinute = serverTime.substring(10, 12);
  servSecond = serverTime.substring(12, 14);

  var userStatus = getDatas.userStatus;
  if (userStatus == true) {
    var com = checkTime();
    if (com == 0) {
      window.setInterval(function() {
        ShowCountDown('ShowRemainTime');
      }, 1000);
      showProblem('showProblem', subjectCPPContent);
      showCodeMirror('showCodeMirror', compeCodeMode);
    } else if (com == 1) {
      showProblem('showProblem', subjectCPPContent);
      showCodeMirror('showCodeMirror', compeCodeMode);
      $('#compeRunTest').attr('disabled', 'disabled');
      $('#compeSubmitCode').attr('disabled', 'disabled');
      $('#compeSaveCode').attr('disabled', 'disabled');
      $('#compeResetCode').attr('disabled', 'disabled');
      var cc = document.getElementById('ShowRemainTime');
      var timeHtml = '<font size="3" color="red">';
      var timeLeft = timeHtml + 'Time is end.' + '</font>';
      cc.innerHTML = timeHtml + timeLeft + '</font>';
    } else {
      window.history.back();
    }
    if (isSubmit == true) {
      // $('#compeRunTest').attr('disabled', 'disabled');
      // $('#compeSubmitCode').attr('disabled', 'disabled');
      // $('#compeSaveCode').attr('disabled', 'disabled');
      // $('#compeResetCode').attr('disabled', 'disabled');
    }
  } else {
    //		alert("This is internal use! If you are one member of AA, please contact your manager or technical support!");
    alert(
      'It is the final time. Please go to the preliminary contest and qualify for the final'
    );
    var quitURL = './data/user/logout';
    var subStatus = getDataFromServer(quitURL, '', false);
    logOutCompe();
  }
}
// change code mode
function changeCodeLanguage(value) {
  $('#showRunResult').empty();
  var selectedOption = value.options[value.selectedIndex];
  var selectValue = selectedOption.value;
  if (oldSelect == selectValue) {
    return;
  }
  if (oldSelect == 'Java') {
    userJavaContent = editor.getValue();
  } else if (oldSelect == 'C') {
    userCPPContent = editor.getValue();
  } else if (oldSelect == 'Python') {
    userPytonContent = editor.getValue();
  }
  oldSelect = selectValue;

  var compeCodeMode;
  if (selectValue == 'Java') {
    compeCodeMode = 'text/x-java';
    if (userJavaContent) {
      editor.setValue(userJavaContent);
    } else {
      editor.setValue(sampleJava);
    }

    showProblem('showProblem', subjectJavaContent);
  } else if (selectValue == 'C') {
    compeCodeMode = 'text/x-c++src';
    if (userCPPContent) {
      editor.setValue(userCPPContent);
    } else {
      editor.setValue(sampleCPlus);
    }

    showProblem('showProblem', subjectCPPContent);
  } else if (selectValue == 'Python') {
    compeCodeMode = {
      name: 'python',
      version: 3,
      singleLineStringErrors: false
    };
    if (userPytonContent) {
      editor.setValue(userPytonContent);
    } else {
      editor.setValue(samplePython);
    }

    showProblem('showProblem', subjectPytonContent);
  }
  setCodeMirrorMode(compeCodeMode);
}
// show code mirror editor
function showCodeMirror(divId, codeMode) {
  //var myTextArea = $(divId)[0];
  var myTextArea = document.getElementById(divId);
  editor = CodeMirror.fromTextArea(myTextArea, {
    mode: codeMode,
    lineNumbers: true,
    lineWrapping: true,
    matchBrackets: true,
    theme: 'neat',
    extraKeys: {
      F11: function(cm) {
        cm.setOption('fullScreen', !cm.getOption('fullScreen'));
      },
      Esc: function(cm) {
        if (cm.getOption('fullScreen')) cm.setOption('fullScreen', false);
      }
    }
  });
  var o = document.getElementById('com-editor');
  var h = o.offsetHeight; //高度
  var w = o.offsetWidth; //宽度
  //	alert(h);
  editor.setSize('auto', h - 2);
  if (userCPPContent) {
    editor.setValue(userCPPContent);
  } else {
    editor.setValue(sampleCPlus);
  }
  //	alert(getQueryString("user"));
}
// set code mirror mode
function setCodeMirrorMode(mode) {
  editor.setOption('mode', mode);
}

function showProblem(divId, content) {
  var problemContainer = document.getElementById(divId);
  problemContainer.innerHTML = content;
}
// button run listening
function runCode() {
  //alert("Running code ..." + editor.getValue());
  $('#showRunResult').empty();

  var subURL = './data/code/testCode';
  var subCodeValue = editor.getValue();
  var subParameters = {
    subCode: subCodeValue,
    codeType: oldSelect,
    userName: compUserName,
    taskName: compTaskName
  };
  var testStatus = postDataToServer(subURL, subParameters, false);
  if (testStatus == 'error') {
    reLogin();
    return;
  }
  var status = testStatus.testStatus.split(';');
  var n = status.length;
  for (var i = 0; i < n; i++) {
    $('#showRunResult').append(status[i] + '\n');
  }
}
// button submit listening
function subCode() {
  var subCodeSign = confirm(
    // "Are you sure that your solution is ready?\nYou can't modify your solution after you submit it."
    "Are you sure that your solution is ready?\n"
  );
  if (subCodeSign == true) {
    submitC();
  }

  function submitC() {
    var subURL = './data/code/submitCode';
    var subCodeValue = editor.getValue();
    var subParameters = {
      subCode: subCodeValue,
      codeType: oldSelect,
      userName: compUserName,
      taskName: compTaskName
    };
    var subStatus = postDataToServer(subURL, subParameters, false);
    if (subStatus == 'error') {
      reLogin();
      return;
    }
    if (subStatus.subStatus == 'ok') {
      isSubmit = true;
      // $('#compeRunTest').attr('disabled', 'disabled');
      // $('#compeSubmitCode').attr('disabled', 'disabled');
      // $('#compeSaveCode').attr('disabled', 'disabled');
      // $('#compeResetCode').attr('disabled', 'disabled');
      alert('Successfully submit code.');
      if (oldSelect === 'C') {
        userCPPContent = subCodeValue;
      } else {
      }
    } else {
      alert('Submit code failed.');
    }
  }
}
function saveCode() {
  saveC();

  function saveC() {
    var saveURL = './data/code/saveCode';
    var saveCodeValue = editor.getValue();
    var saveParameters = {
      subCode: saveCodeValue,
      codeType: oldSelect,
      userName: compUserName,
      taskName: compTaskName
    };
    var saveStatus = postDataToServer(saveURL, saveParameters, false);
    if (saveStatus == 'error') {
      reLogin();
      return;
    }
    if (saveStatus.subStatus == 'ok') {
      alert('Successfully save code.');
    } else {
      alert('Save code failed.');
    }
  }
}
function resetCode() {
  var resetCodeSign = confirm(
    'Are you sure to reset your code?\nYour code will be lost.'
  );
  if (resetCodeSign == true) {
    resetC();
  }
  function resetC() {
    var resetURL = './data/code/resetCode';
    var resetParameters = {
      codeType: oldSelect,
      userName: compUserName,
      taskName: compTaskName
    };
    var resetStatus = getDataFromServer(resetURL, resetParameters, false);
    if (resetStatus == 'error') {
      reLogin();
      return;
    }
    if (resetStatus.subStatus === 'ok') {
      if (oldSelect == 'Java') {
        userJavaContent = '';
        editor.setValue(sampleJava);
      } else if (oldSelect == 'C') {
        userCPPContent = '';
        editor.setValue(sampleCPlus);
      } else if (oldSelect == 'Python') {
        userPytonContent = '';
        editor.setValue(samplePython);
      }
      alert('Successfully reset code.');
      $('#showRunResult').empty();
    } else {
      alert('Reset code failed.');
    }
  }
}
function getQueryString(name) {
  var reg = new RegExp('(^|&)' + name + '=([^&]*)(&|$)', 'i');
  var r = window.location.search.substr(1).match(reg);
  if (r != null) return unescape(r[2]);
  return null;
}
function getCookie(name) {
  var arr,
    reg = new RegExp('(^| )' + name + '=([^;]*)(;|$)');
  if ((arr = document.cookie.match(reg))) return unescape(arr[2]);
  else return null;
}
function reLogin() {
  document.cookie = 'relogin=re';
  window.open(
    'relogin.jsp',
    'newwindow',
    'height=400, width=500, toolbar=no, menubar=no, scrollbars=no, resizable=no, location=no, status=no'
  );
}
function closeLogin() {
  document.getElementById('win').style.display = 'none';
  //   document.getElementById("back").style.display="none";
}
function quitCompe() {
  var quitSign = confirm('Are you sure you want to quit?');
  if (quitSign == true) {
    logOutCompe();
  }
}
function logOutCompe() {
  var localHref = document.location.href;
  var redirectURL = localHref.split(':')[0] + ':' + localHref.split(':')[1];
  var ag = navigator.userAgent.toUpperCase();
  var isOpera = !!window.opera || navigator.userAgent.indexOf(' OPR/') >= 0;
  var isFirefox = typeof InstallTrigger !== 'undefined';
  var isSafari =
    Object.prototype.toString.call(window.HTMLElement).indexOf('Constructor') >
    0;
  var isChrome = !!window.chrome && !isOpera;

  var isIE = false || !!document.documentMode || ag.indexOf('MSIE') !== -1;
  var Host = window.location.host;

  if (isIE) {
    document.execCommand('ClearAuthenticationCache');
    window.location = redirectURL;
  } else if (isChrome) {
    window.location = redirectURL;
  } else if (isSafari) {
    (function(safeLocation) {
      var outcome,
        u,
        m = 'You should be logged out now.';

      try {
        outcome = document.execCommand('ClearAuthenticationCache');
      } catch (e) {}

      if (!outcome) {
        outcome = (function(x) {
          if (x) {
            x.open(
              'HEAD',
              safeLocation || location.href,
              true,
              'logout',
              new Date().getTime().toString()
            );
            x.send('');
            return 1;
          } else {
            return;
          }
        })(
          window.XMLHttpRequest
            ? new window.XMLHttpRequest()
            : window.ActiveXObject
              ? new ActiveXObject('Microsoft.XMLHTTP')
              : u
        );
      }
      if (!outcome) {
        m =
          'Your browser is too old or too weird to support log out functionality. Close all windows and restart the browser.';
      }
      alert(m);
      window.location = redirectURL;
    });
  } else {
    var isFirefox = ag.indexOf('FIREFOX') !== -1;

    if (isFirefox) {
      logoutFirefox(redirectURL);
    }
  }
  function logoutFirefox(URL) {
    window.location = URL;
  }
}
