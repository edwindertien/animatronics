#include "WIFIremote.h"

// ============================
// Embedded HTML UI
// ============================

static const uint8_t WAIT_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8"/>
  <meta name="viewport" content="width=device-width, initial-scale=1, viewport-fit=cover"/>
  <title>Remote Busy</title>
  <style>
    :root{ --bg:#0b0f14; --text:#e8f0fb; --muted:#7f93a8; --border:rgba(255,255,255,.10); }
    body{
      margin:0; min-height:100vh; display:flex; justify-content:center; align-items:center;
      font-family:system-ui,-apple-system,Segoe UI,Roboto,Arial;
      background: radial-gradient(1200px 800px at 20% 0%, rgba(74,163,255,.16), transparent 60%),
                  radial-gradient(900px 700px at 90% 20%, rgba(53,208,127,.12), transparent 55%),
                  var(--bg);
      color:var(--text);
      padding:20px;
    }
    .card{
      width:min(520px, 100%);
      border:1px solid var(--border);
      border-radius:18px;
      background: rgba(255,255,255,.04);
      padding:18px;
    }
    h1{ margin:0 0 8px 0; font-size:18px; }
    p{ margin:0; color:var(--muted); line-height:1.4; }
    .small{ margin-top:10px; font-size:12px; color:var(--muted); }
    button{
      margin-top:14px;
      width:100%;
      border-radius:14px;
      border:1px solid var(--border);
      background: rgba(255,255,255,.06);
      color:var(--text);
      padding:12px 14px;
      font-size:14px;
    }
  </style>
</head>
<body>
  <div class="card">
    <h1>Remote is in use</h1>
    <p>Please await your turn. This page will refresh automatically.</p>
    <div class="small">If the current user stops sending commands, control will become available.</div>
    <button onclick="location.reload()">Refresh</button>
  </div>
  <script>
    setTimeout(() => location.reload(), 2000);
  </script>
</body>
</html>
)HTML";

