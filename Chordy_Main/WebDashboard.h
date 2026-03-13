// ============================================================
//  WebDashboard.h  —  Full Sci-Fi Cyberpunk Dashboard HTML
//  Stored as a PROGMEM string literal to serve from ESP32
// ============================================================
#pragma once

const char DASHBOARD_HTML[] PROGMEM = R"HTMLEOF(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>CHORDY // Control Interface</title>
<style>
  @import url('https://fonts.googleapis.com/css2?family=Orbitron:wght@400;600;700;900&family=Share+Tech+Mono&display=swap');

  :root {
    --c: #00ffe7;
    --c2: #ff2d78;
    --c3: #7b2fff;
    --bg: #030912;
    --bg2: #061525;
    --bg3: #0a1e35;
    --glass: rgba(0,255,231,0.05);
    --border: rgba(0,255,231,0.2);
  }

  * { box-sizing: border-box; margin: 0; padding: 0; }

  body {
    background: var(--bg);
    color: var(--c);
    font-family: 'Share Tech Mono', monospace;
    min-height: 100vh;
    overflow-x: hidden;
  }

  /* Animated grid background */
  body::before {
    content: '';
    position: fixed; inset: 0; z-index: 0;
    background-image:
      linear-gradient(rgba(0,255,231,0.03) 1px, transparent 1px),
      linear-gradient(90deg, rgba(0,255,231,0.03) 1px, transparent 1px);
    background-size: 40px 40px;
    animation: gridMove 20s linear infinite;
  }
  @keyframes gridMove {
    0% { background-position: 0 0; }
    100% { background-position: 40px 40px; }
  }

  /* Scanline effect */
  body::after {
    content: '';
    position: fixed; inset: 0; z-index: 0; pointer-events: none;
    background: repeating-linear-gradient(
      0deg, transparent, transparent 2px,
      rgba(0,0,0,0.05) 2px, rgba(0,0,0,0.05) 4px
    );
  }

  .wrap { position: relative; z-index: 1; max-width: 1200px; margin: 0 auto; padding: 24px 20px; }

  /* Header */
  header {
    display: flex; align-items: center; justify-content: space-between;
    padding: 20px 32px; margin-bottom: 32px;
    background: var(--glass); border: 1px solid var(--border);
    border-radius: 16px;
    backdrop-filter: blur(10px);
  }
  .logo { display: flex; align-items: center; gap: 16px; }
  .logo-icon {
    width: 52px; height: 52px; background: var(--glass);
    border: 2px solid var(--c); border-radius: 12px;
    display: flex; align-items: center; justify-content: center;
    font-size: 1.6rem;
    box-shadow: 0 0 20px rgba(0,255,231,0.3);
    animation: pulse 3s ease-in-out infinite;
  }
  @keyframes pulse {
    0%,100% { box-shadow: 0 0 20px rgba(0,255,231,0.3); }
    50% { box-shadow: 0 0 40px rgba(0,255,231,0.6); }
  }
  .logo-text h1 {
    font-family: 'Orbitron', sans-serif; font-size: 1.6rem; font-weight: 900;
    color: var(--c); letter-spacing: 4px;
    text-shadow: 0 0 20px rgba(0,255,231,0.5);
  }
  .logo-text p { color: rgba(0,255,231,0.5); font-size: .75rem; letter-spacing: 2px; }

  .header-status {
    display: flex; align-items: center; gap: 10px;
    background: rgba(0,255,231,0.05);
    border: 1px solid rgba(0,255,231,0.2); border-radius: 8px;
    padding: 10px 16px;
    font-size: .8rem; color: rgba(0,255,231,0.7);
  }
  .dot {
    width: 8px; height: 8px; border-radius: 50%;
    background: #00ff88;
    box-shadow: 0 0 8px #00ff88;
    animation: blink 1.5s ease-in-out infinite;
  }
  @keyframes blink { 0%,100% { opacity: 1; } 50% { opacity: 0.3; } }

  /* Grid layout */
  .grid { display: grid; grid-template-columns: repeat(12, 1fr); gap: 20px; }
  .col-4 { grid-column: span 4; }
  .col-6 { grid-column: span 6; }
  .col-8 { grid-column: span 8; }
  .col-12 { grid-column: span 12; }
  @media (max-width: 900px) { .col-4, .col-6, .col-8 { grid-column: span 12; } }

  /* Card */
  .card {
    background: var(--glass); border: 1px solid var(--border);
    border-radius: 16px; padding: 24px;
    backdrop-filter: blur(10px);
    position: relative; overflow: hidden;
    transition: border-color .3s;
  }
  .card:hover { border-color: rgba(0,255,231,0.5); }
  .card::before {
    content: ''; position: absolute; top: 0; left: 0; right: 0; height: 2px;
    background: linear-gradient(90deg, transparent, var(--c), transparent);
    opacity: 0.5;
  }

  .card-title {
    font-family: 'Orbitron', sans-serif; font-size: .7rem; font-weight: 600;
    color: rgba(0,255,231,0.6); letter-spacing: 3px; text-transform: uppercase;
    margin-bottom: 20px; display: flex; align-items: center; gap: 10px;
  }
  .card-title::after {
    content: ''; flex: 1; height: 1px;
    background: linear-gradient(90deg, rgba(0,255,231,0.3), transparent);
  }

  /* Metric tiles */
  .metric {
    text-align: center; padding: 16px 12px;
    background: rgba(0,0,0,0.3); border-radius: 12px;
    border: 1px solid rgba(0,255,231,0.1);
  }
  .metric-value {
    font-family: 'Orbitron', sans-serif; font-size: 2rem; font-weight: 700;
    color: var(--c); text-shadow: 0 0 20px rgba(0,255,231,0.5);
    line-height: 1;
  }
  .metric-label { font-size: .7rem; color: rgba(0,255,231,0.5); margin-top: 8px; letter-spacing: 2px; }
  .metrics-row { display: grid; grid-template-columns: repeat(2, 1fr); gap: 12px; }
  .metrics-row-3 { display: grid; grid-template-columns: repeat(3, 1fr); gap: 12px; }

  /* LDR Bar */
  .ldr-bar-wrap { margin-top: 12px; }
  .ldr-bar-track {
    height: 8px; background: rgba(0,0,0,0.5); border-radius: 4px;
    border: 1px solid rgba(0,255,231,0.2); overflow: hidden; margin-top: 6px;
  }
  .ldr-bar-fill {
    height: 100%; border-radius: 4px;
    background: linear-gradient(90deg, #7b2fff, #00ffe7);
    box-shadow: 0 0 8px rgba(0,255,231,0.5);
    transition: width .5s ease;
  }
  .ldr-label { display: flex; justify-content: space-between; font-size: .7rem; color: rgba(0,255,231,0.5); }

  /* Eye preview canvas */
  #eyeCanvas {
    display: block; margin: 0 auto; border-radius: 8px;
    border: 1px solid rgba(0,255,231,0.2); background: #000;
  }

  /* Color picker */
  .color-row { display: flex; align-items: center; gap: 12px; margin-bottom: 12px; }
  .color-row label { width: 48px; font-size: .75rem; color: rgba(0,255,231,0.7); }
  input[type=range] {
    flex: 1; height: 4px; border-radius: 2px;
    -webkit-appearance: none; outline: none; cursor: pointer;
  }
  input[type=range]::-webkit-slider-thumb {
    -webkit-appearance: none; width: 16px; height: 16px;
    border-radius: 50%; border: 2px solid var(--c);
    background: var(--bg); cursor: pointer;
  }
  #rangeR { background: linear-gradient(90deg, #000, #ff2d78); }
  #rangeG { background: linear-gradient(90deg, #000, #00ffe7); }
  #rangeB { background: linear-gradient(90deg, #000, #7b2fff); }

  .color-preview {
    width: 36px; height: 36px; border-radius: 50%;
    border: 2px solid rgba(255,255,255,0.2);
    box-shadow: 0 0 12px rgba(0,255,231,0.4);
  }

  /* Inputs / form elements */
  input[type=text], input[type=password], select {
    width: 100%; background: rgba(0,0,0,0.4);
    border: 1px solid rgba(0,255,231,0.2); border-radius: 8px;
    padding: 10px 14px; color: var(--c);
    font-family: 'Share Tech Mono', monospace; font-size: .9rem;
    outline: none; transition: border-color .2s; margin-bottom: 12px;
  }
  input[type=text]:focus, input[type=password]:focus {
    border-color: var(--c); box-shadow: 0 0 8px rgba(0,255,231,0.2);
  }
  .field-label { font-size: .7rem; color: rgba(0,255,231,0.6); letter-spacing: 2px; margin-bottom: 6px; }

  /* Buttons */
  .btn {
    background: linear-gradient(135deg, rgba(0,255,231,0.1), rgba(0,255,231,0.2));
    border: 1px solid rgba(0,255,231,0.4); border-radius: 8px;
    padding: 10px 20px; color: var(--c);
    font-family: 'Orbitron', sans-serif; font-size: .7rem; font-weight: 700;
    letter-spacing: 2px; cursor: pointer; text-transform: uppercase;
    transition: all .2s; white-space: nowrap;
  }
  .btn:hover {
    background: linear-gradient(135deg, rgba(0,255,231,0.2), rgba(0,255,231,0.4));
    box-shadow: 0 0 16px rgba(0,255,231,0.3);
    transform: translateY(-1px);
  }
  .btn:active { transform: translateY(0); }
  .btn-primary {
    background: linear-gradient(135deg, rgba(0,255,231,0.2), rgba(0,255,231,0.4));
    border-color: var(--c); box-shadow: 0 0 12px rgba(0,255,231,0.2);
  }
  .btn-pink {
    background: linear-gradient(135deg, rgba(255,45,120,0.1), rgba(255,45,120,0.2));
    border-color: rgba(255,45,120,0.5); color: #ff2d78;
  }
  .btn-pink:hover { background: rgba(255,45,120,0.25); box-shadow: 0 0 16px rgba(255,45,120,0.3); }
  .btn-purple {
    background: linear-gradient(135deg, rgba(123,47,255,0.1), rgba(123,47,255,0.2));
    border-color: rgba(123,47,255,0.5); color: #7b2fff;
  }
  .btn-purple:hover { background: rgba(123,47,255,0.25); box-shadow: 0 0 16px rgba(123,47,255,0.3); }

  /* Dev trigger grid */
  .trigger-grid {
    display: grid; grid-template-columns: repeat(3, 1fr); gap: 10px;
  }
  @media (max-width: 600px) { .trigger-grid { grid-template-columns: repeat(2,1fr); } }

  /* Weather card */
  .weather-banner {
    display: flex; align-items: center; gap: 20px;
    padding: 16px; background: rgba(0,0,0,0.3); border-radius: 12px;
    border: 1px solid rgba(0,255,231,0.1); margin-bottom: 16px;
  }
  .weather-icon { font-size: 2.5rem; }
  .weather-temp {
    font-family: 'Orbitron', sans-serif; font-size: 2.2rem; font-weight: 700;
    color: var(--c); text-shadow: 0 0 20px rgba(0,255,231,0.4);
  }
  .weather-desc { color: rgba(0,255,231,0.6); font-size: .8rem; margin-top: 4px; }

  /* Timer */
  .timer-display {
    font-family: 'Orbitron', sans-serif; font-size: 2.8rem; font-weight: 700;
    text-align: center; color: var(--c);
    text-shadow: 0 0 24px rgba(0,255,231,0.6);
    margin: 20px 0; letter-spacing: 4px;
  }
  .timer-bar-track {
    height: 6px; background: rgba(0,0,0,0.5); border-radius: 3px;
    overflow: hidden; margin-bottom: 16px;
  }
  .timer-bar-fill {
    height: 100%; background: linear-gradient(90deg, var(--c3), var(--c));
    box-shadow: 0 0 8px rgba(0,255,231,0.5);
    transition: width 1s linear;
  }
  .timer-inputs { display: flex; gap: 10px; align-items: center; }
  .timer-inputs input[type=number] {
    width: 80px; background: rgba(0,0,0,0.4);
    border: 1px solid rgba(0,255,231,0.3); border-radius: 8px;
    padding: 10px; color: var(--c);
    font-family: 'Orbitron', monospace; font-size: 1.1rem; text-align: center;
    outline: none;
  }

  /* Toast */
  #toast {
    position: fixed; bottom: 32px; right: 32px; z-index: 100;
    background: rgba(0,20,30,0.95); border: 1px solid var(--c);
    border-radius: 12px; padding: 14px 24px;
    color: var(--c); font-size: .9rem;
    box-shadow: 0 0 20px rgba(0,255,231,0.3);
    opacity: 0; transform: translateY(20px);
    transition: all .3s; pointer-events: none;
  }
  #toast.show { opacity: 1; transform: translateY(0); }

  /* State badge */
  .state-badge {
    display: inline-flex; align-items: center; gap: 8px;
    background: rgba(0,255,231,0.05); border: 1px solid rgba(0,255,231,0.2);
    border-radius: 20px; padding: 6px 14px; font-size: .75rem;
    color: rgba(0,255,231,0.8); letter-spacing: 1px;
  }
