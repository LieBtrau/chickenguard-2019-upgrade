/**
 * Based on : ESP32 Remote Control with WebSocket, 2020 © Stéphane Calderoni
 */

var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
var currentPosition = { coords: { latitude: 50.85, longitude: 4.35 } }; //Coordinates of Brussels

// Symbolic constants for the door control
const DoorControl = Object.freeze({
    //Symbol description must match the one in the ESP32
    manualcontrol: Symbol("manual"),
    fixedTimeControl: Symbol("fixedTime"),
    sunControl: Symbol("sun"),
})


// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

window.addEventListener('load', onLoad);
window.addEventListener("unload", function () {
    if (websocket.readyState == WebSocket.OPEN) websocket.close();
});


function onLoad(event) {
    initWebSocket();
    document.getElementById('submit').addEventListener('click', onSubmit);
    document.getElementById("manuallyId").checked = true;

    // Only enable the sun button after the geolocation is available
    document.getElementById("sunId").disabled = true;

    //Hide server feedback
    document.getElementById("serverFeedback").classList.add("hide");

    // Hide the fixed time door timings when it is not selected
    document.getElementById("fixed_time_door_timings").classList.add("hide");
    var radios = document.getElementsByName('doorcontrol');
    for (radio in radios) {
        radios[radio].onclick = function () {
            if (this.value == "fixedTime")
                document.getElementById("fixed_time_door_timings").classList.remove("hide");
            else
                document.getElementById("fixed_time_door_timings").classList.add("hide");
        }
    }
    document.getElementById("AutomaticOpeningTime").value = "08:00";
    document.getElementById("AutomaticClosingTime").value = "20:00";
    document.getElementById("sunId").disabled = false;


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
    switch(data.key)
    {
        case 'feedback':
            document.getElementById('feedback').innerHTML = String(data.status);
            break;
        case 'battery':
            document.getElementById('battery').innerHTML = String(data.status);
            break;
    }
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function onSubmit(event) {
    let doorControlValue;
    switch (document.querySelector('input[name="doorcontrol"]:checked').value) {
        case "manually":
            doorControlValue = DoorControl.manualcontrol;
            break;
        case "fixedTime":
            doorControlValue = DoorControl.fixedTimeControl;
            break;
        case "sun":
            doorControlValue = DoorControl.sunControl;
            break;
        default:
            console.error("Unknown door control value: " + document.querySelector('input[name="doorcontrol"]:checked').value);
    }

    let jsonstring = JSON.stringify(
        {
            'UTCSeconds': Math.floor(Date.now() / 1000),
            'Timezone' : Intl.DateTimeFormat().resolvedOptions().timeZone, 
            'Latitude': currentPosition.coords.latitude,
            'Longitude': currentPosition.coords.longitude,
            'DoorControl': doorControlValue.description,
            'AutomaticOpeningTime': document.getElementById("AutomaticOpeningTime").value,
            'AutomaticClosingTime': document.getElementById("AutomaticClosingTime").value,   
        }
    )
    console.log(jsonstring);
    document.getElementById("serverFeedback").classList.remove("hide");
    document.getElementById("modeSelection").classList.add("hide");
    document.getElementById("submitbutton").classList.add("hide");
    document.getElementById("fixed_time_door_timings").classList.add("hide");
    if (websocket.readyState == WebSocket.OPEN) {
        websocket.send(jsonstring);
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
