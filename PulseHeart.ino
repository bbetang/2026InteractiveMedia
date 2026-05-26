#define NUM_LEDS 5

int PulseSensorPin = 0;        // Single sensor
int heart[NUM_LEDS] = {8, 9, 10, 11, 12};

// FIFO queue
int userPulse[NUM_LEDS];       // Pulse value for each user slot
int threshold = 580;

int nextSlot = 0;              // FIFO pointer (0–4)
int userCounter = 1;           // User ID counter

unsigned long lastSignalTime = 0;
#define userGapTime 3000       // Remove user if no signal for 3 seconds
#define cooldown 500           // Wait 500ms before accepting new user

void setup() {
  for (int i = 0; i < NUM_LEDS; i++) {
    pinMode(heart[i], OUTPUT);
    userPulse[i] = 0;
  }
  
  Serial.begin(115200);
  Serial.println("System ready. Place finger on sensor...");
}

void loop() {
  int signal = analogRead(PulseSensorPin);
  
  Serial.println("Signal: " + String(signal));
  
  // If pulse detected
  if (signal > threshold) {
    unsigned long now = millis();
    
    // If enough time has passed since last signal, treat as NEW heart
    if (now - lastSignalTime > cooldown) {
      assignNewUser(signal);
    } else {
      // Same user: update their pulse
      updateCurrentUserPulse(signal);
    }
    
    lastSignalTime = now;
  }
  
  // Check for timeout (user removed finger)
  checkUserTimeout();
  
  // Update all LEDs
  updateLEDs();
  
  delay(20);
}

// Assign a new user to the next heart
void assignNewUser(int pulseValue) {
  userPulse[nextSlot] = pulseValue;
  
  Serial.print("User #");
  Serial.print(userCounter);
  Serial.print(" assigned to heart ");
  Serial.println(nextSlot + 1);
  
  userCounter++;
  nextSlot = (nextSlot + 1) % NUM_LEDS;  // Wrap around
}

// Update the current pulse value
void updateCurrentUserPulse(int pulseValue) {
  int currentSlot = (nextSlot - 1 + NUM_LEDS) % NUM_LEDS;  // Last assigned slot
  userPulse[currentSlot] = pulseValue;
}

// Clear slot if no signal for too long
void checkUserTimeout() {
  unsigned long now = millis();
  
  if (lastSignalTime > 0 && now - lastSignalTime > userGapTime) {
    Serial.println("User removed finger Slots retained.");
    lastSignalTime = 0;
  }
}

// Update all hearts based on user values
void updateLEDs() {
  for (int i = 0; i < NUM_LEDS; i++) {
    if (userPulse[i] > 0) {
      if (userPulse[i] > threshold) {
        digitalWrite(heart[i], HIGH);
      } else {
        digitalWrite(heart[i], LOW);
      }
    } else {
      digitalWrite(heart[i], LOW);
    }
  }
}
