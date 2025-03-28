const { default: axios } = require("axios")
const bodyParser = require("body-parser")
const express = require("express")
const app = express()
require('dotenv').config()

app.set("view engine", "ejs")
app.use(bodyParser.json())

let pidValues = {Kp:1.0, Ki:1.0, Kd:1.0}
esp32IP = "0.0.0.0"

app.post("/registerESP", (req, res) => {
    const {recv_IP, old_Kp, old_Ki, old_Kd} = req.body

    if (!recv_IP || old_Kp === undefined || old_Ki === undefined || old_Kd === undefined) {
        return res.status(400).send('Invalid payload from ESP32');
    }

    esp32IP = recv_IP;
    pidValues = { Kp: old_Kp, Ki: old_Ki, Kd: old_Kd };
    console.log(`ESP32 Registered: ${esp32IP} | PID: Kp=${pidValues.Kp}, Ki=${pidValues.Ki}, Kd=${pidValues.Kd}`);

    res.render("index", pidValues)
})

app.post("/updatePID", async (req, res) => {

    pidValues = req.body

    console.log(`PID Update: P=${pidValues.Kp}, I=${pidValues.Ki}, D=${pidValues.Kd}`);

    if (!esp32IP) {
        return res.status(400).send("ESP32 not registered")
    }

    try {
        console.log("Sending update to ESP32...\n")
        await axios.post(`http://${esp32IP}/updatePID`, pidValues)
        return res.send("PID Values sent to ESP32")
    } catch {
        return res.status(500).send("Could not send the values to ESP32")
    }

})

app.get("/", (req, res) => {
    res.render("index", pidValues)
})

const PORT = process.env.PORT || 3000
app.listen(PORT, () => {
    console.log("Server opened on port", PORT)
})
