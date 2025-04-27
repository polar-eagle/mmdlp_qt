#include "MotorManager.h"
#include <thread>
#include <chrono>
#include "ConfigManager.h"

ActuatorController *pController = ActuatorController::initController();

MotorManager::MotorManager()
{
    pController = ActuatorController::initController();
    ConfigManager &config = ConfigManager::instance();

    QVariantMap motorConfig = config.getGroup("motor");
    motorNum = motorConfig.value("motor_num").toInt();
    glassMotorID = motorConfig.value("glass_motor_id").toInt();
    plateMotorID = motorConfig.value("plate_motor_id").toInt();
    rotateMotorID = motorConfig.value("rotate_motor_id").toInt();
    qDebug() << "motor number: " << motorNum;
    qDebug() << "glass motor id: " << glassMotorID;
    qDebug() << "plate motor id: " << plateMotorID;
    qDebug() << "rotate motor id: " << rotateMotorID;

    glassMotor = new InnfosMotor(glassMotorID);
    plateMotor = new InnfosMotor(plateMotorID);
    rotateMotor = new InnfosMotor(rotateMotorID);

    glassMotor->setErrorThreshold(0.001);
    plateMotor->setErrorThreshold(0.001);
    rotateMotor->setErrorThreshold(0.01);

    initConfig();

    getAllMotorInfo();
    // saveAllMotorParms();
}

MotorManager::~MotorManager()
{
    delete glassMotor;
    delete plateMotor;
    delete rotateMotor;
}
void MotorManager::initConfig()
{
    ConfigManager &config = ConfigManager::instance();
    QVariantMap motorConfig = config.getGroup("motor");

    // set motor home value from config
    glassMotor->setHome(motorConfig.value("glass_home").toDouble());
    plateMotor->setHome(motorConfig.value("plate_home").toDouble());
    rotateMotor->setHome(motorConfig.value("rotate_home").toDouble());

    // set motor lead value from config
    glassMotor->setLead(motorConfig.value("glass_lead").toDouble());
    plateMotor->setLead(motorConfig.value("plate_lead").toDouble());
    rotateMotor->setLead(motorConfig.value("rotate_lead").toDouble());

    // set motor max position value from config
    glassMotor->setMaxPosition(motorConfig.value("glass_max_position").toDouble());
    plateMotor->setMaxPosition(motorConfig.value("plate_max_position").toDouble());
    rotateMotor->setMaxPosition(motorConfig.value("rotate_max_position").toDouble());

    // set motor min position value from config
    glassMotor->setMinPosition(motorConfig.value("glass_min_position").toDouble());
    plateMotor->setMinPosition(motorConfig.value("plate_min_position").toDouble());
    rotateMotor->setMinPosition(motorConfig.value("rotate_min_position").toDouble());

    // set motor default acc value from config
    glassMotor->setDefaultAcc(motorConfig.value("glass_default_acc").toDouble());
    plateMotor->setDefaultAcc(motorConfig.value("plate_default_acc").toDouble());
    rotateMotor->setDefaultAcc(motorConfig.value("rotate_default_acc").toDouble());

    // set motor default dec value from config
    glassMotor->setDefaultDec(motorConfig.value("glass_default_dec").toDouble());
    plateMotor->setDefaultDec(motorConfig.value("plate_default_dec").toDouble());
    rotateMotor->setDefaultDec(motorConfig.value("rotate_default_dec").toDouble());

    // set motor default vel value from config
    glassMotor->setDefaultVel(motorConfig.value("glass_default_vel").toDouble());
    plateMotor->setDefaultVel(motorConfig.value("plate_default_vel").toDouble());
    rotateMotor->setDefaultVel(motorConfig.value("rotate_default_vel").toDouble());

    qDebug() << "glass home: " << glassMotor->Home();
    qDebug() << "plate home: " << plateMotor->Home();
    qDebug() << "rotate home: " << rotateMotor->Home();

    qDebug() << "glass lead: " << glassMotor->Lead();
    qDebug() << "plate lead: " << plateMotor->Lead();
    qDebug() << "rotate lead: " << rotateMotor->Lead();
}
void MotorManager::saveAllMotorParms()
{
    glassMotor->saveMotorParams();
    plateMotor->saveMotorParams();
    rotateMotor->saveMotorParams();
}
/**
 * @brief Get all motor info
 * @details Get all motor UnifiedID and put them into uIDArray. If error happens, errorCode will be set.
 */
void MotorManager::getAllMotorInfo()
{
    uIDArray = pController->lookupActuators(errorCode);
    for(auto uID : uIDArray)
    {
        cout << "Actuator ID: " << (int)uID.actuatorID << " IP address: " << uID.ipAddress.c_str() << endl;
    }
}

/**
 * @brief Enable all motors
 * @details Get all motor info and enable motors in batch. If motor number is less than or equal to MOTOR_NUM, print error message.
 * If motor info error, print error message. If motor enable success, print success message and motor info.
 * If motor enable fail, print fail message.
 */
void MotorManager::enableAllMotors()
{
    getAllMotorInfo();
    if (errorCode != Actuator::ERR_NONE)
    {
        qDebug() << "Motor Error: " << errorCode ;
    }
    else
    {
        if (uIDArray.size() < motorNum)
        {
            qDebug() << "Motor Number Error: " << uIDArray.size() ;
        }
        else
        {
            if (pController->enableActuatorInBatch(uIDArray))
            {
                qDebug() << "Motor Init Success" ;
                for (auto uID : uIDArray)
                {
                    cout << "Actuator ID: " << (int)uID.actuatorID << " IP address: " << uID.ipAddress.c_str() << endl;
                }
            }
            else
            {
                qDebug() << "Motor Init Fail" ;
            }
        }
    }
}

/**
 * @brief Disable all motor
 * @return Disable all motor success return true, else return false
 */
void MotorManager::disableAllMotors()
{
    if (pController->disableAllActuators())
    {
        qDebug() << "Motor Disable Success" ;
        this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    else
    {
        qDebug() << "Motor Disable Fail" ;
    }
}

/**
 * @brief Set all motor mode
 * @param mode motor mode
 */
void MotorManager::setAllMotorMode(ActuatorMode mode)
{
    glassMotor->setMode(mode);
    plateMotor->setMode(mode);
    rotateMotor->setMode(mode);
}
/**
 * @brief Set all motor profile position maximum velocity
 * @param max_vel maximum velocity value
 */
void MotorManager::setAllMotorProfilePositionMaxVel(int max_vel)
{
    glassMotor->setProfilePositionMaxVel(max_vel);
    plateMotor->setProfilePositionMaxVel(max_vel);
    rotateMotor->setProfilePositionMaxVel(max_vel);
}
/**
 * @brief Set all motor profile position acceleration
 * @param acc acceleration value
 */
void MotorManager::setAllMotorProfilePositionAcc(int acc)
{
    glassMotor->setProfilePositionAcc(acc);
    plateMotor->setProfilePositionAcc(acc);
    rotateMotor->setProfilePositionAcc(acc);
}

/**
 * @brief set all motor profile position deceleration
 * @param dec deceleration value
 */
void MotorManager::setAllMotorProfilePositionDec(int dec)
{
    glassMotor->setProfilePositionDec(dec);
    plateMotor->setProfilePositionDec(dec);
    rotateMotor->setProfilePositionDec(dec);
}