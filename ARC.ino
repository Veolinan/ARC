#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Button pins
const int buttonNextPin = 14;   // Pin for the "Next" button (D5)
const int buttonPrevPin = 4;    // Pin for the "Previous" button (D2)
const int buttonOKPin = 2;      // Pin for the "OK" button (D4)
const int buzzerPin = 17;       // Pin for the buzzer

int currentQuestion = 0;
int selectedOption = 0;
int score = 0;
bool restartTest = false; // Flag to indicate if the test should be restarted

// Debouncing variables
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 200; // Milliseconds for consistent debouncing

// Button states
bool lastButtonStateNext = HIGH;
bool lastButtonStatePrev = HIGH;
bool lastButtonStateOK = HIGH;

// BAI questions
const char* questions[22] = {
  "Are you feeling alright?",  // Decoy question
  "Numbness or tingling",
  "Feeling hot",
  "Wobbliness in legs",
  "Unable to relax",
  "Fear of worst happening",
  "Dizzy or lightheaded",
  "Heart pounding/racing",
  "Unsteady",
  "Terrified or afraid",
  "Nervous",
  "Feeling of choking",
  "Hands trembling",
  "Shaky / unsteady",
  "Fear of losing control",
  "Difficulty breathing",
  "Fear of dying",
  "Scared",
  "Indigestion",
  "Faint / lightheaded",
  "Face flushed",
  "Sweating (not due to heat)"
};

// Options for BAI questions
const char* options[4] = {"0) Not at all", "1) Mildly", "2) Moderately", "3) Severely"};

// Responses array to store user's answers
int responses[21] = {0}; // Only storing answers for the actual BAI questions

void setup() {
  Serial.begin(115200);

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.display();
  delay(2000);  // Pause for 2 seconds

  // Initialize button pins
  pinMode(buttonNextPin, INPUT_PULLUP);
  pinMode(buttonPrevPin, INPUT_PULLUP);
  pinMode(buttonOKPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);

  // Start the BAI questions directly
  currentQuestion = 0; // Ensure starting at the first question
  showQuestion();
}

void loop() {
  unsigned long currentTime = millis();

  // Read button states
  int readingNext = digitalRead(buttonNextPin);
  int readingPrev = digitalRead(buttonPrevPin);
  int readingOK = digitalRead(buttonOKPin);

  // Handle Next button for selecting options
  if (readingNext == LOW && (currentTime - lastDebounceTime) > debounceDelay) {
    if (lastButtonStateNext == HIGH) {
      selectedOption = (selectedOption + 1) % 4; // Cycle through options
      showQuestion(); // Update the display with the new selection
    }
    lastDebounceTime = currentTime;
    lastButtonStateNext = LOW;
  } else if (readingNext == HIGH) {
    lastButtonStateNext = HIGH;
  }

  // Handle Previous button for selecting options
  if (readingPrev == LOW && (currentTime - lastDebounceTime) > debounceDelay) {
    if (lastButtonStatePrev == HIGH) {
      selectedOption = (selectedOption - 1 + 4) % 4; // Cycle through options backwards
      showQuestion(); // Update the display with the new selection
    }
    lastDebounceTime = currentTime;
    lastButtonStatePrev = LOW;
  } else if (readingPrev == HIGH) {
    lastButtonStatePrev = HIGH;
  }

  // Handle OK button
  if (readingOK == LOW && (currentTime - lastDebounceTime) > debounceDelay) {
    if (lastButtonStateOK == HIGH) {
      if (selectedOption >= 0 && selectedOption < 4) {
        if (currentQuestion == 0) {
          // Skip the decoy question
          currentQuestion++;
          showQuestion();
        } else if (currentQuestion < 22) {
          responses[currentQuestion - 1] = selectedOption; // Record the answer for actual questions
          currentQuestion++; // Move to the next question

          if (currentQuestion < 22) {
            showQuestion(); // Show the next question
          } else {
            showScore(); // Show final score after all questions
          }
        } else if (restartTest) {
          restartTest = false;
          currentQuestion = 0; // Restart the test
          for (int i = 0; i < 21; i++) {
            responses[i] = 0; // Reset responses
          }
          showQuestion(); // Start from the first question
        }
      }
      lastDebounceTime = currentTime;
      lastButtonStateOK = LOW;
    }
  } else if (readingOK == HIGH) {
    lastButtonStateOK = HIGH;
  }
}

void showQuestion() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  if (currentQuestion == 0) {
    display.print("Decoy Question:");
  } else {
    display.print("Q");
    display.print(currentQuestion); // Display question number starting from 1
    display.print(": ");
  }
  display.print(questions[currentQuestion]);

  for (int i = 0; i < 4; i++) {
    display.setCursor(0, 20 + i * 10);
    if (i == selectedOption) {
      display.print("> ");
    } else {
      display.print("  ");
    }
    display.print(options[i]);
  }

  display.display();
}

void showScore() {
  score = 0;
  for (int i = 0; i < 21; i++) {
    score += responses[i];
  }

  // Output total score and classification on serial monitor
  Serial.print("Total Score: ");
  Serial.println(score);
  if (score >= 0 && score <= 7) {
    Serial.println("Classification: Minimal");
  } else if (score >= 8 && score <= 15) {
    Serial.println("Classification: Mild");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("Press OK to proceed");
    display.display();
    waitForOKButton();
    diaphragmaticBreathing();
    return;
  } else if (score >= 16 && score <= 25) {
    Serial.println("Classification: Moderate");
  } else if (score >= 26) {
    Serial.println("Classification: Severe");
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("BAI Completed!");
  display.setCursor(0, 10);
  display.print("Total Score: ");
  display.print(score);
  display.setCursor(0, 30);
  display.print("Press OK to restart");

  display.display();
  restartTest = true; // Set flag to restart the test
  waitForOKButton();
}

void diaphragmaticBreathing() {
  // Step 1: Display initial message
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Sit or lie down comfortably.");
  display.setCursor(0, 20);
  display.print("Once ready press Next");
  display.display();

  // Wait for user to press Next
  waitForNextButton();

  // Step 2: Display hand placement message
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Place one hand on your chest");
  display.setCursor(0, 20);
  display.print("and the other on your abdomen");
  display.display();
  delay(2500);

  // Step 3: Display inhalation message
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Inhale deeply through your nose");
  display.setCursor(0, 20);
  display.print("until you hear a sound.");
  display.setCursor(0, 40);
  display.print("Make sure abdomen rises");
  display.setCursor(0, 60);
  display.print("more than your chest");
  display.display();
  delay(4000);

  // Step 4: Start buzzer and display exhalation message
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Exhale slowly through your mouth");
  display.setCursor(0, 20);
  display.print("until sound stops");
  display.display();
  tone(buzzerPin, 2000); // Start buzzer with softer tone
  delay(3000); // Sound buzzer for 3 seconds
  noTone(buzzerPin); // Stop buzzer

  // Step 5: Restart test
  restartTest = false;
  currentQuestion = 0;
  for (int i = 0; i < 21; i++) {
    responses[i] = 0;
  }
  showQuestion();
}

void waitForNextButton() {
  while (digitalRead(buttonNextPin) == HIGH) {
    delay(10); // Wait for the Next button to be pressed
  }
  while (digitalRead(buttonNextPin) == LOW) {
    delay(10); // Debounce delay
  }
}

void waitForOKButton() {
  while (digitalRead(buttonOKPin) == HIGH) {
    delay(10); // Wait for the OK button to be pressed
  }
  while (digitalRead(buttonOKPin) == LOW) {
    delay(10); // Debounce delay
  }
}
