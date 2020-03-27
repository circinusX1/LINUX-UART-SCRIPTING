#ifndef ICOMBASEX_H
#define ICOMBASEX_H

#include <stdint.h>

class IComm
{
public:
    IComm(){}
    IComm(const char* cs){(void)cs;}
    virtual ~IComm(){}
    virtual int open()=0;
    virtual int close()=0;
    virtual int sendTo(const uint8_t*, int)=0;
    virtual int recFrom(unsigned char* bytes, int room)=0;
    virtual void debug(int d) = 0;
    virtual void setTout(int to)=0;
    virtual bool isOpen()=0;
};


#endif // COMBASE_H
