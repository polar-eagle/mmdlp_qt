#ifndef CONTROLTHREAD_H
#define CONTROLTHREAD_H

#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include <QString>
#include <QObject>
#include "MotorManager.h"
#include "ProjectorManager.h"
#include "CameraManager.h"
#include "ConfigManager.h"
#include "SerialSender.h"

class ControlThread : public QThread
{
    Q_OBJECT

public:
    explicit ControlThread(QObject *parent = nullptr);
    ~ControlThread();

    void setSliceName(const QString &sliceName);

    MotorManager *motorManager;
    ProjectorManager *projectorManager;
    CameraManager *cameraManager;
    SerialSender *serialSender;

    InnfosMotor *plateMotor;
    InnfosMotor *glassMotor;
    InnfosMotor *rotateMotor;
public slots:
    void receiveCommand(const QString &command);
    void transSliceName(const QString &sliceName);

protected:
    void run() override;

private:
    QMutex mutex;
    QWaitCondition condition;
    QString currentCommand;
    QString sliceName;

    void work();

signals:
    void proj(const QString &imagePath);
    void commandFinished();
    void commandDisplay(const QString &command);
};

#endif // CONTROLTHREAD_H
