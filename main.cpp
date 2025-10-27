/*
  SmartLights - Dual-mode (SoftAP + STA) web UI with two pages (Dashboard + Settings)
  - Dashboard: animated toggle switches, dynamic background
  - Settings: scan WiFi networks, enter SSID + password + device name
  - Persists credentials using Preferences
  - Starts mDNS using device name after successful connection
  - No Serial output (silent)
*/

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>

const char* AP_SSID = "ESP32-AP-Lights";
const char* AP_PASS = "esp32lights";

WebServer server(80);
Preferences prefs;

bool connectedToRouter = false;
String deviceName = "esp32lights"; // default, overriden from prefs

// ====== Light Pins & data ======
const int PIN_KITCHEN  = 2;
const int PIN_BEDROOM  = 4;
const int PIN_SITTING  = 16;
const int PIN_SECURITY = 17;
const int PIN_BALCONY  = 18;

struct Light {
  const char* name;
  int pin;
  bool state;
};

Light lights[] = {
  {"kitchen",  PIN_KITCHEN,  false},
  {"bedroom",  PIN_BEDROOM,  false},
  {"sitting",  PIN_SITTING,  false},
  {"security", PIN_SECURITY, false},
  {"balcony",  PIN_BALCONY,  false}
};

const int LIGHT_COUNT = sizeof(lights) / sizeof(lights[0]);

