function $(id) {
  return document.getElementById(id);
}

function setText(id, value) {
  const el = $(id);
  if (el) el.textContent = value;
}

function setMsg(id, msg, isError) {
  const el = $(id);
  if (!el) return;
  el.textContent = msg || "";
  el.style.color = isError ? "#ff9a9a" : "#8fd3ff";
}

function readConfigForm() {
  const isDhcp = $("ipMode").value === "dhcp";
  return {
    nodeName: $("nodeName").value.trim(),
    fixtureLabel: $("fixtureLabel").value.trim(),
    dhcp: isDhcp,
    staticIp: $("staticIp").value.trim(),
    subnetMask: $("subnetMask").value.trim(),
    gateway: $("gateway").value.trim()
  };
}

function writeConfigForm(cfg) {
  $("nodeName").value = cfg.nodeName || "";
  $("fixtureLabel").value = cfg.fixtureLabel || "";
  $("ipMode").value = cfg.dhcp ? "dhcp" : "static";
  $("staticIp").value = cfg.staticIp || "";
  $("subnetMask").value = cfg.subnetMask || "";
  $("gateway").value = cfg.gateway || "";
}

async function loadConfig() {
  const res = await fetch("/api/config");
  if (!res.ok) throw new Error("Failed to load config");
  const cfg = await res.json();
  writeConfigForm(cfg);
}

async function saveConfig(path, msgId) {
  try {
    const payload = readConfigForm();
    const res = await fetch(path, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(payload)
    });
    const body = await res.json().catch(() => ({}));
    if (!res.ok || !body.ok) {
      throw new Error(body.error || "Save failed");
    }
    setMsg(msgId, "Saved.", false);
  } catch (e) {
    setMsg(msgId, e.message || String(e), true);
  }
}

async function loadStatus() {
  const res = await fetch("/api/status");
  if (!res.ok) throw new Error("Failed to load status");
  const s = await res.json();
  setText("stNetwork", s.networkReady ? "OK" : "Not ready");
  setText("stDiscovery", s.discoveryReady ? "OK" : "Not ready");
  setText("stControl", s.controlRxReady ? "OK" : "Not ready");
  setText("stDmx", s.dmxReady ? "OK" : "Not ready");
  setText("stClaim", s.claimed ? "Claimed" : "Available");
  setText("stController", s.assignedControllerId || "none");
  setText("stProfile", s.activeProfileId || "none");
  setText("stUptime", `${Math.floor((s.uptimeMs || 0) / 1000)} s`);
}

async function loadProfiles() {
  const res = await fetch("/api/profiles");
  if (!res.ok) throw new Error("Failed to load profiles");
  const p = await res.json();
  const select = $("activeProfile");
  const list = $("profilesList");
  select.innerHTML = "";
  list.innerHTML = "";

  (p.profiles || []).forEach((prof) => {
    const opt = document.createElement("option");
    opt.value = prof.id;
    opt.textContent = prof.displayName || prof.id;
    if (prof.id === p.activeProfileId) opt.selected = true;
    select.appendChild(opt);

    const li = document.createElement("li");
    li.textContent = `${prof.id} – ${prof.displayName || ""}`;
    list.appendChild(li);
  });
}

async function activateProfile() {
  try {
    const profileId = $("activeProfile").value;
    const res = await fetch("/api/profiles/activate", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ profileId })
    });
    const body = await res.json().catch(() => ({}));
    if (!res.ok || !body.ok) {
      throw new Error(body.error || "Activate failed");
    }
    setMsg("profileMsg", "Profile activated.", false);
    await loadProfiles();
  } catch (e) {
    setMsg("profileMsg", e.message || String(e), true);
  }
}

async function importProfile() {
  try {
    const text = $("importProfile").value.trim();
    if (!text) throw new Error("Paste profile JSON first");
    const res = await fetch("/api/profiles", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: text
    });
    const body = await res.json().catch(() => ({}));
    if (!res.ok || !body.ok) {
      throw new Error(body.error || "Import failed");
    }
    setMsg("profileMsg", "Profile imported.", false);
    $("importProfile").value = "";
    await loadProfiles();
  } catch (e) {
    setMsg("profileMsg", e.message || String(e), true);
  }
}

$("saveCfgBtn").addEventListener("click", () => saveConfig("/api/config", "cfgMsg"));
$("applyCfgBtn").addEventListener("click", () => saveConfig("/api/config/apply", "cfgMsg"));
$("activateProfileBtn").addEventListener("click", activateProfile);
$("importProfileBtn").addEventListener("click", importProfile);

async function boot() {
  try {
    await loadConfig();
    await loadProfiles();
    await loadStatus();
  } catch (e) {
    setMsg("cfgMsg", e.message || String(e), true);
  }
  setInterval(() => {
    loadStatus().catch(() => {});
  }, 1000);
}

boot();

