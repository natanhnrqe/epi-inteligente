const API_URL = "/api/status";
const byId = (id) => document.getElementById(id);
const fmt = (v) => Number(v).toLocaleString("pt-BR", {minimumFractionDigits: 2, maximumFractionDigits: 2});
let lastRead = null;

function setConnection(ok) {
    byId("connection").textContent = ok ? "Recebendo dados do ESP32" : "ESP32 desconectado";
    byId("connection").classList.toggle("off", !ok);
    byId("nav-esp").textContent = ok ? "ESP32 conectado" : "ESP32 desconectado";
    byId("nav-esp").classList.toggle("off", !ok);
    byId("system-pill").textContent = ok ? "Sistema online" : "Sistema offline";
    byId("system-pill").classList.toggle("on", ok);
    byId("system-pill").classList.toggle("off", !ok);
}

function updateBattery(level) {
    const n = Math.max(0, Math.min(100, Number(level)));
    const bar = byId("battery-bar"), label = byId("battery-label");
    byId("battery").textContent = n;
    bar.style.width = n + "%";
    let cls = "", text = "Nível adequado";
    if (n < 20) { cls = "danger"; text = "Nível crítico"; }
    else if (n < 50) { cls = "warn"; text = "Nível baixo"; }
    bar.className = cls;
    label.className = "bar-label " + cls;
    label.textContent = text;
}

function updateHelmet(on) {
    byId("helmet").textContent = on ? "Em uso" : "Removido";
    byId("helmet-sub").textContent = on ? "Equipamento detectado" : "Equipamento não detectado";
    const pill = byId("helmet-pill");
    pill.textContent = on ? "Detectado" : "Não detectado";
    pill.classList.toggle("on", on);
    pill.classList.toggle("off", !on);
}

function updateBuzzerStatus(ativo) {
    byId("buzzer-status").textContent = ativo ? "Buzzer acionado" : "Buzzer desligado";
}

function updateLastRead() {
    if (!lastRead) return;
    const s = Math.round((Date.now() - lastRead) / 1000);
    byId("last-read").textContent = "Última leitura: " + (s < 3 ? "agora" : `há ${s}s`);
}

async function updateDashboard() {
    try {
        const response = await fetch(API_URL);
        if (!response.ok) throw new Error("Erro na resposta da API");
        const data = await response.json();

        updateBattery(data.battery);
        updateHelmet(Boolean(data.helmet));
        ["x", "y", "z"].forEach((a) => {
            byId("accel-" + a).textContent = fmt(data.accelerometer[a]);
            byId("gyro-" + a).textContent = fmt(data.gyroscope[a]);
        });
        updateBuzzerStatus(data.buzzer);

        lastRead = Date.now();
        setConnection(true);
    } catch (error) {
        console.error(error);
        setConnection(false);
    }
    updateLastRead();
}

async function setBuzzer(on) {
    try {
        const response = await fetch(`/api/buzzer/${on ? "on" : "off"}`, {method: "POST"});
        if (!response.ok) throw new Error("Erro ao alterar buzzer");
        updateBuzzerStatus(on);
    } catch (error) {
        console.error(error);
        alert(on ? "Não foi possível acionar o buzzer." : "Não foi possível desligar o buzzer.");
    }
}

byId("buzzer-on").addEventListener("click", () => setBuzzer(true));
byId("buzzer-off").addEventListener("click", () => setBuzzer(false));

updateDashboard();
setInterval(updateDashboard, 1000);