</style>
</head>
<body>
<div class="wrap">

<!-- Header -->
<header>
  <div class="logo">
    <div class="logo-icon">👾</div>
    <div class="logo-text">
      <h1 id="hdrName">CHORDY</h1>
      <p>DESK COMPANION // v1.0</p>
    </div>
  </div>
  <div style="display:flex;gap:12px;align-items:center;">
    <div class="state-badge"><span class="dot"></span><span id="stateName">ONLINE</span></div>
    <div class="header-status"><span class="dot"></span>CONNECTED</div>
  </div>
</header>

<!-- Row 1: Telemetry + Weather + Eye Preview -->
<div class="grid">

  <!-- Internal Sensors -->
  <div class="card col-4">
    <div class="card-title">Internal Sensors</div>
    <div class="metrics-row" style="margin-bottom:16px">
      <div class="metric">
        <div class="metric-value" id="temp">--</div>
        <div class="metric-label">TEMP °C</div>
      </div>
      <div class="metric">
        <div class="metric-value" id="hum">--</div>
        <div class="metric-label">HUMIDITY %</div>
      </div>
    </div>
    <div class="ldr-bar-wrap">
      <div class="ldr-label">
        <span style="color:rgba(0,255,231,0.6);font-size:.7rem;letter-spacing:2px">AMBIENT LIGHT</span>
        <span id="ldrVal">--</span>
      </div>
      <div class="ldr-bar-track">
        <div class="ldr-bar-fill" id="ldrBar" style="width:50%"></div>
      </div>
    </div>
    <div style="margin-top:14px;display:flex;align-items:center;gap:10px;font-size:.8rem;color:rgba(0,255,231,0.6)">
      <span>PIR:</span>
      <span id="pirDot" style="width:10px;height:10px;border-radius:50%;background:#444;display:inline-block;transition:background .3s"></span>
      <span id="pirStatus">No Motion</span>
    </div>
  </div>

  <!-- Weather -->
  <div class="card col-4">
    <div class="card-title">External Weather</div>
    <div class="weather-banner">
      <div class="weather-icon" id="weatherIcon">🌤️</div>
      <div>
        <div class="weather-temp" id="extTemp">--°C</div>
        <div class="weather-desc" id="weatherDesc">Fetching...</div>
      </div>
    </div>
    <div class="metrics-row">
      <div class="metric">
        <div class="metric-value" id="windSpeed" style="font-size:1.3rem">--</div>
        <div class="metric-label">WIND km/h</div>
      </div>
      <div class="metric">
        <div class="metric-value" id="weatherCode" style="font-size:1.3rem">--</div>
        <div class="metric-label">COND CODE</div>
      </div>
    </div>
  </div>

  <!-- Eye Preview -->
  <div class="card col-4">
    <div class="card-title">Eye Preview</div>
    <canvas id="eyeCanvas" width="200" height="100"></canvas>
    <div style="margin-top:14px;text-align:center;font-size:.75rem;color:rgba(0,255,231,0.5)">
      Live colour preview
    </div>
  </div>

  <!-- Timer -->
  <div class="card col-6">
    <div class="card-title">Focus Timer</div>
    <div class="timer-display" id="timerDisplay">00:00</div>
    <div class="timer-bar-track">
      <div class="timer-bar-fill" id="timerBar" style="width:0%"></div>
    </div>
    <div class="timer-inputs">
      <input type="number" id="timerMin" value="25" min="1" max="180">
      <span style="color:rgba(0,255,231,0.5)">min</span>
      <button class="btn btn-primary" onclick="setTimer()">▶ START</button>
      <button class="btn" onclick="cancelTimer()">■ STOP</button>
    </div>
  </div>

  <!-- Settings -->
  <div class="card col-6">
    <div class="card-title">Configuration</div>
    <div class="field-label">BOT NAME</div>
    <input type="text" id="cfgName" placeholder="Chordy">
    <div class="field-label">LOCATION</div>
    <input type="text" id="cfgLocation" placeholder="London">
    <div class="field-label">OWM API KEY</div>
    <input type="text" id="cfgOwmKey" placeholder="(optional)">
    <button class="btn btn-primary" onclick="saveSettings()" style="width:100%;margin-top:4px">
      ⬆ SAVE CONFIG
    </button>
  </div>

  <!-- Eye Colour -->
  <div class="card col-6">
    <div class="card-title">Eye Colour Picker</div>
    <div class="color-row">
      <label>RED</label>
      <input type="range" id="rangeR" min="0" max="255" value="0" oninput="updateColor()">
      <span id="valR" style="width:28px;text-align:right;font-size:.8rem">0</span>
    </div>
    <div class="color-row">
      <label>GREEN</label>
      <input type="range" id="rangeG" min="0" max="255" value="255" oninput="updateColor()">
      <span id="valG" style="width:28px;text-align:right;font-size:.8rem">255</span>
    </div>
    <div class="color-row">
      <label>BLUE</label>
      <input type="range" id="rangeB" min="0" max="255" value="200" oninput="updateColor()">
      <span id="valB" style="width:28px;text-align:right;font-size:.8rem">200</span>
    </div>
    <div style="display:flex;align-items:center;gap:16px;margin-top:8px">
      <div class="color-preview" id="colorPreview" style="background:rgb(0,255,200)"></div>
      <span style="font-size:.75rem;color:rgba(0,255,231,0.6)" id="colorHex">#00FFC8</span>
      <button class="btn" onclick="saveSettings()" style="margin-left:auto">APPLY ▶</button>
    </div>
  </div>

  <!-- Developer Section -->
  <div class="card col-6">
    <div class="card-title" style="color:#ff2d78;text-shadow:0 0 10px rgba(255,45,120,0.5)">⚡ Developer Panel</div>
    <p style="font-size:.75rem;color:rgba(255,45,120,0.6);margin-bottom:16px;letter-spacing:1px">
      TRIGGER ANIMATIONS DIRECTLY ON CHORDY
    </p>
    <div class="trigger-grid">
      <button class="btn" onclick="trigger(2)">😄 Happy</button>
      <button class="btn" onclick="trigger(3)">🥰 Very Happy</button>
      <button class="btn btn-pink" onclick="trigger(4)">😱 Scared</button>
      <button class="btn" onclick="trigger(5)">😴 Sleepy</button>
      <button class="btn" onclick="trigger(1)">👁 Blink</button>
      <button class="btn" onclick="trigger(6)">😑 Squint</button>
      <button class="btn btn-purple" onclick="trigger(7)">💕 Heart Eyes</button>
      <button class="btn" onclick="trigger(8)">👈 Look Left</button>
      <button class="btn" onclick="trigger(9)">👉 Look Right</button>
      <button class="btn btn-pink" onclick="trigger(10)">🥶 Cold Anim</button>
      <button class="btn btn-pink" onclick="trigger(11)">🥵 Hot Anim</button>
      <button class="btn btn-pink" onclick="trigger(12)">🌧 Rain Anim</button>
    </div>
  </div>

