/*
 * kalman.h
 *
 *  Created on: 2023年7月12日
 *      Author: GY
 */

#ifndef APP_KALMAN_KALMAN_H_
#define APP_KALMAN_KALMAN_H_

#include <math.h>
#include "time.h"

// 定义卡尔曼滤波器结构体
typedef struct {
    float Q_angle;     // 角度噪声方差
    float Q_bias;      // 偏差噪声方差
    float R_measure;   // 测量噪声方差
    float angle;       // 估计的角度
    float bias;        // 估计的偏差
    float P[2][2];     // 协方差矩阵
} Kalman_t;

extern Kalman_t KalmanX;
extern Kalman_t KalmanY;

double Kalman_getAngle(Kalman_t *Kalman, double newAngle, double newRate, double dt);
#endif /* APP_KALMAN_KALMAN_H_ */
