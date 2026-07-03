document.getElementById("message").textContent = "Welcome!";

setInterval(function() {
    var time = document.getElementById("time");
    if (time) {
        time.textContent = new Date().toLocaleTimeString();
    }
}, 1000);