static const uint8_t INDEX_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8"/>
  <meta name="viewport" content="width=device-width, initial-scale=1, viewport-fit=cover"/>
  <title>Robot Remote</title>
  <style>
    :root{
      --bg:#0b0f14;
      --muted:#7f93a8;
      --text:#e8f0fb;
      --accent:#4aa3ff;
      --accent2:#35d07f;
      --border:rgba(255,255,255,.08);
      --shadow:0 12px 30px rgba(0,0,0,.45);
    }
    *{ box-sizing:border-box; -webkit-tap-highlight-color: transparent; }
    body{
      margin:0; font-family:system-ui,-apple-system,Segoe UI,Roboto,Arial;
      background: radial-gradient(1200px 800px at 20% 0%, rgba(74,163,255,.18), transparent 60%),
                  radial-gradient(900px 700px at 90% 20%, rgba(53,208,127,.12), transparent 55%),
                  var(--bg);
      color:var(--text);
      min-height:100vh;
      display:flex;
      justify-content:center;
      padding:16px;
    }
    .wrap{ width:min(980px, 100%); display:grid; gap:14px; }
    header{ display:flex; align-items:flex-end; justify-content:space-between; gap:12px; padding:10px 6px 4px 6px; }
    h1{ margin:0; font-size:18px; letter-spacing:.2px; }
    .sub{ color:var(--muted); font-size:12px; margin-top:4px; }
    .pill{
      font-size:12px; color:var(--muted);
      border:1px solid var(--border);
      padding:8px 10px; border-radius:999px;
      background:rgba(255,255,255,.03);
    }
    .grid{ display:grid; grid-template-columns: 1.1fr .9fr; gap:14px; }
    @media (max-width: 820px){ .grid{ grid-template-columns: 1fr; } }
    .card{
      background: linear-gradient(180deg, rgba(255,255,255,.05), rgba(255,255,255,.02));
      border:1px solid var(--border);
      border-radius:18px;
      box-shadow:var(--shadow);
      overflow:hidden;
    }
    .card .head{
      display:flex; justify-content:space-between; align-items:center;
      padding:14px 14px 10px 14px;
      border-bottom:1px solid var(--border);
    }
    .title{ font-size:14px; display:flex; align-items:center; gap:10px; }
    .hint{ font-size:12px; color:var(--muted); }
    .padWrap{ padding:14px; display:flex; align-items:center; justify-content:center; }

    .joy{
      width:min(440px, 100%);
      aspect-ratio: 1 / 1;
      border-radius:22px;
      background:
        radial-gradient(circle at 50% 50%, rgba(74,163,255,.18), transparent 45%),
        radial-gradient(circle at 50% 50%, rgba(255,255,255,.06), transparent 60%),
        rgba(0,0,0,.22);
      border:1px solid var(--border);
      position:relative;
      touch-action:none;
      user-select:none;
    }
    .joy::before{
      content:""; position:absolute; inset:16px;
      border-radius:18px; border:1px dashed rgba(255,255,255,.12);
    }
    .cross{ position:absolute; inset:0; display:flex; align-items:center; justify-content:center; pointer-events:none; opacity:.65; }
    .cross svg{ width:76%; height:76%; }
    .knob{
      width:92px; height:92px; border-radius:50%;
      background: radial-gradient(circle at 35% 30%, rgba(255,255,255,.22), rgba(255,255,255,.08) 40%, rgba(0,0,0,.1) 70%),
                  rgba(74,163,255,.18);
      border:1px solid rgba(255,255,255,.16);
      position:absolute; left:50%; top:50%;
      transform: translate(-50%,-50%);
      box-shadow: 0 14px 24px rgba(0,0,0,.35);
    }
    .readout{ display:flex; gap:10px; flex-wrap:wrap; padding:0 14px 14px 14px; }
    .kv{
      flex: 1 1 120px;
      border:1px solid var(--border);
      border-radius:14px;
      padding:10px 12px;
      background:rgba(0,0,0,.18);
    }
    .k{ font-size:11px; color:var(--muted); }
    .v{ font-size:16px; margin-top:4px; letter-spacing:.2px; }

    .toggles{ padding:14px; display:grid; gap:12px; }
    .btn{
      width:100%;
      display:flex; align-items:center; justify-content:space-between; gap:12px;
      padding:14px 14px;
      border-radius:16px;
      border:1px solid var(--border);
      background:rgba(0,0,0,.18);
      color:var(--text);
      font-size:14px;
      cursor:pointer;
      user-select:none;
      transition: transform .05s ease, border-color .2s ease, background .2s ease;
    }
    .btn:active{ transform: scale(.99); }
    .leftSide{ display:flex; align-items:center; gap:12px; }
    .icon{
      width:38px; height:38px; border-radius:12px;
      display:flex; align-items:center; justify-content:center;
      border:1px solid rgba(255,255,255,.12);
      background: rgba(255,255,255,.06);
    }
    .icon svg{ width:22px; height:22px; opacity:.95; }
    .switch{
      width:52px; height:30px; border-radius:999px;
      border:1px solid rgba(255,255,255,.14);
      background: rgba(255,255,255,.06);
      position:relative;
      flex:0 0 auto;
    }
    .dot{
      width:24px; height:24px; border-radius:50%;
      background: rgba(255,255,255,.22);
      position:absolute; top:50%; left:3px;
      transform: translateY(-50%);
      transition: left .2s ease, background .2s ease;
    }
    .btn.on{
      border-color: rgba(53,208,127,.55);
      background: rgba(53,208,127,.10);
    }
    .btn.on .switch{
      background: rgba(53,208,127,.18);
      border-color: rgba(53,208,127,.55);
    }
    .btn.on .dot{
      left:25px;
      background: rgba(53,208,127,.65);
    }
    footer{
      color:var(--muted);
      font-size:12px;
      padding:4px 6px 10px 6px;
      display:flex; justify-content:space-between; gap:12px; flex-wrap:wrap;
    }
    a{ color:var(--accent); text-decoration:none; }
  </style>
