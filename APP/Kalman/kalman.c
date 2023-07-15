/*
 * kalman.c
 *
 *  Created on: 2023年7月12日
 *      Author: GY
 */
#include"kalman.h"
#include "stdlib.h"

// 创建KalmanX实例，并初始化参数
Kalman_t KalmanX = {
    .Q_angle = 0.001f,
    .Q_bias = 0.003f,
    .R_measure = 0.03f
};

// 创建KalmanY实例，并初始化参数
Kalman_t KalmanY = {
    .Q_angle = 0.001f,
    .Q_bias = 0.003f,
    .R_measure = 0.03f,
};


/*用于计算卡尔曼滤波器的输出角度。它接受 Kalman_t 结构体指针（指向 KalmanX 或 KalmanY）以及新的角度、速率和时间增量作为输入参数。*/
double Kalman_getAngle(Kalman_t *Kalman, double newAngle, double newRate, double dt)
{
    double rate = newRate - Kalman->bias;// 计算速率偏差
    // 根据时间增量更新角度
    Kalman->angle += dt * rate;

    // 更新协方差矩阵的元素
    Kalman->P[0][0] += dt * (dt * Kalman->P[1][1] - Kalman->P[0][1] - Kalman->P[1][0] + Kalman->Q_angle);
    Kalman->P[0][1] -= dt * Kalman->P[1][1];
    Kalman->P[1][0] -= dt * Kalman->P[1][1];
    Kalman->P[1][1] += Kalman->Q_bias * dt;

    double S = Kalman->P[0][0] + Kalman->R_measure;  // 计算中间变量S
    double K[2];
    K[0] = Kalman->P[0][0] / S;  // 计算卡尔曼增益矩阵的元素
    K[1] = Kalman->P[1][0] / S;

    double y = newAngle - Kalman->angle;  // 计算测量残差
    Kalman->angle += K[0] * y;  // 使用卡尔曼增益修正角度估计
    Kalman->bias += K[1] * y;   // 使用卡尔曼增益修正偏差估计

    double P00_temp = Kalman->P[0][0];  // 临时变量
    double P01_temp = Kalman->P[0][1];

    // 更新协方差矩阵的元素
    Kalman->P[0][0] -= K[0] * P00_temp;
    Kalman->P[0][1] -= K[0] * P01_temp;
    Kalman->P[1][0] -= K[1] * P00_temp;
    Kalman->P[1][1] -= K[1] * P01_temp;

    return Kalman->angle;  // 返回估计的角度
};

