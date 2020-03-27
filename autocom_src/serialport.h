#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <stdio.h>
#include <string.h>



#if defined(__linux__) || defined(__FreeBSD__)

#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <sys/file.h>
#include <errno.h>

#else

#include <windows.h>

#endif



class SerialPort
{
public:
    SerialPort();
    ~SerialPort();
    int OpenComport(const char*, int, const char *);
    int Read(unsigned char *, int, int to=100);
    int SendByte(unsigned char);
    int SendBuf(unsigned char *, int, int);
    void CloseComport();
    void cputs(const char *);
    int IsDCDEnabled();
    int IsCTSEnabled();
    int IsDSREnabled();
    void enableDTR();
    void disableDTR();
    void enableRTS();
    void disableRTS();
    void flushRX();
    void flushTX();
    void flushRXTX();
    int GetPortnr(const char *);
    bool isOpen();
#ifdef __linux
    struct termios _curTCSett;
    struct termios _oldTCSett;
    int            _port;
#else
    HANDLE         _port;
#endif

    int _error;
    char mode_str[128];
};

#endif // SERIALPORT_H
