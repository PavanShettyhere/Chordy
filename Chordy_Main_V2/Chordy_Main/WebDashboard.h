#pragma once
const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html lang="en"><head>
<meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Chordy Dashboard</title>
<style>
:root{--bg:#06111a;--panel:#0d1c28;--line:#1c4557;--text:#d8faff;--accent:#52f0ff}
*{box-sizing:border-box}body{margin:0;font-family:Arial,sans-serif;background:linear-gradient(180deg,#031018,#0b1824);color:var(--text)}
.wrap{max-width:1100px;margin:0 auto;padding:18px}
h1{margin:0 0 14px;color:var(--accent)}
.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(260px,1fr));gap:16px}
.card{background:var(--panel);border:1px solid var(--line);border-radius:14px;padding:16px}
label{display:block;font-size:12px;opacity:.85;margin:8px 0 4px}
input,select{width:100%;padding:10px;border-radius:8px;border:1px solid #2b617a;background:#08141d;color:var(--text)}
button{padding:10px 12px;border-radius:8px;border:1px solid #2ea0bb;background:#0f3140;color:var(--text);cursor:pointer}
button:hover{background:#154458}.row{display:flex;gap:10px;flex-wrap:wrap}.metric{font-size:28px;font-weight:bold;color:var(--accent)}
.small{font-size:12px;opacity:.8}.tele{display:grid;grid-template-columns:repeat(2,1fr);gap:10px}
.eye-preview{height:90px;background:#000;border-radius:10px;position:relative;overflow:hidden;border:1px solid #284353}
.eye{position:absolute;top:26px;width:48px;height:34px;background:#fff}
.eye.left{left:22px}.eye.right{right:22px}
.rounded{border-radius:16px}.sharp{clip-path:polygon(0 50%,50% 0,100% 50%,50% 100%)}.sleepy{border-radius:16px 16px 10px 10px}
.pupil{width:12px;height:12px;border-radius:50%;background:#000;position:absolute;top:11px;left:18px}
.status{padding:6px 10px;border-radius:999px;border:1px solid #2b617a;display:inline-block}
hr{border:none;border-top:1px solid #20475b;margin:14px 0}
</style></head><body>
<div class="wrap">
  <h1>Chordy Control Panel</h1>
  <div class="grid">
    <div class="card">
      <h3>Live telemetry</h3>
      <div class="tele">
        <div><div class="metric" id="tempC">--</div><div class="small">Temp C</div></div>
        <div><div class="metric" id="hum">--</div><div class="small">Humidity</div></div>
        <div><div class="metric" id="ldr">--</div><div class="small">LDR</div></div>
        <div><div class="metric" id="weather">--</div><div class="small">Weather</div></div>
      </div>
      <hr>
      <div>State: <span class="status" id="stateText">--</span></div>
      <div class="small" id="timerInfo">Timer inactive</div>
    </div>

    <div class="card">
      <h3>Web buttons / screen control</h3>
      <div class="row">
        <button onclick="control('power')">Power</button>
        <button onclick="control('select')">Select</button>
        <button onclick="control('interact')">Interact</button>
        <button onclick="control('extra')">Extra</button>
      </div>
      <hr>
      <div class="row">
        <button onclick="control('face')">Face Screen</button>
        <button onclick="control('menu')">Menu Screen</button>
        <button onclick="control('settings')">Settings Screen</button>
      </div>
      <hr>
      <div class="row">
        <button onclick="triggerAnim(2)">Happy</button>
        <button onclick="triggerAnim(3)">Very Happy</button>
        <button onclick="triggerAnim(4)">Scared</button>
        <button onclick="triggerAnim(5)">Sleepy</button>
        <button onclick="triggerAnim(10)">Cold</button>
        <button onclick="triggerAnim(11)">Hot</button>
        <button onclick="triggerAnim(12)">Rain</button>
      </div>
    </div>

    <div class="card">
      <h3>Timer</h3>
      <label>Minutes</label>
      <input type="number" id="timerMin" min="0" max="180" value="25">
      <label>Seconds</label>
      <input type="number" id="timerSec" min="0" max="59" value="0">
      <div class="row">
        <button onclick="setTimer()">Start timer</button>
        <button onclick="stopTimer()">Stop timer</button>
      </div>
    </div>

    <div class="card">
      <h3>Display / buzzer / animation</h3>
      <label>Bot name</label><input id="botName">
      <label>Location</label><input id="location">
      <label>Eye style</label>
      <select id="eyeStyle">
        <option value="0">Rounded</option>
        <option value="1">Sharp</option>
        <option value="2">Sleepy</option>
      </select>
      <label>Animation speed (%)</label><input type="range" id="animSpeed" min="40" max="180" value="100">
      <label>Buzzer enabled</label>
      <select id="buzzerEnabled"><option value="true">Enabled</option><option value="false">Disabled</option></select>
      <label>Buzzer volume (%)</label><input type="range" id="buzzerVolume" min="0" max="100" value="70">
      <label>Eye color R</label><input type="range" id="eyeR" min="0" max="255" value="0">
      <label>Eye color G</label><input type="range" id="eyeG" min="0" max="255" value="255">
      <label>Eye color B</label><input type="range" id="eyeB" min="0" max="255" value="200">
      <div class="row"><button onclick="saveSettings()">Save settings</button></div>
    </div>

    <div class="card">
      <h3>Eye preview</h3>
      <div class="eye-preview" id="preview">
        <div class="eye left rounded" id="eyeLeft"><div class="pupil"></div></div>
        <div class="eye right rounded" id="eyeRight"><div class="pupil"></div></div>
      </div>
      <div class="small">Preview shows the selected eye style.</div>
    </div>
  </div>
</div>
<script>
const states=["BOOT","WIFI_SETUP","CONNECTING","IDLE","HAPPY","VERY_HAPPY","SCARED","SLEEPY","BRIGHT","COLD","HOT","RAINY","INTERACT","TIMER","SLEEPING"];
function byId(id){return document.getElementById(id);}
function updatePreview(){
  const r=+byId('eyeR').value,g=+byId('eyeG').value,b=+byId('eyeB').value;
  const style=byId('eyeStyle').value;
  ['eyeLeft','eyeRight'].forEach(id=>{
    const el=byId(id);
    el.className='eye '+(id==='eyeLeft'?'left ':'right ')+(style==='1'?'sharp':style==='2'?'sleepy':'rounded');
    el.style.boxShadow='0 0 18px rgb('+r+','+g+','+b+')';
  });
}
async function telemetry(){
  const r=await fetch('/api/telemetry');
  const d=await r.json();
  byId('tempC').textContent=d.temp_c;
  byId('hum').textContent=d.humidity;
  byId('ldr').textContent=d.ldr;
  byId('weather').textContent=d.weather_valid?d.weather_temp:'--';
  byId('stateText').textContent=states[d.state]||d.state;
  byId('timerInfo').textContent=d.timer_active?('Remaining: '+d.timer_remaining+' sec'):'Timer inactive';
  byId('botName').value=d.botName||byId('botName').value;
  byId('location').value=d.location||byId('location').value;
  byId('eyeR').value=d.eyeR;
  byId('eyeG').value=d.eyeG;
  byId('eyeB').value=d.eyeB;
  byId('eyeStyle').value=d.eyeStyle;
  byId('animSpeed').value=d.animSpeedPct;
  byId('buzzerEnabled').value=String(d.buzzerEnabled);
  byId('buzzerVolume').value=d.buzzerVolume;
  updatePreview();
}
async function saveSettings(){
  await fetch('/api/settings',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({
    botName:byId('botName').value,location:byId('location').value,eyeR:+byId('eyeR').value,eyeG:+byId('eyeG').value,eyeB:+byId('eyeB').value,
    eyeStyle:+byId('eyeStyle').value,animSpeedPct:+byId('animSpeed').value,buzzerEnabled:byId('buzzerEnabled').value==='true',buzzerVolume:+byId('buzzerVolume').value
  })});
  telemetry();
}
async function control(action){
  await fetch('/api/control',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({action})});
  setTimeout(telemetry,150);
}
async function triggerAnim(anim){
  await fetch('/api/trigger',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({anim})});
  setTimeout(telemetry,150);
}
async function setTimer(){
  const minutes=+byId('timerMin').value||0;
  const seconds=+byId('timerSec').value||0;
  await fetch('/api/timer',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({minutes,seconds})});
  setTimeout(telemetry,150);
}
async function stopTimer(){
  await fetch('/api/timer',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({minutes:0,seconds:0})});
  setTimeout(telemetry,150);
}
['eyeR','eyeG','eyeB','eyeStyle'].forEach(id=>byId(id).addEventListener('input',updatePreview));
telemetry(); setInterval(telemetry,3000);
</script></body></html>
)rawliteral";
