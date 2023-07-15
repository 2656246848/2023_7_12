/*
 * kalman.c
 *
 *  Created on: 2023��7��12��
 *      Author: GY
 */
#include"kalman.h"
#include "stdlib.h"

// ����KalmanXʵ��������ʼ������
Kalman_t KalmanX = {
    .Q_angle = 0.001f,
    .Q_bias = 0.003f,
    .R_measure = 0.03f
};

// ����KalmanYʵ��������ʼ������
Kalman_t KalmanY = {
    .Q_angle = 0.001f,
    .Q_bias = 0.003f,
    .R_measure = 0.03f,
};


/*���ڼ��㿨�����˲���������Ƕȡ������� Kalman_t �ṹ��ָ�루ָ�� KalmanX �� KalmanY���Լ��µĽǶȡ����ʺ�ʱ��������Ϊ���������*/
double Kalman_getAngle(Kalman_t *Kalman, double newAngle, double newRate, double dt)
{
    double rate = newRate - Kalman->bias;// ��������ƫ��
    // ����ʱ���������½Ƕ�
    Kalman->angle += dt * rate;

    // ����Э��������Ԫ��
    Kalman->P[0][0] += dt * (dt * Kalman->P[1][1] - Kalman->P[0][1] - Kalman->P[1][0] + Kalman->Q_angle);
    Kalman->P[0][1] -= dt * Kalman->P[1][1];
    Kalman->P[1][0] -= dt * Kalman->P[1][1];
    Kalman->P[1][1] += Kalman->Q_bias * dt;

    double S = Kalman->P[0][0] + Kalman->R_measure;  // �����м����S
    double K[2];
    K[0] = Kalman->P[0][0] / S;  // ���㿨������������Ԫ��
    K[1] = Kalman->P[1][0] / S;

    double y = newAngle - Kalman->angle;  // ��������в�
    Kalman->angle += K[0] * y;  // ʹ�ÿ��������������Ƕȹ���
    Kalman->bias += K[1] * y;   // ʹ�ÿ�������������ƫ�����

    double P00_temp = Kalman->P[0][0];  // ��ʱ����
    double P01_temp = Kalman->P[0][1];

    // ����Э��������Ԫ��
    Kalman->P[0][0] -= K[0] * P00_temp;
    Kalman->P[0][1] -= K[0] * P01_temp;
    Kalman->P[1][0] -= K[1] * P00_temp;
    Kalman->P[1][1] -= K[1] * P01_temp;

    return Kalman->angle;  // ���ع��ƵĽǶ�
};

