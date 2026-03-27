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

function toggleNetworkGroups() {
  const wifi = byId("networkMode").value === "wifi-station";
  byId("wifiGroup").style.display = wifi ? "block" : "none";
  byId("ethGroup").style.display = wifi ? "none" : "block";
}

function readForm() {
  const isDhcp = byId("ipMode").value === "dhcp";
  return {
    nodeName: byId("nodeName").value.trim(),
    fixtureLabel: byId("fixtureLabel").value.trim(),
    networkMode: byId("networkMode").value,
    wifiSsid: byId("wifiSsid").value.trim(),
    wifiPassword: byId("wifiPassword").value,
    fallbackToSetupAp: byId("fallbackToSetupAp").checked,
    setupApSsid: byId("setupApSsid").value.trim(),
    setupApPassword: byId("setupApPassword").value,
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
  byId("networkMode").value = cfg.networkMode || "ethernet";
  byId("wifiSsid").value = cfg.wifiSsid || "";
  byId("wifiPassword").value = "";
  byId("fallbackToSetupAp").checked = cfg.fallbackToSetupAp !== false;
  byId("setupApSsid").value = cfg.setupApSsid || "SIGHTLINE-Setup";
  byId("setupApPassword").value = "";
  byId("universe").value = cfg.universe ?? 0;
  byId("dmxStartAddress").value = cfg.dmxStartAddress ?? 1;
  byId("ipMode").value = cfg.dhcp ? "dhcp" : "static";
  byId("staticIp").value = cfg.staticIp || "";
  byId("subnetMask").value = cfg.subnetMask || "";
  byId("gateway").value = cfg.gateway || "";
  toggleStaticGroup();
  toggleNetworkGroups();
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
  byId("netMode").textContent = s.networkMode === "wifi-station" ? "Wi-Fi Station" : (s.networkMode === "setup-ap" ? "Setup AP" : "Ethernet");
  byId("netState").textContent = s.networkState || "-";
  byId("netIp").textContent = s.networkIp || "-";
  byId("netSetupMode").textContent = s.setupMode ? "Active" : "Off";
  byId("netSsid").textContent = s.networkSsid || "-";
  byId("setupBanner").hidden = !s.setupMode;
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
byId("networkMode").addEventListener("change", toggleNetworkGroups);
byId("saveBtn").addEventListener("click", async () => {
  try {
    await postConfig("/api/config");
    setMessage("Saved. Use Save + Apply to switch network mode.");
  } catch (e) {
    setMessage(e.message, true);
  }
});

byId("applyBtn").addEventListener("click", async () => {
  try {
    const res = await postConfig("/api/config/apply");
    setMessage(res.note ? "Saved. Reboot to apply network changes." : "Saved and applied.");
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
