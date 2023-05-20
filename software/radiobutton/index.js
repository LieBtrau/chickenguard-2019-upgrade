var x;

window.addEventListener('load', onLoad);

function onLoad(event) {
const form = document.querySelector("form");

form.addEventListener(
  "submit",
  (event) => {
    console.log(document.querySelector('input[name="doorcontrol"]:checked').value);
    event.preventDefault();
  },
  false
);
}



