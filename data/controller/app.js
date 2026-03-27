const $ = (id) => document.getElementById(id);

const controlConfig = [
  { key: "pan", label: "Pan", min: 0, max: 65535, step: 1 },
  { key: "tilt", label: "Tilt", min: 0, max: 65535, step: 1 },
  { key: "intensity", label: "Intensity", min: 0, max: 255, step: 1 },
  { key: "iris", label: "Iris", min: 0, max: 255, step: 1 },
  { key: "zoom", label: "Zoom", min: 0, max: 255, step: 1 },
  { key: "focus", label: "Focus", min: 0, max: 255, step: 1 },
  { key: "shutter", label: "Shutter", min: 0, max: 255, step: 1 },
  { key: "color", label: "Color", min: 0, max: 255, step: 1 },
];

const POLL_MS = 150;
let pollInFlight = false;
let lastUiInputMs = 0;
let stateCache = {};
let targetClaimed = false;
let statusCache = {};
let fixtureInfoCache = { supportedControls: [] };
let lastPollOkMs = 0;
let activeControlKey = "";

// Parse integer safely for slider + number input sync.
function parseIntSafe(v, fallback = 0) {
  const n = Number.parseInt(String(v ?? ""), 10);
  return Number.isFinite(n) ? n : fallback;
}

function num(v) {
  return parseIntSafe(v, 0);
}

// Clamp helper for 8-bit public control values.
function clamp8(v, fallback = 0) {
  return Math.max(0, Math.min(255, parseIntSafe(v, fallback)));
}

// Clamp helper for 16-bit public pan/tilt values.
function clamp16(v, fallback = 0) {
  return Math.max(0, Math.min(65535, parseIntSafe(v, fallback)));
}

function fmtInt(v, fallback = 0) {
  const n = num(v);
  return String(Number.isFinite(n) ? n : fallback);
}

function textOrFallback(v, fallback = "-") {
  const t = String(v ?? "").trim();
  return t ? t : fallback;
}

function roleText(role) {
  return role === "fixture_node" ? "Fixture Node" : "Controller";
}

function normalizeControlValue(key, value, fallback = 0) {
  if (key === "pan" || key === "tilt") return clamp16(value, fallback);
  return clamp8(value, fallback);
}

async function api(path, options = {}) {
  const res = await fetch(path, {
    headers: { "Content-Type": "application/json" },
    ...options,
  });
  if (!res.ok) throw new Error(`${res.status} ${res.statusText}`);
  return res.json();
}

function formatTimeAny(v) {
  if (!v) return "-";
  if (typeof v === "number") return new Date(v).toLocaleTimeString();
  return String(v);
}

function sourceForKey(state, key) {
  const per = state.perControlSource || {};
  return per[key] || state.lastUpdateSource || "-";
}

function setBadge(id, text, level) {
  const el = $(id);
  el.textContent = text;
  el.classList.remove("ok", "warn", "bad");
  if (level) el.classList.add(level);
}

function connectionHealthText() {
  const age = Date.now() - lastPollOkMs;
  if (!lastPollOkMs) return "Offline";
  if (age > 4000) return "Offline";
  return "Online";
}

function renderControlRows() {
  const host = $("controlsHost");
  host.innerHTML = "";
  controlConfig.forEach((c) => {
    const row = document.createElement("div");
    row.className = "control-row";
    row.id = `row_${c.key}`;
    row.innerHTML = `
      <label for="ctl_${c.key}">${c.label}</label>
      <input id="ctl_${c.key}" type="range" min="${c.min}" max="${c.max}" step="${c.step}" />
      <input id="num_${c.key}" type="number" min="${c.min}" max="${c.max}" step="${c.step}" />
      <span id="val_${c.key}" class="value">-</span>
    `;
    host.appendChild(row);

    const slider = $(`ctl_${c.key}`);
    const number = $(`num_${c.key}`);

    const onInput = (raw, sourceEl) => {
      lastUiInputMs = Date.now();
      activeControlKey = c.key;
      const currentFallback = parseIntSafe(sourceEl?.value, c.min);
      const value = normalizeControlValue(c.key, raw, currentFallback);
      slider.value = String(value);
      number.value = String(value);
      $(`val_${c.key}`).textContent = fmtInt(value);
      sendPatch({ [c.key]: value });
    };

    const clearInteraction = () => {
      activeControlKey = "";
    };

    slider.addEventListener("input", () => onInput(slider.value, slider));
    number.addEventListener("input", () => onInput(number.value, number));
    slider.addEventListener("change", clearInteraction);
    number.addEventListener("change", clearInteraction);
    slider.addEventListener("pointerup", clearInteraction);
    number.addEventListener("blur", clearInteraction);
  });
}

