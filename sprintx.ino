#define BLYNK_TEMPLATE_ID "TMPL6WXzIId-K"
#define BLYNK_TEMPLATE_NAME "SprintX"

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// Blynk credentials - REPLACE WITH YOUR VALUES
char auth[] = "xKddf2lI-w86jwJ_sH8OhyG220i8f-kM";      // Replace with your actual Auth Token
char ssid[] = "praveen";    // Replace with your WiFi name
char pass[] = "praveen123";       // Replace with your WiFi password

// Pin definitions (same as your original code)
const int buttonPin = 14;
const int buzzerPin = 12;
const int piezoLeftPin = 34;
const int piezoRightPin = 35;
const int FSR_PIN = 33;

LiquidCrystal_I2C lcd(0x27, 16, 2);
BlynkTimer timer;

// Thresholds and constants (same as your original)
const int PRESS_THRESHOLD = 500;
const int RELEASE_THRESHOLD = 100;
const int FSR_GATE_THRESHOLD = 2000;
const unsigned long FALSE_START_THRESHOLD = 100;
const unsigned long MIN_PRESS_DURATION = 50;

// Timing variables (same as your original)
unsigned long startTime = 0;
bool running = false;
unsigned long lastUpdate = 0;
bool displayingResult = false;
unsigned long lastButtonPress = 0;
const unsigned long DEBOUNCE_TIME = 50;
unsigned long buzzerStart = 0;
bool buzzerOn = false;
bool buttonReleased = true;

// Left foot variables
bool piezo_pressed_left = false;
int peak_left = 0;
unsigned long release_time_left = 0;
unsigned long piezo_press_time_left = 0;

// Right foot variables
bool piezo_pressed_right = false;
int peak_right = 0;
unsigned long release_time_right = 0;
unsigned long piezo_press_time_right = 0;

// FSR variables
int fsrValue = 0;

// Blynk display variables (send-only)
bool sendingGraphData = false;
unsigned long graphStartTime = 0;

// Status enumeration for Blynk
enum StatusEnum {
  STATUS_READY = 0,
  STATUS_RUNNING = 1, 
  STATUS_VALID_START = 2,
  STATUS_FALSE_START = 3
};

