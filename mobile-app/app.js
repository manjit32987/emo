/**
 * Mobo Bot - EMO Companion Web App Controller
 * Features: Web Bluetooth Low Energy (BLE 5.0), Multi-Alarms, Audio Control, Eye Customizer
 */

// Nordic UART Service (NUS) UUIDs
const NUS_SERVICE = "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
const NUS_RX      = "6e400002-b5a3-f393-e0a9-e50e24dcca9e"; // Browser -> ESP32
const NUS_TX      = "6e400003-b5a3-f393-e0a9-e50e24dcca9e"; // ESP32 -> Browser

let bleDevice = null;
let rxCharacteristic = null;
let txCharacteristic = null;
let isConnected = false;

// Register Service Worker for PWA (Installable on mobile home screens)
if ('serviceWorker' in navigator) {
  navigator.serviceWorker.register('./sw.js').catch(console.error);
}

// ============================================================================
// BLUETOOTH LOW ENERGY (BLE) MANAGEMENT
// ============================================================================

async function connectBle() {
  if (isConnected && bleDevice && bleDevice.gatt.connected) {
    bleDevice.gatt.disconnect();
    return;
  }

  if (!navigator.bluetooth) {
    alert("Web Bluetooth is supported on Google Chrome (Android/Windows/Mac) or Bluefy/WebBLE (iPhone/iPad).");
    return;
  }

  try {
    console.log("[BLE] Requesting EMO-Companion...");
    bleDevice = await navigator.bluetooth.requestDevice({
      filters: [{ name: "EMO-Companion" }],
      optionalServices: [NUS_SERVICE]
    });

    bleDevice.addEventListener('gattserverdisconnected', onDisconnected);

    console.log("[BLE] Connecting to GATT Server...");
    const server = await bleDevice.gatt.connect();

    console.log("[BLE] Getting Primary Service...");
    const service = await server.getPrimaryService(NUS_SERVICE);

    rxCharacteristic = await service.getCharacteristic(NUS_RX);
    txCharacteristic = await service.getCharacteristic(NUS_TX);

    await txCharacteristic.startNotifications();
    txCharacteristic.addEventListener('characteristicvaluechanged', onTelemetryReceived);

    setConnectionState(true);
    triggerMood('HAPPY', 1); // Play greeting on connect
  } catch (err) {
    console.error("[BLE] Connection error:", err);
    setConnectionState(false);
  }
}

function onDisconnected() {
  console.log("[BLE] Disconnected from EMO-Companion");
  setConnectionState(false);
}

function setConnectionState(connected) {
  isConnected = connected;
  const dot = document.getElementById('bleDot');
  const btn = document.getElementById('bleConnectBtn');

  if (connected) {
    dot.classList.add('online');
    btn.classList.add('connected');
    btn.innerHTML = "<span>• Online</span>";
  } else {
    dot.classList.remove('online');
    btn.classList.remove('connected');
    btn.innerHTML = "<span>Connect BLE</span>";
  }
}

// Send JSON string packet to ESP32 over BLE
function sendBleCommand(obj) {
  const json = JSON.stringify(obj);
  console.log("[BLE TX]", json);

  if (rxCharacteristic && isConnected) {
    const encoder = new TextEncoder();
    rxCharacteristic.writeValueWithoutResponse(encoder.encode(json)).catch(console.error);
  }
}

// Receive telemetry from ESP32
function onTelemetryReceived(event) {
  const decoder = new TextDecoder();
  const text = decoder.decode(event.target.value);
  try {
    const data = JSON.parse(text);
    // Alarm ringing state
    const banner = document.getElementById('alarmBanner');
    if (data.alarmRing) {
      banner.style.display = 'flex';
    } else {
      banner.style.display = 'none';
    }
  } catch (err) {
    // Non-JSON packet ignored
  }
}

// ============================================================================
// MOODS, AUDIO & CONTROLS
// ============================================================================

function triggerMood(moodName, soundTrack = 0) {
  document.querySelectorAll('.mood-btn').forEach(btn => {
    btn.classList.remove('active');
    if (btn.innerText.includes(moodName)) btn.classList.add('active');
  });

  sendBleCommand({ cmd: "mood", val: moodName });
  if (soundTrack > 0) {
    sendBleCommand({ cmd: "sound", val: soundTrack });
  }
}

function playSound(trackNumber) {
  sendBleCommand({ cmd: "sound", val: trackNumber });
}