// ---------- Frontend Pages (PROGMEM) ----------
// Control / Dashboard page (modern, animated toggles, dynamic background)
const char control_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html lang="en"><head>
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Smart Lights</title>
<style>
:root{
  --bg1: linear-gradient(135deg,#081224,#082a3a);
  --bg2: linear-gradient(135deg,#042f2e,#07313a);
  --accent: #20c997;
  --muted: #94a3b8;
}
*{box-sizing:border-box}
body{
  margin:0;
  min-height:100vh;
  font-family:Inter,system-ui,-apple-system,"Segoe UI",Roboto,Arial;
  color:#f8fafc;
  display:flex;
  flex-direction:column;
  align-items:center;
  background:var(--bg1);
  transition:background 450ms ease;
}
.header{
  width:100%;
  padding:14px 18px;
  display:flex;
  justify-content:space-between;
  align-items:center;
  background:rgba(0,0,0,0.08);
  backdrop-filter: blur(6px);
}
.title{font-weight:600; font-size:18px;}
.header button{
  background:transparent;border:1px solid rgba(255,255,255,0.08);color:inherit;padding:8px 10px;border-radius:8px;cursor:pointer;
}
.container{
  width:100%;
  max-width:980px;
  padding:22px;
}
.grid{
  display:grid;
  grid-template-columns: repeat(auto-fill,minmax(220px,1fr));
  gap:18px;
  margin-top:18px;
}
.card{
  background: linear-gradient(180deg, rgba(255,255,255,0.03), rgba(255,255,255,0.01));
  border-radius:14px;
  padding:18px;
  box-shadow: 0 8px 24px rgba(2,6,23,0.45);
  display:flex;
  flex-direction:column;
  align-items:center;
  gap:12px;
}
.lname{font-size:16px;color:var(--muted);text-transform:capitalize;}
.switch{
  position:relative;
  width:68px;height:40px;border-radius:999px;background:#1f2937;cursor:pointer;transition:all 280ms ease;box-shadow: inset 0 2px 6px rgba(0,0,0,0.6);
}
.knob{
  position:absolute;left:6px;top:6px;width:28px;height:28px;border-radius:50%;background:white;transition:transform 280ms ease, box-shadow 200ms;box-shadow:0 4px 10px rgba(2,6,23,0.35);
}
.switch.on{background:linear-gradient(90deg,#16a34a,#22c55e);}
.switch.on .knob{transform: translateX(28px);box-shadow: 0 8px 18px rgba(16,185,129,0.18);}
.status{font-size:13px;color:var(--muted);}
.footer{margin-top:26px;color:var(--muted);font-size:13px;}
@media (max-width:420px){ .grid{grid-template-columns:1fr} .card{padding:14px} }
</style>
</head><body>
<div class="header">
  <div class="title">Smart Lights</div>
  <div>
    <button onclick="gotoSettings()">⚙ Settings</button>
  </div>
</div>

<div class="container">
  <div id="grid" class="grid"></div>
  <div class="footer">Access: <span id="accessHint">local</span></div>
</div>

<script>
const LIGHT_NAMES = ['kitchen','bedroom','sitting','security','balcony']; // must match backend
function makeCard(name, idx, state){
  const card = document.createElement('div'); card.className='card';
  const nm = document.createElement('div'); nm.className='lname'; nm.textContent=name;
  const sw = document.createElement('div'); sw.className='switch'; sw.id='sw'+idx;
  const knob = document.createElement('div'); knob.className='knob';
  const st = document.createElement('div'); st.className='status'; st.id='st'+idx;
  sw.appendChild(knob);
  if(state) sw.classList.add('on');
  st.textContent = state ? 'ON' : 'OFF';
  sw.onclick = ()=>toggle(idx, name);
  card.appendChild(nm); card.appendChild(sw); card.appendChild(st);
  return card;
}

async function refresh(){
  try{
    const res = await fetch('/status');
    const data = await res.json();
    const grid = document.getElementById('grid'); grid.innerHTML='';
    let onCount = 0;
    LIGHT_NAMES.forEach((n,i)=>{
      const state = !!data[n];
      if(state) onCount++;
      grid.appendChild(makeCard(n,i,state));
    });
    // dynamic background: change based on how many lights are on
    if(onCount === 0) document.body.style.background='var(--bg1)';
    else if(onCount < LIGHT_NAMES.length/2) document.body.style.background='var(--bg2)';
    else document.body.style.background='linear-gradient(135deg,#2b2b2b,#053a2b)';
    // show access hint depending on whether router reachable
    const ah = document.getElementById('accessHint');
    // try to fetch /whoami (optionally) but backend will set an indicator via /status? (not implemented). We'll show 'local' here.
    ah.textContent = 'local / AP';
  }catch(e){
    // silent fail
  }
}

async function toggle(idx, name){
  // optimistic UI toggle
  const sw = document.getElementById('sw'+idx);
  const st = document.getElementById('st'+idx);
  const willBe = !sw.classList.contains('on');
  if(willBe) sw.classList.add('on'); else sw.classList.remove('on');
  st.textContent = willBe ? 'ON' : 'OFF';
  try{
    await fetch('/toggle?id='+encodeURIComponent(name));
    await refresh();
  }catch(e){
    // ignore
  }
}

function gotoSettings(){ location.href='/settings'; }

document.addEventListener('DOMContentLoaded',()=>{
  refresh();
  setInterval(refresh,3000);
});
</script>
</body></html>
)rawliteral";

// Settings page (scan networks, enter password and device name)
const char settings_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html lang="en"><head>
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Settings - Smart Lights</title>
<style>
body{margin:0;font-family:Inter,system-ui,-apple-system,Segoe UI,Roboto,Arial;background:#071123;color:#e6eef8;display:flex;flex-direction:column;align-items:center;}
.header{width:100%;padding:14px 18px;background:linear-gradient(90deg,#0ea5e9,#6366f1);display:flex;justify-content:space-between;align-items:center;color:white}
.header .title{font-weight:600}
.container{width:100%;max-width:640px;padding:20px}
.card{background:#041426;padding:18px;border-radius:12px;box-shadow:0 8px 24px rgba(2,6,23,0.6)}
.row{display:flex;gap:12px;flex-wrap:wrap;margin-bottom:12px}
select,input{padding:10px;border-radius:8px;border:none;background:#082033;color:#e6eef8;flex:1}
button{padding:10px 14px;border-radius:8px;border:none;background:#0ea5e9;color:#04233a;font-weight:600;cursor:pointer}
.note{color:#94a3b8;font-size:13px;margin-top:8px}
.footer{margin-top:12px;color:#7f8da0;font-size:13px;text-align:center}
@media(max-width:480px){.row{flex-direction:column}}
</style>
</head><body>
<div class="header"><div class="title">Device Settings</div><div><button onclick="back()">← Dashboard</button></div></div>
<div class="container">
  <div class="card">
    <div class="row">
      <select id="ssid"><option>Scanning...</option></select>
      <button onclick="scan()">Scan</button>
    </div>
    <div class="row"><input id="pass" type="password" placeholder="Wi-Fi Password"></div>
    <div class="row"><input id="dname" type="text" placeholder="Device Name (mDNS)"></div>
    <div class="row"><button onclick="save()">Save & Connect</button></div>
    <div class="note" id="msg">Connect to your Wi-Fi to enable network access (mDNS)</div>
  </div>
  <div class="footer">If connection succeeds you'll be able to visit <b id="hint">esp32lights.local</b></div>
</div>

<script>
async function scan(){
  const sel=document.getElementById('ssid'); sel.innerHTML='<option>Scanning...</option>';
  try{
    const res=await fetch('/scan');
    const list=await res.json();
    if(list.length===0){ sel.innerHTML='<option>No networks</option>'; return;}
    sel.innerHTML=list.map(s=>`<option>${s}</option>`).join('');
  }catch(e){ sel.innerHTML='<option>Error</option>'; }
}
async function save(){
  const ssid=document.getElementById('ssid').value;
  const pass=document.getElementById('pass').value;
  const name=document.getElementById('dname').value || '';
  const body = `ssid=${encodeURIComponent(ssid)}&pass=${encodeURIComponent(pass)}&name=${encodeURIComponent(name)}`;
  const msg=document.getElementById('msg'); msg.textContent='Connecting...';
  try{
    const res = await fetch('/savewifi',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body});
    const txt = await res.text();
    msg.innerHTML = txt;
    // if success likely contains 'Connected', show hint
    if(txt.toLowerCase().includes('connected')) {
      document.getElementById('hint').textContent = name? name + '.local' : 'esp32lights.local';
    }
  }catch(e){ msg.textContent='Network error'; }
}
function back(){ location.href='/'; }
document.addEventListener('DOMContentLoaded',()=>{ scan(); });
</script>
</body></html>
)rawliteral";

// ---------- Helper functions (backend) ----------
int findLightIndex(const String &name){
  for(int i=0;i<LIGHT_COUNT;i++){
    if(name.equalsIgnoreCase(lights[i].name)) return i;
  }
  return -1;
}

String getStatusJSON(){
  String s = "{";
  for(int i=0;i<LIGHT_COUNT;i++){
    s += "\"";
    s += lights[i].name;
    s += "\":";
    s += (lights[i].state ? "1" : "0");
    if(i < LIGHT_COUNT - 1) s += ",";
  }
  s += "}";
  return s;
}

// ---------- HTTP Handlers ----------
void handleRoot(){
  server.send_P(200, "text/html", control_html);
}

void handleSettings(){
  server.send_P(200, "text/html", settings_html);
}

void handleStatus(){
  server.send(200, "application/json", getStatusJSON());
}

void handleToggle(){
  if(!server.hasArg("id")){
    server.send(400, "text/plain", "missing id");
    return;
  }
  String id = server.arg("id");
  int idx = findLightIndex(id);
  if(idx < 0){
    server.send(404, "text/plain", "unknown id");
    return;
  }
  lights[idx].state = !lights[idx].state;
  digitalWrite(lights[idx].pin, lights[idx].state ? HIGH : LOW);
  server.send(200, "application/json", getStatusJSON());
}

void handleScan(){
  int n = WiFi.scanNetworks();
  String json = "[";
  for(int i=0;i<n;i++){
    json += "\"" + WiFi.SSID(i) + "\"";
    if(i < n-1) json += ",";
  }
  json += "]";
  server.send(200, "application/json", json);
}

void handleSaveWifi(){
  // expects application/x-www-form-urlencoded body with ssid, pass, name
  if(!server.hasArg("ssid") || !server.hasArg("pass")){
    server.send(400, "text/plain", "Missing parameters");
    return;
  }
  String ssid = server.arg("ssid");
  String pass = server.arg("pass");
  String name = server.hasArg("name") ? server.arg("name") : "";

  // store to preferences
  prefs.begin("wifi", false);
  prefs.putString("ssid", ssid);
  prefs.putString("pass", pass);
  if(name.length()) {
    prefs.putString("name", name);
    deviceName = name;
  } else {
    // leave existing name or default
    if(prefs.isKey("name")) deviceName = prefs.getString("name", deviceName);
  }
  prefs.end();

  // Try to connect
  WiFi.mode(WIFI_AP_STA); // keep AP + STA
  WiFi.begin(ssid.c_str(), pass.c_str());

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 12000) {
    delay(200);
  }

  if (WiFi.status() == WL_CONNECTED) {
    connectedToRouter = true;
    // start mdns with chosen deviceName
    if(deviceName.length()) MDNS.begin(deviceName.c_str());
    else MDNS.begin("esp32lights");
    String reply = "✅ Connected! Access via http://" + (deviceName.length() ? deviceName : String("esp32lights")) + ".local<br><br>";
    reply += "<button onclick=\"location.href='/'\">Go to Dashboard</button>";
    server.send(200, "text/html", reply);
  } else {
    connectedToRouter = false;
    server.send(200, "text/plain", "❌ Failed to connect. Please check password and try again.");
  }
}

// ---------- Setup ----------
void setup(){
  // initialize pins
  for(int i=0;i<LIGHT_COUNT;i++){
    pinMode(lights[i].pin, OUTPUT);
    digitalWrite(lights[i].pin, LOW);
    lights[i].state = false;
  }

  // read stored credentials
  prefs.begin("wifi", true); // read-only
  String stored_ssid = prefs.getString("ssid", "");
  String stored_pass = prefs.getString("pass", "");
  String stored_name = prefs.getString("name", "");
  prefs.end();

  if(stored_name.length()) deviceName = stored_name;

  // start AP mode always so user can connect to configure
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);

  // if we have stored creds, attempt to connect (STA) but keep AP
  if(stored_ssid.length()){
    WiFi.mode(WIFI_AP_STA); // enable both
    WiFi.begin(stored_ssid.c_str(), stored_pass.c_str());
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 12000) {
      delay(200);
    }
    if(WiFi.status() == WL_CONNECTED){
      connectedToRouter = true;
      if(deviceName.length()) MDNS.begin(deviceName.c_str());
      else MDNS.begin("esp32lights");
    } else {
      connectedToRouter = false;
    }
  } else {
    connectedToRouter = false;
  }

  // register HTTP routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/settings", HTTP_GET, handleSettings);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/toggle", HTTP_GET, handleToggle);
  server.on("/scan", HTTP_GET, handleScan);
  server.on("/savewifi", HTTP_POST, handleSaveWifi);

  server.begin();
}

// ---------- Loop ----------
void loop(){
  server.handleClient();
  // Note: MDNS on ESP32 runs in background — no MDNS.update() needed
}