</head>
<body>
  <div class="wrap">
    <header>
      <div>
        <h1>Robot Remote</h1>
        <div class="sub">Drive with the touch pad. Toggle actuators on the right.</div>
      </div>
      <div class="pill" id="pill">Connecting…</div>
    </header>

    <div class="grid">
      <section class="card">
        <div class="head">
          <div class="title">
            <span class="icon" aria-hidden="true">
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                <path d="M7 14l-2 6" /><path d="M17 14l2 6" />
                <path d="M6 14h12l-1.5-6h-9z" />
                <path d="M8 14v-2" /><path d="M16 14v-2" />
              </svg>
            </span>
            Drive
          </div>
          <div class="hint">touch + drag</div>
        </div>

        <div class="padWrap">
          <div class="joy" id="joy">
            <div class="cross" aria-hidden="true">
              <svg viewBox="0 0 100 100" fill="none" stroke="rgba(255,255,255,.22)" stroke-width="3">
                <path d="M50 8v84" /><path d="M8 50h84" />
                <path d="M50 16l-6 6m6-6l6 6" />
                <path d="M50 84l-6-6m6 6l6-6" />
                <path d="M16 50l6-6m-6 6l6 6" />
                <path d="M84 50l-6-6m6 6l-6 6" />
              </svg>
            </div>
            <div class="knob" id="knob"></div>
          </div>
        </div>

        <div class="readout">
          <div class="kv"><div class="k">Left</div><div class="v" id="L">0</div></div>
          <div class="kv"><div class="k">Right</div><div class="v" id="R">0</div></div>
          <div class="kv"><div class="k">Link</div><div class="v" id="S">…</div></div>
        </div>
      </section>

      <section class="card">
        <div class="head">
          <div class="title">
            <span class="icon" aria-hidden="true">
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                <path d="M4 7h16" /><path d="M4 12h16" /><path d="M4 17h16" />
                <path d="M7 7v10" /><path d="M17 12v5" />
              </svg>
            </span>
            Actuators
          </div>
          <div class="hint">tap to toggle</div>
        </div>

        <div class="toggles">
          <button class="btn" id="btnWide">
            <span class="leftSide">
              <span class="icon" aria-hidden="true">
                <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                  <path d="M4 15h16" />
                  <path d="M6 15v5" /><path d="M10 15v5" /><path d="M14 15v5" /><path d="M18 15v5" />
                  <path d="M5 11c2-2 4-2 7 0s5 2 7 0" />
                </svg>
              </span>
              Wide brush
            </span>
            <span class="switch"><span class="dot"></span></span>
          </button>

          <button class="btn" id="btnCorner">
            <span class="leftSide">
              <span class="icon" aria-hidden="true">
                <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                  <path d="M6 6l12 12" />
                  <path d="M6 12h6" /><path d="M12 18v-6" />
                  <path d="M4 4l4 2-2 4-4-2z" />
                </svg>
              </span>
              Corner brush
            </span>
            <span class="switch"><span class="dot"></span></span>
          </button>

          <button class="btn" id="btnBlow">
            <span class="leftSide">
              <span class="icon" aria-hidden="true">
                <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                  <path d="M12 3v6" /><path d="M8 7l4 4 4-4" />
                  <path d="M6 14c2-2 10-2 12 0" /><path d="M7 18c3 2 7 2 10 0" />
                </svg>
              </span>
              Blower / sucker
            </span>
            <span class="switch"><span class="dot"></span></span>
          </button>
        </div>
      </section>
    </div>

    <footer>
      <div>Tip: Add to home screen for full-screen remote.</div>
      <div><a href="/status" target="_blank" rel="noopener">/status</a></div>
    </footer>
  </div>

