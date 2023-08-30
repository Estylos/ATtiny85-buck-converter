/*
 * Copyright (c) 2023 Estylos, caneuze
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * Generates a 50khZ PWM on motorPin, with the duty cycle set by a potentiometer on potPin.
 */

#include <Arduino.h>
#include <SoftwareSerial.h>

#define MAX_VOLTAGE_OUT      12
#define MAX_ANALOG_VOLTAGE  (1023 * (MAX_VOLTAGE_OUT + 1) / 29) // Voltage divider dimensioned for 29V. We get the maximum ADC value for our maximum voltage (+1V to guarantee it).

#define PIN_PWM             1
#define PIN_POT             A1
#define PIN_FEEBACK         A2

const float coefKp = 0.4; // Coef for the P controller
float dutyCycle = 0;

SoftwareSerial SerialT(3, 0); // UART, pin 0 for TX for debugging

void setup()
{
    pinMode(PIN_PWM, OUTPUT);
    pinMode(PIN_POT, INPUT);
    pinMode(PIN_FEEBACK, INPUT);

    // Manipulating timer 0 registers to generate PWM
    TCCR0B = 0x08;                // 0000 1000 : PWM mode
    TCCR0A = 0x21;                // 0010 0001 : return to 0 after overflow
    OCR0A = 80;                   // PWM à 50 KHz
    OCR0B = (byte)(OCR0A * 0.25); // Start-up duty cycle
    TCNT0 = 0x0;
    TCCR0B |= 1; // Activate timer 0

    SerialT.begin(9600);
}

void loop()
{
    int potValueCalibrated = map(analogRead(PIN_POT), 0, 1023, 0, MAX_ANALOG_VOLTAGE); // Potentiometer value relative to max output voltage
    int feedbackValue = analogRead(PIN_FEEBACK);
    int error = potValueCalibrated - feedbackValue;

    dutyCycle += coefKp * error; // P controller
    dutyCycle = constrain(dutyCycle, 0, MAX_ANALOG_VOLTAGE);

    OCR0B = (byte)(OCR0A * dutyCycle / MAX_ANALOG_VOLTAGE); // Changing the duty cycle

    // Debug
    SerialT.print(potValueCalibrated);
    SerialT.print("    	");
    SerialT.print(feedbackValue);
    SerialT.print("    	");
    SerialT.println(dutyCycle / MAX_ANALOG_VOLTAGE);

    delay(10); // We put our system on pause (to reduce power consumption and avoid unnecessary PWM variation).
}
