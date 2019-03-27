#include <Arduino.h>
#include <Wire.h>
#include <LiquidMenu.h>

#include <AD9850.h>
#include <ClickEncoder.h>
#include <TimerOne.h>
#include "Utils.h"


#define LCD_CHARS   16
#define LCD_LINES    2
//#define LCD_I2C_ADDRESS 0x3f
#define LCD_I2C_ADDRESS 0x27

#define W_CLK_PIN 8   // Pin 8 - connect to AD9850 module word load clock pin (CLK)
#define FQ_UD_PIN 9   // Pin 9 - connect to freq update pin (FQ)
#define DATA_PIN 10   // Pin 10 - connect to serial data load pin (DATA)
#define RESET_PIN 11  // Pin 11 - connect to reset pin (RST)

int32_t freqHz = 10000;//1MHz
int32_t lastFreqHz = freqHz;

double calibratedFreq = 124992500;
FrequencyIncrement currentIncrement = increments[6];


LCD_1602_RUS lcd(LCD_I2C_ADDRESS, LCD_CHARS, LCD_LINES);
ClickEncoder *encoder;



char* formattedFrequency = "";

LiquidLine welcome_line1(3, 0, "Генератор");
LiquidLine welcome_line2(0, 1, "сигнала 0-40MHz");
LiquidScreen welcome_screen(welcome_line1, welcome_line2);

LiquidLine frequency_title(0, 0, "Рабочая частота:");
LiquidLine frequency_line(0, 1, formattedFrequency);
LiquidLine frequency_suffix(14, 1, "Hz");
LiquidScreen working_screen(frequency_title, frequency_line, frequency_suffix);

LiquidLine step_title(0, 0, "Шаг частоты:");
LiquidLine step_line(0, 1, currentIncrement.description);
LiquidScreen step_screen(step_title, step_line);

LiquidMenu menu(lcd);

void timerIsr() {
    encoder->service();
}


void setup() {
    Serial.begin(9600);
    encoder = new ClickEncoder(A1, A0, A2);
    encoder->setAccelerationEnabled(true);
    //encoder->setDoubleClickEnabled(false);


    lcd.init();
    lcd.backlight();
    lcd.clear();


    menu.add_screen(welcome_screen);
    menu.add_screen(working_screen);
    menu.add_screen(step_screen);
    menu.update();


    Timer1.initialize(1000);
    Timer1.attachInterrupt(timerIsr);


    DDS.begin(W_CLK_PIN, FQ_UD_PIN, DATA_PIN, RESET_PIN);
    DDS.calibrate(calibratedFreq);
    DDS.setfreq(freqHz, 0);

    formatFrequency(formattedFrequency, freqHz);
    menu.next_screen();
}


void loop() {
    if (menu.get_currentScreen() == &working_screen) {
        freqHz += encoder->getValue() * currentIncrement.step;
        if (freqHz != lastFreqHz) {
            lastFreqHz = freqHz;
            DDS.setfreq(freqHz, 0);
            formatFrequency(formattedFrequency, freqHz);
            menu.update();
        }
    } else if (menu.get_currentScreen() == &step_screen) {
        int nextIndex = (currentIncrement.num + encoder->getValue()) % INCREMENTS_SIZE;
        if (nextIndex < 0)
            nextIndex = increments[INCREMENTS_SIZE - 1].num;

        if (nextIndex != currentIncrement.num) {
            currentIncrement = increments[nextIndex];
            Serial.println(currentIncrement.description);
            menu.update();
        }
    }

    ClickEncoder::Button b = encoder->getButton();
    if (b != ClickEncoder::Open) {
        Serial.print("Button: ");
        switch (b) {
            case ClickEncoder::Pressed:
                Serial.println("ClickEncoder::Pressed");
                break;
            case ClickEncoder::Held:
                Serial.println("ClickEncoder::Held");
                break;
            case ClickEncoder::Released:
                Serial.println("ClickEncoder::Released");
                break;
            case ClickEncoder::Clicked:
                Serial.println("ClickEncoder::Clicked");
                if (menu.get_currentScreen() == &working_screen) {
                    menu.change_screen(step_screen);
                    encoder->setAccelerationEnabled(false);
                    Serial.println("in Step Screen");
                } else {
                    menu.previous_screen();
                    Serial.println("in working screen");
                    encoder->setAccelerationEnabled(true);
                }
                menu.update();
                break;
        }
    }

    //debounce
    while(encoder->getValue() != 0){}
}