function controlSupported(key) {
  const supported = fixtureInfoCache.supportedControls || [];
  // If backend does not provide fixture info, default to visible controls.
  return supported.length === 0 || supported.includes(key);
}

function renderFixtureAwareControls() {
  let visibleCount = 0;
  controlConfig.forEach((c) => {
    const row = $(`row_${c.key}`);
    const supported = controlSupported(c.key);
    row.classList.toggle("unsupported", !supported);
    row.style.display = supported ? "" : "none";
    if (supported) visibleCount += 1;
  });
  $("controlsHint").textContent =
    visibleCount > 0
      ? `${visibleCount} control(s) available for active fixture profile`
      : "No adjustable controls available for active fixture profile";
}

function updateModeUi(mode) {
  const monitor = mode === "monitor_only";
  controlConfig.forEach((c) => {
    const slider = $(`ctl_${c.key}`);
    const number = $(`num_${c.key}`);
    slider.disabled = monitor || !controlSupported(c.key);
    number.disabled = monitor || !controlSupported(c.key);
  });
  $("sensitivity").disabled = monitor;
  $("blackoutBtn").disabled = monitor;
}

function applyState(state) {
  stateCache = state || {};
  const online = statusCache.networkReady && connectionHealthText() === "Online";
  setBadge("stOnline", online ? "Online" : "Offline", online ? "ok" : "bad");
  $("stTarget").textContent = textOrFallback(fixtureInfoCache.fixtureLabel || state.activeTargetNodeId, "No target");
  const claimedText = statusCache.ownsTarget ? (statusCache.controllerId || "sim-controller-1") : "Waiting for controller";
  setBadge("stClaimed", claimedText, statusCache.ownsTarget ? "ok" : "warn");

  const allowPull = Date.now() - lastUiInputMs > 250;
  controlConfig.forEach((c) => {
    const value = normalizeControlValue(c.key, state[c.key] ?? c.min);
    if (allowPull && activeControlKey !== c.key) {
      $(`ctl_${c.key}`).value = String(value);
      $(`num_${c.key}`).value = String(value);
    }
    const src = sourceForKey(state, c.key);
    const valueText = fmtInt(value);
    $(`val_${c.key}`).textContent = `${valueText} (${src})`;
  });

  if (typeof state.sensitivity === "number") {
    $("sensitivity").value = String(state.sensitivity);
    $("sensitivityVal").textContent = String(state.sensitivity);
  }

  $("interactionMode").value = state.inputMode || "monitor_only";
  updateModeUi($("interactionMode").value);

  $("stPan").textContent = fmtInt(clamp16(state.pan ?? 0));
  $("stTilt").textContent = fmtInt(clamp16(state.tilt ?? 0));
  $("stIntensity").textContent = fmtInt(clamp8(state.intensity ?? 0));

  // Device Information
  const role = statusCache.deviceRole || "controller";
  $("headerRoleText").textContent = `Role: ${roleText(role)}`;
  $("dbgControllerId").textContent = statusCache.deviceId || statusCache.controllerId || "sim-controller-1";
  $("dbgDeviceRole").textContent = roleText(role);
  $("dbgControllerName").textContent = statusCache.deviceName || "SIGHTLINE Controller";
  $("dbgFirmwareVersion").textContent = statusCache.firmwareVersion || "sim-v1.0";
  $("ovTargetNode").textContent = state.activeTargetNodeId || "-";
  $("ovProfileName").textContent = fixtureInfoCache.profileName || "-";
  $("ovControllerStatus").textContent = statusCache.controllerStatus || (statusCache.controlTxReady ? "running" : "idle");
  $("ovLastUpdate").textContent = formatTimeAny(state.lastUpdateMs || statusCache.lastUpdateMs);

  $("dbgRawState").textContent = JSON.stringify(state, null, 2);
  $("dbgLastBackendUpdate").textContent = formatTimeAny(state.lastUpdateMs || statusCache.lastUpdateMs);
  $("dbgLastCommandSource").textContent = state.lastUpdateSource || "-";
  $("dbgTargetNodeId").textContent = state.activeTargetNodeId || "-";
  $("dbgSignalHealth").textContent = statusCache.signalHealth || "ok";
  $("dbgProfileName").textContent = fixtureInfoCache.profileName || "-";
}

