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

        document.getElementById("gyro-x").textContent =
            data.gyro.x;

        document.getElementById("gyro-y").textContent =
            data.gyro.y;

        document.getElementById("gyro-z").textContent =
            data.gyro.z;

        document.getElementById("connection").textContent =
            "● Conectado ao ESP32";

    } catch (error) {

        console.error(error);

        document.getElementById("connection").textContent =
            "● ESP32 desconectado";
    }
}

updateDashboard();

setInterval(updateDashboard, 1000);