
#include <iostream>
#include "main.h"
#include "serialcom.h"

SerialComm::SerialComm()
{

}

SerialComm::SerialComm(const char* cred):IComm(cred)
{
    //COM,115200,8N1
    _timout = 64;
    ::strcpy(_credentials, cred);
}

SerialComm::~SerialComm()
{
    close();
}

int SerialComm::open()
{
    const char* dev = ::strtok(_credentials,",");
    int speed = ::atoi(::strtok(0,","));
    const char* mode = ::strtok(0,",");
    return _device.OpenComport(dev, speed, mode);
}

int SerialComm::close()
{
    if(_device._port!=0)
        _device.CloseComport();
    _device._port = 0;
    return 0;
}

bool SerialComm::isOpen()
{
#ifdef __linux
    return _device._port > 0;
#else
    return _device._port > (HANDLE)0;
#endif
}

int SerialComm::sendTo(const uint8_t* data, int length)
{
    if(_device.isOpen())
        return _device.SendBuf((unsigned char*)data, length, _timout);
    return -1;
}

int SerialComm::recFrom(unsigned char* buff, int bytes)
{
    if(_device.isOpen())
        return _device.Read(buff,bytes,_timout);
    return -1;
}

void SerialComm::debug(int d){_Debug=d;}
void SerialComm::setTout(int to){_timout = to;}
