#ifndef MOTOR_MONITOR_THREAD_H
#define MOTOR_MONITOR_THREAD_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include "actuatorcontroller.h"
#include <QMutex>

class MotorMonitorThread : public QThread
{
    Q_OBJECT

public:
    explicit MotorMonitorThread(QObject *parent = nullptr);
    ~MotorMonitorThread();

    void setMotorID(int motorId);
    void setThresholds(double currentThreshold, double velocityThreshold);
    void run();
    void onMonitor();

    static bool disableFlag;
    static QMutex disableFlagMutex;

signals:
    void sendPosition(int motorId, double pos);
    void stopSignal();

private:
    int motorId = -1;         // 电机ID
    double currentThreshold;  // 电流阈值
    double velocityThreshold; // 速度阈值

    double current = 0.0;  // 电流
    double velocity = 0.0; // 速度
    double position = 0.0; // 位置

    QMutex currentValueMutex;  // 电流值互斥锁
    QMutex velocityValueMutex; // 速度值互斥锁
    QMutex positionValueMutex; // 位置值互斥锁

    void disableMotor();

    void paramFeedback(ActuatorController::UnifiedID uID, int paramType, double paramValue);
};

#endif // MOTOR_MONITOR_THREAD_H
