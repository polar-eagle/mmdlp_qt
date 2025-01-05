#ifndef PROJECTORMANAGER_H
#define PROJECTORMANAGER_H

#include <stdint.h>
#include "LibUSB3DPrinter-x64.h"
#include <cstdint>
#include <string>

class ProjectorManager
{
private:
    bool online_flag = false;
    U8 id = 0; // only have one projector now
    U8 current = 0;

public:
    ProjectorManager();
    ~ProjectorManager();

    unsigned char checkUSBOnline();
    unsigned char enumUsbDevice();

    void setCurrent(uint8_t current);
    U8 getCurrent()
    {
        GetCurrent(id, &current);
        return current;
    };
    void delayMS10x(int times);
    void delayMS1x(int times);
    void ledOn();
    void ledOff();

    static ProjectorManager *getInstance()
    {
        static ProjectorManager instance;
        return &instance;
    };
};

#endif // PROJECTORMANAGER_H
