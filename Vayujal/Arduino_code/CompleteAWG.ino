#include <Wire.h>

// Pin Definitions
const int trigPin = 6;
const int echoPin = 5;
const int flowSensorPin = 2;
const int relayPin = 7;
const int voltageSensorPin = A0;
const int tdsPin = A1;

// Flow Calculation
volatile int pulseCount = 0;
float flowRate = 0.0;
float totalFlow = 0.0;
unsigned long lastFlowTime = 0;
unsigned long lastAvgTime = 0;
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

    digitalWrite(relayPin, LOW);
    Serial.println("System Initialized!");
}

void loop() {
    float distance = readDistance();
    float voltage = readVoltage();
    float temperature = readTemperature();
    float humidity = readHumidity();
    int tdsValue = analogRead(tdsPin);
    float tds = (tdsValue * 5.0 / 1023.0) * 100; // crude conversion (adjust via calibration)

    if (millis() - lastFlowTime > 1000) {
        flowRate = pulseCount / calibrationFactor;  // L/min
        totalFlow += (flowRate / 60.0);             // Accumulate L/sec every second
        pulseCount = 0;
        lastFlowTime = millis();
    }

    if (millis() - lastAvgTime > 900000) {  // Every 15 mins
        float avgLph = (totalFlow * 4);     // Extrapolated to hourly (4x 15min blocks)
        Serial.print("Average Flow: ");
        Serial.print(avgLph);
        Serial.println(" L/hr");
        totalFlow = 0;
        lastAvgTime = millis();
    }

    Serial.print("Distance: "); Serial.print(distance); Serial.println(" cm");
    Serial.print("Flow Rate: "); Serial.print(flowRate); Serial.println(" L/min");
    Serial.print("Temperature: "); Serial.print(temperature); Serial.println(" °C");
    Serial.print("Humidity: "); Serial.print(humidity); Serial.println(" %");
    Serial.print("Voltage: "); Serial.print(voltage); Serial.println(" V");
    Serial.print("TDS: "); Serial.print(tds); Serial.println(" ppm");

    if (tds > 50) {
        Serial.println("⚠️ Water Quality is Low (TDS > 50 ppm)");
    }

    checkConditions(distance, temperature, humidity, voltage);
    Serial.println("---------------------------------");
    delay(1000);
}

// ISR for flow sensor
void countPulses() {
    pulseCount++;
}

float readDistance() {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH, 50000);
    if (duration == 0) return -1;
    return (duration * 0.034 / 2);
}

float readHumidity() {
    Wire.beginTransmission(SHT25_ADDR);
    Wire.write(0xE5);
    Wire.endTransmission(false);
    Wire.requestFrom(SHT25_ADDR, 2);
    if (Wire.available() < 2) return -99;
    uint16_t humRaw = (Wire.read() << 8) | (Wire.read() & 0xFC);
    float humidity = -6 + (125.0 * humRaw / 65536.0);
    return constrain(humidity, 0, 100);
}

float readTemperature() {
    Wire.beginTransmission(SHT25_ADDR);
    Wire.write(0xE3);
    Wire.endTransmission(false);
    Wire.requestFrom(SHT25_ADDR, 2);
    if (Wire.available() < 2) return -99;
    uint16_t tempRaw = (Wire.read() << 8) | (Wire.read() & 0xFC);
    return -46.85 + (175.72 * tempRaw / 65536.0);
}

float readVoltage() {
    int raw = analogRead(voltageSensorPin);
    return (raw * 5.0 / 1023.0) * (310.0 / 5.0); // Adjust scale based on voltage divider
}

void checkConditions(float dist, float temp, float hum, float volt) {
    if (dist > 0 && dist < 6) {
        Serial.println("Machine OFF: Tank already full (distance < 6cm)");
        digitalWrite(relayPin, LOW);
        return;
    }

    if (temp == -99 || hum == -99) {
        Serial.println("Machine OFF: SHT25 Read Failed!");
        digitalWrite(relayPin, LOW);
        return;
    }

    if (hum < 65) {
        Serial.println("Machine OFF: Humidity too low!");
        digitalWrite(relayPin, LOW);
    } else if (temp < 15 || temp > 40) {
        Serial.println("Machine OFF: Temperature out of range!");
        digitalWrite(relayPin, LOW);
    } else if (volt < 150 || volt > 200) {
        Serial.println("Machine OFF: Voltage out of range!");
        digitalWrite(relayPin, LOW);
    } else {
        Serial.println("Machine ON");
        digitalWrite(relayPin, HIGH);
    }
}