function disconnectedUi(errMsg) {
  setBadge("stOnline", "Offline", "bad");
  setBadge("stClaimed", "Waiting for controller", "warn");
  $("msg").textContent = errMsg || "-";
}

async function sendPatch(patch) {
  try {
    await api("/api/state", { method: "POST", body: JSON.stringify(patch) });
  } catch (err) {
    disconnectedUi(`POST failed: ${err.message}`);
  }
}

async function refreshNodes() {
  try {
    const data = await api("/api/nodes");
    const select = $("nodeSelect");
    const prev = select.value;
    select.innerHTML = "";
    (data.nodes || []).forEach((n) => {
      const o = document.createElement("option");
      o.value = n.nodeId || n.id || "";
      o.textContent = n.friendlyName || n.nodeId || n.id || "Unknown Node";
      if (o.value === prev || o.value === (stateCache.activeTargetNodeId || "")) o.selected = true;
      select.appendChild(o);
    });
  } catch (err) {
    disconnectedUi(`Nodes failed: ${err.message}`);
  }
}

async function selectNode() {
  const nodeId = $("nodeSelect").value;
  if (!nodeId) return;
  try {
    await api("/api/select-node", { method: "POST", body: JSON.stringify({ nodeId }) });
  } catch (err) {
    disconnectedUi(`Select failed: ${err.message}`);
  }
}

async function pollState() {
  if (pollInFlight) return;
  pollInFlight = true;
  try {
    const [state, status] = await Promise.all([api("/api/state"), api("/api/status")]);
    lastPollOkMs = Date.now();
    statusCache = status || {};
    applyState(state);
  } catch (err) {
    disconnectedUi(`GET failed: ${err.message}`);
  } finally {
    pollInFlight = false;
  }
}

async function refreshTargetFixture() {
  try {
    fixtureInfoCache = await api("/api/target-fixture");
  } catch {
    fixtureInfoCache = { supportedControls: [] };
  }
  renderFixtureAwareControls();
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

function bindActions() {
  setupTabs();
  $("refreshNodesBtn").addEventListener("click", refreshNodes);
  $("selectNodeBtn").addEventListener("click", selectNode);
  $("claimBtn").addEventListener("click", async () => {
    targetClaimed = !targetClaimed;
    await sendPatch({ claim: targetClaimed });
    $("claimBtn").textContent = "Claim / Release";
  });
  $("blackoutBtn").addEventListener("click", async () => {
    const next = !stateCache.blackout;
    await sendPatch({ blackout: next });
    $("blackoutBtn").textContent = next ? "Blackout Off" : "Blackout";
  });
  $("homeBtn").addEventListener("click", async () => {
    // Placeholder action keeps API compatibility for future real homing.
    await sendPatch({ home: true });
  });
  $("interactionMode").addEventListener("change", async (e) => {
    const mode = e.target.value;
    await sendPatch({ inputMode: mode });
    updateModeUi(mode);
  });
  $("sensitivity").addEventListener("input", async (e) => {
    const v = Number.parseInt(e.target.value, 10) || 0;
    $("sensitivityVal").textContent = String(v);
    lastUiInputMs = Date.now();
    await sendPatch({ sensitivity: v });
  });
  $("svcRebootBtn").addEventListener("click", () => {
    $("msg").textContent = "Reboot placeholder triggered";
  });
  $("svcResetBtn").addEventListener("click", () => {
    $("msg").textContent = "Reset placeholder triggered";
  });
}

function init() {
  renderControlRows();
  renderFixtureAwareControls();
  bindActions();
  refreshNodes();
  refreshTargetFixture();
  pollState();
  setInterval(pollState, POLL_MS);
  setInterval(refreshNodes, 2000);
  setInterval(refreshTargetFixture, 1500);
  // Lightweight heartbeat refresh for connection label when stream goes stale.
  setInterval(() => {
    if (stateCache && Object.keys(stateCache).length > 0) {
      setBadge("stOnline", connectionHealthText(), connectionHealthText() === "Online" ? "ok" : "bad");
    }
  }, 1000);
}

window.addEventListener("DOMContentLoaded", init);
