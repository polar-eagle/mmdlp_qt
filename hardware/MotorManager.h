#ifndef MOTORMANAGER_H
#define MOTORMANAGER_H
#include "Innfosmotor.h"

class MotorManager
{
private:
    InnfosMotor *glassMotor;
    InnfosMotor *plateMotor;
    InnfosMotor *rotateMotor;

    ActuatorController *pController;
    std::vector<ActuatorController::UnifiedID> uIDArray;
    Actuator::ErrorsDefine errorCode;

public:
    MotorManager();
    ~MotorManager();

    int motorNum = 0;
    uint8_t glassMotorID = 0;
    uint8_t plateMotorID = 0;
    uint8_t rotateMotorID = 0;

    InnfosMotor *GlassMotor() { return glassMotor; }
    InnfosMotor *PlateMotor() { return plateMotor; }
    InnfosMotor *RotateMotor() { return rotateMotor; }
    void initConfig();

    void getAllMotorInfo();

    void enableAllMotors();
    void disableAllMotors();

    void setAllMotorMode(ActuatorMode mode);

    void setAllMotorProfilePositionMaxVel(int max_vel);

    void setAllMotorProfilePositionAcc(int acc);

    void setAllMotorProfilePositionDec(int dec);

    void saveAllMotorParms();
};

#endif // INNFOSMOTORMANAGER_H