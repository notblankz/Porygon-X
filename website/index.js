const express = require("express");
const app = express();
const path = require("path");
const PORT = 3000;

app.use(express.json());

app.use(express.static(path.join(__dirname, "public")));



let esp32IP = null;
let pidValues = { kp: 0, ki: 0, kd: 0 };

// Root route
app.get("/", (req, res) => {
  res.sendFile(path.join(__dirname, "public", "index.html"));
});

// ESP32 sends IP
app.post("/registerEsp", (req, res) => {
  if (!req.body.ip) {
    return res.status(400).send({ message: "IP address is required" });
  }
  esp32IP = req.body.ip;
  console.log(`ESP32 registered with IP ${esp32IP}`);
  res.send({ message: "ESP32 registered successfully", esp32IP });
});

// Website updates PID values
app.post("/updatePid", (req, res) => {
  const { kp, ki, kd } = req.body;
  if (kp === undefined || ki === undefined || kd === undefined) {
    return res.status(400).send({ message: "Missing PID values" });
  }
  pidValues = { kp, ki, kd };
  console.log(`Updated PID values: KP=${kp}, KI=${ki}, KD=${kd}`);
  res.send({ message: "PID values updated successfully", pidValues });
});

// Server sends current PID values to ESP
app.get("/getPid", (req, res) => {
  if (!esp32IP) {
    return res.status(400).send({ message: "ESP32 is not registered yet" });
  }
  res.send(pidValues);
});

// Server starts
app.listen(PORT, () => {
  console.log(`Server is running on http://localhost:${PORT}`);
});





