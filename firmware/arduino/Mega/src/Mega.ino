#include <Servo.h>
#include <PID_v1.h>
#include <math.h>

#include "Pins.h"
#include "Target.h"
#include "Variables.h"
#include "Constants.h"
#include "States.h"
#include "CommandHandlers.h"
#include "RCReceiver.h"
#include "InterruptServiceRoutines.h"
#include "PIDHelper.h"
#include "Utils.h"

unsigned long lastRun;
volatile unsigned long buttonPressStart;
SonicArray sonicArray(PIN_ULTRASSONIC_TRIGGER);

void setup()
{
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(BATTERY_FEEDBACK_PIN, INPUT);
    pinMode(RPI_COOLER_PWM_PIN, OUTPUT);
    pinMode(LED_DEBUG_PIN, OUTPUT);
    pinMode(LED_SIGNAL_PIN, OUTPUT);
    
    attachHandlers();
    attachRCInterrupts();
    setupPID();

    Serial.begin(RPI_BAUD_RATE);
    Serial3.begin(MPU_BAUD_RATE);
    sonicArray.setupInterrupts();

    cameraServo.attach(CAMERA_SERVO_PIN);
    steeringServo.attach(STEERING_SERVO_PIN);
    esc.attach(ESC_PIN);
    esc.write(ESC_ZERO);

    targets.add(Target(40, 20, true));
    targets.add(Target(30, 2, true));
    targets.add(Target(6, 20, false));
    targets.add(Target(6, 18, true));
    currentTarget = targets.get(0);

    state = &idle;
    previousState = &idle;
    isRunning = true;
    lastRun = 0;
}

void loop()
{
    int batteryFeedback = analogRead(BATTERY_FEEDBACK_PIN);
    float batteryVolts = mapf(batteryFeedback, 0, 920.7, 0, 21);
    
    Serial.print(batteryFeedback);
    Serial.print("\t");
    Serial.println(batteryVolts);
    
    // digitalWrite(LED_SIGNAL_PIN, LOW);
    // if(batteryVolts <= 18)
    // {
    //     digitalWrite(LED_DEBUG_PIN, HIGH);
    // }
    
    if (millis() - lastSignalTime >= 1000)
    {
        esc.write(ESC_ZERO);
        return;
    }

    if (gearPulseWidth > 1500)
    {
        digitalWrite(LED_DEBUG_PIN, HIGH);
        digitalWrite(LED_SIGNAL_PIN, HIGH);
        
        // Manual
        cameraPid.SetMode(MANUAL);
        steeringPid.SetMode(MANUAL);
        speedPid.SetMode(MANUAL);

        esc.writeMicroseconds(elevPulseWidth);
        steeringServo.writeMicroseconds(ruddPulseWidth);
        cameraServo.write(90);

        return;
    }
    else
    {
        digitalWrite(LED_DEBUG_PIN, LOW);
        digitalWrite(LED_SIGNAL_PIN, LOW);

        // Automático
        cameraPid.SetMode(AUTOMATIC);
        steeringPid.SetMode(AUTOMATIC);
        speedPid.SetMode(AUTOMATIC);
    }
    rPiCmdMessenger.feedinSerialData();
    //mpuCmdMessenger.feedinSerialData();

    handleButton();

    if (isRunning)
    {
        setAuto();
        actuatorsWrite = ExecutionFlags::kAll;
        (*state)(millis() - lastRun);
    }

    computePID();
    writeInActuators();
    lastRun = millis();
}

void handleButton()
{
    if (digitalRead(BUTTON_PIN) == LOW && buttonPressStart == 0) // Pressionado
    {
        delay(200);
        buttonPressStart = millis();
        return;
    }
    if (buttonPressStart == 0)
        return;

    while (digitalRead(BUTTON_PIN) == LOW)
    {
    }

    if (millis() - buttonPressStart > BUTTON_STOP_TIME)
    {
        buttonPressStart = 0;
        changeState(reset);
        return;
    }
    if (state == idle)
    {
        changeState(previousState);
        setManual();
    }
    else
    {
        changeState(idle);
        setAuto();
    }
    buttonPressStart = 0;
}

void writeInActuators()
{
    int value;
    if (actuatorsWrite & ExecutionFlags::kSpeed)
    {
        value = round(mapf(linearSpeed * linearSpeedLock, 0, 1, ESC_ZERO, ESC_MAX_FORWARD));
        esc.write(value);
    }
    if (actuatorsWrite & ExecutionFlags::kCamera)
    {
        /*
        value = round(mapf(cameraServoPosition, -1, 1, CAMERA_SERVO_LIMIT_MIN, CAMERA_SERVO_LIMIT_MAX));
        cameraServo.write(value);
        */
    }
    if (actuatorsWrite & ExecutionFlags::kSteering)
    {
        value = round(mapf(steeringServoPosition, -1, 1, STEERING_SERVO_MIN_LIMIT, STEERING_SERVO_MAX_LIMIT));
        steeringServo.write(value);
    }
}
