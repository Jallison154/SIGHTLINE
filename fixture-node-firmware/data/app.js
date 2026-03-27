function byId(id) { return document.getElementById(id); }
let lastConfig = null;

function setMessage(msg, isError) {
  const el = byId("message");
  el.textContent = msg || "";
  el.style.color = isError ? "#ff9a9a" : "#8fd3ff";
}

function setBadge(el, text, kind) {
  el.textContent = text;
  el.className = `badge${kind ? ` ${kind}` : ""}`;
}

function modeText(mode) {
  if (mode === "wifi-station") return "Wi-Fi";
  if (mode === "setup-ap") return "Setup AP";
  return "Ethernet";
}

function roleText(role) {
  return role === "controller" ? "Controller" : "Fixture Node";
}

function stateText(state) {
  if (state === "wifi-connected" || state === "ethernet-connected") return "Connected";
  if (state === "wifi-connecting") return "Connecting";
  if (state === "ethernet-link-down") return "Cable disconnected";
  if (state === "setup-ap") return "Setup AP active";
  return state || "-";
}

function isValidIpv4(text) {
  const parts = String(text || "").trim().split(".");
  if (parts.length !== 4) return false;
  for (const p of parts) {
    if (!/^\d+$/.test(p)) return false;
    const n = Number(p);
    if (!Number.isInteger(n) || n < 0 || n > 255) return false;
  }
  return true;
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

function validateForm(payload) {
  if (!payload.nodeName || payload.nodeName.length > 32) {
    return "Node name must be 1-32 characters.";
  }
  if (payload.networkMode === "wifi-station" && !payload.wifiSsid) {
    return "Wi-Fi SSID is required in Wi-Fi mode.";
  }
  if (payload.wifiPassword && payload.wifiPassword.length < 8) {
    return "Wi-Fi password must be at least 8 characters (or left blank to keep current).";
  }
  if (payload.setupApPassword && payload.setupApPassword.length < 8) {
    return "Setup AP password must be at least 8 characters (or blank for open AP).";
  }
  if (!payload.dhcp) {
    if (!isValidIpv4(payload.staticIp) || !isValidIpv4(payload.subnetMask) || !isValidIpv4(payload.gateway)) {
      return "Static IP, subnet mask, and gateway must be valid IPv4 addresses.";
    }
  }
  return "";
}

function writeForm(cfg) {
  lastConfig = {
    networkMode: cfg.networkMode || "ethernet",
    wifiSsid: cfg.wifiSsid || "",
    dhcp: cfg.dhcp !== false,
    staticIp: cfg.staticIp || "",
    subnetMask: cfg.subnetMask || "",
    gateway: cfg.gateway || ""
  };
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
  byId("devRole").textContent = roleText(cfg.deviceRole || "fixture_node");
  byId("devId").textContent = cfg.deviceId || "-";
  byId("devFw").textContent = cfg.firmwareVersion || "-";
  byId("roleBanner").textContent = `Role: ${roleText(cfg.deviceRole || "fixture_node")}`;
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
  byId("netMode").textContent = modeText(s.networkMode);
  setBadge(byId("netConnected"), s.networkConnected ? "Online" : "Offline", s.networkConnected ? "ok" : "warn");
  byId("netState").textContent = stateText(s.networkState);
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
  byId("devRole").textContent = roleText(s.deviceRole || "fixture_node");
  byId("devId").textContent = s.deviceId || byId("devId").textContent || "-";
  byId("devFw").textContent = s.firmwareVersion || byId("devFw").textContent || "-";
  byId("roleBanner").textContent = `Role: ${roleText(s.deviceRole || "fixture_node")}`;
}

async function postConfig(path, extra = {}) {
  const payload = readForm();
  Object.assign(payload, extra);
  const formError = validateForm(payload);
  if (formError) {
    throw new Error(formError);
  }
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

function networkSettingsChanged(payload) {
  if (!lastConfig) return false;
  return payload.networkMode !== lastConfig.networkMode ||
    payload.wifiSsid !== lastConfig.wifiSsid ||
    payload.dhcp !== lastConfig.dhcp ||
    payload.staticIp !== lastConfig.staticIp ||
    payload.subnetMask !== lastConfig.subnetMask ||
    payload.gateway !== lastConfig.gateway;
}

async function postSimple(path) {
  const res = await fetch(path, { method: "POST" });
  const data = await res.json().catch(() => ({}));
  if (!res.ok || !data.ok) {
    throw new Error(data.error || "Request failed");
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
    const payload = readForm();
    const changed = networkSettingsChanged(payload);
    const forceApply = byId("forceNetworkApply").checked;
    if (changed && forceApply) {
      const ok = window.confirm("Applying network changes now can disconnect this page. Continue?");
      if (!ok) return;
    }
    const res = await postConfig("/api/config/apply", { forceNetworkApply: forceApply && changed });
    if (res.note === "network-settings-saved-pending-reconnect") {
      setMessage("Saved. Network changes are staged; use Reconnect or Reboot when ready.");
    } else {
      setMessage("Saved and applied. Network reconfigure requested.");
    }
    await loadConfig();
    await loadStatus();
  } catch (e) {
    setMessage(e.message, true);
  }
});

byId("reconnectBtn").addEventListener("click", async () => {
  try {
    await postSimple("/api/network/reconnect");
    setMessage("Reconnect requested.");
    await loadStatus();
  } catch (e) {
    setMessage(e.message, true);
  }
});

byId("rebootBtn").addEventListener("click", async () => {
  try {
    await postSimple("/api/reboot");
    setMessage("Reboot requested. Device will briefly disconnect.");
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