<script>
(() => {
  const joy = document.getElementById('joy');
  const knob = document.getElementById('knob');
  const pill = document.getElementById('pill');
  const L = document.getElementById('L');
  const R = document.getElementById('R');
  const S = document.getElementById('S');
  const btnWide = document.getElementById('btnWide');
  const btnCorner = document.getElementById('btnCorner');
  const btnBlow = document.getElementById('btnBlow');

  let wide=false, corner=false, blow=false;
  let lastSend=0;

  function clamp(v,min,max){ return Math.max(min, Math.min(max,v)); }
  function setBtn(btn, on){ btn.classList.toggle('on', !!on); }

  async function loadStatus(){
    try{
      const r = await fetch('/status', {cache:'no-store'});
      if(r.status === 403){
        pill.textContent = 'Busy';
        S.textContent = 'wait';
        return;
      }
      const j = await r.json();
      wide=!!j.wide; corner=!!j.corner; blow=!!j.blower;
      setBtn(btnWide, wide); setBtn(btnCorner, corner); setBtn(btnBlow, blow);
      pill.textContent = 'Connected'; S.textContent='OK';
    }catch(e){
      pill.textContent = 'Not connected'; S.textContent='…';
    }
  }

  function mix(x, y){
    const throttle = -y;
    const steer = x;
    let left = throttle + steer;
    let right = throttle - steer;
    left = clamp(left, -1, 1);
    right = clamp(right, -1, 1);
    return { Li: Math.round(left*255), Ri: Math.round(right*255) };
  }

  async function sendApi(left, right){
    const now = performance.now();
    if(now - lastSend < 40) return; // ~25 Hz
    lastSend = now;
    try{
      const qs = `l=${left}&r=${right}&w=${wide?1:0}&c=${corner?1:0}&b=${blow?1:0}`;
      const r = await fetch('/api?' + qs, {cache:'no-store'});
      if(r.status === 403){
        pill.textContent = 'Busy';
        S.textContent = 'wait';
        return;
      }
      pill.textContent = 'Connected'; S.textContent='OK';
    }catch(e){
      pill.textContent = 'Not connected'; S.textContent='…';
    }
  }

  let active=false;
  let center=null;
  const maxR = () => (joy.getBoundingClientRect().width * 0.33);

  function updateKnob(dx, dy){
    const r = maxR();
    const mag = Math.hypot(dx,dy) || 1;
    const scale = mag > r ? r/mag : 1;
    const x = dx*scale, y = dy*scale;
    knob.style.transform = `translate(calc(-50% + ${x}px), calc(-50% + ${y}px))`;

    const nx = clamp(x / r, -1, 1);
    const ny = clamp(y / r, -1, 1);
    const {Li, Ri} = mix(nx, ny);
    L.textContent = Li; R.textContent = Ri;
    sendApi(Li, Ri);
  }

  function resetKnob(){
    knob.style.transform = 'translate(-50%,-50%)';
    const {Li, Ri} = mix(0,0);
    L.textContent = Li; R.textContent = Ri;
    sendApi(Li, Ri);
  }

  joy.addEventListener('pointerdown', (e) => {
    joy.setPointerCapture(e.pointerId);
    active=true;
    const rect = joy.getBoundingClientRect();
    center = {x: rect.left + rect.width/2, y: rect.top + rect.height/2};
    updateKnob(e.clientX - center.x, e.clientY - center.y);
  });

  joy.addEventListener('pointermove', (e) => {
    if(!active || !center) return;
    updateKnob(e.clientX - center.x, e.clientY - center.y);
  });

  function endPointer(){
    if(!active) return;
    active=false;
    center=null;
    resetKnob();
  }
  joy.addEventListener('pointerup', endPointer);
  joy.addEventListener('pointercancel', endPointer);
  joy.addEventListener('lostpointercapture', endPointer);

  function toggle(which){
    if(which==='wide') wide=!wide;
    if(which==='corner') corner=!corner;
    if(which==='blow') blow=!blow;
    setBtn(btnWide, wide); setBtn(btnCorner, corner); setBtn(btnBlow, blow);
    sendApi(parseInt(L.textContent||'0',10), parseInt(R.textContent||'0',10));
  }

  btnWide.addEventListener('click', () => toggle('wide'));
  btnCorner.addEventListener('click', () => toggle('corner'));
  btnBlow.addEventListener('click', () => toggle('blow'));

  loadStatus();
  resetKnob();
  setInterval(loadStatus, 5000);
})();
</script>
</body>
</html>
)HTML";


// ----------------------------

WIFIremote::WIFIremote() {}

bool WIFIremote::begin(const Config& cfg) {
  _cfg = cfg;

  if (_server) { delete _server; _server = nullptr; }
  _server = new WiFiServer(80);

  restartAP();

  const uint32_t now = millis();
  _lastApiMs = now;
  _lastActivityMs = now;
  releaseOwner();
  return _apUp;
}

void WIFIremote::loop() {
  if (!_apUp) {
    restartAP();
    delay(10);
    return;
  }

  // Captive portal DNS must be pumped frequently
  if (_dnsUp) _dns.processNextRequest();

  handleClient();
  applySafety();
}

bool WIFIremote::inUse() const {
  return (millis() - _lastApiMs) <= _cfg.inUseWindowMs;
}

