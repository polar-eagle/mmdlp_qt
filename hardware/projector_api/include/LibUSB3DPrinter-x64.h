// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the LIBUSB3DPRINTER_EXPORTS
// symbol defined on the command line. This symbol should not be defined in any project
// that uses this DLL. This way, any other project whose source files include this file will see 
// LIBUSB3DPRINTER_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

#ifndef __LIBUSB3DPRINTER_H__
#define __LIBUSB3DPRINTER_H__

#include <stdint.h> // Ensure type definitions are based on standard headers

typedef uint16_t U16; // 16-bit unsigned integer
typedef uint8_t  U8;  // 8-bit unsigned integer
typedef int16_t  S16; // 16-bit signed integer
typedef uint32_t U32; // 32-bit unsigned integer

#ifndef Bool
typedef unsigned char Bool; // If no standard boolean type exists, define as unsigned char
#endif

#ifdef LIBUSB3DPRINTER_EXPORTS
#define LIBUSB3DPRINTER_API __declspec(dllexport)
#else
#define LIBUSB3DPRINTER_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define USB_VID 0xC251 // USB Vendor ID
#define USB_PID 0x1706 // USB Product ID
//#define USB_DEV_SERIAL_NO  "0002a0000000" // USB device serial number (if needed)

LIBUSB3DPRINTER_API void Delay10Ms(U16 m10s);
LIBUSB3DPRINTER_API void Delay1Ms(U16 ms);

LIBUSB3DPRINTER_API unsigned char GetStatus(void);
LIBUSB3DPRINTER_API Bool LedOnOff(U8 index, unsigned char flag);
LIBUSB3DPRINTER_API Bool GetCurrent(U8 index, U8 *Value);
LIBUSB3DPRINTER_API Bool SetCurrent(U8 index, U8 Value);
LIBUSB3DPRINTER_API Bool Flip(unsigned char FlipX, unsigned char FlipY);
LIBUSB3DPRINTER_API unsigned char GetProductID(unsigned char *pID);
LIBUSB3DPRINTER_API unsigned char GetSysStatus(void);
LIBUSB3DPRINTER_API unsigned char PowerOnOff(unsigned char flag);
LIBUSB3DPRINTER_API Bool GetLedDefaultStatus(U8 *flag);
LIBUSB3DPRINTER_API Bool SetLedDefaultStatus(U8 flag);
LIBUSB3DPRINTER_API Bool GetTemperature(S16 *Temperature);
LIBUSB3DPRINTER_API Bool GetUseTime(U32 *UseTime);
LIBUSB3DPRINTER_API Bool GetMyVersion(U16 *Value);
//LIBUSB3DPRINTER_API Bool ShakeHands(U8 dwPort, U32 dwBaudRate);
LIBUSB3DPRINTER_API unsigned char GetLedSourceSwitch(unsigned char *flag);
LIBUSB3DPRINTER_API Bool LedSourceSwitce(unsigned char flag);

//LIBUSB3DPRINTER_API unsigned char CheckUsbVidPid(char *DevicePath, int vendorID, int productID);
LIBUSB3DPRINTER_API unsigned char EnumUsbDevice(void); // Enumerate USB devices matching this product; returns the number of devices found
LIBUSB3DPRINTER_API void SetUsbDeviceIndex(unsigned char UsbIndex); // UsbIndex >= 0, set the USB device index. When multiple USB devices exist, use this to select which device to control
LIBUSB3DPRINTER_API void GetUsbDeviceSerial(unsigned char UsbIndex, char *SerialNo); // Return the serial number of the specified USB device index
LIBUSB3DPRINTER_API unsigned char CheckUSBOnline(void); // Check whether the currently selected USB device is disconnected

#ifdef __cplusplus
}
#endif

#endif
