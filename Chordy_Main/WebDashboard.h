// ============================================================
//  WebDashboard.h  —  Chordy v2.0 Complete Dashboard
//  Features: virtual buttons, eye style, speed slider,
//  seconds timer, buzzer toggle/volume, factory reset,
//  screen control, new emotion triggers, clock display
// ============================================================
#pragma once

const char DASHBOARD_HTML[] PROGMEM = R"HTMLEOF(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>CHORDY // Control Interface v2</title>
<style>
@import url('https://fonts.googleapis.com/css2?family=Orbitron:wght@400;600;700;900&family=Share+Tech+Mono&display=swap');
:root{
  --c:#00ffe7;--c2:#ff2d78;--c3:#7b2fff;--c4:#ffaa00;
  --bg:#030912;--bg2:#061525;--bg3:#0a1e35;
  --glass:rgba(0,255,231,0.04);--border:rgba(0,255,231,0.18);
}
*{box-sizing:border-box;margin:0;padding:0}
body{background:var(--bg);color:var(--c);font-family:'Share Tech Mono',monospace;min-height:100vh;overflow-x:hidden}
body::before{content:'';position:fixed;inset:0;z-index:0;
  background-image:linear-gradient(rgba(0,255,231,0.025) 1px,transparent 1px),linear-gradient(90deg,rgba(0,255,231,0.025) 1px,transparent 1px);
  background-size:32px 32px;animation:gridMove 24s linear infinite;pointer-events:none}
@keyframes gridMove{0%{background-position:0 0}100%{background-position:32px 32px}}
.wrap{position:relative;z-index:1;max-width:1280px;margin:0 auto;padding:20px 18px}

/* Header */
header{display:flex;align-items:center;justify-content:space-between;flex-wrap:wrap;gap:12px;
  padding:16px 24px;margin-bottom:24px;background:var(--glass);border:1px solid var(--border);
  border-radius:14px;backdrop-filter:blur(8px)}
.logo{display:flex;align-items:center;gap:14px}
.logo-icon{width:46px;height:46px;background:var(--glass);border:2px solid var(--c);border-radius:10px;
  display:flex;align-items:center;justify-content:center;font-size:1.4rem;
  box-shadow:0 0 20px rgba(0,255,231,0.25);animation:pulse 3s ease-in-out infinite}
@keyframes pulse{0%,100%{box-shadow:0 0 16px rgba(0,255,231,0.25)}50%{box-shadow:0 0 36px rgba(0,255,231,0.5)}}
.logo-text h1{font-family:'Orbitron',sans-serif;font-size:1.4rem;font-weight:900;
  color:var(--c);letter-spacing:4px;text-shadow:0 0 16px rgba(0,255,231,0.4)}
.logo-text p{color:rgba(0,255,231,0.45);font-size:.7rem;letter-spacing:2px}
.header-right{display:flex;gap:10px;align-items:center;flex-wrap:wrap}
.status-pill{display:flex;align-items:center;gap:8px;background:rgba(0,255,231,0.05);
  border:1px solid var(--border);border-radius:20px;padding:6px 14px;font-size:.72rem}
