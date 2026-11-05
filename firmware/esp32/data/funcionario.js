const byId = (id) => document.getElementById(id);
const form = byId("employee-form");
const msg = byId("form-msg");
const DEFAULT_MSG = "Confira as informações antes de salvar.";

function say(text, type = "") {
    msg.textContent = text;
    msg.className = "hint " + type;
}

// Máscara de telefone: (00) 00000-0000
byId("contato_telefone").addEventListener("input", (e) => {
    let d = e.target.value.replace(/\D/g, "").slice(0, 11);
    if (d.length > 6) d = d.replace(/^(\d{2})(\d{4,5})(\d{4})$/, "($1) $2-$3");
    else if (d.length > 2) d = d.replace(/^(\d{2})(\d+)/, "($1) $2");
    else if (d.length) d = "(" + d;
    e.target.value = d;
});

function validate() {
    const rules = {
        nome: (v) => v.trim().length >= 3 || "Informe o nome completo.",
        contato_nome: (v) => v.trim().length >= 2 || "Informe o nome do contato.",
        contato_telefone: (v) => v.replace(/\D/g, "").length >= 10 || "Informe um telefone com DDD."
    };
    let ok = true;
    Object.entries(rules).forEach(([id, rule]) => {
        const input = byId(id);
        const result = rule(input.value);
        const err = document.querySelector(`[data-for="${id}"]`);
        input.classList.toggle("invalid", result !== true);
        err.textContent = result === true ? "" : result;
        if (result !== true) ok = false;
    });
    return ok;
}

form.addEventListener("input", (e) => {
    e.target.classList.remove("invalid");
    const err = document.querySelector(`[data-for="${e.target.id}"]`);
    if (err) err.textContent = "";
});

form.addEventListener("reset", () => {
    document.querySelectorAll(".err").forEach((el) => (el.textContent = ""));
    document.querySelectorAll(".invalid").forEach((el) => el.classList.remove("invalid"));
    say(DEFAULT_MSG);
});

form.addEventListener("submit", async (e) => {
    e.preventDefault();
    if (!validate()) {
        say("Corrija os campos destacados.", "bad");
        return;
    }
    const data = Object.fromEntries(new FormData(form));
    byId("btn-save").disabled = true;
    say("Salvando...");
    try {
        const response = await fetch("/api/funcionario", {
            method: "POST",
            headers: {"Content-Type": "application/json"},
            body: JSON.stringify(data)
        });
        if (!response.ok) throw new Error("Erro ao salvar");
        say("Cadastro salvo.", "ok");
    } catch (error) {
        console.error(error);
        say("Não foi possível salvar o cadastro. Tente novamente.", "bad");
    } finally {
        byId("btn-save").disabled = false;
    }
});

// Carrega o cadastro existente (se o ESP32 tiver um salvo)
(async function load() {
    try {
        const r = await fetch("/api/funcionario");
        if (!r.ok) return;
        const d = await r.json();
        Object.entries(d).forEach(([k, v]) => { if (byId(k)) byId(k).value = v; });
    } catch (_) {}
})();

// Estado do ESP32 na barra superior
(async function ping() {
    try {
        const r = await fetch("/api/status");
        if (!r.ok) throw 0;
        byId("nav-esp").textContent = "ESP32 conectado";
        byId("nav-esp").className = "pill on";
    } catch (_) {
        byId("nav-esp").textContent = "ESP32 desconectado";
        byId("nav-esp").className = "pill off";
    }
    setTimeout(ping, 5000);
})();