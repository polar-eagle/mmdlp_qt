#include "Innfosmotor.h"
#include <thread>
#include <chrono>
#include <QDebug>

extern std::atomic<bool> safeStop;

InnfosMotor::InnfosMotor(int id_, QObject *parent) : id(id_), QObject(parent)
{
    pController = ActuatorController::initController();
}

void InnfosMotor::setHome(double home_value)
{
    home = home_value;
}
void InnfosMotor::setLead(double lead_value)
{
    lead = lead_value;
}

void InnfosMotor::setMaxPosition(double max_position_value)
{
    max_position = max_position_value;
}

void InnfosMotor::setMinPosition(double min_position_value)
{
    min_position = min_position_value;
}

void InnfosMotor::setDefaultAcc(double default_acc_value)
{
    default_acc = default_acc_value;
}

void InnfosMotor::setDefaultDec(double default_dec_value)
{
    default_dec = default_dec_value;
}

void InnfosMotor::setDefaultVel(double default_vel_value)
{
    default_vel = default_vel_value;
}

void InnfosMotor::setMode(ActuatorMode mode_)
{
    mode = mode_;
    pController->activateActuatorMode((uint8_t)id, mode);
}

void InnfosMotor::saveMotorParams()
{
    pController->saveAllParams((uint8_t)id);
}
void InnfosMotor::clearError()
{
    pController->clearError((uint8_t)id);
}
void InnfosMotor::setProfilePositionMaxVel(double vel)
{
    pController->setProfilePositionMaxVelocity((uint8_t)id, vel);
}
void InnfosMotor::setProfilePositionAcc(double acc)
{
    pController->setProfilePositionAcceleration((uint8_t)id, acc);
}
void InnfosMotor::setProfilePositionDec(double dec)
{
    pController->setProfilePositionAcceleration((uint8_t)id, dec);
}
double InnfosMotor::getEndEffectorPosition()
{
    return (this->position - home) * lead;
}
void InnfosMotor::moveToEEPotision(double pos)
{
    double absPos = pos / lead + home;
    emit targetPosition(id, pos);
    qDebug() << id << ' ' << pos;
    pController->setPosition((uint8_t)id, absPos);

    auto t1 = std::chrono::system_clock::now();
    while (std::abs(this->position.load() - absPos) > errorThreshold)
    {
        if (safeStop.load())
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        auto t2 = std::chrono::system_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count() > 120)
        {
            emit timeOutError();
            break;
        }
    }
    qDebug() << id << " " << std::abs(this->position.load() - absPos);
    error_code = pController->getErrorCode((uint8_t)id);
    emit motorErrorCode(this->id, this->error_code);
}

void InnfosMotor::setSpeed(double acc, double dec, double vel)
{
    std::cout << "acc: " << acc << " dec: " << dec << " vel: " << vel << std::endl;
    pController->setProfilePositionAcceleration((uint8_t)id, acc);
    this_thread::sleep_for(std::chrono::milliseconds(50));
    pController->setProfilePositionDeceleration((uint8_t)id, dec);
    this_thread::sleep_for(std::chrono::milliseconds(50));
    pController->setProfilePositionMaxVelocity((uint8_t)id, vel);
}

void InnfosMotor::enable()
{
    try
    {
        pController->clearError((uint8_t)id);
        this_thread::sleep_for(std::chrono::milliseconds(50));
        pController->enableActuator((uint8_t)id);
        setMode(ActuatorMode::Mode_Profile_Pos);
        // setProfilePositionAcc(100);
        // setProfilePositionDec(-100);
        // setProfilePositionMaxVel(100);

        setSpeed(default_acc, default_dec, default_vel);
        error_code = pController->getErrorCode((uint8_t)id);
        this_thread::sleep_for(std::chrono::milliseconds(100));
        qDebug() << pController->getActuatorMode((uint8_t)id);
        emit motorErrorCode(this->id, this->error_code);
    }
    catch (...)
    {
        qDebug() << "Motor Enable Fail";
        emit motorErrorCode(this->id, -1);
    }
}

void InnfosMotor::disable()
{
    pController->disableActuator((uint8_t)id);
}

void InnfosMotor::setOrinPosition(double potisition)
{
    qDebug() << "++++" << potisition;
    pController->clearError((uint8_t)id);
    pController->activateActuatorMode((uint8_t)id, ActuatorMode::Mode_Homing);
    qDebug() << pController->getActuatorMode((uint8_t)id);
    pController->clearHomingInfo((uint8_t)id);
    pController->setHomingPosition((uint8_t)id, 0);
    // pController->enablePositionLimit(id, true);
    pController->setMaximumPosition((uint8_t)id, 100);
    pController->setMinimumPosition((uint8_t)id, -100);
    pController->activateActuatorMode((uint8_t)id, ActuatorMode::Mode_Profile_Pos);
    pController->saveAllParams((uint8_t)id);
}

void InnfosMotor::setPosition(double position_)
{
    position.store(position_);
}