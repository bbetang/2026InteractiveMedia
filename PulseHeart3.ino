#include <PulseSensorPlayground.h>

#define NUM_LEDS 5

const int PULSE_INPUT = A0;
const int THRESHOLD = 550;

int heart[NUM_LEDS] = {8, 9, 10, 11, 12};

int userBPM[NUM_LEDS];           // Store BPM for each user
int nextSlot = 0;
int userCounter = 1;

unsigned long lastSignalTime = 0;
#define userGapTime 3000
#define cooldown 500

PulseSensorPlayground pulseSensor;

void setup() {
  for (int i = 0; i < NUM_LEDS; i++) {
    pinMode(heart[i], OUTPUT);
    userBPM[i] = 0;
  }
  
  Serial.begin(115200);
  
  // Configure PulseSensor
  pulseSensor.analogInput(PULSE_INPUT);
  pulseSensor.setSerial(Serial);
  pulseSensor.setThreshold(THRESHOLD);
  
  if (!pulseSensor.begin()) {
    Serial.println("PulseSensor initialization failed!");
    for (;;) {
      digitalWrite(LED_BUILTIN, LOW);
      delay(50);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(50);
    }
  }
  
  Serial.println("System ready. Place finger on sensor...");
}

void loop() {
  if (pulseSensor.sawNewSample()) {
    /*
       Check if a heartbeat was detected.
       sawStartOfBeat() returns true when a new beat is detected.
    */
    if (pulseSensor.sawStartOfBeat()) {
      int bpm = pulseSensor.getBeatsPerMinute();
      
      Serial.print("BPM: ");
      Serial.println(bpm);
      
      unsigned long now = millis();
      
      // If enough time has passed, treat as NEW user
      if (now - lastSignalTime > cooldown) {
        assignNewUser(bpm);
      } else {
        // Same user: update their BPM
        updateCurrentUserPulse(bpm);
      }
      
      lastSignalTime = now;
    }
  }
  
  checkUserTimeout();
  updateLEDs();
}

void assignNewUser(int bpm) {
  userBPM[nextSlot] = bpm;
  
  Serial.print("User #");
  Serial.print(userCounter);
  Serial.print(" assigned to heart ");
  Serial.print(nextSlot + 1);
  Serial.print(" with BPM: ");
  Serial.println(bpm);
  
  userCounter++;
  nextSlot = (nextSlot + 1) % NUM_LEDS;
}

void updateCurrentUserPulse(int bpm) {
  int currentSlot = (nextSlot - 1 + NUM_LEDS) % NUM_LEDS;
  userBPM[currentSlot] = bpm;
  
  Serial.print("Updated heart ");
  Serial.print(currentSlot + 1);
  Serial.print(" BPM: ");
  Serial.println(bpm);
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
    if (userBPM[i] > 0) {
      // LED on when user has a stored BPM
      digitalWrite(heart[i], HIGH);
    } else {
      // LED off when slot is empty
      digitalWrite(heart[i], LOW);
    }
  }
}
