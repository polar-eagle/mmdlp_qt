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
    int motorId = -1;
    double currentThreshold;
    double velocityThreshold;

    double current = 0.0;
    double velocity = 0.0;
    double position = 0.0;

    QMutex currentValueMutex;
    QMutex velocityValueMutex;
    QMutex positionValueMutex;

    void disableMotor();

    void paramFeedback(ActuatorController::UnifiedID uID, int paramType, double paramValue);
};

#endif // MOTOR_MONITOR_THREAD_H
