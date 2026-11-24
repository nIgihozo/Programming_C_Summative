#include <LiquidCrystal.h>
#include <stdlib.h>

//  Initializing LCD Pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Pin's Definitions
const int ENTRY_BTN = 6;
const int EXIT_BTN = 7;
const int GREEN_LED = 8;
const int RED_LED = 9;

// Structure for  Data Occupancy
struct Facility {
    int current;
    int maxCapacity;
};

// Pointer for dynamically allocated struct
struct Facility *room = NULL;

// Timers for millis 
unsigned long lastDebounceTime = 0;
unsigned long lastLcdUpdated = 0;

// 200ms Debounce button
const unsigned long debounceDelay = 200;

// Refreshing LCD every 500ms
const unsigned long lcdInterval = 500;

// Function Prototypes
void handleEntry(struct Facility *f);
void handleExit(struct Facility *f);
void updateLEDs(struct Facility *f);
void updateLCD(struct Facility *f);
void printStatus(struct Facility *f, const char *event);

// Setup
void setup() {
    Serial.begin(9600);
    lcd.begin(16, 2);

    // Pin Modes
    pinMode(ENTRY_BTN, INPUT_PULLUP);
    pinMode(EXIT_BTN, INPUT_PULLUP);
    pinMode(GREEN_LED, OUTPUT);
    pinMode(RED_LED, OUTPUT);

    // Dynamic Memory Allocation
    room = (struct Facility *)malloc(sizeof(struct Facility));

    // Validating Memory Allocation
    if (room == NULL) {
        Serial.println("Error: Memory Allocation Failed!");
        while(1); // This is for stopping program if there is out of memory.
    }

    // Initializing Values
    room->current = 0;
    room->maxCapacity = 10;

    updateLEDs(room);
    updateLCD(room);
    printStatus(room, "System Has Been Started");
}

    void loop() {
        unsigned long now = millis();

        // Checking Buttons Using Debounce
        if (now - lastDebounceTime > debounceDelay) {
            if (digitalRead(ENTRY_BTN) == LOW) {
                handleEntry(room);
                lastDebounceTime = now;
            }
            else if (digitalRead(EXIT_BTN) == LOW) {
                handleExit(room);
                lastDebounceTime = now;
            }
        }

        // Updating LCD Periodically using millis()
        if (now - lastLcdUpdated >= lcdInterval) {
            updateLCD(room);
            lastLcdUpdated = now;
        }
    }


    // Person Entering Handles
    void handleEntry(struct Facility *f) {
        if (f->current < f->maxCapacity) {
            f->current++;
            updateLEDs(f);
            printStatus(f, "Entry OK");
        } else {
            Serial.println("ERROR: Facility is FULL! Entry rejected.");
        }
    }

    // Handle Person Leaving
    void handleExit(struct Facility *f) {
        if (f->current > 0) {
            f->current--;
            updateLEDs(f);
            printStatus(f, "Ok! Exited");
        } else {
            Serial.println("ERROR: Facility is Empty. Exit Rejected.");
        }
    }

    // Controlling GREEN & RED LEDs
    void updateLEDs(struct Facility *f) {
        if (f->current < f->maxCapacity) {
            digitalWrite(GREEN_LED, HIGH);
            digitalWrite(RED_LED, LOW);
        } else {
            digitalWrite(GREEN_LED, LOW);
            digitalWrite(RED_LED, HIGH);
        }
    }

    // Updating LCD Screen
    void updateLCD(struct Facility *f) {
        int available = f->maxCapacity - f->current;

        lcd.setCursor(0, 0);
        lcd.print("Inside: ");
        lcd.print(f->current);
        lcd.print("   "); // This is for clearing extra digits

        lcd.setCursor(0, 1);
        lcd.print("Avail: ");
        lcd.print(available);
        lcd.print("   ");
    }

    // Printing Details to Serial Monitor
    void printStatus(struct Facility *f, const char *event) {
        int available = f->maxCapacity - f->current;
        Serial.print("[");
        Serial.print(event);
        Serial.print("] Inside: ");
        Serial.print(f->current);
        Serial.print(" | Available: ");
        Serial.println(available);
       }

       // Clean Up Function to Satisfy free()
       void cleanup() {
        if (room != NULL) {
            free(room);
            room = NULL;
        }
       }
