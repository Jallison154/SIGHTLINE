const $ = (id) => document.getElementById(id);

const sliderIds = ["pan", "tilt", "intensity", "iris", "zoom"];
let lastUiInputMs = 0;
let pollInFlight = false;

function updateSliderLabels() {
  $("panVal").textContent = Number($("pan").value).toFixed(3);
  $("tiltVal").textContent = Number($("tilt").value).toFixed(3);
  $("intensityVal").textContent = $("intensity").value;
  $("irisVal").textContent = $("iris").value;
  $("zoomVal").textContent = $("zoom").value;
}

async function api(path, options = {}) {
  const res = await fetch(path, {
    headers: { "Content-Type": "application/json" },
    ...options,
  });
  if (!res.ok) {
    throw new Error(`${res.status} ${res.statusText}`);
  }
  return res.json();
}

async function pushControlPatch(patch) {
  try {
    await api("/api/control-state", {
      method: "POST",
      body: JSON.stringify(patch),
    });
    $("controlMsg").textContent = "Control update sent";
  } catch (err) {
    $("controlMsg").textContent = `Control update failed: ${err.message}`;
  }
}

function bindControlInputs() {
  sliderIds.forEach((id) => {
    $(id).addEventListener("input", () => {
      lastUiInputMs = Date.now();
      updateSliderLabels();
      const patch = {};
      patch[id] = id === "pan" || id === "tilt" ? Number($(id).value) : Number.parseInt($(id).value, 10);
      pushControlPatch(patch);
    });
  });

  $("inputMode").addEventListener("change", () => {
    pushControlPatch({ inputMode: $("inputMode").value });
  });

  $("blackoutOnBtn").addEventListener("click", () => postAction("blackout_on"));
  $("blackoutOffBtn").addEventListener("click", () => postAction("blackout_off"));
  $("homeBtn").addEventListener("click", () => postAction("home"));
  $("speedBtn").addEventListener("click", () => {
    $("controlMsg").textContent = "TODO: expose full speed/sensitivity tuning UI";
  });

  $("claimBtn").addEventListener("click", () => postOwnership("claim"));
  $("releaseBtn").addEventListener("click", () => postOwnership("release"));
  $("refreshTargetsBtn").addEventListener("click", refreshTargets);
  $("selectTargetBtn").addEventListener("click", selectTarget);
}

async function postAction(action) {
  try {
    await api("/api/action", { method: "POST", body: JSON.stringify({ action }) });
    $("controlMsg").textContent = `Action sent: ${action}`;
  } catch (err) {
    $("controlMsg").textContent = `Action failed: ${err.message}`;
  }
}

async function postOwnership(action) {
  try {
    await api("/api/ownership", { method: "POST", body: JSON.stringify({ action }) });
    $("controlMsg").textContent = `Ownership action: ${action}`;
  } catch (err) {
    $("controlMsg").textContent = `Ownership failed: ${err.message}`;
  }
}

async function refreshTargets() {
  try {
    const d = await api("/api/targets");
    const select = $("targetNode");
    const prior = select.value;
    select.innerHTML = "";
    (d.targets || []).forEach((t) => {
      const option = document.createElement("option");
      option.value = t.nodeId;
      option.textContent = `${t.friendlyName || t.nodeId} (${t.status}) ${t.ip || ""}`;
      if (t.nodeId === d.activeTargetNodeId || t.nodeId === prior) option.selected = true;
      select.appendChild(option);
    });
  } catch (err) {
    $("controlMsg").textContent = `Target refresh failed: ${err.message}`;
  }
}

async function selectTarget() {
  const nodeId = $("targetNode").value;
  if (!nodeId) return;
  try {
    await api("/api/targets/select", { method: "POST", body: JSON.stringify({ nodeId }) });
    $("controlMsg").textContent = `Active target set: ${nodeId}`;
  } catch (err) {
    $("controlMsg").textContent = `Select target failed: ${err.message}`;
  }
}

function applyStateToUi(state) {
  // Avoid immediate slider jump while user is dragging.
  const allowPull = Date.now() - lastUiInputMs > 350;
  if (allowPull) {
    $("pan").value = state.pan ?? 0.5;
    $("tilt").value = state.tilt ?? 0.5;
    $("intensity").value = state.intensity ?? 0;
    $("iris").value = state.iris ?? 0;
    $("zoom").value = state.zoom ?? 0;
    $("inputMode").value = state.inputMode || "live_mixed";
    updateSliderLabels();
  }
  const src = state.perControlSource || {};
  ["pan", "tilt", "intensity", "iris", "zoom"].forEach((key) => {
    const el = $(`${key}Src`);
    if (el) el.textContent = `src:${src[key] || "-"}`;
  });
}

function applyStatusToUi(status) {
  $("stControllerId").textContent = status.controllerId || "-";
  $("stNetwork").textContent = String(!!status.networkReady);
  $("stDiscovery").textContent = String(!!status.discoveryReady);
  $("stOwnership").textContent = String(!!status.ownershipReady);
  $("stControlTx").textContent = String(!!status.controlTxReady);
  $("stWeb").textContent = String(!!status.webReady);
  $("stOwnsTarget").textContent = String(!!status.ownsTarget);
  $("stFramesSent").textContent = String(status.controlFramesSent ?? 0);
  $("stLastSource").textContent = status.lastUpdateSource || "-";
  $("stUptime").textContent = `${Math.floor((status.uptimeMs || 0) / 1000)}s`;
}

async function poll() {
  if (pollInFlight) return;
  pollInFlight = true;
  try {
    const [state, status] = await Promise.all([api("/api/control-state"), api("/api/status")]);
    applyStateToUi(state);
    applyStatusToUi(status);
  } catch (err) {
    $("controlMsg").textContent = `Polling failed: ${err.message}`;
  } finally {
    pollInFlight = false;
  }
}

function init() {
  bindControlInputs();
  updateSliderLabels();
  refreshTargets();
  poll();
  setInterval(poll, 250);
  setInterval(refreshTargets, 2000);
}

window.addEventListener("DOMContentLoaded", init);
