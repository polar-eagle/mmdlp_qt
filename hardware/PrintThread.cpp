#include "PrintThread.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

PrintThread::PrintThread(QObject *parent)
    : QThread(parent), stopRequested(false) {}

void PrintThread::setSliceName(const QString &name)
{
    QMutexLocker locker(&mutex);
    sliceName = name;
}


void PrintThread::sendCommand()
{
    if (!isPrinting)
    {
        return;
    }
    mutex.lock();
    if (commandIndex >= commandList.size())
    {
        isPrinting = false;
        emit finished();
        mutex.unlock();
        return;
    }  
    mutex.unlock();
    stopMutex.lock();
    if (stopRequested)
    {
        stopMutex.unlock();
        return;
    }
    stopMutex.unlock();
    mutex.lock();
    emit transitCommand(commandList[commandIndex++]);
    mutex.unlock();
}
void PrintThread::run()
{
    commandList.clear();
    if (sliceName.isEmpty())
        return;

    QFile file(sliceName + "/run.gcode");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        commandList.push_back(line);
    }
    commandIndex = 0;

    emit transSliceName(sliceName);
    sendCommand();
}
