
const byId = (id) => document.getElementById(id);

const form = byId("employee-form");
const msg = byId("form-msg");

const DEFAULT_MSG = "Confira as informações antes de salvar.";

function say(text, type = "") {
    msg.textContent = text;
    msg.className = "hint " + type;
}

// Alterna entre formulário e visualização do perfil.
function showForm() {
    byId("profile-form-view").hidden = false;
    byId("profile-view").hidden = true;
}

function showProfile(profile) {
    byId("profile-form-view").hidden = true;
    byId("profile-view").hidden = false;

    const fields = {
        "profile-nome": profile.nome,
        "profile-contato-nome": profile.contato_nome,
        "profile-contato-telefone": profile.contato_telefone,
        "profile-tipo-sanguineo": profile.tipo_sanguineo,
        "profile-alergias": profile.alergias,
        "profile-observacoes": profile.observacoes
    };

    Object.entries(fields).forEach(([id, value]) => {
        byId(id).textContent = value?.trim() || "Não informado";
    });
}

// Máscara de telefone.
byId("contato_telefone").addEventListener("input", (e) => {
    let d = e.target.value.replace(/\D/g, "").slice(0, 11);

    if (d.length > 6) {
        d = d.replace(/^(\d{2})(\d{4,5})(\d{4})$/, "($1) $2-$3");
    } else if (d.length > 2) {
        d = d.replace(/^(\d{2})(\d+)/, "($1) $2");
    } else if (d.length) {
        d = "(" + d;
    }

    e.target.value = d;
});

function validate() {
    const rules = {
        nome: (v) => v.trim().length >= 3 || "Informe o nome completo.",
        contato_nome: (v) => v.trim().length >= 2 || "Informe o nome do contato.",
        contato_telefone: (v) =>
            v.replace(/\D/g, "").length >= 10 ||
            "Informe um telefone com DDD."
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

// Limpa erros conforme o usuário digita.
form.addEventListener("input", (e) => {
    e.target.classList.remove("invalid");

    const err = document.querySelector(`[data-for="${e.target.id}"]`);
    if (err) err.textContent = "";
});

form.addEventListener("reset", () => {
    document.querySelectorAll(".err").forEach((el) => {
        el.textContent = "";
    });

    document.querySelectorAll(".invalid").forEach((el) => {
        el.classList.remove("invalid");
    });

    say(DEFAULT_MSG);
});

// Converte os dados do formulário em objeto.
function getFormData() {
    return Object.fromEntries(new FormData(form));
}

// Consulta o perfil salvo no ESP32.
async function loadProfile() {
    try {
        const response = await fetch("/api/funcionario", {
            cache: "no-store"
        });

        if (response.status === 404) {
            showForm();
            return;
        }

        if (!response.ok) {
            throw new Error("Não foi possível consultar o perfil.");
        }

        const profile = await response.json();
        showProfile(profile);

    } catch (error) {
        console.error(error);
        showForm();
        say("Erro ao consultar o perfil salvo.", "bad");
    }
}

// Cadastra o funcionário.
form.addEventListener("submit", async (e) => {
    e.preventDefault();

    if (!validate()) {
        say("Corrija os campos destacados.", "bad");
        return;
    }

    const data = getFormData();
    const button = byId("btn-save");

    button.disabled = true;
    say("Salvando...");

    try {
        const response = await fetch("/api/funcionario", {
            method: "POST",
            headers: {
                "Content-Type": "application/json"
            },
            body: JSON.stringify(data)
        });

        if (response.status === 409) {
            say("Este capacete já possui um perfil cadastrado.", "bad");
            await loadProfile();
            return;
        }

        if (!response.ok) {
            throw new Error("Erro ao salvar o perfil.");
        }

        say("Cadastro salvo.", "ok");
        await loadProfile();

    } catch (error) {
        console.error(error);
        say("Não foi possível salvar o cadastro.", "bad");
    } finally {
        button.disabled = false;
    }
});

// Exclui o perfil do ESP32.
byId("btn-delete").addEventListener("click", async () => {
    const confirmed = confirm(
        "Tem certeza que deseja excluir o perfil deste funcionário?"
    );

    if (!confirmed) return;

    const button = byId("btn-delete");
    const profileMsg = byId("profile-msg");

    button.disabled = true;
    profileMsg.textContent = "Excluindo perfil...";

    try {
        const response = await fetch("/api/funcionario", {
            method: "DELETE"
        });

        if (!response.ok) {
            throw new Error("Erro ao excluir o perfil.");
        }

        form.reset();
        showForm();
        say("Perfil excluído com sucesso.", "ok");

    } catch (error) {
        console.error(error);
        profileMsg.textContent = "Não foi possível excluir o perfil.";
    } finally {
        button.disabled = false;
    }
});

// Verifica se existe perfil ao abrir a página.
loadProfile();

// Estado do ESP32 na barra superior.
async function ping() {
    try {
        const response = await fetch("/api/status", {
            cache: "no-store"
        });

        if (!response.ok) throw new Error();

        byId("nav-esp").textContent = "ESP32 conectado";
        byId("nav-esp").className = "pill on";

    } catch (_) {
        byId("nav-esp").textContent = "ESP32 desconectado";
        byId("nav-esp").className = "pill off";
    }

    setTimeout(ping, 5000);
}

ping();
