const API_URL = "http://localhost:8080/api/status";

async function updateDashboard() {

    try {

        const response = await fetch(API_URL);

        if (!response.ok) {
            throw new Error("Erro na resposta da API");
        }

        const data = await response.json();

        // Bateria
        document.getElementById("battery").textContent =
            `${data.battery}%`;

        // Capacete
        document.getElementById("helmet").textContent =
            data.helmet ? "OK" : "REMOVIDO";

        // Giroscópio
        document.getElementById("gyro-x").textContent =
            data.gyro.x;

        document.getElementById("gyro-y").textContent =
            data.gyro.y;

        document.getElementById("gyro-z").textContent =
            data.gyro.z;

        // Status
        document.getElementById("connection").textContent =
            "● Conectado ao simulador";

    } catch (error) {

        console.error(error);

        document.getElementById("connection").textContent =
            "● Simulador desconectado";
    }
}

// Atualiza imediatamente
updateDashboard();

// Atualiza a cada 1 segundo
setInterval(updateDashboard, 1000);