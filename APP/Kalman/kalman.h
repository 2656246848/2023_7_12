/*
 * kalman.h
 *
 *  Created on: 2023��7��12��
 *      Author: GY
 */

#ifndef APP_KALMAN_KALMAN_H_
#define APP_KALMAN_KALMAN_H_

#include <math.h>
#include "time.h"

// ���忨�����˲����ṹ��
typedef struct {
    float Q_angle;     // �Ƕ���������
    float Q_bias;      // ƫ����������
    float R_measure;   // ������������
    float angle;       // ���ƵĽǶ�
    float bias;        // ���Ƶ�ƫ��
    float P[2][2];     // Э�������
} Kalman_t;

extern Kalman_t KalmanX;
extern Kalman_t KalmanY;

double Kalman_getAngle(Kalman_t *Kalman, double newAngle, double newRate, double dt);
#endif /* APP_KALMAN_KALMAN_H_ */
