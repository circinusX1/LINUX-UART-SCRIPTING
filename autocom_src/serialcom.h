#ifndef SERIALCOM_H
#define SERIALCOM_H

#include "icombase.h"

#include "serialport.h"


class SerialComm : public IComm
{
public:
    SerialComm();
    SerialComm(const char* cred);
    virtual ~SerialComm();

    int open();
    int close();
    int sendTo(const uint8_t*, int);
    int recFrom(unsigned char* buff, int bytes);
    void debug(int d);
    void setTout(int to);
    bool isOpen();

private:
    char        _credentials[128];
    SerialPort  _device;
    int         _Debug;
    int         _timout;
};


#endif // SERIALCOM_H