uint8_t WIFIremote::stationCount() const {
#if WIFIREMOTE_TRY_STATIONNUM
  return (uint8_t)WiFi.softAPgetStationNum();
#else
  return 0;
#endif
}

bool WIFIremote::hasOwner() const {
  return _hasOwner && ownerLeaseValid();
}

int32_t WIFIremote::ownerId() const {
  if (!hasOwner()) return -1;
  return (int32_t)packIP(_ownerIP); // packed IPv4 -> 32-bit int
}

void WIFIremote::clearDrive() { _leftCmd = 0; _rightCmd = 0; }
void WIFIremote::clearAll() { clearDrive(); _wide=false; _corner=false; _blower=false; }

void WIFIremote::releaseOwner() {
  _hasOwner = false;
  _ownerIP = IPAddress(0,0,0,0);
  _ownerLastSeenMs = 0;
}

bool WIFIremote::ownerLeaseValid() const {
  if (!_hasOwner) return false;
  return (millis() - _ownerLastSeenMs) <= _cfg.ownerLeaseMs;
}

bool WIFIremote::isOwnerOrFree(const IPAddress& remoteIP) const {
  if (!_hasOwner) return true;
  if (!ownerLeaseValid()) return true;
  return remoteIP == _ownerIP;
}

uint32_t WIFIremote::packIP(const IPAddress& ip) {
  return ((uint32_t)ip[0] << 24) | ((uint32_t)ip[1] << 16) | ((uint32_t)ip[2] << 8) | (uint32_t)ip[3];
}

// ----------------------------
// AP + DNS
// ----------------------------

void WIFIremote::startDns() {
  stopDns();
  // wildcard domain -> AP IP
  _dnsUp = _dns.start(53, "*", _apIP);
}

void WIFIremote::stopDns() {
  if (_dnsUp) {
    _dns.stop();
    _dnsUp = false;
  }
}

void WIFIremote::restartAP() {
  stopAP();

  WiFi.mode(WIFI_AP);

  // Make AP IP stable for sticker use
  // If your build complains, comment this out (but usually supported ESP-style).
  WiFi.softAPConfig(_cfg.apIP, _cfg.apGateway, _cfg.apMask);

  bool ok = WiFi.softAP(_cfg.ssid, _cfg.pass, _cfg.channel, _cfg.hidden);
  delay(200);

  if (ok) {
    _apUp = true;
    _apIP = WiFi.softAPIP();   // should match cfg.apIP
    if (_server) _server->begin();
    startDns();
  } else {
    _apUp = false;
  }
}

void WIFIremote::stopAP() {
  if (_server) _server->stop();
  stopDns();
  WiFi.softAPdisconnect(true);
  delay(80);
  _apUp = false;
}

// ----------------------------
// Safety
// ----------------------------

void WIFIremote::applySafety() {
  const uint32_t now = millis();
  const uint32_t idle = now - _lastActivityMs;

  // Release owner if lease expired
  if (_hasOwner && !ownerLeaseValid()) {
    releaseOwner();
  }

  // 30s idle -> stop drive
  if (idle > _cfg.motorIdleMs) {
    if (_leftCmd != 0 || _rightCmd != 0) clearDrive();
  }

  // 2 min idle -> bounce AP & DNS so everyone can reconnect
  if (idle > _cfg.disconnectIdleMs) {
    restartAP();
    _lastActivityMs = now;
    _lastApiMs = now;
    releaseOwner();
  }

// Auto-off actuators after max on-time
if (_wide && _wideOnSinceMs && (now - _wideOnSinceMs > _cfg.actuatorMaxOnMs)) {
  _wide = false;
  _wideOnSinceMs = 0;
}
if (_corner && _cornerOnSinceMs && (now - _cornerOnSinceMs > _cfg.actuatorMaxOnMs)) {
  _corner = false;
  _cornerOnSinceMs = 0;
}
if (_blower && _blowerOnSinceMs && (now - _blowerOnSinceMs > _cfg.actuatorMaxOnMs)) {
  _blower = false;
  _blowerOnSinceMs = 0;
}


}

// ----------------------------
// Captive portal logic
// ----------------------------

