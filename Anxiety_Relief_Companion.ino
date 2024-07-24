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

int currentQuestion = 0;
int selectedOption = 0;
int score = 0;

// Debouncing variables
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 200; // Milliseconds for consistent debouncing

// Button states
bool lastButtonStateNext = HIGH;
bool lastButtonStatePrev = HIGH;
bool lastButtonStateOK = HIGH;

// BAI questions
const char* questions[21] = {
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

int responses[21] = {0};  // To store user responses

void setup() {
  Serial.begin(115200);

  // Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.display();
  delay(2000);  // Pause for 2 seconds

  // Initialize button pins
  pinMode(buttonNextPin, INPUT_PULLUP);
  pinMode(buttonPrevPin, INPUT_PULLUP);
  pinMode(buttonOKPin, INPUT_PULLUP);

  showQuestion();
}

void loop() {
  unsigned long currentTime = millis();

  // Read button states
  int readingNext = digitalRead(buttonNextPin);
  int readingPrev = digitalRead(buttonPrevPin);
  int readingOK = digitalRead(buttonOKPin);

  // Handle Next button
  if (readingNext == LOW && (currentTime - lastDebounceTime) > debounceDelay) {
    if (lastButtonStateNext == HIGH) { // Button pressed
      selectedOption = (selectedOption + 1) % 4;
      showQuestion();
    }
    lastDebounceTime = currentTime;
    lastButtonStateNext = LOW;
  } else if (readingNext == HIGH) {
    lastButtonStateNext = HIGH;
  }

  // Handle Previous button
  if (readingPrev == LOW && (currentTime - lastDebounceTime) > debounceDelay) {
    if (lastButtonStatePrev == HIGH) { // Button pressed
      selectedOption = (selectedOption - 1 + 4) % 4;
      showQuestion();
    }
    lastDebounceTime = currentTime;
    lastButtonStatePrev = LOW;
  } else if (readingPrev == HIGH) {
    lastButtonStatePrev = HIGH;
  }

  // Handle OK button
  if (readingOK == LOW && (currentTime - lastDebounceTime) > debounceDelay) {
    if (lastButtonStateOK == HIGH) { // Button pressed
      if (currentQuestion < 21) {
        responses[currentQuestion] = selectedOption;
        displayAnswerRecorded(); // Show confirmation message
        delay(2000); // Show message for 2 seconds
        selectedOption = 0;
        currentQuestion = (currentQuestion + 1) % 21;
        if (currentQuestion == 0) {
          showScore();
        } else {
          showQuestion();
        }
      } else {
        showEndMenu(); // Show restart/exit menu
      }
    }
    lastDebounceTime = currentTime;
    lastButtonStateOK = LOW;
  } else if (readingOK == HIGH) {
    lastButtonStateOK = HIGH;
  }
}

void showQuestion() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(0, 0);
  display.print("Q");
  display.print(currentQuestion + 1);
  display.print(": ");
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
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(0, 0);
  display.print("BAI Completed!");
  display.setCursor(0, 10);
  display.print("Total Score: ");
  display.print(score);
  
  display.display();
}

void displayAnswerRecorded() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(0, 0);
  display.print("Answer Recorded!");
  
  display.display();
}

void showEndMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(0, 0);
  display.print("Test Completed!");
  display.setCursor(0, 10);
  display.print("1) Restart");
  display.setCursor(0, 20);
  display.print("2) Exit");

  display.display();
  handleEndMenu();
}

void handleEndMenu() {
  unsigned long currentTime = millis();
  bool restartSelected = false;
  bool exitSelected = false;

  while (!restartSelected && !exitSelected) {
    unsigned long now = millis();

    int readingNext = digitalRead(buttonNextPin);
    int readingPrev = digitalRead(buttonPrevPin);
    int readingOK = digitalRead(buttonOKPin);

    // Handle navigation in end menu
    if (readingNext == LOW && (now - lastDebounceTime) > debounceDelay) {
      if (lastButtonStateNext == HIGH) {
        // Switch between Restart and Exit options
        if (display.getCursorY() == 10) {
          display.setCursor(0, 20);
        } else {
          display.setCursor(0, 10);
        }
        display.display();
      }
      lastDebounceTime = now;
      lastButtonStateNext = LOW;
    } else if (readingNext == HIGH) {
      lastButtonStateNext = HIGH;
    }

    if (readingPrev == LOW && (now - lastDebounceTime) > debounceDelay) {
      if (lastButtonStatePrev == HIGH) {
        // Switch between Restart and Exit options
        if (display.getCursorY() == 20) {
          display.setCursor(0, 10);
        } else {
          display.setCursor(0, 20);
        }
        display.display();
      }
      lastDebounceTime = now;
      lastButtonStatePrev = LOW;
    } else if (readingPrev == HIGH) {
      lastButtonStatePrev = HIGH;
    }

    if (readingOK == LOW && (now - lastDebounceTime) > debounceDelay) {
      if (lastButtonStateOK == HIGH) {
        if (display.getCursorY() == 10) {
          // Restart test
          currentQuestion = 0;
          score = 0;
          for (int i = 0; i < 21; i++) {
            responses[i] = 0;
          }
          showQuestion();
        } else if (display.getCursorY() == 20) {
          // Exit test
          while (true); // Stop further execution
        }
        restartSelected = true;
        exitSelected = true;
      }
      lastDebounceTime = now;
      lastButtonStateOK = LOW;
    } else if (readingOK == HIGH) {
      lastButtonStateOK = HIGH;
    }
  }
}
