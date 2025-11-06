# 🏃‍♂️ SprintX – Smart Starting Block System

**SprintX** is a smart IoT-based starting block system designed for sprinters and athletes.  
It detects **reaction time**, **false starts**, and **force pressure** using **ESP32**, **Piezo sensors**, and an **FSR (Force Sensitive Resistor)** — integrated with a **Blynk IoT dashboard** and **LCD display**.

## 🔧 Features

- ⏱ **Real-time Reaction Time Measurement**  
  Measures how quickly an athlete reacts after the start signal.

- 🚫 **False Start Detection**  
  Detects premature foot movement before the official start.

- 📱 **Blynk IoT Integration**  
  View live reaction data, pressure graphs, and alerts in the Blynk mobile app.

- 💡 **LCD Display Output**  
  Shows “Ready”, “False Start”, or “Reaction Time” directly on the 16x2 I2C LCD.

- 🔊 **Buzzer Alerts**  
  Gives audio feedback for valid and false starts.

## 🧠 Components Used

| Component | Description |
|------------|--------------|
| ESP32 | Main microcontroller (Wi-Fi enabled) |
| Piezo Sensors (2x) | Detects foot pressure and release |
| FSR Sensor | Detects starting block readiness |
| LCD 16x2 I2C | Displays status and time |
| Buzzer | Provides start/false-start indication |
| Push Button | Manual control/reset |


## 📲 Blynk Dashboard Setup

1. Create a new **Blynk template** and get your `Auth Token`.
2. Set up the following virtual pins:
   - **V1:** Reaction time (ms)  
   - **V2:** Status (Ready / Running / Valid / False Start)  
   - **V3:** FSR weight (kg)  
   - **V4:** Live force graph
3. Add widgets: Gauge, Graph, and LED indicators.


## 🚀 How It Works

1. The system waits for the “Ready” state.  
2. When the button is pressed, the **buzzer beeps** — the timer starts.  
3. When the athlete pushes off, the piezo sensors detect foot release.  
4. If release happens too early → **False Start** alert.  
5. Otherwise → **Reaction Time** displayed on LCD and sent to Blynk.

## 🧰 Technologies Used

- **Arduino IDE**
- **ESP32**
- **Blynk IoT**
- **C++ / Embedded Programming**
- **IoT Sensors**



## 🧑‍💻 Developed By
**Team SprintX**  
-  P.S.Jayathilaka
-  K.A.N.T. Nethsiri
-  K.A.D.Perera
-  T.P.A.Wickramasinghe 
📍 University of SLIIT


## 💡 Future Improvements
- Add mobile app analytics dashboard
- Integrate start signal light system
- Include cloud data storage for athlete performance history


⭐ *If you like this project, give it a star on GitHub!*



