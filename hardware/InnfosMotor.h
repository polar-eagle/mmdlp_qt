#ifndef INNFOSMOTOR_H
#define INNFOSMOTOR_H

#include <string>
#include <vector>
#include "actuatorcontroller.h"
#include <QObject>

class InnfosMotor : public QObject
{
    Q_OBJECT
private:
    int id = 0;
    ActuatorMode mode = ActuatorMode::Mode_None;
    ActuatorController *pController;

    double home = 0;
    double lead = 0.0;
    double max_position = 0.0;
    double min_position = 0.0;
    double default_acc = 0.0;
    double default_dec = 0.0;
    double default_vel = 0.0;

    double errorThreshold = 0.1; // 允许的误差范围

    std::atomic<double> position = 0.0;

    uint32_t error_code = Actuator::ErrorsDefine::ERR_NONE;

public:
    explicit InnfosMotor(int id_ = 0, QObject *parent = nullptr);
    InnfosMotor() = delete;
    ~InnfosMotor() = default;

    void setID(int id_) { id = id_; };
    int ID() { return id; };

    void setHome(double home_value);
    double Home() { return home; };

    void setLead(double lead_value);
    double Lead() { return lead; };

    void setMaxPosition(double max_position_value);
    double MaxPosition() { return max_position; };

    void setMinPosition(double min_position_value);
    double MinPosition() { return min_position; };

    void setDefaultAcc(double default_acc_value);
    double DefaultAcc() { return default_acc; };

    void setDefaultDec(double default_dec_value);
    double DefaultDec() { return default_dec; };

    void setDefaultVel(double default_vel_value);
    double DefaultVel() { return default_vel; };

    void setMode(ActuatorMode mode_);
    ActuatorMode Mode() { return mode; };

    void saveMotorParams();

    void setProfilePositionMaxVel(double vel);
    void setProfilePositionAcc(double acc);
    void setProfilePositionDec(double dec);

    double getEndEffectorPosition();

    void moveToEEPotision(double pos);

    void setSpeed(double acc, double dec, double vel);

    void enable();
    void disable();

    void clearError();

    void setOrinPosition(double potisition);

    void setPosition(double potisition_);
    double Position() { return position; };

    void setErrorThreshold(double threshold) { errorThreshold = threshold; };

signals:
    void motorErrorCode(int id, uint32_t error_code);
    void targetPosition(int id, double position);
    void timeOutError();
};

#endif // INNFOSMOTOR_H
