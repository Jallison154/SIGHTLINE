const $ = (id) => document.getElementById(id);

const POLL_STATUS_MS = 200;
const POLL_CONFIG_MS = 2000;
let pollInFlight = false;
let cachedProfiles = [];
let currentStatus = {};
let currentConfig = {};
const nodeId = new URLSearchParams(window.location.search).get("nodeId") || "fixture-1";

function setMsg(text) {
  $("msg").textContent = text;
}

function setBadge(id, text, level) {
  const el = $(id);
  el.textContent = text;
  el.classList.remove("ok", "warn", "bad");
  if (level) el.classList.add(level);
}

async function api(path, options = {}) {
  const res = await fetch(path, {
    headers: { "Content-Type": "application/json" },
    ...options,
  });
  if (!res.ok) throw new Error(`${res.status} ${res.statusText}`);
  return res.json();
}

function withNodeId(path) {
  const sep = path.includes("?") ? "&" : "?";
  return `${path}${sep}nodeId=${encodeURIComponent(nodeId)}`;
}

function num(id, fallback = 0) {
  const v = Number.parseInt($(id).value, 10);
  return Number.isFinite(v) ? v : fallback;
}

function intText(v, fallback = 0) {
  const n = Number.parseInt(String(v ?? ""), 10);
  return String(Number.isFinite(n) ? n : fallback);
}

function textOrFallback(v, fallback = "-") {
  const t = String(v ?? "").trim();
  return t ? t : fallback;
}

function applyConfig(cfg) {
  currentConfig = cfg || {};
  $("cfgNodeName").value = cfg.nodeName || "";
  $("cfgFixtureLabel").value = cfg.fixtureLabel || "";
  $("cfgUniverse").value = cfg.universe ?? 0;
  $("cfgDmxStart").value = cfg.dmxStartAddress ?? 1;
  $("cfgDhcp").checked = !!cfg.dhcp;
  $("cfgStaticIp").value = cfg.staticIp || "";
  $("cfgSubnet").value = cfg.subnetMask || "";
  $("cfgGateway").value = cfg.gateway || "";
  $("headerNodeName").textContent = `Node: ${cfg.nodeName || "-"}`;
  $("infoNodeName").textContent = cfg.nodeName || "-";
  $("infoTarget").textContent = cfg.fixtureLabel || cfg.nodeName || nodeId;
  toggleStaticIpFields();
}

function applyStatus(st) {
  currentStatus = st || {};
  setBadge("stOnline", st.networkReady ? "Online" : "Offline", st.networkReady ? "ok" : "bad");
  $("stTarget").textContent = textOrFallback(currentConfig.fixtureLabel || currentConfig.nodeName || st.nodeId, "No target");
  setBadge("stClaimed", st.assignedControllerId || "Waiting for controller", st.assignedControllerId ? "ok" : "warn");
  $("stPan").textContent = intText(st.pan, 0);
  $("stTilt").textContent = intText(st.tilt, 0);
  $("stIntensity").textContent = intText(st.intensity, 0);

  $("infoNodeId").textContent = st.nodeId || "-";
  $("infoIp").textContent = st.ip || "-";
  $("infoFirmware").textContent = st.firmwareVersion || "-";
  $("infoController").textContent = st.assignedControllerId || "Waiting for controller";
  $("infoClaimStatus").textContent = st.assignedControllerId ? "Claimed" : "Waiting for controller";
  $("infoLastUpdate").textContent = st.lastControlUpdateMs ? new Date(st.lastControlUpdateMs).toLocaleTimeString() : "-";
  $("dbgNodeState").textContent = JSON.stringify(st, null, 2);

  $("stDmxSubset").textContent = formatDmxSubset(st.dmxPreview || []);
  $("stControlUpdate").textContent = st.lastControlUpdateMs ? new Date(st.lastControlUpdateMs).toLocaleTimeString() : "-";
  $("stSignal").textContent = st.signalStatus || (st.controlRxReady ? "Receiving" : "No signal");
}