.dot{width:7px;height:7px;border-radius:50%;background:#00ff88;box-shadow:0 0 7px #00ff88;animation:blink 1.5s ease-in-out infinite}
@keyframes blink{0%,100%{opacity:1}50%{opacity:.25}}

/* Grid */
.grid{display:grid;grid-template-columns:repeat(12,1fr);gap:16px}
.c3{grid-column:span 3}.c4{grid-column:span 4}.c6{grid-column:span 6}.c8{grid-column:span 8}.c12{grid-column:span 12}
@media(max-width:1000px){.c3,.c4,.c6,.c8{grid-column:span 6}}
@media(max-width:640px){.c3,.c4,.c6,.c8,.c12{grid-column:span 12}}

/* Card */
.card{background:var(--glass);border:1px solid var(--border);border-radius:14px;padding:20px;
  backdrop-filter:blur(8px);position:relative;overflow:hidden;transition:border-color .25s}
.card:hover{border-color:rgba(0,255,231,0.4)}
.card::before{content:'';position:absolute;top:0;left:0;right:0;height:1px;
  background:linear-gradient(90deg,transparent,rgba(0,255,231,0.4),transparent);opacity:.6}
.card-title{font-family:'Orbitron',sans-serif;font-size:.65rem;font-weight:700;
  color:rgba(0,255,231,0.55);letter-spacing:3px;text-transform:uppercase;margin-bottom:16px;
  display:flex;align-items:center;gap:8px}
.card-title::after{content:'';flex:1;height:1px;background:linear-gradient(90deg,rgba(0,255,231,0.25),transparent)}

/* Metrics */
.metric{text-align:center;padding:14px 10px;background:rgba(0,0,0,0.3);border-radius:10px;
  border:1px solid rgba(0,255,231,0.08)}
.mv{font-family:'Orbitron',sans-serif;font-size:1.7rem;font-weight:700;color:var(--c);
  text-shadow:0 0 16px rgba(0,255,231,0.4);line-height:1}
.ml{font-size:.62rem;color:rgba(0,255,231,0.45);margin-top:6px;letter-spacing:2px}
.grid2{display:grid;grid-template-columns:1fr 1fr;gap:10px}
.grid3{display:grid;grid-template-columns:1fr 1fr 1fr;gap:10px}

/* Inputs */
input[type=text],input[type=password],input[type=number],select{
  width:100%;background:rgba(0,0,0,0.4);border:1px solid rgba(0,255,231,0.2);
  border-radius:8px;padding:9px 13px;color:var(--c);
  font-family:'Share Tech Mono',monospace;font-size:.85rem;outline:none;transition:border-color .2s;margin-bottom:10px}
input:focus,select:focus{border-color:var(--c);box-shadow:0 0 8px rgba(0,255,231,0.15)}
select option{background:#030912}
.fl{font-size:.65rem;color:rgba(0,255,231,0.55);letter-spacing:2px;margin-bottom:5px}

/* Range slider */
input[type=range]{width:100%;height:4px;-webkit-appearance:none;outline:none;border-radius:2px;cursor:pointer;margin:6px 0}
input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:16px;height:16px;border-radius:50%;
  border:2px solid var(--c);background:var(--bg);cursor:pointer}
.sr{background:linear-gradient(90deg,#000,#ff2d78)}
.sg{background:linear-gradient(90deg,#000,#00ffe7)}
.sb{background:linear-gradient(90deg,#000,#7b2fff)}
.speed-range{background:linear-gradient(90deg,#7b2fff,#00ffe7)}
.vol-range{background:linear-gradient(90deg,#333,#ffaa00)}

/* Buttons */
.btn{background:rgba(0,255,231,0.06);border:1px solid rgba(0,255,231,0.3);border-radius:8px;
  padding:9px 16px;color:var(--c);font-family:'Orbitron',sans-serif;font-size:.62rem;font-weight:700;
  letter-spacing:1.5px;cursor:pointer;text-transform:uppercase;transition:all .18s;white-space:nowrap;display:inline-flex;align-items:center;gap:6px}
.btn:hover{background:rgba(0,255,231,0.16);box-shadow:0 0 14px rgba(0,255,231,0.25);transform:translateY(-1px)}
.btn:active{transform:translateY(0)}
.btn-p{background:rgba(0,255,231,0.12);border-color:var(--c);box-shadow:0 0 10px rgba(0,255,231,0.15)}
.btn-red{background:rgba(255,45,120,0.07);border-color:rgba(255,45,120,0.4);color:var(--c2)}
.btn-red:hover{background:rgba(255,45,120,0.2);box-shadow:0 0 14px rgba(255,45,120,0.3)}
.btn-pur{background:rgba(123,47,255,0.07);border-color:rgba(123,47,255,0.4);color:var(--c3)}
.btn-pur:hover{background:rgba(123,47,255,0.2);box-shadow:0 0 14px rgba(123,47,255,0.3)}
.btn-ora{background:rgba(255,170,0,0.07);border-color:rgba(255,170,0,0.4);color:var(--c4)}
.btn-ora:hover{background:rgba(255,170,0,0.2);box-shadow:0 0 14px rgba(255,170,0,0.3)}
.btn-full{width:100%;justify-content:center;margin-top:6px}

/* Virtual buttons (physical button simulator) */
.vbtn-row{display:grid;grid-template-columns:1fr 1fr;gap:10px;margin-bottom:6px}
.vbtn{display:flex;flex-direction:column;align-items:center;justify-content:center;gap:5px;
  background:rgba(0,0,0,0.35);border:2px solid rgba(0,255,231,0.2);border-radius:12px;
  padding:14px 10px;cursor:pointer;transition:all .15s;font-family:'Orbitron',sans-serif;
  font-size:.6rem;letter-spacing:1px;color:rgba(0,255,231,0.7);user-select:none;min-height:72px}
.vbtn:hover{background:rgba(0,255,231,0.08);border-color:rgba(0,255,231,0.5);transform:scale(1.02)}
.vbtn:active{transform:scale(0.97);background:rgba(0,255,231,0.18)}
.vbtn-icon{font-size:1.4rem}
.vbtn.active{border-color:var(--c);box-shadow:0 0 16px rgba(0,255,231,0.3);background:rgba(0,255,231,0.1)}

/* Eye canvas */
#eyeCanvas{display:block;margin:0 auto;border-radius:8px;border:1px solid rgba(0,255,231,0.15);background:#000}

/* LDR bar */
.ldr-track{height:6px;background:rgba(0,0,0,0.5);border-radius:3px;overflow:hidden;margin:6px 0;
  border:1px solid rgba(0,255,231,0.15)}
.ldr-fill{height:100%;background:linear-gradient(90deg,var(--c3),var(--c));
  box-shadow:0 0 6px rgba(0,255,231,0.4);transition:width .5s ease}

/* Timer */
.timer-clock{font-family:'Orbitron',sans-serif;font-size:2.4rem;font-weight:700;
  text-align:center;color:var(--c);text-shadow:0 0 20px rgba(0,255,231,0.5);
  letter-spacing:4px;margin:14px 0}
.timer-prog{height:5px;background:rgba(0,0,0,0.5);border-radius:3px;overflow:hidden;margin-bottom:14px}
.timer-bar{height:100%;background:linear-gradient(90deg,var(--c3),var(--c));
  box-shadow:0 0 8px rgba(0,255,231,0.4);transition:width .8s linear}
.timer-inputs{display:flex;gap:8px;align-items:center;flex-wrap:wrap}
.timer-inputs input[type=number]{width:64px;text-align:center;font-family:'Orbitron',monospace;font-size:1rem;margin-bottom:0}
.timer-sep{color:rgba(0,255,231,0.5);font-size:1.2rem}

/* Weather */
.weather-banner{display:flex;align-items:center;gap:16px;padding:14px;
  background:rgba(0,0,0,0.25);border-radius:10px;border:1px solid rgba(0,255,231,0.08);margin-bottom:14px}
.w-icon{font-size:2.2rem}
.w-temp{font-family:'Orbitron',sans-serif;font-size:2rem;font-weight:700;color:var(--c);
  text-shadow:0 0 16px rgba(0,255,231,0.35)}
.w-desc{color:rgba(0,255,231,0.55);font-size:.75rem;margin-top:4px}

/* Clock */
.clock-big{font-family:'Orbitron',sans-serif;font-size:2rem;font-weight:700;
  text-align:center;color:var(--c);text-shadow:0 0 16px rgba(0,255,231,0.4);letter-spacing:3px;margin:8px 0}
.clock-date{text-align:center;color:rgba(0,255,231,0.5);font-size:.8rem;letter-spacing:2px}

/* Trigger grid */
.trig-grid{display:grid;grid-template-columns:repeat(3,1fr);gap:8px}
@media(max-width:500px){.trig-grid{grid-template-columns:repeat(2,1fr)}}

/* Color preview */
.c-preview{width:34px;height:34px;border-radius:50%;border:2px solid rgba(255,255,255,0.15);transition:all .3s;flex-shrink:0}

/* Screen selector */
.screen-pills{display:flex;gap:8px;flex-wrap:wrap;margin-bottom:14px}
.screen-pill{padding:6px 14px;border-radius:20px;font-size:.65rem;font-family:'Orbitron',sans-serif;
  letter-spacing:1px;cursor:pointer;border:1px solid rgba(0,255,231,0.2);
  background:transparent;color:rgba(0,255,231,0.5);transition:all .2s}
.screen-pill.active,.screen-pill:hover{border-color:var(--c);color:var(--c);background:rgba(0,255,231,0.08);box-shadow:0 0 8px rgba(0,255,231,0.2)}

/* Toggle switch */
.toggle-row{display:flex;align-items:center;justify-content:space-between;margin:8px 0}
.toggle{position:relative;display:inline-block;width:44px;height:22px}
.toggle input{opacity:0;width:0;height:0}
.toggle-slider{position:absolute;cursor:pointer;inset:0;background:rgba(0,0,0,0.4);
  border-radius:22px;border:1px solid rgba(0,255,231,0.2);transition:.3s}
.toggle-slider:before{position:absolute;content:'';height:16px;width:16px;left:3px;bottom:3px;
  background:rgba(0,255,231,0.4);border-radius:50%;transition:.3s}
.toggle input:checked+.toggle-slider{background:rgba(0,255,231,0.12);border-color:var(--c);box-shadow:0 0 8px rgba(0,255,231,0.2)}
.toggle input:checked+.toggle-slider:before{transform:translateX(21px);background:var(--c)}
.toggle-label{font-size:.75rem;color:rgba(0,255,231,0.7)}

/* Toast */
#toast{position:fixed;bottom:28px;right:28px;z-index:200;background:rgba(0,20,30,0.96);
  border:1px solid var(--c);border-radius:10px;padding:12px 22px;color:var(--c);font-size:.85rem;
  box-shadow:0 0 20px rgba(0,255,231,0.25);opacity:0;transform:translateY(16px);
  transition:all .28s;pointer-events:none}
#toast.show{opacity:1;transform:translateY(0)}

/* Danger zone */
.danger-zone{border-color:rgba(255,45,120,0.25);background:rgba(255,45,120,0.03)}
.danger-zone .card-title{color:rgba(255,45,120,0.6)}
.danger-zone .card-title::after{background:linear-gradient(90deg,rgba(255,45,120,0.2),transparent)}

/* Section label */
.section-label{font-family:'Orbitron',sans-serif;font-size:.6rem;letter-spacing:3px;
  color:rgba(0,255,231,0.35);margin:20px 0 10px;padding-left:4px}
</style>
</head>
<body>
<div class="wrap">

<!-- ── HEADER ──────────────────────────────────────────────── -->
<header>
  <div class="logo">
    <div class="logo-icon">👾</div>
    <div class="logo-text">
      <h1 id="hdrName">CHORDY</h1>
      <p>DESK COMPANION // v2.0</p>
    </div>
  </div>
  <div class="header-right">
    <div class="status-pill"><span class="dot"></span><span id="stateName">ONLINE</span></div>
    <div class="status-pill" id="timeDisplay">--:--:--</div>
    <div class="status-pill"><span class="dot" style="background:#7b2fff;box-shadow:0 0 7px #7b2fff"></span><span id="locationName">--</span></div>
  </div>
</header>

<div class="grid">

<!-- ── COLUMN 1-4: Virtual Buttons + Eye Preview ──────────── -->

<!-- Virtual Physical Buttons -->
<div class="card c4">
  <div class="card-title">Virtual Buttons</div>
  <div class="vbtn-row">
    <div class="vbtn" id="vb0" onclick="vBtn(0)">
      <span class="vbtn-icon">⏻</span>POWER
    </div>
    <div class="vbtn" id="vb1" onclick="vBtn(1)">
      <span class="vbtn-icon">🔄</span>SCREEN
    </div>
  </div>
  <div class="vbtn-row">
    <div class="vbtn" id="vb2" onclick="vBtn(2)">
      <span class="vbtn-icon">🤚</span>INTERACT
    </div>
    <div class="vbtn" id="vb3" onclick="vBtn(3)">
      <span class="vbtn-icon">⏱</span>TIMER
    </div>
  </div>
  <!-- Screen selector -->
  <div style="margin-top:12px">
    <div class="fl" style="margin-bottom:8px">ACTIVE SCREEN</div>
    <div class="screen-pills">
      <div class="screen-pill active" id="sp0" onclick="setScreen(0)">FACE</div>
      <div class="screen-pill" id="sp1" onclick="setScreen(1)">CLOCK</div>
      <div class="screen-pill" id="sp2" onclick="setScreen(2)">WEATHER</div>
      <div class="screen-pill" id="sp3" onclick="setScreen(3)">SENSORS</div>
    </div>
  </div>
</div>

<!-- Eye Preview Canvas -->
<div class="card c4">
  <div class="card-title">Eye Preview</div>
  <canvas id="eyeCanvas" width="192" height="96"></canvas>
  <div style="margin-top:12px">
    <div class="fl">EYE COLOUR</div>
    <div style="display:flex;align-items:center;gap:10px;margin-bottom:8px">
      <div class="c-preview" id="colorPrev" style="background:rgb(0,255,200);box-shadow:0 0 12px rgba(0,255,200,0.4)"></div>
      <span id="colorHex" style="font-size:.75rem;color:rgba(0,255,231,0.6)">#00FFC8</span>
    </div>
    <div style="display:flex;align-items:center;gap:8px;margin-bottom:4px">
      <span style="width:14px;font-size:.65rem;color:#ff2d78">R</span>
      <input type="range" class="sr" id="rangeR" min="0" max="255" value="0" oninput="updateColor()">
    </div>
    <div style="display:flex;align-items:center;gap:8px;margin-bottom:4px">
      <span style="width:14px;font-size:.65rem;color:#00ffe7">G</span>
      <input type="range" class="sg" id="rangeG" min="0" max="255" value="255" oninput="updateColor()">
    </div>
    <div style="display:flex;align-items:center;gap:8px">
      <span style="width:14px;font-size:.65rem;color:#7b2fff">B</span>
      <input type="range" class="sb" id="rangeB" min="0" max="255" value="200" oninput="updateColor()">
    </div>
  </div>
</div>

<!-- Sensors -->
<div class="card c4">
  <div class="card-title">Sensors</div>
  <div class="grid2" style="margin-bottom:12px">
    <div class="metric"><div class="mv" id="temp">--</div><div class="ml">TEMP °C</div></div>
    <div class="metric"><div class="mv" id="hum">--</div><div class="ml">HUMIDITY%</div></div>
  </div>
  <div class="fl">AMBIENT LIGHT (LDR)</div>
  <div class="ldr-track"><div class="ldr-fill" id="ldrBar" style="width:50%"></div></div>
  <div style="display:flex;justify-content:space-between;font-size:.65rem;color:rgba(0,255,231,0.4);margin-bottom:10px">
    <span>DARK</span><span id="ldrVal">--</span><span>BRIGHT</span>
  </div>
  <div style="display:flex;align-items:center;gap:10px;font-size:.78rem">
    <span style="color:rgba(0,255,231,0.5)">PIR:</span>
    <span id="pirDot" style="width:9px;height:9px;border-radius:50%;background:#333;display:inline-block;transition:all .3s"></span>
    <span id="pirTxt">No Motion</span>
  </div>
</div>

<!-- ── ROW 2 ─────────────────────────────────────────────── -->

<!-- Weather -->
<div class="card c4">
  <div class="card-title">External Weather</div>
  <div class="weather-banner">
    <div class="w-icon" id="wIcon">🌤️</div>
    <div>
      <div class="w-temp" id="extTemp">--°C</div>
      <div class="w-desc" id="wDesc">Fetching...</div>
    </div>
  </div>
  <div class="grid2">
    <div class="metric"><div class="mv" id="windSpd" style="font-size:1.1rem">--</div><div class="ml">WIND km/h</div></div>
    <div class="metric"><div class="mv" id="wCode" style="font-size:1.1rem">--</div><div class="ml">WMO CODE</div></div>
  </div>
</div>

<!-- Clock -->
<div class="card c4">
  <div class="card-title">Clock</div>
  <div class="clock-big" id="clockDisplay">--:--:--</div>
  <div class="clock-date" id="dateDisplay">-- / -- / ----</div>
  <div style="margin-top:14px;font-size:.7rem;color:rgba(0,255,231,0.4);text-align:center">
    NTP synced via WiFi
  </div>
</div>

<!-- Focus Timer -->
<div class="card c4">
  <div class="card-title">Focus Timer</div>
  <div class="timer-clock" id="timerDisplay">00:00</div>
  <div class="timer-prog"><div class="timer-bar" id="timerBar" style="width:0%"></div></div>
  <div class="timer-inputs">
    <input type="number" id="timerMin" value="25" min="0" max="180" placeholder="min">
    <span class="timer-sep">m</span>
    <input type="number" id="timerSec" value="0" min="0" max="59" placeholder="sec">
    <span class="timer-sep">s</span>
    <button class="btn btn-p" onclick="setTimer()">▶ GO</button>
    <button class="btn" onclick="cancelTimer()">■</button>
  </div>
</div>

<!-- ── ROW 3: Settings ────────────────────────────────────── -->

<!-- Eye Style + Speed + Config -->
<div class="card c4">
  <div class="card-title">Appearance</div>
  <div class="fl">BOT NAME</div>
  <input type="text" id="cfgName" placeholder="Chordy">
  <div class="fl">LOCATION</div>
  <input type="text" id="cfgLocation" placeholder="Dortmund">
  <div class="fl">EYE STYLE</div>
  <select id="cfgEyeStyle" style="margin-bottom:12px">
    <option value="0">Round (default)</option>
    <option value="1">Oval (tall)</option>
    <option value="2">Cute (anime)</option>
    <option value="3">Wide</option>
  </select>
  <div class="fl">ANIMATION SPEED &nbsp;<span id="speedVal" style="color:var(--c)">5</span>/10</div>
  <input type="range" class="speed-range" id="cfgSpeed" min="1" max="10" value="5" oninput="document.getElementById('speedVal').textContent=this.value">
  <button class="btn btn-p btn-full" onclick="saveSettings()" style="margin-top:14px">⬆ SAVE</button>
</div>

<!-- Audio -->
<div class="card c4">
  <div class="card-title">Audio</div>
  <div class="toggle-row">
    <span class="toggle-label">🔊 Buzzer Enabled</span>
    <label class="toggle"><input type="checkbox" id="buzzerOn" checked onchange="saveBuzzer()"><span class="toggle-slider"></span></label>
  </div>
  <div style="margin-top:14px">
    <div class="fl">VOLUME &nbsp;<span id="volVal" style="color:var(--c4)">7</span>/10</div>
    <input type="range" class="vol-range" id="buzVol" min="1" max="10" value="7"
      oninput="document.getElementById('volVal').textContent=this.value;saveBuzzer()">
  </div>
  <div style="margin-top:16px;font-size:.7rem;color:rgba(0,255,231,0.4)">
    Volume affects PWM duty of the buzzer.<br>
    Some passive buzzers may need higher values.
  </div>
</div>

<!-- ── Developer Panel ───────────────────────────────────── -->
<div class="card c4">
  <div class="card-title" style="color:rgba(255,45,120,0.6)">⚡ Developer Panel</div>
  <div class="section-label">BASIC EMOTIONS</div>
  <div class="trig-grid">
    <button class="btn" onclick="trig(2)">😊 Happy</button>
    <button class="btn" onclick="trig(3)">🥰 V.Happy</button>
    <button class="btn btn-red" onclick="trig(4)">😱 Scared</button>
    <button class="btn" onclick="trig(5)">😴 Sleepy</button>
    <button class="btn" onclick="trig(1)">👁 Blink</button>
    <button class="btn" onclick="trig(6)">😑 Squint</button>
    <button class="btn btn-pur" onclick="trig(7)">💕 Hearts</button>
    <button class="btn" onclick="trig(8)">👈 Left</button>
    <button class="btn" onclick="trig(9)">👉 Right</button>
    <button class="btn" onclick="trig(20)">⬆ Up</button>
    <button class="btn" onclick="trig(21)">⬇ Down</button>
    <button class="btn btn-pur" onclick="trig(22)">😉 Wink</button>
  </div>
  <div class="section-label">ACTIVITIES</div>
  <div class="trig-grid">
    <button class="btn btn-ora" onclick="trig(14)">🎸 Guitar</button>
    <button class="btn btn-ora" onclick="trig(15)">🎵 Whistle</button>
    <button class="btn btn-ora" onclick="trig(16)">🪰 Fly</button>
    <button class="btn btn-ora" onclick="trig(17)">⌨ Typing</button>
    <button class="btn btn-ora" onclick="trig(18)">☕ Coffee</button>
    <button class="btn btn-ora" onclick="trig(19)">🔍 Magnify</button>
    <button class="btn" onclick="trig(23)">🤔 Think</button>
    <button class="btn btn-red" onclick="trig(10)">🥶 Cold</button>
    <button class="btn btn-red" onclick="trig(11)">🥵 Hot</button>
  </div>
</div>

<!-- ── Danger Zone: Factory Reset ───────────────────────── -->
<div class="card c12 danger-zone">
  <div class="card-title">⚠ Danger Zone</div>
  <div style="display:flex;align-items:center;justify-content:space-between;flex-wrap:wrap;gap:12px">
    <div style="font-size:.78rem;color:rgba(255,45,120,0.7)">
      Factory Reset clears all WiFi credentials, name, and settings.<br>
      Chordy will restart in setup mode.
    </div>
    <button class="btn btn-red" onclick="factoryReset()" style="padding:12px 24px">
      🗑 FACTORY RESET
    </button>
  </div>
</div>

</div><!-- /grid -->
</div><!-- /wrap -->

<div id="toast">✓ Done</div>

<script>
// ── State ────────────────────────────────────────────────────
let eyeR=0, eyeG=255, eyeB=200, eyeStyle=0, eyeFrame=0;
let timerTotal=0, timerRemaining=0, timerInterval=null;

const STATE_NAMES=['BOOT','SETUP','CONNECTING','IDLE','HAPPY','V.HAPPY',
  'SCARED','SLEEPY','BRIGHT','COLD','HOT','RAINY','INTERACT','TIMER','SLEEP','CLOCK','WEATHER','SENSORS'];
const SCREEN_NAMES=['FACE','CLOCK','WEATHER','SENSORS'];

// ── Eye canvas ────────────────────────────────────────────────
const canvas=document.getElementById('eyeCanvas');
const ctx=canvas.getContext('2d');
const W=canvas.width, H=canvas.height;

// Eye style geometry
const EYE_STYLES=[
  {w:36,h:28,r:9},   // round
  {w:30,h:36,r:10},  // oval
  {w:28,h:28,r:14},  // cute
  {w:42,h:22,r:7},   // wide
];

function drawCanvasEye(cx,cy,ew,eh,er,pox,poy,bh,color){
  ctx.save();
  ctx.beginPath();
  ctx.roundRect(cx-ew/2, cy-bh/2, ew, bh, er);
  ctx.fillStyle=color; ctx.shadowColor=color; ctx.shadowBlur=10; ctx.fill(); ctx.shadowBlur=0;
  const pr=6, px=Math.max(cx-ew/2+pr+2,Math.min(cx+ew/2-pr-2,cx+pox));
  const py=Math.max(cy-bh/2+pr+2,Math.min(cy+bh/2-pr-2,cy+poy));
  ctx.beginPath(); ctx.arc(px,py,pr,0,Math.PI*2); ctx.fillStyle='#000'; ctx.fill();
  ctx.beginPath(); ctx.arc(px+2,py-2,1.5,0,Math.PI*2); ctx.fillStyle='rgba(255,255,255,0.9)'; ctx.fill();
  ctx.restore();
}

function animateEyes(){
  const style=EYE_STYLES[Math.min(eyeStyle,3)];
  const ew=style.w*(W/192), eh=style.h*(H/96), er=style.r*(W/192);
  const lCx=W*0.33, rCx=W*0.67, cy=H*0.46;
  const color=`rgb(${eyeR},${eyeG},${eyeB})`;

  // Blink
  const blink=eyeFrame%90<8 ? Math.max(0.12,1-(eyeFrame%8)*0.12) : 1;
  const bh=eh*blink;

  // Idle drift
  const t=eyeFrame*0.025;
  const pox=(Math.sin(t)*0.12)*ew;
  const poy=(Math.cos(t*0.7)*0.1)*eh;

  ctx.clearRect(0,0,W,H);
  ctx.fillStyle='#000'; ctx.fillRect(0,0,W,H);

  // Cute style: extra rounded
  if(eyeStyle===2){
    // Extra inner glow ring
    ctx.save(); ctx.beginPath(); ctx.roundRect(lCx-ew/2-2,cy-eh/2-2,ew+4,bh+4,er+2);
    ctx.strokeStyle=`rgba(${eyeR},${eyeG},${eyeB},0.2)`; ctx.lineWidth=2; ctx.stroke(); ctx.restore();
    ctx.save(); ctx.beginPath(); ctx.roundRect(rCx-ew/2-2,cy-eh/2-2,ew+4,bh+4,er+2);
    ctx.strokeStyle=`rgba(${eyeR},${eyeG},${eyeB},0.2)`; ctx.lineWidth=2; ctx.stroke(); ctx.restore();
  }

  drawCanvasEye(lCx,cy,ew,eh,er,pox,poy,bh,color);
  drawCanvasEye(rCx,cy,ew,eh,er,pox,poy,bh,color);

  eyeFrame++;
  requestAnimationFrame(animateEyes);
}
animateEyes();

// ── Color ────────────────────────────────────────────────────
function updateColor(){
  eyeR=parseInt(document.getElementById('rangeR').value);
  eyeG=parseInt(document.getElementById('rangeG').value);
  eyeB=parseInt(document.getElementById('rangeB').value);
  const hex='#'+[eyeR,eyeG,eyeB].map(v=>v.toString(16).padStart(2,'0')).join('').toUpperCase();
  document.getElementById('colorHex').textContent=hex;
  const s=`rgb(${eyeR},${eyeG},${eyeB})`;
  document.getElementById('colorPrev').style.background=s;
  document.getElementById('colorPrev').style.boxShadow=`0 0 14px ${s}`;
}

// ── Telemetry ─────────────────────────────────────────────────
const WICONS = {
  0:'☀️',
  1:'🌤️',
  2:'⛅',
  3:'☁️',
  45:'🌫️',
  48:'🌫️',
  51:'🌦️',
  53:'🌦️',
  55:'🌧️',
  61:'🌧️',
  63:'🌧️',
  65:'🌧️',
  71:'❄️',
  73:'❄️',
  75:'❄️',
  80:'🌧️',
  81:'🌧️',
  82:'🌧️',
  95:'⛈️'
};

function wIcon(code){
  return WICONS[code] || '🌤️';
}
function wIcon(code){const p=String(Math.floor(code/100));return WICONS[String(code)]||WICONS[p]||'🌤️'}

async function poll(){
  try{
    const d=await fetch('/api/telemetry').then(r=>r.json());
    document.getElementById('hdrName').textContent=d.bot_name||'CHORDY';
    document.getElementById('locationName').textContent=d.location||'--';
    document.getElementById('stateName').textContent=STATE_NAMES[d.state]||'--';
    document.getElementById('temp').textContent=d.temp_c.toFixed(1);
    document.getElementById('hum').textContent=d.humidity.toFixed(0);
    document.getElementById('ldrVal').textContent=d.ldr;
    const lpct=Math.round((4095-d.ldr)/4095*100);
    document.getElementById('ldrBar').style.width=lpct+'%';
    const p=document.getElementById('pirDot');
    p.style.background=d.pir?'#00ff88':'#333';
    p.style.boxShadow=d.pir?'0 0 7px #00ff88':'none';
    document.getElementById('pirTxt').textContent=d.pir?'Motion!':'No Motion';
    if(d.weather_valid){
      document.getElementById('extTemp').textContent=d.weather_temp.toFixed(1)+'°C';
      document.getElementById('wDesc').textContent=d.weather_desc;
      document.getElementById('windSpd').textContent=d.weather_wind.toFixed(1);
      document.getElementById('wCode').textContent=d.weather_code;
      document.getElementById('wIcon').textContent=wIcon(d.weather_code);
    }
    // Clock
    document.getElementById('clockDisplay').textContent=d.time;
    document.getElementById('timeDisplay').textContent=d.time;
    document.getElementById('dateDisplay').textContent=d.date;
    // Sliders sync on first load
    if(!_settingsLoaded){
      _settingsLoaded=true;
      document.getElementById('cfgName').value=d.bot_name||'Chordy';
      document.getElementById('cfgLocation').value=d.location||'Dortmund';
      document.getElementById('cfgEyeStyle').value=d.eye_style||0;
      document.getElementById('cfgSpeed').value=d.anim_speed||5;
      document.getElementById('speedVal').textContent=d.anim_speed||5;
      document.getElementById('buzzerOn').checked=d.buzzer;
      document.getElementById('buzVol').value=d.buz_vol||7;
      document.getElementById('volVal').textContent=d.buz_vol||7;
      eyeR=d.eye_r||0; eyeG=d.eye_g||255; eyeB=d.eye_b||200;
      document.getElementById('rangeR').value=eyeR;
      document.getElementById('rangeG').value=eyeG;
      document.getElementById('rangeB').value=eyeB;
      eyeStyle=d.eye_style||0;
      updateColor();
    }
    // Timer
    if(d.timer_active){
      timerRemaining=d.timer_remaining;
      timerTotal=d.timer_total;
      updateTimerDisplay();
    }
    // Screen pills
    for(let i=0;i<4;i++){
      const sp=document.getElementById('sp'+i);
      sp&&sp.classList.toggle('active',d.screen===i);
    }
    // Power button state
    document.getElementById('vb0').classList.toggle('active', d.state===14); // SLEEPING
  }catch(e){}
}
let _settingsLoaded=false;
setInterval(poll,2500); poll();

// ── Timer display ─────────────────────────────────────────────
function updateTimerDisplay(){
  const m=Math.floor(timerRemaining/60).toString().padStart(2,'0');
  const s=(timerRemaining%60).toString().padStart(2,'0');
  document.getElementById('timerDisplay').textContent=m+':'+s;
  const pct=timerTotal>0?(1-timerRemaining/timerTotal)*100:0;
  document.getElementById('timerBar').style.width=pct+'%';
}
async function setTimer(){
  const mins=parseInt(document.getElementById('timerMin').value)||0;
  const secs=parseInt(document.getElementById('timerSec').value)||0;
  if(mins===0&&secs===0){toast('Enter a time first');return}
  timerTotal=(mins*60+secs);
  timerRemaining=timerTotal;
  await fetch('/api/timer',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({minutes:mins,seconds:secs})});
  if(timerInterval)clearInterval(timerInterval);
  timerInterval=setInterval(()=>{if(timerRemaining>0){timerRemaining--;updateTimerDisplay();}else clearInterval(timerInterval);},1000);
  toast(`Timer: ${mins}m ${secs}s`);
}
async function cancelTimer(){
  await fetch('/api/timer',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({minutes:0,seconds:0})});
  if(timerInterval)clearInterval(timerInterval);
  timerRemaining=0;timerTotal=0;updateTimerDisplay();
  toast('Timer cancelled');
}

// ── Settings ─────────────────────────────────────────────────
async function saveSettings(){
  eyeStyle=parseInt(document.getElementById('cfgEyeStyle').value);
  const p={
    botName:document.getElementById('cfgName').value||'Chordy',
    location:document.getElementById('cfgLocation').value||'Dortmund',
    eyeR,eyeG,eyeB,eyeStyle,
    animSpeed:parseInt(document.getElementById('cfgSpeed').value)||5
  };
  await fetch('/api/settings',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(p)});
  toast('Settings saved!');
}
async function saveBuzzer(){
  const p={buzzer:document.getElementById('buzzerOn').checked,buzVol:parseInt(document.getElementById('buzVol').value)||7};
  await fetch('/api/settings',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(p)});
}

// ── Triggers ─────────────────────────────────────────────────
async function trig(id){
  await fetch('/api/trigger',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({anim:id})});
  const names=['IDLE','BLINK','HAPPY','V.HAPPY','SCARED','SLEEPY','SQUINT','HEARTS','LEFT','RIGHT',
    'COLD','HOT','RAIN','STARTUP','GUITAR','WHISTLE','FLY','TYPING','COFFEE','MAGNIFY','THINKING','LOOKUP','LOOKDOWN','WINK'];
  toast('▶ '+( names[id]||'Anim '+id));
}

// ── Virtual buttons ───────────────────────────────────────────
async function vBtn(n){
  const vb=document.getElementById('vb'+n);
  vb&&vb.classList.add('active');
  setTimeout(()=>vb&&vb.classList.remove('active'),300);
  await fetch('/api/button',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({btn:n})});
  const names=['POWER','SCREEN','INTERACT','TIMER'];
  toast('Button: '+names[n]);
}

// ── Screen ────────────────────────────────────────────────────
async function setScreen(n){
  await fetch('/api/screen',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({screen:n})});
  for(let i=0;i<4;i++) document.getElementById('sp'+i)?.classList.toggle('active',i===n);
  const sn=['FACE','CLOCK','WEATHER','SENSORS'];
  toast('Screen: '+sn[n]);
}

// ── Factory reset ─────────────────────────────────────────────
async function factoryReset(){
  if(!confirm('⚠ Reset Chordy? All settings and WiFi credentials will be erased.')) return;
  await fetch('/api/factoryreset',{method:'POST'});
  toast('Resetting Chordy...');
}

// ── Toast ─────────────────────────────────────────────────────
let _tt=null;
function toast(msg){
  const el=document.getElementById('toast');
  el.textContent='✓ '+msg; el.classList.add('show');
  if(_tt)clearTimeout(_tt);
  _tt=setTimeout(()=>el.classList.remove('show'),2400);
}
</script>
</body>
</html>
)HTMLEOF";
