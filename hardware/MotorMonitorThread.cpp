#include "MotorMonitorThread.h"
#include <iostream>
#include <QTimer>
#include <thread>
#include <QMutex>
#include <QDebug>

std::atomic<bool> safeStop(false);
// QMutex safeStopMutex;

bool MotorMonitorThread::disableFlag = true;
QMutex MotorMonitorThread::disableFlagMutex;

MotorMonitorThread::MotorMonitorThread(QObject *parent)
    : QThread(parent), motorId(-1), currentThreshold(10.0), velocityThreshold(200.0)
{
    std::function<void(ActuatorController::UnifiedID, int, double)> callback =
        std::bind(&MotorMonitorThread::paramFeedback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

    ActuatorController::getInstance()->addParaRequestCallback(callback);
}

MotorMonitorThread::~MotorMonitorThread()
{
}

void MotorMonitorThread::run()
{
    while (true)
    {
        // disableFlagMutex.lock();
        // if (disableFlag)
        // {
            onMonitor();
        // }
        // disableFlagMutex.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
void MotorMonitorThread::setMotorID(int motorId)
{
    this->motorId = motorId;
}

void MotorMonitorThread::setThresholds(double currentThreshold, double velocityThreshold)
{
    this->currentThreshold = currentThreshold;
    this->velocityThreshold = velocityThreshold;
}

void MotorMonitorThread::onMonitor()
{
    if (motorId < 0)
        return;
    ActuatorController::processEvents();
    ActuatorController::getInstance()->requestCVPValue(motorId);

    if (abs(current) > currentThreshold && abs(velocity) < velocityThreshold)
    {
        qDebug() << "Motor " << motorId << " exceeds current threshold and is under velocity threshold!" ;
        try
        {
            disableMotor();
            this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        catch (...)
        {
            qDebug() << "disable motor error" ;
        }
        current = 0.0;
        velocity = 0.0;
    }
}

void MotorMonitorThread::disableMotor()
{
    try
    {
        qDebug() << "##disable motor" ;
        ActuatorController::getInstance()->disableAllActuators();
        safeStop.store(true);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}

void MotorMonitorThread::paramFeedback(ActuatorController::UnifiedID uID, int paramType, double paramValue)
{
    if (uID.actuatorID != this->motorId)
        return;

    switch (paramType)
    {
    case Actuator::ACTUAL_CURRENT:
        currentValueMutex.lock();
        current = paramValue;
        currentValueMutex.unlock();
        // qDebug() << "Motor " << motorId << " current: " << paramValue << " A" ;
        break;

    case Actuator::ACTUAL_VELOCITY:
        velocityValueMutex.lock();
        velocity = paramValue;
        velocityValueMutex.unlock();
        // qDebug() << "Motor " << motorId << " velocity: " << paramValue << " RPM" ;
        break;

    case Actuator::ACTUAL_POSITION:
        positionValueMutex.lock();
        position = paramValue;
        emit sendPosition(motorId, position);
        positionValueMutex.unlock();
        // qDebug() << "Motor " << motorId << " position: " << paramValue ;
        break;

    default:
        break;
    }
}