function formatDmxSubset(arr) {
  if (!Array.isArray(arr) || arr.length === 0) return "-";
  return arr
    .slice(0, 16)
    .map((v, idx) => `${idx + 1}:${v}`)
    .join(" ");
}

function applyProfiles(data) {
  const select = $("profileSelect");
  const prev = select.value;
  cachedProfiles = data.profiles || [];
  select.innerHTML = "";
  cachedProfiles.forEach((p) => {
    const o = document.createElement("option");
    o.value = p.id;
    o.textContent = p.fixtureName || p.id;
    if (o.value === (data.activeProfileId || prev)) o.selected = true;
    select.appendChild(o);
  });
  $("currentProfileName").textContent = data.activeProfileId || "-";
  loadProfileEditorFromSelected();
}

function getSelectedProfile() {
  const id = $("profileSelect").value;
  return cachedProfiles.find((p) => p.id === id) || null;
}

function loadProfileEditorFromSelected() {
  const p = getSelectedProfile();
  if (!p) return;
  const ch = p.channels || {};

  $("pfPanCoarse").value = ch.pan?.coarse ?? "";
  $("pfPanFine").value = ch.pan?.fine ?? "";
  $("pfTiltCoarse").value = ch.tilt?.coarse ?? "";
  $("pfTiltFine").value = ch.tilt?.fine ?? "";
  $("pfIntensity").value = ch.intensity?.channel ?? ch.dimmer?.channel ?? "";
  $("pfIris").value = ch.iris?.channel ?? "";
  $("pfZoom").value = ch.zoom?.channel ?? "";
  $("pfFocus").value = ch.focus?.channel ?? "";
  $("pfShutter").value = ch.shutter?.channel ?? "";
  $("pfColor").value = ch.color?.channel ?? "";
}

function buildProfileFromEditor() {
  const p = getSelectedProfile() || {};
  const out = {
    id: p.id || "edited_profile",
    fixtureName: p.fixtureName || "Edited Profile",
    channels: {
      pan: { coarse: num("pfPanCoarse", 1), fine: num("pfPanFine", 2) },
      tilt: { coarse: num("pfTiltCoarse", 3), fine: num("pfTiltFine", 4) },
      intensity: { channel: num("pfIntensity", 5) },
      iris: { channel: num("pfIris", 0) },
      zoom: { channel: num("pfZoom", 0) },
      focus: { channel: num("pfFocus", 0) },
      shutter: { channel: num("pfShutter", 0) },
      color: { channel: num("pfColor", 0) },
    },
  };
  return out;
}

function toggleStaticIpFields() {
  $("staticIpFields").style.display = $("cfgDhcp").checked ? "none" : "block";
}

async function loadConfig() {
  const cfg = await api(withNodeId("/api/config"));
  applyConfig(cfg);
}

async function saveConfig() {
  const payload = {
    nodeName: $("cfgNodeName").value.trim(),
    fixtureLabel: $("cfgFixtureLabel").value.trim(),
    universe: num("cfgUniverse", 0),
    dmxStartAddress: num("cfgDmxStart", 1),
    dhcp: $("cfgDhcp").checked,
    staticIp: $("cfgStaticIp").value.trim(),
    subnetMask: $("cfgSubnet").value.trim(),
    gateway: $("cfgGateway").value.trim(),
  };
  await api(withNodeId("/api/config"), { method: "POST", body: JSON.stringify(payload) });
  setMsg("Config saved");
}

async function loadStatus() {
  const st = await api(withNodeId("/api/status"));
  applyStatus(st);
}

async function loadProfiles() {
  const data = await api(withNodeId("/api/profiles"));
  applyProfiles(data);
}

async function applySelectedProfile() {
  const profileId = $("profileSelect").value;
  if (!profileId) return;
  await api(withNodeId("/api/profile"), { method: "POST", body: JSON.stringify({ action: "apply", profileId }) });
  $("currentProfileName").textContent = profileId;
  setMsg(`Applied profile: ${profileId}`);
}

