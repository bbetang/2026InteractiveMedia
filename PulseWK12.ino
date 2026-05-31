#include <PulseSensorPlayground.h>
#define NUM_LEDS 5
int heart[NUM_LEDS] = {8, 9, 10, 11, 12};
int userBPM[NUM_LEDS] = {0};           // Stored BPM for each LED
unsigned long lastFlashTime[NUM_LEDS] = {0}; // Last flash time per LED
bool ledState[NUM_LEDS] = {false};     // Current state of each LED
unsigned long lastSignalTime = 0;      // Last time ANY signal detected
int currentUserSlot = -1;              // Which slot is currently being assigned
int nextUserSlot = -1;                 // Which slot will be assigned next
int lastAssignedSlot = -1;             // Track the last slot assigned to a user
unsigned long slotAssignmentTime = 0;  // When current slot was assigned
#define userGapTime 3000       // Remove user if no signal for 3 seconds
#define beatDebounceTime 100   // Ignore beats within 100ms of last beat
const int OUTPUT_TYPE = SERIAL_PLOTTER;
const int PULSE_INPUT = A0;
const int PULSE_BLINK = 13;
const int PULSE_FADE = 5;
const int THRESHOLD = 550;
PulseSensorPlayground pulseSensor;
unsigned long lastBeatTime = 0;  // Debounce for beat detection
void setup() {
Serial.begin(115200);
for (int i = 0; i < NUM_LEDS; i++) {
pinMode(heart[i], OUTPUT);
digitalWrite(heart[i], LOW);
    userBPM[i] = 0;
    lastFlashTime[i] = 0;
    ledState[i] = false;
  }
Serial.println("System ready. User 1, place finger on sensor...");
pulseSensor.analogInput(PULSE_INPUT);
pulseSensor.blinkOnPulse(PULSE_BLINK);
pulseSensor.fadeOnPulse(PULSE_FADE);
pulseSensor.setSerial(Serial);
pulseSensor.setOutputType(OUTPUT_TYPE);
pulseSensor.setThreshold(THRESHOLD);
if (!pulseSensor.begin()) {
for (;;) {
digitalWrite(PULSE_BLINK, LOW);
delay(50);
digitalWrite(PULSE_BLINK, HIGH);
delay(50);
    }
  }
  // Set the initial next slot indicator
updateNextSlotIndicator();
}
void loop() {
if (pulseSensor.UsingHardwareTimer) {
delay(20);
    pulseSensor.outputSample();
  } else {
if (pulseSensor.sawNewSample()) {
if ((pulseSensor.samplesUntilReport--) == 0) {
        pulseSensor.samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;
        pulseSensor.outputSample();
      }
    }
  }
  // DETECT NEW HEARTBEAT FROM SENSOR (with debouncing)
if (pulseSensor.sawStartOfBeat()) {
unsigned long currentTime = millis();
    // Debounce: ignore beats within 100ms of last beat
if (currentTime - lastBeatTime < beatDebounceTime) {
return; // Ignore this beat, it's noise
    }
    lastBeatTime = currentTime;
    pulseSensor.outputBeat();
    // If no user currently being read, assign to next available slot
if (currentUserSlot == -1) {
      currentUserSlot = getNextAvailableSlot();
      lastAssignedSlot = currentUserSlot;
      slotAssignmentTime = currentTime;
      Serial.print(">>> User assigned to LED slot ");
      Serial.print(currentUserSlot);
      Serial.println(" <<<");
      // Update the indicator to the NEXT slot immediately
      updateNextSlotIndicator();
    }
    // Update the BPM for the current user
int newBPM = pulseSensor.getBeatsPerMinute();
    // Only update BPM if it's in a reasonable range (40-200 BPM)
if (newBPM >= 40 && newBPM <= 200) {
      userBPM[currentUserSlot] = newBPM;
      lastSignalTime = currentTime;
      // Flash the assigned LED immediately
digitalWrite(heart[currentUserSlot], HIGH);
      lastFlashTime[currentUserSlot] = currentTime;
      ledState[currentUserSlot] = true;
      // Serial.print("LED ");
      // Serial.print(currentUserSlot);
      // Serial.print(" BPM: ");
      //Serial.println(userBPM[currentUserSlot]);
    }
  }
  // Turn off the flash pulse (short duration)
if (pulseSensor.isInsideBeat() == false && currentUserSlot != -1) {
if (ledState[currentUserSlot]) {
digitalWrite(heart[currentUserSlot], LOW);
      ledState[currentUserSlot] = false;
    }
  }
  // Update all LEDs with their stored BPM rhythm
updateAllLEDs();
  // Check if current user has removed their finger
checkUserTimeout();
}
void updateAllLEDs() {
unsigned long currentTime = millis();
for (int i = 0; i < NUM_LEDS; i++) {
if (userBPM[i] > 0) {
      // Calculate the interval between beats (in milliseconds)
unsigned long beatInterval = 60000 / userBPM[i];
      // Flash duration is 20% of the beat interval
unsigned long flashDuration = beatInterval / 5;
      // Check if it's time to flash
if (i == nextUserSlot) {
      // Next slot indicator: keep it OFF to show user where pulse will appear
digitalWrite(heart[i], LOW);
    }
else if (currentTime - lastFlashTime[i] >= beatInterval) {
        // Time for next beat
digitalWrite(heart[i], HIGH);
        lastFlashTime[i] = currentTime;
      } 
else if (currentTime - lastFlashTime[i] >= flashDuration) {
        // Flash duration has passed, turn off
digitalWrite(heart[i], LOW);
      }
    }
  }
}
int getNextAvailableSlot() {
  // Find the first empty slot
  for (int i = 0; i < NUM_LEDS; i++) {
    if (userBPM[i] == 0) {
      return i;
    }
  }
  // If all slots full, overwrite the one after the last assigned one
  int next = (lastAssignedSlot + 1) % NUM_LEDS;
  Serial.print("All slots full. Overwriting slot ");
  Serial.println(next);
  return next;
}
void updateNextSlotIndicator() {
  // Calculate which slot will be assigned next
  nextUserSlot = getNextAvailableSlot();
Serial.print("Next available slot: ");
Serial.println(nextUserSlot);
}
void checkUserTimeout() {
unsigned long currentTime = millis();
  // If current user hasn't sent a signal in 3 seconds, they've removed their finger
if (currentUserSlot != -1) {
if (currentTime - lastSignalTime > userGapTime) {
      // Serial.print("User removed finger from slot ");
      // Serial.print(currentUserSlot);
      // Serial.print(". BPM ");
      // Serial.print(userBPM[currentUserSlot]);
      // Serial.println(" saved to LED.");
      currentUserSlot = -1; // Release the slot
      // Update the next slot indicator
updateNextSlotIndicator();
      // Find next empty slot for next user
int nextSlot = getNextAvailableSlot();
if (nextSlot == 0 && userBPM[0] != 0) {
        // All slots full
        Serial.println("All LED slots full. Waiting for next user...");
      } else {
        // Serial.print("User ");
        // Serial.print(nextSlot + 1);
        // Serial.print(", place finger on sensor (LED ");
        // Serial.print(nextSlot);
        // Serial.println(" is your indicator)...");
      }
    }
  }
}
