#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "Macros.h"

#define DISTANCE_BETWEEN_AXES   0.35            // Distância entre os eixo dianteiro e traseiro (eixos das rodas) em metros

#define RPI_BAUD_RATE           9600
#define MPU_BAUD_RATE           9600

#define CONE_REAL_HEIGHT        0.5             // Cone height: 50 cm = 0.5 m
#define FOCAL_LENGHT            3.04            // Focal lenght: 3.04 mm
#define SENSOR_HEIGHT           3.68            // Sensor image area: 3.68 x 2.76 mm (4.6 mm diagonal)
#define IMAGE_PIXEL_HEIGHT      3280            // Height size of the image in sensor: 3280 pixels

#define CAMERA_SERVO_LIMIT      PI
#define STEERING_SERVO_LIMIT    PI/2
#define ESC_MAX_FORWARD         1               // Apesar do ESC ser controlado como um servo, é mais fácil 
#define ESC_MAX_BACKWARD        -1              // pensar as velocidades como valores entre [-1; 1]
#define ESC_ZERO                90

#define ULTRASSONIC_MAX_DIST    300               
#define ULTRASSONIC_TIMEOUT     CM_TO_MS(ULTRASSONIC_MAX_DIST)

#define REFINED_SEARCH_DISTANCE 7.0             // Quanto maior a distância de confiança conseguirmos com a câmera melhor
                                                // Precisa ser maior que 5 pra já estar em refinedSearch quando chegar perto dos obstáculos
#define GOAL_THRESHOLD          0.5             // Distância aceitável de um objetivo pra dizer que chagou (m)

#define BUTTON_STOP_TIME        5000

#endif
