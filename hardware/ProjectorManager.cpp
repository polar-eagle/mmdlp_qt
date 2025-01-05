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
/**
 * @brief 枚举USB设备
 * @return unsigned char 是否枚举成功
 */
unsigned char ProjectorManager::enumUsbDevice()
{
    return EnumUsbDevice();
}
/**
 * @brief 检查USB是否在线
 * @return unsigned char 是否在线
 */
unsigned char ProjectorManager::checkUSBOnline()
{
    return CheckUSBOnline();
}
/**
 * @brief 设置投影仪电流
 * @param current 投影仪电流
 * @return void
 */
void ProjectorManager::setCurrent(uint8_t current)
{
    SetCurrent(id, current);
}
/**
 * @brief 延时10ms
 * @param times 延时次数
 * @return void
 */
void ProjectorManager::delayMS10x(int times)
{
    Delay10Ms(times);
}
/**
 * @brief 延时1ms
 * @param times 延时次数
 * @return void
 */
void ProjectorManager::delayMS1x(int times)
{
    Delay1Ms(times);
}
/**
 * @brief 打开投影仪灯光
 * @return void
 */
void ProjectorManager::ledOn()
{
    LedOnOff(id, true);
}
/**
 * @brief 关闭投影仪灯光
 * @return void
 */
void ProjectorManager::ledOff()
{
    LedOnOff(id, false);
}
