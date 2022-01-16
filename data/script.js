var xhttp = new XMLHttpRequest();
xhttp.onreadystatechange = function () {
  var t1 = document.getElementById("input1");
  var t2 = document.getElementById("input2");
  var t3 = document.getElementById("input3");
  t1.value = this.responseText.split(",")[0];
  t2.value = this.responseText.split(",")[1];
  t3.value = this.responseText.split(",")[2];
};
xhttp.open("GET", "/getpara", true);
xhttp.send();