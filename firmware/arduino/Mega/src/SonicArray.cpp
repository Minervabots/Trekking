#include "SonicArray.h"
#include "Constants.h"
#include "Variables.h"
#include "Utils.h"

SonicArray::SonicArray(unsigned int triggerPin)
{
    triggerPin_ = triggerPin;
    pinMode(triggerPin_, OUTPUT);
}

bool SonicArray::update(int *detectedCount)
{
    if (isTimeToMeasure())
    {
        // Disable the interrupts so we can prepare to calculate
        // the distances and then we are ready to start a new
        // set of measurements.
        noInterrupts(); // Begin critical section
        for (int i = 0; i < NUM_OF_SENSORS; i++)
        {
            // Copy the (volatile) start and end times of the pulses
            // so that we can begin immediately measuring again,
            // while distances of the (now previous) measurements are
            // being calculated.
            sensors_[i].prepareToCalculate();
            // Then reset previous the echoes so we can
            // conduct new measurements
            sensors_[i].reset();
        }
        // Finally start a new set of measurements
        // by triggering all sensors at once
        triggerSensors();
        interrupts(); // End critical section

        // Now that we are certain that our measurements are consistent
        // time-wise, calculate the distance.
        for (int i = 0; i < NUM_OF_SENSORS; i++)
        {
            // Calculate distance for each sensor.
            // Will also timeout any pending measurements
            if (sensors_[i].calculateDistance() > 0)
            {
                *detectedCount = *detectedCount + 1;
                if (sensors_[i].getDistance() < 255)
                {
                    Serial.print(i);
                    Serial.print(": \t");
                    Serial.println(sensors_[i].getDistance());
                }
            }
        }
    }
    return *detectedCount > 0;
}

/**
 * Retorna uma direção mista entre a evasão do obstáculo e a direção do alvo
*/
float SonicArray::obstacleAvoidance()
{
    int closestDetectionId = -1;

    for (int i = 0; i < NUM_OF_SENSORS; i++)
    {
        auto distance = sensors_[i].getDistance();
        auto direction = sensors_[i].getDirection();

        // Nada foi detectado
        if (distance == 0)
        {
            continue;
        }
        // Diferença muito grande entre a direção do obstáculo e do alvo
        if (abs(targetDirection - direction) > 0.5)
        {
            continue;
        }

        // Vamos sempre tratar o obstáculo mais próximo
        if (closestDetectionId == -1 || sensors_[i].getDistance() < sensors_[closestDetectionId].getDistance())
        {
            closestDetectionId = i;
            continue;
        }
    }

    if (closestDetectionId == -1)
    {
        return targetDirection;
    }

    return (OBSTACLE_AVOIDANCE_CONSTANT / sensors_[closestDetectionId].getDistance()) * sensors_[closestDetectionId].getDirection();
}

/**
 * Handles the echo signal from an ultrasonic sensor.
 * To be called from within an interrupt, it checks all the echo
 * pins to see if any of them is HIGH or LOW and updates the echo
 * pulse times of the respective sensor.
 * If a pulse has already been set, then the new value is ignored.
 * @param intVector The vector that the interrupt originated from
 */
void SonicArray::handleEcho(Vector intVector)
{
    int cnt = 0;
    //Serial.println("handleEcho");
    // Determine which vector the interrupt originated from
    // so we only check signals from those specific sensors
    int sensorsInVector[7] = {0}; // We have up to 7 sensors in each vector
    switch (intVector)
    {
    case VECTOR_0:
        // Ultrasonic 0 is on PCINT0_vect
        sensorsInVector[0] = 0;
        cnt = 1;
        break;

    case VECTOR_2:
        // Ultrasonics 1, 2, 3, 4, 5, 6, 7 and 8 are on PCINT2_vect
        sensorsInVector[0] = 1;
        sensorsInVector[1] = 2;
        sensorsInVector[2] = 3;
        sensorsInVector[3] = 4;
        sensorsInVector[4] = 5;
        sensorsInVector[5] = 6;
        sensorsInVector[6] = 7;
        // sensorsInVector[7] = 8;
        cnt = 7;
        break;
    default:
        break; // We should not be here
    }

    // Iterate through the specific vector's ultrasonic echo pins
    for (int i = 0; i < cnt; i++)
    {
        int sensorId = sensorsInVector[i];
        // If a pin is HIGH, it means that a pulse
        // is either just starting or has previously started.
        // We only care about the former.
        if (digitalRead(sensors_[sensorId].getEchoPin()) == HIGH)
        {
            // We only care for newly generated pulses and not ones
            // we have handled before.
            if (sensors_[sensorId].getStartOfPulse() == 0)
            {
                sensors_[sensorId].setStartOfPulse(micros());
            }
        }
        else
        {
            // If a pin is LOW, it means that a pulse has just ended,
            // has already ended or not started. We only care about
            // the first case. We can determine this by pulses which
            // we have not handled before AND that have already started
            if (sensors_[i].getEndOfPulse() == 0 && sensors_[i].getStartOfPulse() != 0)
            {
                sensors_[i].setEndOfPulse(micros());
            }
        }
    }
}

void SonicArray::setupInterrupts()
{
    for (int i = 0; i < NUM_OF_SENSORS; i++)
    {
        setupChangeInterrupt(sensors_[i].getEchoPin());
    }
}

void SonicArray::triggerSensors()
{
    digitalWrite(triggerPin_, LOW);
    delayMicroseconds(3);
    digitalWrite(triggerPin_, HIGH);
    delayMicroseconds(10);
    digitalWrite(triggerPin_, LOW);
}

bool SonicArray::isTimeToMeasure()
{
    unsigned long currentTime = millis();
    if (currentTime - previousMeasurement_ >= measuramentInterval_)
    {
        previousMeasurement_ = currentTime;
        return true;
    }
    return false;
}