async function saveProfile() {
  const profile = buildProfileFromEditor();
  await api(withNodeId("/api/profile"), {
    method: "POST",
    body: JSON.stringify({ action: "save", profile }),
  });
  setMsg(`Profile saved: ${profile.id}`);
  await loadProfiles();
}

async function exportProfile() {
  const profileId = $("profileSelect").value;
  if (!profileId) return;
  const data = await api(withNodeId(`/api/profile/export?profileId=${encodeURIComponent(profileId)}`));
  const blob = new Blob([JSON.stringify(data, null, 2)], { type: "application/json" });
  const url = URL.createObjectURL(blob);
  const a = document.createElement("a");
  a.href = url;
  a.download = `${profileId}.json`;
  a.click();
  URL.revokeObjectURL(url);
}

async function importProfilePlaceholder() {
  const file = $("importProfileFile").files[0];
  if (!file) {
    setMsg("Choose a JSON file first");
    return;
  }
  // Placeholder uses text payload to keep transport lightweight and framework-free.
  const text = await file.text();
  await api(withNodeId("/api/profile/import"), {
    method: "POST",
    body: JSON.stringify({ fileName: file.name, profileJson: text }),
  });
  setMsg(`Imported profile: ${file.name}`);
  await loadProfiles();
}

async function pollStatusLoop() {
  if (pollInFlight) return;
  pollInFlight = true;
  try {
    await loadStatus();
  } catch (err) {
    setMsg(`Status poll failed: ${err.message}`);
  } finally {
    pollInFlight = false;
  }
}

function setupTabs() {
  const tabButtons = [$("tabControlBtn"), $("tabInfoBtn"), $("tabServiceBtn")];
  const panels = {
    control: $("tabControl"),
    info: $("tabInfo"),
    service: $("tabService"),
  };
  tabButtons.forEach((btn) => {
    btn.addEventListener("click", () => {
      tabButtons.forEach((b) => b.classList.remove("active"));
      btn.classList.add("active");
      Object.keys(panels).forEach((k) => panels[k].classList.toggle("active", k === btn.dataset.tab));
    });
  });
}

function bindUi() {
  setupTabs();
  $("cfgDhcp").addEventListener("change", toggleStaticIpFields);
  $("saveConfigBtn").addEventListener("click", async () => {
    try {
      await saveConfig();
    } catch (err) {
      setMsg(`Save config failed: ${err.message}`);
    }
  });
  $("rebootBtn").addEventListener("click", () => {
    setMsg("TODO: bind reboot endpoint");
  });
  $("profileSelect").addEventListener("change", loadProfileEditorFromSelected);
  $("applyProfileBtn").addEventListener("click", async () => {
    try {
      await applySelectedProfile();
    } catch (err) {
      setMsg(`Apply profile failed: ${err.message}`);
    }
  });
  $("saveProfileBtn").addEventListener("click", async () => {
    try {
      await saveProfile();
    } catch (err) {
      setMsg(`Save profile failed: ${err.message}`);
    }
  });
  $("exportProfileBtn").addEventListener("click", async () => {
    try {
      await exportProfile();
    } catch (err) {
      setMsg(`Export failed: ${err.message}`);
    }
  });
  $("importProfileBtn").addEventListener("click", async () => {
    try {
      await importProfilePlaceholder();
    } catch (err) {
      setMsg(`Import failed: ${err.message}`);
    }
  });
}

async function init() {
  bindUi();
  $("headerNodeName").textContent = `Node: ${nodeId}`;
  try {
    await Promise.all([loadConfig(), loadStatus(), loadProfiles()]);
    setMsg("Ready");
  } catch (err) {
    setMsg(`Initial load failed: ${err.message}`);
  }
  setInterval(pollStatusLoop, POLL_STATUS_MS);
  setInterval(async () => {
    try {
      await Promise.all([loadConfig(), loadProfiles()]);
    } catch {
      // Silent periodic sync; detailed errors remain in explicit actions / status poll.
    }
  }, POLL_CONFIG_MS);
}

window.addEventListener("DOMContentLoaded", init);
