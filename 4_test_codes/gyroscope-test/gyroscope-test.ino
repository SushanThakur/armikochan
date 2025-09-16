#include <Wire.h>
#include <I2Cdev.h>
#include <MPU6050.h>

MPU6050 mpu;

void setup() {
    Serial.begin(115200);
    Wire.begin();
    delay(2000);
    mpu.initialize();
}

void loop() {
    print_acceleration();
    delay(500);
}

void print_acceleration() {
    // Read accelerometer data
    int16_t ax, ay, az;
    mpu.getAcceleration(&ax, &ay, &az);

    // Calculate movement in X, Y, and Z directions
    float accelerationX = ax / 16384.0;  // 16384 LSB per g
    float accelerationY = ay / 16384.0;
    float accelerationZ = az / 16384.0;

    Serial.print("Acceleration (g): X = ");
    Serial.print(accelerationX, 4);
    Serial.print(", Y = ");
    Serial.print(accelerationY, 4);
    Serial.print(", Z = ");
    Serial.println(accelerationZ, 4);
}