void setup() {
  Serial.begin(115200);

  // Hardware setup (exactly same as your original)
  pinMode(buttonPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
  pinMode(piezoLeftPin, INPUT);
  pinMode(piezoRightPin, INPUT);
  pinMode(FSR_PIN, INPUT);

  // LCD setup
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");

  // Connect to Blynk (non-blocking)
  Serial.println("Connecting to Blynk...");
  Blynk.begin(auth, ssid, pass);
  
  // Setup timer for sending data to Blynk
  timer.setInterval(100L, sendDataToBlynk);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Stopwatch Ready");
  
  // Send initial status to app
  Blynk.virtualWrite(V1, 0);                    // Reaction time: 0
  Blynk.virtualWrite(V2, STATUS_READY);         // Status: READY
  Blynk.virtualWrite(V3, 0);                    // FSR weight: 0
}

float mapFSRToKg(int fsrVal) {
  // Same mapping function as your original
  int fsr_min = 200;
  int fsr_max = 3000;
  float weight_max = 10;

  if (fsrVal < fsr_min) fsrVal = fsr_min;
  if (fsrVal > fsr_max) fsrVal = fsr_max;

  return (float)(fsrVal - fsr_min) * weight_max / (fsr_max - fsr_min);
}

void sendDataToBlynk() {
  // Send FSR weight data
  float weight = mapFSRToKg(fsrValue);
  Blynk.virtualWrite(V3, weight);
  
  // Send force graph data if recording
  if (sendingGraphData) {
    float timeSeconds = (millis() - graphStartTime) / 1000.0;
    
    // Send the higher of the two piezo readings
    int current_left = analogRead(piezoLeftPin);
    int current_right = analogRead(piezoRightPin);
    int maxPiezo = max(current_left, current_right);
    
    Blynk.virtualWrite(V4, timeSeconds, maxPiezo);
    
    // Stop sending graph data after 15 seconds
    if (timeSeconds > 15.0) {
      sendingGraphData = false;
      Serial.println("Graph recording completed");
    }
  }
}

void loop() {
  // Run Blynk (non-blocking)
  Blynk.run();
  timer.run();
  
  // ALL YOUR ORIGINAL HARDWARE LOGIC REMAINS EXACTLY THE SAME
  // Handle button press with software debouncing
  if (digitalRead(buttonPin) == HIGH && (millis() - lastButtonPress > DEBOUNCE_TIME)) {
    if (!running && !displayingResult && buttonReleased) {
      startTimer();
      buttonReleased = false;
    } else if (displayingResult) {
      resetToReady();
      buttonReleased = false;
    }
    lastButtonPress = millis();
  } else if (digitalRead(buttonPin) == LOW) {
    buttonReleased = true;
  }

  // Turn off buzzer after 50ms
  if (buzzerOn && millis() - buzzerStart >= 50) {
    digitalWrite(buzzerPin, LOW);
    buzzerOn = false;
  }

  // Read sensor values
  int current_left = analogRead(piezoLeftPin);
  int current_right = analogRead(piezoRightPin);
  fsrValue = analogRead(FSR_PIN);

  // Debug output
  Serial.print("Piezo Left: ");
  Serial.print(current_left);
  Serial.print(" | Piezo Right: ");
  Serial.print(current_right);
  Serial.print(" | FSR: ");
  Serial.println(fsrValue);

  // Process piezo sensors only if FSR gate is active (SAME AS ORIGINAL)
  if (fsrValue > FSR_GATE_THRESHOLD) {
    // Process left foot
    if (!piezo_pressed_left && current_left > PRESS_THRESHOLD) {
      piezo_pressed_left = true;
      peak_left = current_left;
      piezo_press_time_left = millis();
    } else if (piezo_pressed_left && current_left < RELEASE_THRESHOLD && 
               (millis() - piezo_press_time_left > MIN_PRESS_DURATION)) {
      piezo_pressed_left = false;
      if (!running && !displayingResult) {
        triggerFalseStartPreButton();
      } else if (running && release_time_left == 0) {
        release_time_left = millis();
        processRelease();
      }
    }

    // Process right foot
    if (!piezo_pressed_right && current_right > PRESS_THRESHOLD) {
      piezo_pressed_right = true;
      peak_right = current_right;
      piezo_press_time_right = millis();
    } else if (piezo_pressed_right && current_right < RELEASE_THRESHOLD && 
               (millis() - piezo_press_time_right > MIN_PRESS_DURATION)) {
      piezo_pressed_right = false;
      if (!running && !displayingResult) {
        triggerFalseStartPreButton();
      } else if (running && release_time_right == 0) {
        release_time_right = millis();
        processRelease();
      }
    }
  }

  // Update LCD with elapsed time if running (SAME AS ORIGINAL)
  if (running) {
    unsigned long currentTime = millis();
    unsigned long elapsed = currentTime - startTime;
    
    int minutes = (elapsed / 60000) % 60;
    int seconds = (elapsed / 1000) % 60;
    int milliseconds = elapsed % 1000;
    
    if (currentTime - lastUpdate >= 50) {
      lcd.setCursor(0, 1);
      char timeStr[16];
      snprintf(timeStr, sizeof(timeStr), "Time: %02d:%02d.%03d", minutes, seconds, milliseconds);
      lcd.print(timeStr);
      lastUpdate = currentTime;
    }
  }
}

// SAME AS YOUR ORIGINAL + send data to app
void processRelease() {
  if (!running || displayingResult) return;

  running = false;
  displayingResult = true;
  sendingGraphData = false; // Stop graph data
  lcd.clear();

  // Same selection logic as original
  unsigned long selected_release_time = 0;
  if (release_time_left != 0 && (release_time_right == 0 || release_time_left <= release_time_right)) {
    selected_release_time = release_time_left;
  } else if (release_time_right != 0) {
    selected_release_time = release_time_right;
  }

  // Debug logging (same as original)
  Serial.print("startTime: "); Serial.println(startTime);
  Serial.print("release_time_left: "); Serial.println(release_time_left);
  Serial.print("release_time_right: "); Serial.println(release_time_right);
  Serial.print("selected_release_time: "); Serial.println(selected_release_time);
  Serial.print("Reaction Time: "); Serial.println(selected_release_time - startTime);

  unsigned long elapsed = selected_release_time - startTime;
  int minutes = (elapsed / 60000) % 60;
  int seconds = (elapsed / 1000) % 60;
  int milliseconds = elapsed % 1000;

  if (elapsed < FALSE_START_THRESHOLD) {
    // FALSE START - same LCD display as original
    lcd.setCursor(0, 0);
    lcd.print("False Start");
    lcd.setCursor(0, 1);
    char timeStr[16];
    snprintf(timeStr, sizeof(timeStr), "Time: %02d:%02d.%03d", minutes, seconds, milliseconds);
    lcd.print(timeStr);
    
    // Send to app (display only)
    Blynk.virtualWrite(V1, 0);                          // No reaction time for false start
    Blynk.virtualWrite(V2, STATUS_FALSE_START);         // Status: FALSE START (will show in red)
    Blynk.logEvent("false_start", "False start detected!");
    
    // Same buzzer pattern as original
    for (int i = 0; i < 4; i++) {
      digitalWrite(buzzerPin, HIGH);
      delay(200);
      digitalWrite(buzzerPin, LOW);
      delay(200);
    }
  } else {
    // VALID START - same LCD display as original
    lcd.setCursor(0, 0);
    lcd.print("Reaction Time");
    lcd.setCursor(0, 1);
    char timeStr[16];
    snprintf(timeStr, sizeof(timeStr), "Time: %02d:%02d.%03d", minutes, seconds, milliseconds);
    lcd.print(timeStr);
    
    // Send to app (display only)
    Blynk.virtualWrite(V1, elapsed);                    // Show reaction time in ms
    Blynk.virtualWrite(V2, STATUS_VALID_START);         // Status: VALID START (will show in green)
  }
}

// SAME AS YOUR ORIGINAL + send status to app
void startTimer() {
  digitalWrite(buzzerPin, HIGH);
  buzzerStart = millis();
  buzzerOn = true;
  startTime = millis();
  running = true;
  lastUpdate = 0;
  
  // Start sending graph data to app
  sendingGraphData = true;
  graphStartTime = millis();
  Serial.println("Started graph recording");
  
  // Reset all piezo states (same as original)
  piezo_pressed_left = false;
  peak_left = 0;
  release_time_left = 0;
  piezo_press_time_left = 0;
  piezo_pressed_right = false;
  peak_right = 0;
  release_time_right = 0;
  piezo_press_time_right = 0;
  
  // Update LCD (same as original)
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Time:");
  
  // Send status to app (display only)
  Blynk.virtualWrite(V2, STATUS_RUNNING);               // Status: RUNNING
}

// SAME AS YOUR ORIGINAL + send status to app  
void resetToReady() {
  running = false;
  displayingResult = false;
  sendingGraphData = false;
  startTime = 0;
  
  // Reset all states (same as original)
  piezo_pressed_left = false;
  peak_left = 0;
  release_time_left = 0;
  piezo_press_time_left = 0;
  piezo_pressed_right = false;
  peak_right = 0;
  release_time_right = 0;
  piezo_press_time_right = 0;
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Stopwatch Ready");
  
  // Send status to app (display only)
  Blynk.virtualWrite(V1, 0);                           // Reset reaction time
  Blynk.virtualWrite(V2, STATUS_READY);                // Status: READY
  Serial.println("System reset to ready state");
}

// SAME AS YOUR ORIGINAL + send to app
void triggerFalseStartPreButton() {
  displayingResult = true;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("False Start");
  lcd.setCursor(0, 1);
  lcd.print("Time: 00:00.000");
  
  // Send to app (display only)
  Blynk.virtualWrite(V1, 0);                           // No reaction time
  Blynk.virtualWrite(V2, STATUS_FALSE_START);          // Status: FALSE START
  Blynk.logEvent("false_start", "Pre-start movement detected!");
  
  // Same buzzer pattern as original
  for (int i = 0; i < 4; i++) {
    digitalWrite(buzzerPin, HIGH);
    delay(200);
    digitalWrite(buzzerPin, LOW);
    delay(200);
  }
}

// Connection status only - NO CONTROL FUNCTIONS
BLYNK_CONNECTED() {
  Serial.println("Connected to Blynk server - Display only mode");
  // Sync current state on reconnection
  Blynk.virtualWrite(V1, 0);
  Blynk.virtualWrite(V2, STATUS_READY);
  Blynk.virtualWrite(V3, mapFSRToKg(fsrValue));
}