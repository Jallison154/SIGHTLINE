function byId(id) { return document.getElementById(id); }

function setMessage(msg, isError) {
  const el = byId("message");
  el.textContent = msg || "";
  el.style.color = isError ? "#ff9a9a" : "#8fd3ff";
}

function toggleStaticGroup() {
  const isDhcp = byId("ipMode").value === "dhcp";
  byId("staticGroup").style.display = isDhcp ? "none" : "block";
}

function readForm() {
  const isDhcp = byId("ipMode").value === "dhcp";
  return {
    nodeName: byId("nodeName").value.trim(),
    fixtureLabel: byId("fixtureLabel").value.trim(),
    universe: Number(byId("universe").value),
    dmxStartAddress: Number(byId("dmxStartAddress").value),
    dhcp: isDhcp,
    staticIp: byId("staticIp").value.trim(),
    subnetMask: byId("subnetMask").value.trim(),
    gateway: byId("gateway").value.trim()
  };
}

function writeForm(cfg) {
  byId("nodeName").value = cfg.nodeName || "";
  byId("fixtureLabel").value = cfg.fixtureLabel || "";
  byId("universe").value = cfg.universe ?? 0;
  byId("dmxStartAddress").value = cfg.dmxStartAddress ?? 1;
  byId("ipMode").value = cfg.dhcp ? "dhcp" : "static";
  byId("staticIp").value = cfg.staticIp || "";
  byId("subnetMask").value = cfg.subnetMask || "";
  byId("gateway").value = cfg.gateway || "";
  toggleStaticGroup();
}

async function loadConfig() {
  const res = await fetch("/api/config");
  if (!res.ok) throw new Error("Failed to load config");
  writeForm(await res.json());
}

async function loadStatus() {
  const res = await fetch("/api/status");
  if (!res.ok) throw new Error("Failed to load status");
  const s = await res.json();
  byId("sigState").textContent = s.artNetHasSignal ? "Receiving" : "No signal";
  byId("sigUniverse").textContent = String(s.artNetUniverse);
  byId("sigInterval").textContent = `${s.artNetLastFrameIntervalMs} ms`;
  byId("sigAccepted").textContent = String(s.artNetPacketsAccepted);
  byId("sigIgnored").textContent = String(s.artNetPacketsIgnoredUniverse);
  byId("sigBad").textContent = String(s.artNetPacketsBad);
}

async function postConfig(path) {
  const payload = readForm();
  const res = await fetch(path, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(payload)
  });
  const data = await res.json().catch(() => ({}));
  if (!res.ok || !data.ok) {
    throw new Error(data.error || "Save failed");
  }
  return data;
}

byId("ipMode").addEventListener("change", toggleStaticGroup);
byId("saveBtn").addEventListener("click", async () => {
  try {
    await postConfig("/api/config");
    setMessage("Saved.");
  } catch (e) {
    setMessage(e.message, true);
  }
});

byId("applyBtn").addEventListener("click", async () => {
  try {
    const res = await postConfig("/api/config/apply");
    setMessage(res.note ? `Saved + apply requested (${res.note}).` : "Saved + applied.");
  } catch (e) {
    setMessage(e.message, true);
  }
});

async function boot() {
  try {
    await loadConfig();
    await loadStatus();
  } catch (e) {
    setMessage(e.message, true);
  }
  setInterval(() => { loadStatus().catch(() => {}); }, 1000);
}

boot();