bool WIFIremote::isCaptiveProbePath(const String& path) const {
  // Common OS probe endpoints (Android/iOS/Windows)
  // We redirect them all to "/" so the portal opens our UI.
  return
    path == "/generate_204" ||
    path == "/gen_204" ||
    path == "/hotspot-detect.html" ||
    path == "/library/test/success.html" ||
    path == "/ncsi.txt" ||
    path == "/connecttest.txt" ||
    path == "/redirect" ||
    path == "/success.txt" ||
    path == "/fwlink" ||
    path == "/wpad.dat";
}

void WIFIremote::redirectToRoot(WiFiClient& client) {
  // 302 redirect to "/" on our AP IP
  client.println("HTTP/1.1 302 Found");
  client.print("Location: http://");
  client.print(_apIP);
  client.println("/");
  client.println("Cache-Control: no-store");
  client.println("Connection: close");
  client.println();
}

// ----------------------------
// HTTP server
// ----------------------------

void WIFIremote::handleClient() {
  WiFiClient client = _server->available();
  if (!client) return;

  client.setTimeout(150);
  const IPAddress rip = client.remoteIP();

  String reqLine = client.readStringUntil('\n');
  reqLine.trim();

  // Drain headers
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r" || line.length() == 0) break;
  }

  int sp1 = reqLine.indexOf(' ');
  int sp2 = reqLine.indexOf(' ', sp1 + 1);
  if (sp1 < 0 || sp2 < 0) { client.stop(); return; }

  String method = reqLine.substring(0, sp1);
  String url = reqLine.substring(sp1 + 1, sp2);

  if (method != "GET") {
    sendResponse(client, "405 Method Not Allowed", "text/plain; charset=utf-8", "Only GET supported");
    client.stop();
    return;
  }

  // Split path from query quickly
  String path = url;
  int q = path.indexOf('?');
  if (q >= 0) path = path.substring(0, q);

  // Captive portal probes -> redirect to root
  if (isCaptiveProbePath(path)) {
    redirectToRoot(client);
    client.stop();
    return;
  }

  // Known endpoints
  if (path == "/" || path == "/index.html") {
    serveIndex(client, rip);
  } else if (path == "/api") {
    handleApi(client, url, rip);
  } else if (path == "/status") {
    serveStatus(client);
  } else {
    // Anything else: redirect to root (this is key for "type any URL")
    redirectToRoot(client);
  }

  client.stop();
}

void WIFIremote::serveIndex(WiFiClient& client, const IPAddress& remoteIP) {
  if (_cfg.lockControlPage && !isOwnerOrFree(remoteIP)) {
    sendResponseProgmem(client, "200 OK", "text/html; charset=utf-8", WAIT_HTML, sizeof(WAIT_HTML) - 1);
    return;
  }
  sendResponseProgmem(client, "200 OK", "text/html; charset=utf-8", INDEX_HTML, sizeof(INDEX_HTML) - 1);
}

void WIFIremote::serveStatus(WiFiClient& client) {
  String body = "{";
  body += "\"left\":" + String(_leftCmd) + ",";
  body += "\"right\":" + String(_rightCmd) + ",";
  body += "\"wide\":" + String(_wide ? "true" : "false") + ",";
  body += "\"corner\":" + String(_corner ? "true" : "false") + ",";
  body += "\"blower\":" + String(_blower ? "true" : "false") + ",";
  body += "\"inUse\":" + String(inUse() ? "true" : "false") + ",";
  body += "\"ownerId\":" + String(ownerId());
  body += "}";
  sendResponse(client, "200 OK", "application/json; charset=utf-8", body);
}

