#include <PulseSensorPlayground.h>

#define NUM_LEDS 5

const int PulseSensorPin = A0;
int heart[NUM_LEDS] = {8, 9, 10, 11, 12};

int userPulse[NUM_LEDS];
int nextSlot = 0;
int userCounter = 1;

unsigned long lastSignalTime = 0;
#define userGapTime 3000
#define cooldown 500

PulseSensorPlayground pulseSensor;

void setup() {
  for (int i = 0; i < NUM_LEDS; i++) {
    pinMode(heart[i], OUTPUT);
    userPulse[i] = 0;
  }
  
  pulseSensor.analogInput(PulseSensorPin);
  pulseSensor.begin();
  
  Serial.begin(115200);
  Serial.println("System ready. Place finger on sensor...");
}

void loop() {
  if (pulseSensor.sawNewSample()) {
    int signal = pulseSensor.getLatestSample();
    
    Serial.println("Signal: " + String(signal));
    
    if (pulseSensor.isPulse()) {  // Detects actual heartbeat
      unsigned long now = millis();
      
      if (now - lastSignalTime > cooldown) {
        assignNewUser(pulseSensor.getBeatsPerMinute());
      } else {
        updateCurrentUserPulse(pulseSensor.getBeatsPerMinute());
      }
      
      lastSignalTime = now;
    }
  }
  
  checkUserTimeout();
  updateLEDs();
}

void assignNewUser(int pulseValue) {
  userPulse[nextSlot] = pulseValue;
  
  Serial.print("User #");
  Serial.print(userCounter);
  Serial.print(" assigned to heart ");
  Serial.println(nextSlot + 1);
  
  userCounter++;
  nextSlot = (nextSlot + 1) % NUM_LEDS;
}

void updateCurrentUserPulse(int pulseValue) {
  int currentSlot = (nextSlot - 1 + NUM_LEDS) % NUM_LEDS;
  userPulse[currentSlot] = pulseValue;
}

void checkUserTimeout() {
  unsigned long now = millis();
  
  if (lastSignalTime > 0 && now - lastSignalTime > userGapTime) {
    Serial.println("User removed finger. Slots retained.");
    lastSignalTime = 0;
  }
}

void updateLEDs() {
  for (int i = 0; i < NUM_LEDS; i++) {
    if (userPulse[i] > 0) {
      // Pulse LED based on BPM
      digitalWrite(heart[i], pulseSensor.getBeatsPerMinute() > 0 ? HIGH : LOW);
    } else {
      digitalWrite(heart[i], LOW);
    }
  }
}
