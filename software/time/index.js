var x;

window.addEventListener('load', onLoad);

function onLoad(event) {
  x = document.getElementById("demo");
  getTime();
  const date = new Date();
  document.getElementById("door_open").value = formatTime(date);
  document.getElementById("door_closed").value = formatTime(date);
}

// Function which returns a minimum of two digits in case the value is less than 10
const getTwoDigits = (value) => value < 10 ? `0${value}` : value;

const formatTime = (date) => {
  const hours = getTwoDigits(date.getHours());
  const mins = getTwoDigits(date.getMinutes());

  return `${hours}:${mins}`;
}

function getTime() {
  var jsonUtc = JSON.stringify(
    { 'action': 'toggle', 'UTCSeconds': Math.floor(Date.now() / 1000) });
  console.log(jsonUtc);
}

function sendTime() {
  var jsonTime = JSON.stringify(
    {
      'door_open': document.getElementById("door_open").value,
      'door_closed': document.getElementById("door_closed").value,
      //Alternatively, hours and minutes can be sent separately:
      //'door_open_hours': document.getElementById("door_open").value.split(':')[0],
      //'door_open_minutes': document.getElementById("door_open").value.split(':')[1],
      //'door_closed_hours': document.getElementById("door_closed").value.split(':')[0],
      //'door_closed_minutes': document.getElementById("door_closed").value.split(':')[1],
    });
    //Value of input field time is always in 24h format
  console.log(jsonTime);
}