void WIFIremote::handleApi(WiFiClient& client, const String& url, const IPAddress& remoteIP) {
  const uint32_t now = millis();

  // Enforce single-user owner
  if (_hasOwner && ownerLeaseValid() && remoteIP != _ownerIP) {
    sendResponse(client, "403 Forbidden", "text/plain; charset=utf-8", "Busy");
    return;
  }

  // Acquire owner if free/expired
  if (!_hasOwner || !ownerLeaseValid()) {
    _hasOwner = true;
    _ownerIP = remoteIP;
  }
  _ownerLastSeenMs = now;

  _lastActivityMs = now;
  _lastApiMs = now;

  String v;
  if (getQueryParam(url, "l", v)) _leftCmd = clampi16(v.toInt(), -255, 255);
  if (getQueryParam(url, "r", v)) _rightCmd = clampi16(v.toInt(), -255, 255);

  // if (getQueryParam(url, "w", v)) _wide = parseBool01(v, _wide);
  // if (getQueryParam(url, "c", v)) _corner = parseBool01(v, _corner);
  // if (getQueryParam(url, "b", v)) _blower = parseBool01(v, _blower);
// wide
if (getQueryParam(url, "w", v)) {
  bool newWide = parseBool01(v, _wide);
  if (newWide && !_wide) _wideOnSinceMs = now;     // just turned ON
  if (!newWide) _wideOnSinceMs = 0;               // turned OFF
  _wide = newWide;
}

// corner
if (getQueryParam(url, "c", v)) {
  bool newCorner = parseBool01(v, _corner);
  if (newCorner && !_corner) _cornerOnSinceMs = now;
  if (!newCorner) _cornerOnSinceMs = 0;
  _corner = newCorner;
}

// blower
if (getQueryParam(url, "b", v)) {
  bool newBlower = parseBool01(v, _blower);
  if (newBlower && !_blower) _blowerOnSinceMs = now;
  if (!newBlower) _blowerOnSinceMs = 0;
  _blower = newBlower;
}


  sendResponse(client, "200 OK", "text/plain; charset=utf-8", "OK");
}

// ----------------------------
// HTTP helpers
// ----------------------------

void WIFIremote::sendResponse(WiFiClient& client, const char* code, const char* contentType, const String& body) {
  client.print("HTTP/1.1 "); client.println(code);
  client.print("Content-Type: "); client.println(contentType);
  client.print("Content-Length: "); client.println(body.length());
  client.println("Cache-Control: no-store");
  client.println("Connection: close");
  client.println();
  client.print(body);
}

void WIFIremote::sendResponseProgmem(WiFiClient& client, const char* code, const char* contentType, const uint8_t* data, size_t len) {
  client.print("HTTP/1.1 "); client.println(code);
  client.print("Content-Type: "); client.println(contentType);
  client.print("Content-Length: "); client.println(len);
  client.println("Cache-Control: no-store");
  client.println("Connection: close");
  client.println();

  const size_t chunk = 512;
  size_t sent = 0;
  while (sent < len) {
    size_t n = (len - sent) > chunk ? chunk : (len - sent);
    for (size_t i = 0; i < n; i++) client.write(pgm_read_byte(data + sent + i));
    sent += n;
    delay(0);
  }
}

// ----------------------------
// Query parsing
// ----------------------------

String WIFIremote::urlDecode(const String& s) {
  String out; out.reserve(s.length());
  for (size_t i = 0; i < s.length(); i++) {
    char c = s[i];
    if (c == '+') { out += ' '; continue; }
    if (c == '%' && i + 2 < s.length()) {
      char h1 = s[i+1], h2 = s[i+2];
      auto hex = [](char x)->int {
        if (x >= '0' && x <= '9') return x - '0';
        if (x >= 'a' && x <= 'f') return 10 + (x - 'a');
        if (x >= 'A' && x <= 'F') return 10 + (x - 'A');
        return 0;
      };
      out += (char)((hex(h1) << 4) | hex(h2));
      i += 2;
      continue;
    }
    out += c;
  }
  return out;
}

bool WIFIremote::getQueryParam(const String& url, const char* key, String& outValue) {
  int q = url.indexOf('?');
  if (q < 0) return false;
  String qs = url.substring(q + 1);

  String k = String(key) + "=";
  int start = 0;
  while (start < (int)qs.length()) {
    int amp = qs.indexOf('&', start);
    if (amp < 0) amp = qs.length();
    String part = qs.substring(start, amp);
    if (part.startsWith(k)) {
      outValue = urlDecode(part.substring(k.length()));
      return true;
    }
    start = amp + 1;
  }
  return false;
}

int16_t WIFIremote::clampi16(int v, int16_t lo, int16_t hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return (int16_t)v;
}

bool WIFIremote::parseBool01(const String& v, bool fallback) {
  if (v == "1") return true;
  if (v == "0") return false;
  if (v == "true" || v == "TRUE") return true;
  if (v == "false" || v == "FALSE") return false;
  return fallback;
}
