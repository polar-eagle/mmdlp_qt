#include "Innfosmotor.h"
#include <thread>
#include <chrono>
#include <QDebug>

extern std::atomic<bool> safeStop;

InnfosMotor::InnfosMotor(int id_, QObject *parent) : id(id_), QObject(parent)
{
    pController = ActuatorController::initController();
}
/**
 * @brief 设置电机home值
 * @param home_value home值
 * @return void
 */
void InnfosMotor::setHome(double home_value)
{
    home = home_value;
}

/**
 * @brief 设置电机lead值
 * @param lead_value lead值
 * @return void
 */
void InnfosMotor::setLead(double lead_value)
{
    lead = lead_value;
}

/**
 * @brief 设置电机最大位置
 * @param max_position_value 最大位置值
 * @return void
 */
void InnfosMotor::setMaxPosition(double max_position_value)
{
    max_position = max_position_value;
}

/**
 * @brief 设置电机最小位置
 * @param min_position_value 最小位置值
 * @return void
 */
void InnfosMotor::setMinPosition(double min_position_value)
{
    min_position = min_position_value;
}

/**
 * @brief 设置电机默认加速度
 * @param default_acc_value 默认加速度值
 * @return void
 */
void InnfosMotor::setDefaultAcc(double default_acc_value)
{
    default_acc = default_acc_value;
}

/**
 * @brief 设置电机默认减速度
 * @param default_dec_value 默认减速度值
 * @return void
 */
void InnfosMotor::setDefaultDec(double default_dec_value)
{
    default_dec = default_dec_value;
}

/**
 * @brief 设置电机默认速度
 * @param default_vel_value 默认速度值
 * @return void
 */
void InnfosMotor::setDefaultVel(double default_vel_value)
{
    default_vel = default_vel_value;
}

/**
 * @brief 设置电机模式
 * @param mode_ 电机模式
 * @return void
 */
void InnfosMotor::setMode(ActuatorMode mode_)
{
    mode = mode_;
    pController->activateActuatorMode((uint8_t)id, mode);
}

/**
 * @brief 保存电机参数
 * @return void
 */
void InnfosMotor::saveMotorParams()
{
    pController->saveAllParams((uint8_t)id);
}

/**
 * @brief 清除电机错误
 * @return void
 */
void InnfosMotor::clearError()
{
    pController->clearError((uint8_t)id);
}

/**
 * @brief 设置电机profile position最大速度
 * @param vel 最大速度值
 * @return void
 */
void InnfosMotor::setProfilePositionMaxVel(double vel)
{
    pController->setProfilePositionMaxVelocity((uint8_t)id, vel);
}

/**
 * @brief 设置电机profile position加速度
 * @param acc 加速度值
 * @return void
 */
void InnfosMotor::setProfilePositionAcc(double acc)
{
    pController->setProfilePositionAcceleration((uint8_t)id, acc);
}

/**
 * @brief 设置电机profile position减速度
 * @param dec 减速度值
 * @return void
 */
void InnfosMotor::setProfilePositionDec(double dec)
{
    pController->setProfilePositionAcceleration((uint8_t)id, dec);
}

/**
 * @brief 设置电机profile position速度
 * @param vel 速度值
 * @return void
 */
void InnfosMotor::setProfilePositionVel(double vel)
{
    pController->setProfilePositionVelocity((uint8_t)id, vel);
}

/**
 * @brief 获取末端执行器位置
 * @return double 末端执行器位置
 */
double InnfosMotor::getEndEffectorPosition()
{
    return (this->position - home) * lead;
}

/**
 * @brief 移动到末端执行器位置
 * @param pos 末端执行器位置
 * @return void
 */
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
        if (std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count() > 60)
        {
            emit timeOutError();
            break;
        }
    }
    qDebug() << id << " " << std::abs(this->position.load() - absPos);
    error_code = pController->getErrorCode((uint8_t)id);
    emit motorErrorCode(this->id, this->error_code);
}

/**
 * @brief 设置电机速度
 * @param acc 加速度
 * @param dec 减速度
 * @param vel 速度
 * @return void
 */
void InnfosMotor::setSpeed(double acc, double dec, double vel)
{
    std::cout << "acc: " << acc << " dec: " << dec << " vel: " << vel << std::endl;
    pController->setProfilePositionAcceleration((uint8_t)id, acc);
    this_thread::sleep_for(std::chrono::milliseconds(50));
    pController->setProfilePositionDeceleration((uint8_t)id, dec);
    this_thread::sleep_for(std::chrono::milliseconds(50));
    pController->setProfilePositionMaxVelocity((uint8_t)id, vel);
}

/**
 * @brief 使能电机
 * @return void
 */
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

/**
 * @brief 失能电机
 * @return void
 */
void InnfosMotor::disable()
{
    pController->disableActuator((uint8_t)id);
}

/**
 * @brief 设置电机原点
 * @param potisition 原点位置
 * @return void
 */
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

/**
 * @brief 设置电机当前位置，来自monitor线程异步获取的位置
 * @param position_ 位置
 * @return void
 */
void InnfosMotor::setPosition(double position_)
{
    position.store(position_);
}