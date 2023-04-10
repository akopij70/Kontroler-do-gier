#include <Arduino.h>
#include <Wire.h>
#include <MPU6050_light.h>
#include <BleMouse.h>

#define INTERVAL 25
#define NOT_PRESSED 1
#define PRESSED 0
#define PICKAXE_LEFT_MOUSE 19
#define RIGHT_MOUSE 23
#define NORMAL_LEFT_MOUSE 4
#define NUM_OF_CLICKERS 3

void StartMpu();
void SetActions();
void NormalLeftClick();
void PickaxeLeftClick();
void NormalRightClick();
void NoneClick();
void AdjustPosition();
void ShowInfo();

MPU6050 mpu(Wire);
BleMouse ble_mouse("totylkotest");

int clickers[NUM_OF_CLICKERS] = {19, 23, 4};
int active_mouse_buttons[NUM_OF_CLICKERS] = {5, 18}; // RPM, LPM
double X, Y, horizontal, vertical, click_guard;
unsigned long timer = 0;

void setup()
{
    for (int i = 0; i < NUM_OF_CLICKERS; i++)
    {
        pinMode(clickers[i], INPUT_PULLUP);
        pinMode(active_mouse_buttons[i], OUTPUT);
    }
    Serial.begin(115200);
    while (!Serial)
        delay(10);
    Serial.println("Starting BLE work!");
    ble_mouse.begin();
    StartMpu();
    Serial.println("Done!\n");
}

void loop()
{
    if (((millis() - timer) > INTERVAL) && ble_mouse.isConnected())
    {
        SetActions();
        timer = millis();
    }
}

void StartMpu()
{
    Wire.begin();
    byte status = mpu.begin();
    Serial.print(F("MPU6050 status: "));
    Serial.println(status);
    while (status != 0)
        ; // czekaj na polaczenie
    Serial.println(F("Calculating offsets, do not move MPU6050"));
    delay(1000);
    mpu.calcOffsets(); // gyro and accelero
}

void SetActions()
{
    mpu.update();
    X = mpu.getAngleX();
    Y = mpu.getAngleY();
    click_guard = X;
    
    AdjustPosition();

    if (digitalRead(PICKAXE_LEFT_MOUSE) == PRESSED && digitalRead(RIGHT_MOUSE) == NOT_PRESSED)
        PickaxeLeftClick();

    else if (digitalRead(RIGHT_MOUSE) == PRESSED && digitalRead(PICKAXE_LEFT_MOUSE) == NOT_PRESSED)
        NormalRightClick();
    
    else if (digitalRead(NORMAL_LEFT_MOUSE) == PRESSED && digitalRead(RIGHT_MOUSE) == NOT_PRESSED && digitalRead(PICKAXE_LEFT_MOUSE) == NOT_PRESSED)
        NormalLeftClick();

    else
        NoneClick();

    AdjustPosition();
    ble_mouse.move(horizontal, vertical);
}

void NormalLeftClick()
{
    while (digitalRead(NORMAL_LEFT_MOUSE) == PRESSED)
    {
        Serial.print("KLIKACZ ZWYKLY");
        digitalWrite(active_mouse_buttons[0], LOW);
        digitalWrite(active_mouse_buttons[1], HIGH);
        mpu.update();
        ble_mouse.press();
        X = mpu.getAngleX();
        Y = mpu.getAngleY();

        AdjustPosition();
        ble_mouse.move(horizontal, vertical);
        delay(INTERVAL);
    }
    ble_mouse.release();
}

void PickaxeLeftClick()
{
    while (click_guard > -25.0 && digitalRead(PICKAXE_LEFT_MOUSE) == PRESSED)
    {
        mpu.update();
        click_guard = mpu.getAngleX();
    }
        
    while (click_guard <= -25.0 && digitalRead(PICKAXE_LEFT_MOUSE) == PRESSED)
    {
        Serial.print("\tpowinno swiecic LPM");
        digitalWrite(active_mouse_buttons[1], HIGH);
        digitalWrite(active_mouse_buttons[0], LOW);
        mpu.update();
        ble_mouse.press();
        delay(INTERVAL);
    }
    ble_mouse.release();
    X = 0.0;
    Y = 0.0;
    delay(INTERVAL);
}

void NormalRightClick() 
{
    while (digitalRead(RIGHT_MOUSE) == PRESSED)
    {
        Serial.print("\tpowinno swiecic RPM");
        digitalWrite(active_mouse_buttons[1], LOW);
        digitalWrite(active_mouse_buttons[0], HIGH);
        mpu.update();
        ble_mouse.press(MOUSE_RIGHT);
        X = mpu.getAngleX();
        Y = mpu.getAngleY();
        AdjustPosition();
        ble_mouse.move(horizontal, vertical);
        delay(INTERVAL);
    }
    ble_mouse.release(MOUSE_RIGHT);
}

void NoneClick()
{
    digitalWrite(active_mouse_buttons[0], LOW);
    digitalWrite(active_mouse_buttons[1], LOW);
}

void AdjustPosition()
{
    if (abs(X) > 15.0)
        vertical = map(X, -85, 85, 25, -25);
    else
        vertical = 0;

    if (abs(Y) > 15.0)
        horizontal = map(Y, -75, 75, -25, 25);
    else
        horizontal = 0;
}

void ShowInfo()
{
    if (digitalRead(PICKAXE_LEFT_MOUSE) == PRESSED)
        Serial.print("LPM wcisniety");
    if (digitalRead(RIGHT_MOUSE) == PRESSED)
        Serial.print("\tRPM wcisniety");
    if (digitalRead(NORMAL_LEFT_MOUSE) == PRESSED)
        Serial.print("\tklikacz wcisniety");

    Serial.print("\tX : ");
    Serial.print(X);
    Serial.print("\tY : ");
    Serial.println(Y);
}