</div><!-- /grid -->
</div><!-- /wrap -->

<div id="toast">✓ Command sent</div>

<script>
const STATE_NAMES = ["BOOT","WIFI_SETUP","CONNECTING","IDLE","HAPPY","VERY_HAPPY",
  "SCARED","SLEEPY","BRIGHT","COLD","HOT","RAINY","INTERACTING","TIMER","SLEEPING"];

const WEATHER_ICONS = {
  "2": "⛈️", "3": "🌦️", "5": "🌧️", "6": "❄️", "7": "🌫️",
  "800": "☀️", "80": "⛅", "": "🌤️"
};

let eyeR = 0, eyeG = 255, eyeB = 200;
let timerTotal = 0, timerRemaining = 0, timerInterval = null;

// ── Canvas eye preview ────────────────────────────────────────
const canvas = document.getElementById('eyeCanvas');
const ctx = canvas.getContext('2d');
let eyeFrame = 0;

function drawEyePreview() {
  ctx.clearRect(0, 0, 200, 100);
  ctx.fillStyle = '#000';
  ctx.fillRect(0, 0, 200, 100);

  const color = `rgb(${eyeR},${eyeG},${eyeB})`;
  const blink = 1 - Math.abs(Math.sin(eyeFrame * 0.02)) * 0.1;
  const pupilX = Math.sin(eyeFrame * 0.03) * 4;
  const pupilY = Math.cos(eyeFrame * 0.04) * 3;

  [[52, 50], [148, 50]].forEach(([cx, cy]) => {
    // Eye white
    ctx.fillStyle = color;
    ctx.shadowColor = color;
    ctx.shadowBlur = 12;
    ctx.beginPath();
    ctx.ellipse(cx, cy, 26, 20 * blink, 0, 0, Math.PI * 2);
    ctx.fill();
    // Pupil
    ctx.fillStyle = '#000';
    ctx.shadowBlur = 0;
    ctx.beginPath();
    ctx.arc(cx + pupilX, cy + pupilY, 7, 0, Math.PI * 2);
    ctx.fill();
    // Shine
    ctx.fillStyle = 'rgba(255,255,255,0.9)';
    ctx.beginPath();
    ctx.arc(cx + pupilX + 3, cy + pupilY - 3, 2, 0, Math.PI * 2);
    ctx.fill();
  });
  ctx.shadowBlur = 0;
  eyeFrame++;
  requestAnimationFrame(drawEyePreview);
}
drawEyePreview();

