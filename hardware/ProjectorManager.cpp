#include "ProjectorManager.h"
#include <iostream>
#include <QDebug>

ProjectorManager::ProjectorManager()
{
    enumUsbDevice();
    online_flag = (bool)checkUSBOnline();

    if (!online_flag)
    {
        qDebug() << "ProjectorManager is not online!" ;
    }
    else
    {
        qDebug() << "ProjectorManager is online!" ;
    }
}

ProjectorManager::~ProjectorManager()
{
}

unsigned char ProjectorManager::enumUsbDevice()
{
    return EnumUsbDevice();
}

unsigned char ProjectorManager::checkUSBOnline()
{
    return CheckUSBOnline();
}

void ProjectorManager::setCurrent(uint8_t current)
{
    SetCurrent(id, current);
}

void ProjectorManager::delayMS10x(int times)
{
    Delay10Ms(times);
}

void ProjectorManager::delayMS1x(int times)
{
    Delay1Ms(times);
}

void ProjectorManager::ledOn()
{
    LedOnOff(id, true);
}

void ProjectorManager::ledOff()
{
    LedOnOff(id, false);
}
