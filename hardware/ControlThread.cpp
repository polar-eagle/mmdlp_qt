#include "ControlThread.h"
#include <QDebug>
#include <QThread>
#include <QEventLoop>

ControlThread::ControlThread(QObject *parent)
    : QThread(parent)
{
    projectorManager = ProjectorManager::getInstance();

    motorManager = new MotorManager();
    plateMotor = motorManager->PlateMotor();
    glassMotor = motorManager->GlassMotor();
    rotateMotor = motorManager->RotateMotor();

    serialSender = new SerialSender();
    serialSender->openPort("COM3", 9600);

    cameraManager = new CameraManager();
    cameraManager->setCameraIndex(0);

    cameraManager->openCamera();
}
ControlThread::~ControlThread()
{
    delete motorManager;
    delete serialSender;
    delete projectorManager;
}

/**
 * @brief 接收各个地方来的命令
 * @param command 命令
 * @return void
 */
void ControlThread::receiveCommand(const QString &command)
{
    QMutexLocker locker(&mutex);
    this->currentCommand = command;
}

/**
 * @brief 接收切片名称
 * @param sliceName 切片名称
 * @return void
 */
void ControlThread::setSliceName(const QString &sliceName)
{
    this->sliceName = sliceName;
}

/**
 * @brief 控制线程
 * @return void
 */
void ControlThread::run()
{
    while (true)
    {
        {
            QMutexLocker locker(&mutex);
            if (currentCommand.isEmpty())
            {
                continue;
            }
        }
        work();
        {
            QMutexLocker locker(&mutex);
            currentCommand.clear();
        }
        emit commandFinished();
    }
}

/**
 * @brief 根据命令执行相应操作
 * @return void
 * @note 通过串口发送数据，控制电机运动，投影仪显示，相机拍照等
 */
void ControlThread::work()
{
    if (currentCommand.isEmpty())
    {
        return;
    }

    qDebug() << "Received command:" << currentCommand;
    emit commandDisplay(currentCommand);
    QStringList line = currentCommand.split(" ");

    if (line[0] == "tank")
    {

        plateMotor->moveToEEPotision(plateMotor->MaxPosition());
        glassMotor->moveToEEPotision(glassMotor->MinPosition());
        double pos = line[1].toDouble();
        // qDebug()<<"pos1: "<<pos;
        if (line.size() > 2)
        {
            double acc = line[2].toDouble();
            double dec = line[3].toDouble();
            double speed = line[4].toDouble();
            rotateMotor->setSpeed(acc, dec, speed);
        }
        pos = qBound<double>(rotateMotor->MinPosition(), pos, rotateMotor->MaxPosition());
        // qDebug()<<"pos2: "<<pos;
        rotateMotor->moveToEEPotision(pos);
    }
    else if (line[0] == "fan")
    {
        if (line[1] == "open")
        {
            serialSender->sendData("a");
        }
        else if (line[1] == "close")
        {
            serialSender->sendData("b");
        }
    }
    else if (line[0] == "clean")
    {
        if (line[1] == "open")
        {
            serialSender->sendData("c");
        }
        else if (line[1] == "close")
        {
            serialSender->sendData("d");
        }
    }
    else if (line[0] == "glass")
    {
        double pos = line[1].toDouble();
        if (line.size() > 2)
        {
            double acc = line[2].toDouble();
            double dec = line[3].toDouble();
            double speed = line[4].toDouble();
            glassMotor->setSpeed(acc, dec, speed);
        }
        pos = qBound<double>(glassMotor->MinPosition(), pos, glassMotor->MaxPosition());
        glassMotor->moveToEEPotision(pos);
    }
    else if (line[0] == "plate")
    {
        double pos = line[1].toDouble();
        if (line.size() > 2)
        {
            double acc = line[2].toDouble();
            double dec = line[3].toDouble();
            double speed = line[4].toDouble();
            plateMotor->setSpeed(acc, dec, speed);
        }
        pos = qBound<double>(plateMotor->MinPosition(), pos, plateMotor->MaxPosition());
        plateMotor->moveToEEPotision(pos);
    }
    else if (line[0] == "proj")
    {
        if (line.size() > 3)
        {
            QString path = sliceName + "/" + line[1];
            float displayTime = line[2].toFloat();
            int cur = line[3].toInt();
            emit proj(path);
            QThread::msleep(200);
            projectorManager->setCurrent(cur);
            projectorManager->ledOn();
            QThread::msleep(static_cast<int>(displayTime * 1000));
            projectorManager->ledOff();
            emit proj("closeWindow");
        }
    }
    else if (line[0] == "wait")
    {
        QThread::msleep(static_cast<int>(line[1].toFloat() * 1000));
    }
    else if (line[0] == "capture")
    {
        QString captureName = sliceName + "/" + line[1] + ".png";
        cameraManager->captureImage(captureName.toStdString());
    }
    else if (line[0] == "projector_close")
    {
        projectorManager->ledOff();
    }
    else if (line[0] == "home")
    {
        plateMotor->moveToEEPotision(0);
        glassMotor->moveToEEPotision(0);
    }
    else if (line[0] == "AMS")
    {
        int materialIndex = line[1].toInt();
        int materialNum = line[3].toInt();
        serialSender->sendData("e");
        // plateMotor->moveToEEPotision(plateMotor->MaxPosition());
        // glassMotor->moveToEEPotision(glassMotor->MinPosition());
        if (line[2] == "backflow")
        {
            switch (materialIndex)
            {
            case 0:
                rotateMotor->moveToEEPotision(2.5);
                for (int i = 0; i < materialNum; i++)
                    serialSender->sendData("g");
                break;
            case 1:
                for (int i = 0; i < materialNum; i++)
                    serialSender->sendData("h");
                break;
            case 2:
                for (int i = 0; i < materialNum; i++)
                    serialSender->sendData("i");
                break;
            }
        }
        else if (line[2] == "feed")
        {
            switch (materialIndex)
            {
            case 0:
                rotateMotor->moveToEEPotision(2.5);
                for (int i = 0; i < materialNum; i++)
                    serialSender->sendData("g");
                break;
            case 1:
                for (int i = 0; i < materialNum; i++)
                    serialSender->sendData("h");
                break;
            case 2:
                for (int i = 0; i < materialNum; i++)
                    serialSender->sendData("i");
                break;
            }
        }
        serialSender->sendData("f");
    }
    else if (line[0] == "ASS")
    {
        serialSender->sendData("m");
        if (line[1] == "input")
        {
            for (int i=0;i<line[2].toInt();i++)
                serialSender->sendData("o");
        }
        else if (line[1] == "output")
        {
            for (int i=0;i<line[2].toInt();i++)
                serialSender->sendData("p");
        }
        serialSender->sendData("n");
    }
    else if (line[0] == "plateEnable")
    {
        plateMotor->enable();
    }
    else if (line[0] == "plateDisable")
    {
        plateMotor->disable();
    }
    else if (line[0] == "glassEnable")
    {
        glassMotor->enable();
    }
    else if (line[0] == "glassDisable")
    {
        glassMotor->disable();
    }
    else if (line[0] == "rotateEnable")
    {
        rotateMotor->enable();
    }
    else if (line[0] == "rotateDisable")
    {
        rotateMotor->disable();
    }
}

/**
 * @brief 根据printTab发送的切片名称设置切片名称
 */
void ControlThread::transSliceName(const QString &sliceName)
{
    setSliceName(sliceName);
}