// ── Color picker ─────────────────────────────────────────────
function updateColor() {
  eyeR = parseInt(document.getElementById('rangeR').value);
  eyeG = parseInt(document.getElementById('rangeG').value);
  eyeB = parseInt(document.getElementById('rangeB').value);
  document.getElementById('valR').textContent = eyeR;
  document.getElementById('valG').textContent = eyeG;
  document.getElementById('valB').textContent = eyeB;
  const hex = '#' + [eyeR,eyeG,eyeB].map(v=>v.toString(16).padStart(2,'0')).join('').toUpperCase();
  document.getElementById('colorPreview').style.background = `rgb(${eyeR},${eyeG},${eyeB})`;
  document.getElementById('colorHex').textContent = hex;
  document.getElementById('colorPreview').style.boxShadow = `0 0 16px rgb(${eyeR},${eyeG},${eyeB})`;
}

// ── Telemetry polling ─────────────────────────────────────────
async function pollTelemetry() {
  try {
    const r = await fetch('/api/telemetry');
    const d = await r.json();

    document.getElementById('temp').textContent = d.temp_c.toFixed(1);
    document.getElementById('hum').textContent = d.humidity.toFixed(0);
    document.getElementById('ldrVal').textContent = d.ldr;
    const pct = Math.round((4095 - d.ldr) / 4095 * 100);
    document.getElementById('ldrBar').style.width = pct + '%';

    const pirEl = document.getElementById('pirDot');
    const pirTxt = document.getElementById('pirStatus');
    pirEl.style.background = d.pir ? '#00ff88' : '#444';
    pirEl.style.boxShadow = d.pir ? '0 0 8px #00ff88' : 'none';
    pirTxt.textContent = d.pir ? 'Motion!' : 'No Motion';

    if (d.weather_valid) {
      document.getElementById('extTemp').textContent = d.weather_temp.toFixed(1) + '°C';
      document.getElementById('weatherDesc').textContent = d.weather_desc;
      document.getElementById('windSpeed').textContent = d.weather_wind.toFixed(1);
      document.getElementById('weatherCode').textContent = d.weather_code || '--';
      const prefix = String(Math.floor((d.weather_code || 800) / 100));
      const icon = WEATHER_ICONS[String(d.weather_code)] || WEATHER_ICONS[prefix] || '🌤️';
      document.getElementById('weatherIcon').textContent = icon;
    }

    const sn = STATE_NAMES[d.state] || 'UNKNOWN';
    document.getElementById('stateName').textContent = sn;

    // Timer
    if (d.timer_active) {
      timerRemaining = d.timer_remaining;
      updateTimerDisplay();
    }

  } catch(e) { /* offline */ }
}

