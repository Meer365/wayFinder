#include <WiFi.h>
#include <WebServer.h>

const char* AP_SSID = "IndoorNav";
const char* AP_PASS = "12345678";

WebServer server(80);

String destination = "";
String currentNode = "ENTRANCE";
String instruction = "Select destination";
String trackedName = "NAV_USER_01";

String incomingUnoCmd = "";

String buildPage() {
  String page = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Local Navigation</title>
  <style>
    :root{
      --bg1:#0f172a;
      --bg2:#111827;
      --glass:rgba(255,255,255,0.08);
      --glass-strong:rgba(255,255,255,0.12);
      --border:rgba(255,255,255,0.14);
      --text:#f8fafc;
      --muted:#cbd5e1;
      --accent:#60a5fa;
      --accent2:#22d3ee;
      --green:#22c55e;
      --yellow:#facc15;
      --red:#ef4444;
      --shadow:0 20px 60px rgba(0,0,0,0.35);
    }

    * { box-sizing: border-box; }

    body {
      margin: 0;
      min-height: 100vh;
      color: var(--text);
      font-family: Inter, Arial, sans-serif;
      background:
        radial-gradient(circle at 15% 20%, rgba(34,211,238,0.18), transparent 25%),
        radial-gradient(circle at 85% 15%, rgba(96,165,250,0.22), transparent 26%),
        radial-gradient(circle at 50% 85%, rgba(168,85,247,0.16), transparent 30%),
        linear-gradient(135deg, var(--bg1), var(--bg2));
      overflow-x: hidden;
    }

    .blob {
      position: fixed;
      filter: blur(60px);
      opacity: 0.45;
      z-index: 0;
      pointer-events: none;
      animation: float 10s ease-in-out infinite;
    }

    .blob.one {
      width: 180px; height: 180px;
      left: -30px; top: 40px;
      background: rgba(34,211,238,0.28);
    }

    .blob.two {
      width: 220px; height: 220px;
      right: -40px; top: 120px;
      background: rgba(59,130,246,0.22);
      animation-delay: -3s;
    }

    .blob.three {
      width: 220px; height: 220px;
      left: 20%; bottom: -60px;
      background: rgba(168,85,247,0.18);
      animation-delay: -6s;
    }

    @keyframes float {
      0%,100% { transform: translateY(0px) translateX(0px); }
      50% { transform: translateY(-14px) translateX(10px); }
    }

    .wrap {
      position: relative;
      z-index: 1;
      max-width: 980px;
      margin: 0 auto;
      padding: 22px 16px 32px;
    }

    .hero {
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 16px;
      padding: 18px 20px;
      background: var(--glass);
      border: 1px solid var(--border);
      border-radius: 24px;
      backdrop-filter: blur(16px);
      box-shadow: var(--shadow);
      margin-bottom: 18px;
    }

    .hero-left h1 {
      margin: 0 0 6px 0;
      font-size: 28px;
      letter-spacing: 0.2px;
    }

    .hero-left p {
      margin: 0;
      color: var(--muted);
      font-size: 14px;
    }

    .pill {
      display: inline-flex;
      align-items: center;
      gap: 8px;
      background: rgba(255,255,255,0.08);
      border: 1px solid var(--border);
      color: var(--muted);
      padding: 10px 14px;
      border-radius: 999px;
      font-size: 13px;
      white-space: nowrap;
    }

    .dot {
      width: 10px;
      height: 10px;
      border-radius: 50%;
      background: var(--green);
      box-shadow: 0 0 12px var(--green);
      animation: pulse 1.6s infinite;
    }

    @keyframes pulse {
      0%,100% { transform: scale(1); opacity: 1; }
      50% { transform: scale(1.3); opacity: 0.7; }
    }

    .grid {
      display: grid;
      grid-template-columns: 1.05fr 1fr;
      gap: 18px;
    }

    .panel {
      background: var(--glass);
      border: 1px solid var(--border);
      border-radius: 24px;
      backdrop-filter: blur(16px);
      box-shadow: var(--shadow);
      overflow: hidden;
    }

    .panel-body {
      padding: 20px;
    }

    .section-title {
      margin: 0 0 14px 0;
      font-size: 14px;
      color: var(--muted);
      letter-spacing: 0.08em;
      text-transform: uppercase;
    }

    .button-row {
      display: flex;
      gap: 12px;
      flex-wrap: wrap;
      margin-bottom: 18px;
    }

    button {
      border: 0;
      border-radius: 18px;
      padding: 14px 18px;
      font-size: 16px;
      font-weight: 700;
      color: white;
      cursor: pointer;
      transition: transform .16s ease, box-shadow .16s ease, opacity .16s ease;
      box-shadow: 0 12px 24px rgba(0,0,0,0.22);
    }

    button:hover { transform: translateY(-2px); }
    button:active { transform: translateY(0px) scale(0.98); }

    .btn-a { background: linear-gradient(135deg, #10b981, #22c55e); }
    .btn-b { background: linear-gradient(135deg, #3b82f6, #06b6d4); }
    .btn-reset { background: linear-gradient(135deg, #f43f5e, #ef4444); }

    .cards {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      gap: 12px;
    }

    .mini-card {
      background: rgba(255,255,255,0.05);
      border: 1px solid rgba(255,255,255,0.09);
      border-radius: 18px;
      padding: 14px;
      min-height: 98px;
    }

    .mini-label {
      color: var(--muted);
      font-size: 12px;
      margin-bottom: 10px;
      text-transform: uppercase;
      letter-spacing: 0.06em;
    }

    .mini-value {
      font-size: 22px;
      font-weight: 800;
      line-height: 1.15;
      word-break: break-word;
    }

    .nav-stage {
      position: relative;
      min-height: 100%;
      padding: 26px 20px 20px;
      display: flex;
      flex-direction: column;
      gap: 16px;
      justify-content: center;
      align-items: center;
    }

    .ring {
      width: 220px;
      height: 220px;
      border-radius: 50%;
      position: relative;
      display: grid;
      place-items: center;
      background:
        radial-gradient(circle at center, rgba(255,255,255,0.08) 0 36%, transparent 37%),
        conic-gradient(from 0deg, rgba(34,211,238,0.45), rgba(96,165,250,0.25), rgba(168,85,247,0.3), rgba(34,211,238,0.45));
      animation: spinSlow 12s linear infinite;
      box-shadow:
        inset 0 0 30px rgba(255,255,255,0.07),
        0 18px 60px rgba(0,0,0,0.35);
    }

    .ring::before {
      content: "";
      position: absolute;
      inset: 14px;
      border-radius: 50%;
      background: rgba(10,15,30,0.92);
      border: 1px solid rgba(255,255,255,0.08);
    }

    @keyframes spinSlow {
      from { transform: rotate(0deg); }
      to { transform: rotate(360deg); }
    }

    .arrow-wrap {
      position: absolute;
      inset: 0;
      display: grid;
      place-items: center;
      z-index: 2;
    }

    .arrow {
      width: 0;
      height: 0;
      border-left: 22px solid transparent;
      border-right: 22px solid transparent;
      border-bottom: 90px solid var(--yellow);
      transform-origin: 50% 78%;
      filter: drop-shadow(0 0 18px rgba(250,204,21,0.35));
      transition: transform .35s ease, filter .35s ease;
      animation: breathe 1.4s ease-in-out infinite;
      position: relative;
    }

    .arrow::after {
      content: "";
      position: absolute;
      left: -8px;
      top: 86px;
      width: 16px;
      height: 28px;
      border-radius: 8px;
      background: var(--yellow);
    }

    @keyframes breathe {
      0%,100% { transform: scale(1) translateY(0px); }
      50% { transform: scale(1.04) translateY(-2px); }
    }

    .dir-text { text-align: center; }
    .dir-main {
      font-size: 28px;
      font-weight: 900;
      margin-bottom: 8px;
      letter-spacing: 0.02em;
    }
    .dir-sub {
      font-size: 15px;
      color: var(--muted);
      line-height: 1.5;
      max-width: 340px;
    }

    .status-banner {
      margin-top: 6px;
      padding: 12px 14px;
      border-radius: 16px;
      font-size: 14px;
      color: var(--text);
      background: rgba(255,255,255,0.06);
      border: 1px solid rgba(255,255,255,0.08);
      text-align: center;
      width: 100%;
    }

    .reached {
      background: rgba(34,197,94,0.15);
      border-color: rgba(34,197,94,0.35);
      color: #dcfce7;
      box-shadow: 0 0 22px rgba(34,197,94,0.16);
    }

    .node-chip {
      display: inline-flex;
      align-items: center;
      justify-content: center;
      min-width: 74px;
      padding: 10px 12px;
      border-radius: 999px;
      background: rgba(255,255,255,0.07);
      border: 1px solid rgba(255,255,255,0.08);
      font-size: 13px;
      color: var(--muted);
      gap: 8px;
    }

    .chip-row {
      display: flex;
      gap: 10px;
      flex-wrap: wrap;
      justify-content: center;
    }

    .node-chip.active {
      color: white;
      border-color: rgba(96,165,250,0.45);
      background: rgba(96,165,250,0.18);
      box-shadow: 0 0 18px rgba(96,165,250,0.14);
    }

    @media (max-width: 860px) {
      .grid { grid-template-columns: 1fr; }
      .hero { flex-direction: column; align-items: flex-start; }
      .cards { grid-template-columns: 1fr; }
      .ring { width: 190px; height: 190px; }
      .dir-main { font-size: 24px; }
    }
  </style>

  <script>
    function parseDirection(instruction) {
      const text = (instruction || "").toLowerCase();
      if (text.includes("left")) return "left";
      if (text.includes("right")) return "right";
      if (text.includes("straight")) return "straight";
      if (text.includes("reached")) return "done";
      return "none";
    }

    function directionAngle(dir) {
      if (dir === "left") return -90;
      if (dir === "right") return 90;
      if (dir === "straight") return 0;
      if (dir === "done") return 180;
      return 0;
    }

    function directionLabel(dir) {
      if (dir === "left") return "TURN LEFT";
      if (dir === "right") return "TURN RIGHT";
      if (dir === "straight") return "GO STRAIGHT";
      if (dir === "done") return "DESTINATION REACHED";
      return "WAITING";
    }

    function updateArrow(dir) {
      const arrow = document.getElementById("arrow");
      const banner = document.getElementById("statusBanner");
      const dirMain = document.getElementById("dirMain");
      const angle = directionAngle(dir);
      arrow.style.transform = `rotate(${angle}deg)`;

      if (dir === "done") {
        arrow.style.borderBottomColor = "#22c55e";
        arrow.style.filter = "drop-shadow(0 0 18px rgba(34,197,94,0.45))";
        banner.classList.add("reached");
      } else if (dir === "none") {
        arrow.style.borderBottomColor = "#94a3b8";
        arrow.style.filter = "drop-shadow(0 0 16px rgba(148,163,184,0.28))";
        banner.classList.remove("reached");
      } else {
        arrow.style.borderBottomColor = "#facc15";
        arrow.style.filter = "drop-shadow(0 0 18px rgba(250,204,21,0.35))";
        banner.classList.remove("reached");
      }

      dirMain.textContent = directionLabel(dir);
    }

    function setActiveNode(node) {
      const chips = document.querySelectorAll(".node-chip");
      chips.forEach(chip => chip.classList.remove("active"));

      const map = {
        "ENTRANCE": "chipEntrance",
        "MIDDLE": "chipMiddle",
        "A": "chipA",
        "B": "chipB"
      };

      const id = map[node];
      if (id) {
        const el = document.getElementById(id);
        if (el) el.classList.add("active");
      }
    }

    async function setDest(dest) {
      await fetch('/setdest?d=' + encodeURIComponent(dest));
      refreshStatus();
    }

    async function resetNav() {
      await fetch('/reset');
      refreshStatus();
    }

    async function refreshStatus() {
      try {
        const res = await fetch('/status');
        const data = await res.json();

        const dest = data.destination || "-";
        const node = data.currentNode || "ENTRANCE";
        const inst = data.instruction || "Select destination";

        document.getElementById('dest').innerText = dest;
        document.getElementById('node').innerText = node;
        document.getElementById('inst').innerText = inst;
        document.getElementById('statusBanner').innerText = inst;

        setActiveNode(node);
        updateArrow(parseDirection(inst));
      } catch (e) {
        document.getElementById('statusBanner').innerText = "Connection issue. Retrying...";
      }
    }

    setInterval(refreshStatus, 1200);
    window.onload = refreshStatus;
  </script>
</head>
<body>
  <div class="blob one"></div>
  <div class="blob two"></div>
  <div class="blob three"></div>

  <div class="wrap">
    <div class="hero">
      <div class="hero-left">
        <h1>Local Navigation</h1>
        <p>Live indoor route guidance with real-time node updates</p>
      </div>
      <div class="pill">
        <span class="dot"></span>
        Live navigation active
      </div>
    </div>

    <div class="grid">
      <div class="panel">
        <div class="panel-body">
          <div class="section-title">Destination Control</div>

          <div class="button-row">
            <button class="btn-a" onclick="setDest('A')">Go to A</button>
            <button class="btn-b" onclick="setDest('B')">Go to B</button>
            <button class="btn-reset" onclick="resetNav()">Reset</button>
          </div>

          <div class="cards">
            <div class="mini-card">
              <div class="mini-label">Destination</div>
              <div class="mini-value" id="dest">-</div>
            </div>
            <div class="mini-card">
              <div class="mini-label">Current Node</div>
              <div class="mini-value" id="node">ENTRANCE</div>
            </div>
            <div class="mini-card">
              <div class="mini-label">Instruction</div>
              <div class="mini-value" id="inst" style="font-size:18px;">Select destination</div>
            </div>
          </div>
        </div>
      </div>

      <div class="panel">
        <div class="nav-stage">
          <div class="ring">
            <div class="arrow-wrap">
              <div class="arrow" id="arrow"></div>
            </div>
          </div>

          <div class="dir-text">
            <div class="dir-main" id="dirMain">WAITING</div>
            <div class="dir-sub">The direction arrow updates live from the latest navigation instruction.</div>
          </div>

          <div class="chip-row">
            <div class="node-chip active" id="chipEntrance">ENTRANCE</div>
            <div class="node-chip" id="chipMiddle">MIDDLE</div>
            <div class="node-chip" id="chipA">A</div>
            <div class="node-chip" id="chipB">B</div>
          </div>

          <div class="status-banner" id="statusBanner">Select destination</div>
        </div>
      </div>
    </div>
  </div>
</body>
</html>
)rawliteral";
  return page;
}

char getDestCode(String dest) {
  if (dest == "A") return 'A';
  if (dest == "B") return 'B';
  return '-';
}

char getNodeCode(String node) {
  if (node == "ENTRANCE") return 'E';
  if (node == "MIDDLE") return 'M';
  if (node == "A") return 'A';
  if (node == "B") return 'B';
  return '-';
}

char getDirCode() {
  String s = instruction;
  s.toLowerCase();
  if (s.indexOf("left") != -1) return 'L';
  if (s.indexOf("right") != -1) return 'R';
  if (s.indexOf("straight") != -1) return 'S';
  if (s.indexOf("reached") != -1) return 'D';
  return 'N';
}

void sendStateToUNO() {
  String packet = "";
  packet += getDestCode(destination);
  packet += "|";
  packet += getNodeCode(currentNode);
  packet += "|";
  packet += getDirCode();
  packet += "|";
  packet += instruction;

  Serial2.println(packet);

  Serial.print("Sent to UNO: ");
  Serial.println(packet);
}

void updateInstruction() {
  if (destination == "") {
    instruction = "Select destination";
    return;
  }

  if (destination == "A") {
    if (currentNode == "ENTRANCE") instruction = "Go straight";
    else if (currentNode == "MIDDLE") instruction = "Turn left";
    else if (currentNode == "A") instruction = "You reached A";
  }

  if (destination == "B") {
    if (currentNode == "ENTRANCE") instruction = "Go straight";
    else if (currentNode == "MIDDLE") instruction = "Turn right";
    else if (currentNode == "B") instruction = "You reached B";
  }
}

void applyUnoCommand(char cmd) {
  if (cmd == 'A') {
    destination = "A";
    currentNode = "ENTRANCE";
    updateInstruction();
    sendStateToUNO();
    Serial.println("UNO selected A");
  }
  else if (cmd == 'B') {
    destination = "B";
    currentNode = "ENTRANCE";
    updateInstruction();
    sendStateToUNO();
    Serial.println("UNO selected B");
  }
  else if (cmd == 'R') {
    destination = "";
    currentNode = "ENTRANCE";
    updateInstruction();
    sendStateToUNO();
    Serial.println("UNO reset");
  }
}

void readUnoSerial() {
  while (Serial2.available()) {
    char c = Serial2.read();

    if (c == '\n') {
      incomingUnoCmd.trim();

      if (incomingUnoCmd.length() > 0) {
        Serial.print("From UNO: ");
        Serial.println(incomingUnoCmd);

        if (incomingUnoCmd == "A") applyUnoCommand('A');
        else if (incomingUnoCmd == "B") applyUnoCommand('B');
        else if (incomingUnoCmd == "R") applyUnoCommand('R');
      }

      incomingUnoCmd = "";
    } else {
      incomingUnoCmd += c;
    }
  }
}

void handleRoot() {
  server.send(200, "text/html", buildPage());
}

void handleSetDest() {
  if (server.hasArg("d")) {
    destination = server.arg("d");
    currentNode = "ENTRANCE";
    updateInstruction();
    sendStateToUNO();
  }
  server.send(200, "text/plain", "OK");
}

void handleReset() {
  destination = "";
  currentNode = "ENTRANCE";
  updateInstruction();
  sendStateToUNO();
  server.send(200, "text/plain", "RESET");
}

void handleStatus() {
  String json = "{";
  json += "\"destination\":\"" + destination + "\",";
  json += "\"currentNode\":\"" + currentNode + "\",";
  json += "\"instruction\":\"" + instruction + "\"";
  json += "}";
  server.send(200, "application/json", json);
}

void handleReport() {
  if (!server.hasArg("node") || !server.hasArg("name")) {
    server.send(400, "text/plain", "Missing args");
    return;
  }

  String node = server.arg("node");
  String name = server.arg("name");

  if (name != trackedName) {
    server.send(403, "text/plain", "Wrong user");
    return;
  }

  if (destination == "A") {
    if (currentNode == "ENTRANCE" && node == "MIDDLE") currentNode = "MIDDLE";
    else if (currentNode == "MIDDLE" && node == "A") currentNode = "A";
  }

  if (destination == "B") {
    if (currentNode == "ENTRANCE" && node == "MIDDLE") currentNode = "MIDDLE";
    else if (currentNode == "MIDDLE" && node == "B") currentNode = "B";
  }

  updateInstruction();
  sendStateToUNO();
  server.send(200, "text/plain", "RECEIVED");
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);  // RX2=16, TX2=17

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);

  Serial.println();
  Serial.print("Hotspot started. IP: ");
  Serial.println(WiFi.softAPIP());

  updateInstruction();

  server.on("/", handleRoot);
  server.on("/setdest", handleSetDest);
  server.on("/reset", handleReset);
  server.on("/status", handleStatus);
  server.on("/report", handleReport);

  server.begin();
  Serial.println("Web server started");

  delay(500);
  sendStateToUNO();
}

void loop() {
  server.handleClient();
  readUnoSerial();
}