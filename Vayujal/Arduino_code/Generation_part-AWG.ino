#include <Wire.h>

// Sensor Pins
const int trigPin = 6;
const int echoPin = 5;
const int flowSensorPin = 2;
const int relayPin = 7;
const int voltageSensorPin = A0;

// Water Flow Variables
volatile int pulseCount = 0;
float flowRate = 0.0;
unsigned long lastTime = 0;
const float calibrationFactor = 7.5;

// SHT25 I2C Address
#define SHT25_ADDR 0x40  

void setup() {
    Serial.begin(9600);
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    pinMode(flowSensorPin, INPUT);
    pinMode(relayPin, OUTPUT);
    
    attachInterrupt(digitalPinToInterrupt(flowSensorPin), countPulses, RISING);
    
    Wire.begin();  
    Serial.println("System Initialized!");

    digitalWrite(relayPin, LOW);
}

void loop() {
    Serial.println("Loop Running...");

    float distance = readDistance();
    float voltage = readVoltage();
    float temperature = readTemperature();
    float humidity = readHumidity();

    if (millis() - lastTime > 1000) {
        flowRate = (pulseCount / calibrationFactor);
        pulseCount = 0;
        lastTime = millis();
    }

    Serial.print("Distance: "); Serial.print(distance); Serial.println(" cm");
    Serial.print("Water Flow: "); Serial.print(flowRate); Serial.println(" L/min");
    Serial.print("Temperature: "); Serial.print(temperature); Serial.println(" °C");
    Serial.print("Humidity: "); Serial.print(humidity); Serial.println(" %");
    Serial.print("Voltage: "); Serial.print(voltage); Serial.println(" V");

    checkConditions();

    Serial.println("------------------------");
    delay(1000);
}

// Interrupt for Water Flow Sensor
void countPulses() {
    pulseCount++;
}

// **Ultrasonic Sensor**
float readDistance() {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH, 50000);  // Increased timeout
    if (duration == 0) {
        Serial.println("Error: No echo detected!");
        return -1;
    }
    return (duration * 0.034 / 2);
}

// **Humidity Calculation**
float readHumidity() {
    Wire.beginTransmission(SHT25_ADDR);
    Wire.write(0xE5);
    Wire.endTransmission(false);

    Wire.requestFrom(SHT25_ADDR, 2);
    if (Wire.available() < 2) {
        Serial.println("Error: SHT25 No Data (Humidity)");
        return -99;
    }

    uint16_t humRaw = (Wire.read() << 8) | (Wire.read() & 0xFC);  // Mask last 2 bits

    float humidity = -6 + (125.0 * humRaw / 65536.0);  
    if (humidity < 0) humidity = 0;  // Prevent negative values
    if (humidity > 100) humidity = 100;  // Cap at 100%
    
    return humidity;
}

// **Temperature Calculation**
float readTemperature() {
    Wire.beginTransmission(SHT25_ADDR);
    Wire.write(0xE3);
    Wire.endTransmission(false);

    Wire.requestFrom(SHT25_ADDR, 2);
    if (Wire.available() < 2) {
        Serial.println("Error: SHT25 No Data (Temperature)");
        return -99;
    }

    uint16_t tempRaw = (Wire.read() << 8) | (Wire.read() & 0xFC);
    return -46.85 + (175.72 * tempRaw / 65536.0);
}

// **Voltage Sensor**
float readVoltage() {
    int rawValue = analogRead(voltageSensorPin);
    return (rawValue * 5.0) / 1023.0 * (310.0 / 5.0);
}

// **Check Machine Conditions**
void checkConditions() {
    float temperature = readTemperature();
    float humidity = readHumidity();
    float voltage = readVoltage();

    if (temperature == -99 || humidity == -99) {
        Serial.println("Machine OFF: SHT25 Read Failed!");
        digitalWrite(relayPin, LOW);
        return;
    }

    if (humidity < 65) {
        Serial.println("Machine OFF: Humidity too low!");
        digitalWrite(relayPin, LOW);
    } else if (temperature < 15 || temperature > 40) {
        Serial.println("Machine OFF: Temperature out of range!");
        digitalWrite(relayPin, LOW);
    } else if (voltage < 150 || voltage > 200) {
        Serial.println("Machine OFF: Voltage out of range!");
        digitalWrite(relayPin, LOW);
    } else {
        Serial.println("Machine ON");
        digitalWrite(relayPin, HIGH);
    }
}
