const bApps = document.getElementById("Apps");
const key = document.getElementById("key");
const lockAccept = document.getElementById("accept-lock");
const lockDecline = document.getElementById("decline-lock");
const appTemplate = document.querySelector(".app");
const hiddenApps = [];
const blockedApps = [];

async function sendReq(command) {
    try {
        const res = await fetch("http://localhost:8080", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ command })
        });
        if (!res.ok) throw new Error("failed request");
        return res;
    } catch (err) {
        console.log("fetch error:", err);
        return null;
    }
}

bApps.onclick = async() => {
    const motivation = document.getElementById("motivation");
    const dApps = document.getElementById("dApps");
    dApps.innerHTML = "";
    const output = [];

    motivation.style.display = "none";

    const res = await sendReq("apps");
    if (!res) return;

    const apps = await res.json();
    const processNames = apps.process || [];

    processNames.forEach(process => {
        if (output.includes(process)) return;
        if (hiddenApps.includes(process)) return;

        const app = appTemplate.cloneNode(true);
        app.removeAttribute("id");
        app.querySelector("h2").innerText = process;
        dApps.appendChild(app);

        const toggle = app.querySelector("a");
        toggle.innerText = "off";
        toggle.style.color = "red";

        toggle.onclick = () => {
            if (toggle.innerText === "off") {
                toggle.innerText = "on";
                toggle.style.color = "green";
                blockedApps.push(process);
            } else {
                toggle.innerText = "off";
                toggle.style.color = "red";
                blockedApps.splice(blockedApps.indexOf(process), 1);
            }
        };

        output.push(process);
    });

    dApps.style.display = "block";
    key.style.visibility = "visible";
};

key.onclick = () => {
    const warning = document.getElementById("warning");
    warning.style.display = "block";

    lockAccept.onclick = () => {
        blockedApps.forEach(app => {
            sendReq(`${app} blocked`);
        });
        warning.style.display = "none";
        warning.style.position = "relative";
    };

    lockDecline.onclick = () => {
        warning.style.display = "none";
        warning.style.position = "relative";
    };
};