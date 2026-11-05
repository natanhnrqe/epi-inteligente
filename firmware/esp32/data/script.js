
const API_URL = "/api/status";

async function updateDashboard() {

    try {

        const response = await fetch(API_URL);

        if (!response.ok) {
            throw new Error("Erro na resposta da API");
        }

        const data = await response.json();

        document.getElementById("battery").textContent =
            `${data.battery}%`;

        document.getElementById("helmet").textContent =
            data.helmet ? "OK" : "REMOVIDO";

        document.getElementById("accel-x").textContent =
            Number(data.accelerometer.x).toFixed(2);

        document.getElementById("accel-y").textContent =
            Number(data.accelerometer.y).toFixed(2);

        document.getElementById("accel-z").textContent =
            Number(data.accelerometer.z).toFixed(2);

        document.getElementById("gyro-x").textContent =
            Number(data.gyroscope.x).toFixed(2);

        document.getElementById("gyro-y").textContent =
            Number(data.gyroscope.y).toFixed(2);

        document.getElementById("gyro-z").textContent =
            Number(data.gyroscope.z).toFixed(2);


        updateBuzzerStatus(data.buzzer);


        document.getElementById("connection").textContent =
            "● Conectado ao ESP32";

    } catch (error) {

        console.error(error);

        document.getElementById("connection").textContent =
            "● ESP32 desconectado";
    }
}

function updateBuzzerStatus(ativo) {

    const status =
        document.getElementById("buzzer-status");


    if (ativo) {

        status.textContent =
            "🔊 Buzzer acionado";

    } else {

        status.textContent =
            "🔇 Buzzer desligado";
    }
}

async function ligarBuzzer() {

    try {

        const response = await fetch(
            "/api/buzzer/on",
            {
                method: "POST"
            }
        );


        if (!response.ok) {
            throw new Error("Erro ao acionar buzzer");
        }


        updateBuzzerStatus(true);

    } catch (error) {

        console.error(error);

        alert("Não foi possível acionar o buzzer.");
    }
}

async function desligarBuzzer() {

    try {

        const response = await fetch(
            "/api/buzzer/off",
            {
                method: "POST"
            }
        );


        if (!response.ok) {
            throw new Error("Erro ao desligar buzzer");
        }


        updateBuzzerStatus(false);

    } catch (error) {

        console.error(error);

        alert("Não foi possível desligar o buzzer.");
    }
}


document
    .getElementById("buzzer-on")
    .addEventListener(
        "click",
        ligarBuzzer
    );


document
    .getElementById("buzzer-off")
    .addEventListener(
        "click",
        desligarBuzzer
    );


updateDashboard();

setInterval(
    updateDashboard,
    1000
);