setInterval(pollTelemetry, 3000);
pollTelemetry();

// ── Timer display ─────────────────────────────────────────────
function updateTimerDisplay() {
  const m = Math.floor(timerRemaining / 60).toString().padStart(2,'0');
  const s = (timerRemaining % 60).toString().padStart(2,'0');
  document.getElementById('timerDisplay').textContent = m + ':' + s;
  const pct = timerTotal > 0 ? (1 - timerRemaining / timerTotal) * 100 : 0;
  document.getElementById('timerBar').style.width = pct + '%';
}

async function setTimer() {
  const mins = parseInt(document.getElementById('timerMin').value) || 25;
  timerTotal = mins * 60;
  timerRemaining = timerTotal;
  const r = await fetch('/api/timer', {
    method: 'POST', headers: {'Content-Type':'application/json'},
    body: JSON.stringify({minutes: mins})
  });
  toast('Timer set: ' + mins + ' min');
  if (timerInterval) clearInterval(timerInterval);
  timerInterval = setInterval(() => {
    if (timerRemaining > 0) { timerRemaining--; updateTimerDisplay(); }
    else clearInterval(timerInterval);
  }, 1000);
}

async function cancelTimer() {
  const r = await fetch('/api/timer', {
    method: 'POST', headers: {'Content-Type':'application/json'},
    body: JSON.stringify({minutes: 0})
  });
  if (timerInterval) clearInterval(timerInterval);
  timerRemaining = 0; timerTotal = 0;
  updateTimerDisplay();
  toast('Timer cancelled');
}

