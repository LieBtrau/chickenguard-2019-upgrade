/**
 * Based on : ESP32 Remote Control with WebSocket, 2020 © Stéphane Calderoni
 */

var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
var currentPosition = { coords: { latitude: 0, longitude: 0 } };

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

window.addEventListener('load', onLoad);

function onLoad(event) {
    initWebSocket();
    document.getElementById('submit').addEventListener('click', onSubmit);
    document.getElementById("manuallyId").checked = true;

    // Only enable the sun button after the geolocation is available
    document.getElementById("sunId").disabled = true;

    // Hide the fixed time door timings when it is not selected
    document.getElementById("fixed_time_door_timings").classList.add("hide");
    var radios = document.getElementsByName('doorcontrol');
    for(radio in radios) {
        radios[radio].onclick = function() {
            if(this.value == "fixedTime") 
                document.getElementById("fixed_time_door_timings").classList.remove("hide");
            else
                document.getElementById("fixed_time_door_timings").classList.add("hide");
        }
    }
    document.getElementById("AutomaticOpeningTime").value = "08:00";
    document.getElementById("AutomaticClosingTime").value = "20:00";

    if (navigator.geolocation) {
        navigator.geolocation.getCurrentPosition(showPosition, showError);
    } else {
        alert("Geolocation is not supported by this browser.");
    }
}

// ----------------------------------------------------------------------------
// WebSocket handling
// ----------------------------------------------------------------------------

function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    console.log('Connection opened');
}

function onMessage(event) {
    let data = JSON.parse(event.data);
    console.log(data);
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function onSubmit(event) {
    let jsonstring = JSON.stringify(
        {
            'UTCSeconds': Math.floor(Date.now() / 1000),
            'Latitude': currentPosition.coords.latitude,
            'Longitude': currentPosition.coords.longitude,
            'DoorControl': document.querySelector('input[name="doorcontrol"]:checked').value,
            'AutomaticOpeningTime': document.getElementById("AutomaticOpeningTime").value,
            'AutomaticClosingTime': document.getElementById("AutomaticClosingTime").value
        }
    )
    console.log(jsonstring);
    location.replace("done.html");
    websocket.send(jsonstring);
}

// ----------------------------------------------------------------------------
// Geolocation handling
// ----------------------------------------------------------------------------
function showPosition(position) {
    currentPosition = position;
    console.log("Latitude: " + currentPosition.coords.latitude + "\r\nLongitude: " + currentPosition.coords.longitude);
    document.getElementById("sunId").disabled = false;
}

function showError(error) {
    switch (error.code) {
        case error.PERMISSION_DENIED:
            alert("User denied the request for Geolocation.");
            break;
        case error.POSITION_UNAVAILABLE:
            alert("Location information is unavailable.");
            break;
        case error.TIMEOUT:
            alert("The request to get user location timed out.");
            break;
        case error.UNKNOWN_ERROR:
            alert("An unknown error occurred.");
            break;
    }
}

// ----------------------------------------------------------------------------
// Time handling
// ----------------------------------------------------------------------------
// Function which returns a minimum of two digits in case the value is less than 10
const getTwoDigits = (value) => value < 10 ? `0${value}` : value;

const formatTime = (date) => {
    const hours = getTwoDigits(date.getHours());
    const mins = getTwoDigits(date.getMinutes());

    return `${hours}:${mins}`;
}