function setVolume(val, btnElement) {
  document.querySelectorAll('.vol-pill').forEach(p => p.classList.remove('active'));
  if (btnElement) btnElement.classList.add('active');

  document.getElementById('volLabel').innerText = val + " / 30";
  sendBleCommand({ cmd: "vol", val: val });
}

function sendStyle(styleIndex) {
  document.querySelectorAll('.style-btn').forEach((btn, i) => {
    btn.classList.toggle('active', i === styleIndex);
  });
  sendBleCommand({ cmd: "style", val: styleIndex });
}

function toggleAutoMode(isEnabled) {
  sendBleCommand({ cmd: "mode", val: isEnabled ? "auto" : "manual" });
}

// ============================================================================
// ALARMS & TIME SYNC
// ============================================================================

function syncPhoneTime() {
  const now = new Date();
  sendBleCommand({
    cmd: "time",
    h: now.getHours(),
    m: now.getMinutes(),
    s: now.getSeconds()
  });
  alert("Synced companion clock to phone time: " + now.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' }));
}

function updateAlarm(index) {
  const timeInput = document.getElementById('alarmTime' + index);
  const enableSwitch = document.getElementById('alarmEn' + index);
  const [h, m] = timeInput.value.split(':').map(Number);

  sendBleCommand({
    cmd: "alarm",
    idx: index,
    h: h,
    m: m,
    en: enableSwitch.checked ? 1 : 0
  });
}

function dismissAlarm() {
  document.getElementById('alarmBanner').style.display = 'none';
  sendBleCommand({ cmd: "alarm_act", val: "dismiss" });
}

// ============================================================================
// NAVIGATION & TABS
// ============================================================================

function switchTab(viewId, navBtn) {
  document.querySelectorAll('.view').forEach(v => v.classList.remove('active'));
  document.querySelectorAll('.nav-btn').forEach(b => b.classList.remove('active'));

  const targetView = document.getElementById(viewId);
  if (targetView) targetView.classList.add('active');
  if (navBtn) navBtn.classList.add('active');
}

// Filter pills on Controls view (All, Alarms, Sound, Modes, Eyes)
function filterControls(category, pillElement) {
  document.querySelectorAll('.pill-tag').forEach(p => p.classList.remove('active'));
  if (pillElement) pillElement.classList.add('active');

  const cards = document.querySelectorAll('#tab-controls .card');
  cards.forEach(c => {
    if (category === 'all' || c.dataset.category === category) {
      c.style.display = 'block';
    } else {
      c.style.display = 'none';
    }
  });
}

// ============================================================================
// COMPANION INTERACTIVE CHAT
// ============================================================================

function sendChat() {
  const input = document.getElementById('chatInput');
  const txt = input.value.trim();
  if (!txt) return;

  const chatList = document.getElementById('chatList');
  chatList.innerHTML += `
    <div class="bubble-wrap user">
      <div class="bubble user">${txt}</div>
    </div>
  `;
  input.value = '';

  const q = txt.toLowerCase();
  setTimeout(() => {
    let reply = "I am listening! 🤖";

    if (q.includes("happy") || q.includes("smile")) {
      triggerMood("HAPPY", 2);
      reply = "Giggling happily with smiling crescent cheeks! 🥰";
    } else if (q.includes("love") || q.includes("heart") || q.includes("kiss")) {
      triggerMood("LOVE", 3);
      reply = "Beating heart eyes! I love you too! 💖";
    } else if (q.includes("sleep") || q.includes("night") || q.includes("tired")) {
      triggerMood("SLEEPY", 6);
      reply = "Yawning and drifting to sleep... Zzz 😴";
    } else if (q.includes("dizzy") || q.includes("spin") || q.includes("shake")) {
      triggerMood("DIZZY", 4);
      reply = "Whoa, everything is spinning around! 😵";
    } else if (q.includes("alarm") || q.includes("wake")) {
      reply = "You can manage and toggle all your daily alarms in the Controls tab!";
    } else if (q.includes("who are you") || q.includes("what are you")) {
      reply = "I'm your EMO desktop companion powered by ESP32-C3!";
    }

    chatList.innerHTML += `
      <div class="bubble-wrap bot">
        <div class="bubble bot">${reply}</div>
      </div>
    `;
    chatList.scrollTop = chatList.scrollHeight;
  }, 450);
}