// ── Save settings ─────────────────────────────────────────────
async function saveSettings() {
  const payload = {
    botName: document.getElementById('cfgName').value || 'Chordy',
    location: document.getElementById('cfgLocation').value || 'London',
    owmKey: document.getElementById('cfgOwmKey').value || '',
    eyeR, eyeG, eyeB
  };
  const r = await fetch('/api/settings', {
    method: 'POST', headers: {'Content-Type':'application/json'},
    body: JSON.stringify(payload)
  });
  toast('Settings saved!');
}

// ── Trigger animation ─────────────────────────────────────────
async function trigger(id) {
  const r = await fetch('/api/trigger', {
    method: 'POST', headers: {'Content-Type':'application/json'},
    body: JSON.stringify({anim: id})
  });
  const names = ['IDLE','BLINK','HAPPY','VERY_HAPPY','SCARED','SLEEPY',
    'SQUINT','HEART_EYES','LOOK_LEFT','LOOK_RIGHT','COLD','HOT','RAIN'];
  toast('▶ Triggered: ' + (names[id] || id));
}

// ── Toast ─────────────────────────────────────────────────────
let toastTimer = null;
function toast(msg) {
  const el = document.getElementById('toast');
  el.textContent = '✓ ' + msg;
  el.classList.add('show');
  if (toastTimer) clearTimeout(toastTimer);
  toastTimer = setTimeout(() => el.classList.remove('show'), 2500);
}
</script>
</body>
</html>
)HTMLEOF";
