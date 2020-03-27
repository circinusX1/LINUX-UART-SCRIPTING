
#include "osthread.h"
#include "serialport.h"

SerialPort::SerialPort()
{

}

SerialPort::~SerialPort()
{
    this->CloseComport();
}

bool SerialPort::isOpen()
{
    return _port>0;
}

#ifdef __linux

int SerialPort::OpenComport(const char* comname, int baudrate, const char *mode)
{
    int baudr,status;

    switch(baudrate)
    {
    case      50 : baudr = B50;
        break;
    case      75 : baudr = B75;
        break;
    case     110 : baudr = B110;
        break;
    case     134 : baudr = B134;
        break;
    case     150 : baudr = B150;
        break;
    case     200 : baudr = B200;
        break;
    case     300 : baudr = B300;
        break;
    case     600 : baudr = B600;
        break;
    case    1200 : baudr = B1200;
        break;
    case    1800 : baudr = B1800;
        break;
    case    2400 : baudr = B2400;
        break;
    case    4800 : baudr = B4800;
        break;
    case    9600 : baudr = B9600;
        break;
    case   19200 : baudr = B19200;
        break;
    case   38400 : baudr = B38400;
        break;
    case   57600 : baudr = B57600;
        break;
    case  115200 : baudr = B115200;
        break;
    case  230400 : baudr = B230400;
        break;
    case  460800 : baudr = B460800;
        break;
    case  500000 : baudr = B500000;
        break;
    case  576000 : baudr = B576000;
        break;
    case  921600 : baudr = B921600;
        break;
    case 1000000 : baudr = B1000000;
        break;
    case 1500000 : baudr = B1500000;
        break;
    case 2000000 : baudr = B2000000;
        break;
    case 2500000 : baudr = B2500000;
        break;
    case 3000000 : baudr = B3000000;
        break;
    case 3500000 : baudr = B3500000;
        break;
    case 4000000 : baudr = B4000000;
        break;
    default      : printf("invalid baudrate\n");
        return(1);
        break;
    }

    int cbits=CS8,
            cpar=0,
            ipar=IGNPAR,
            bstop=0;

    if(strlen(mode) != 3)
    {
        printf("invalid mode \"%s\"\n", mode);
        return(1);
    }

    switch(mode[0])
    {
    case '8': cbits = CS8;
        break;
    case '7': cbits = CS7;
        break;
    case '6': cbits = CS6;
        break;
    case '5': cbits = CS5;
        break;
    default : printf("invalid number of data-bits '%c'\n", mode[0]);
        return(1);
        break;
    }

    switch(mode[1])
    {
    case 'N':
    case 'n': cpar = 0;
        ipar = IGNPAR;
        break;
    case 'E':
    case 'e': cpar = PARENB;
        ipar = INPCK;
        break;
    case 'O':
    case 'o': cpar = (PARENB | PARODD);
        ipar = INPCK;
        break;
    default : printf("invalid parity '%c'\n", mode[1]);
        return(1);
        break;
    }

    switch(mode[2])
    {
    case '1': bstop = 0;
        break;
    case '2': bstop = CSTOPB;
        break;
    default : printf("invalid number of stop bits '%c'\n", mode[2]);
        return(1);
        break;
    }

    /*
http://pubs.opengroup.org/onlinepubs/7908799/xsh/termios.h.html

http://man7.org/linux/man-pages/man3/termios.3.html
*/

    _port = open(comname, O_RDWR | O_NOCTTY | O_NDELAY);
    if(_port==-1)
    {
        perror("unable to open com_port ");
        return(_port);
    }

    /* lock access so that another process can't also use the _port */
    if(flock(_port, LOCK_EX | LOCK_NB) != 0)
    {
        close(_port);
        perror("Another process has locked the com");
        return(1);
    }

    _error = tcgetattr(_port, &_oldTCSett);
    if(_error==-1)
    {
        close(_port);
        flock(_port, LOCK_UN);  /* free the _port so that others can use it. */
        perror("unable to read _portsettings ");
        return(1);
    }
    memset(&_curTCSett, 0, sizeof(_curTCSett));  /* clear the new struct */

    _curTCSett.c_cflag = cbits | cpar | bstop | CLOCAL | CREAD;
    _curTCSett.c_iflag = ipar;
    _curTCSett.c_oflag = 0;
    _curTCSett.c_lflag = 0;
    _curTCSett.c_cc[VMIN] = 0;      /* block untill n bytes are received */
    _curTCSett.c_cc[VTIME] = 0;     /* block untill a timer expires (n * 100 mSec.) */

    cfsetispeed(&_curTCSett, baudr);
    cfsetospeed(&_curTCSett, baudr);

    _error = tcsetattr(_port, TCSANOW, &_curTCSett);
    if(_error==-1)
    {
        tcsetattr(_port, TCSANOW, &_oldTCSett);
        close(_port);
        flock(_port, LOCK_UN);  /* free the _port so that others can use it. */
        _port=0;
        perror("unable to adjust _portsettings ");
        return(1);
    }

    /* http://man7.org/linux/man-pages/man4/tty_ioctl.4.html */

    if(ioctl(_port, TIOCMGET, &status) == -1)
    {
        tcsetattr(_port, TCSANOW, &_oldTCSett);
        flock(_port, LOCK_UN);  /* free the _port so that others can use it. */
        perror("unable to get _portstatus 1");
        return(1);
    }

    status |= TIOCM_DTR;    /* turn on DTR */
    status |= TIOCM_RTS;    /* turn on RTS */

    if(ioctl(_port, TIOCMSET, &status) == -1)
    {
        tcsetattr(_port, TCSANOW, &_oldTCSett);
        flock(_port, LOCK_UN);  /* free the _port so that others can use it. */
        perror("unable to set _portstatus");
        return(1);
    }

    return(0);
}

int SerialPort::Read(unsigned char *buf, int size, int to)
{
    int got = 0;
    struct timeval tv = { 0L, to*1000 };
    fd_set fds;

    FD_ZERO(&fds);
    FD_SET(_port, &fds);
    tv.tv_sec = 0;
    tv.tv_usec = to+8;
    if(::select(_port+1, &fds, NULL, NULL, &tv)>0 &&
       FD_ISSET(_port, &fds))
    {
        int nbytes = ::read(_port, buf+got, size-got);
        if(nbytes > 0)
        {
            buf[nbytes] = 0;
            return nbytes;
        }
    }
    return 0;
}


int SerialPort::SendByte(unsigned char byte)
{
    int n = write(_port, &byte, 1);
    if(n < 0)
    {
        if(errno == EAGAIN)
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }

    return(0);
}


int SerialPort::SendBuf(unsigned char *buf, int size, int to)
{
    size_t ct = ::tick_count() + to;
    int left = size;
    int sent = 0;
    do{
        int n = ::write(_port, buf+sent, left);
        if(n < 0)
        {
            if(errno == EAGAIN)
            {
                ::msleep(64);
                continue;
            }
            else
            {
                return -1;
            }
        }
        if( n > 0)
        {
            left -= n;
        }
    }while(left>0 && ::tick_count() < ct);

    return (left==0) ? 0 : 1/*tout*/;
}


void SerialPort::CloseComport()
{
    int status;

    ::msleep(100);
    this->flushRXTX();
    ::msleep(100);
    if(ioctl(_port, TIOCMGET, &status) == -1)
    {
        //perror("unable to get _portstatus Closing");
    }
    else
    {
        status &= ~TIOCM_DTR;    /* turn off DTR */
        status &= ~TIOCM_RTS;    /* turn off RTS */

        if(ioctl(_port, TIOCMSET, &status) == -1)
        {
            perror("unable to set _portstatus");
        }
    }

    tcsetattr(_port, TCSANOW, &_oldTCSett);
    ::close(_port);
    flock(_port, LOCK_UN);  /* free the _port so that others can use it. */
    _port = 0;
}

/*
Constant  Description
TIOCM_LE        DSR (data set ready/line enable)
TIOCM_DTR       DTR (data terminal ready)
TIOCM_RTS       RTS (request to send)
TIOCM_ST        Secondary TXD (transmit)
TIOCM_SR        Secondary RXD (receive)
TIOCM_CTS       CTS (clear to send)
TIOCM_CAR       DCD (data carrier detect)
TIOCM_CD        see TIOCM_CAR
TIOCM_RNG       RNG (ring)
TIOCM_RI        see TIOCM_RNG
TIOCM_DSR       DSR (data set ready)

http://man7.org/linux/man-pages/man4/tty_ioctl.4.html
*/

int SerialPort::IsDCDEnabled()
{
    int status;

    ioctl(_port, TIOCMGET, &status);

    if(status&TIOCM_CAR) return(1);
    else return(0);

}


int SerialPort::IsCTSEnabled()
{
    int status;

    ioctl(_port, TIOCMGET, &status);

    if(status&TIOCM_CTS) return(1);
    else return(0);
}


int SerialPort::IsDSREnabled()
{
    int status;

    ioctl(_port, TIOCMGET, &status);

    if(status&TIOCM_DSR) return(1);
    else return(0);
}


void SerialPort::enableDTR()
{
    int status;

    if(ioctl(_port, TIOCMGET, &status) == -1)
    {
        perror("unable to get _portstatus 3");
    }

    status |= TIOCM_DTR;    /* turn on DTR */

    if(ioctl(_port, TIOCMSET, &status) == -1)
    {
        perror("unable to set _portstatus");
    }
}


void SerialPort::disableDTR()
{
    int status;

    if(ioctl(_port, TIOCMGET, &status) == -1)
    {
        perror("unable to get _portstatus 4");
    }

    status &= ~TIOCM_DTR;    /* turn off DTR */

    if(ioctl(_port, TIOCMSET, &status) == -1)
    {
        perror("unable to set _portstatus");
    }
}


void SerialPort::enableRTS()
{
    int status;

    if(ioctl(_port, TIOCMGET, &status) == -1)
    {
        perror("unable to get _portstatus 5");
    }

    status |= TIOCM_RTS;    /* turn on RTS */

    if(ioctl(_port, TIOCMSET, &status) == -1)
    {
        perror("unable to set _portstatus");
    }
}


void SerialPort::disableRTS()
{
    int status;

    if(ioctl(_port, TIOCMGET, &status) == -1)
    {
        perror("unable to get _portstatus 6");
    }

    status &= ~TIOCM_RTS;    /* turn off RTS */

    if(ioctl(_port, TIOCMSET, &status) == -1)
    {
        perror("unable to set _portstatus");
    }
}


void SerialPort::flushRX()
{
    tcflush(_port, TCIFLUSH);
}


void SerialPort::flushTX()
{
    tcflush(_port, TCOFLUSH);
}


void SerialPort::flushRXTX()
{
    tcflush(_port, TCIOFLUSH);
}


#else  /* windows */

//#define SerialPort::_portNR  16

//HANDLE C_port[SerialPort::_portNR];

/*
char *com_ports[SerialPort::_portNR]={"\\\\.\\COM1",  "\\\\.\\COM2",  "\\\\.\\COM3",  "\\\\.\\COM4",
                              "\\\\.\\COM5",  "\\\\.\\COM6",  "\\\\.\\COM7",  "\\\\.\\COM8",
                              "\\\\.\\COM9",  "\\\\.\\COM10", "\\\\.\\COM11", "\\\\.\\COM12",
                              "\\\\.\\COM13", "\\\\.\\COM14", "\\\\.\\COM15", "\\\\.\\COM16"};
*/



int SerialPort::OpenComport(const char* comnamep, int baudrate, const char *mode)
{
    char comname[128];

    ::sprintf(comname,"\\\\.\\%s",comnamep);


    switch(baudrate)
    {
    case     110 : strcpy(mode_str, "baud=110");
        break;
    case     300 : strcpy(mode_str, "baud=300");
        break;
    case     600 : strcpy(mode_str, "baud=600");
        break;
    case    1200 : strcpy(mode_str, "baud=1200");
        break;
    case    2400 : strcpy(mode_str, "baud=2400");
        break;
    case    4800 : strcpy(mode_str, "baud=4800");
        break;
    case    9600 : strcpy(mode_str, "baud=9600");
        break;
    case   19200 : strcpy(mode_str, "baud=19200");
        break;
    case   38400 : strcpy(mode_str, "baud=38400");
        break;
    case   57600 : strcpy(mode_str, "baud=57600");
        break;
    case  115200 : strcpy(mode_str, "baud=115200");
        break;
    case  128000 : strcpy(mode_str, "baud=128000");
        break;
    case  256000 : strcpy(mode_str, "baud=256000");
        break;
    case  500000 : strcpy(mode_str, "baud=500000");
        break;
    case 100000 : strcpy(mode_str, "baud=100000");
        break;
    default      : printf("invalid baudrate\n");
        return(1);
        break;
    }

    if(strlen(mode) != 3)
    {
        printf("invalid mode \"%s\"\n", mode);
        return(1);
    }

    switch(mode[0])
    {
    case '8': strcat(mode_str, " data=8");
        break;
    case '7': strcat(mode_str, " data=7");
        break;
    case '6': strcat(mode_str, " data=6");
        break;
    case '5': strcat(mode_str, " data=5");
        break;
    default : printf("invalid number of data-bits '%c'\n", mode[0]);
        return(1);
        break;
    }

    switch(mode[1])
    {
    case 'N':
    case 'n': strcat(mode_str, " parity=n");
        break;
    case 'E':
    case 'e': strcat(mode_str, " parity=e");
        break;
    case 'O':
    case 'o': strcat(mode_str, " parity=o");
        break;
    default : printf("invalid parity '%c'\n", mode[1]);
        return(1);
        break;
    }

    switch(mode[2])
    {
    case '1': strcat(mode_str, " stop=1");
        break;
    case '2': strcat(mode_str, " stop=2");
        break;
    default : printf("invalid number of stop bits '%c'\n", mode[2]);
        return(1);
        break;
    }

    strcat(mode_str, " dtr=on rts=on");

    /*
http://msdn.microsoft.com/en-us/library/windows/desktop/aa363145%28v=vs.85%29.aspx

http://technet.microsoft.com/en-us/library/cc732236.aspx
*/

    _port = CreateFileA(comname,
                                        GENERIC_READ|GENERIC_WRITE,
                                        0,                          /* no share  */
                                        NULL,                       /* no security */
                                        OPEN_EXISTING,
                                        0,                          /* no threads */
                                        0);                      /* no templates */

    if(_port==0)
    {
        printf("unable to open com_port\n");
        return(1);
    }

    DCB _port_settings;
    memset(&_port_settings, 0, sizeof(_port_settings));  /* clear the new struct  */
    _port_settings.DCBlength = sizeof(_port_settings);

    if(!BuildCommDCBA(mode_str, &_port_settings))
    {
        printf("unable to set com_port dcb settings\n");
        CloseHandle(_port);
        _port=0;
        return(1);
    }

    if(!SetCommState(_port, &_port_settings))
    {
        printf("unable to set com_port cfg settings\n");
        CloseHandle(_port);
        _port=0;
        return(1);
    }

    COMMTIMEOUTS Cptimeouts;

    Cptimeouts.ReadIntervalTimeout         = MAXDWORD;
    Cptimeouts.ReadTotalTimeoutMultiplier  = 0;
    Cptimeouts.ReadTotalTimeoutConstant    = 0;
    Cptimeouts.WriteTotalTimeoutMultiplier = 0;
    Cptimeouts.WriteTotalTimeoutConstant   = 0;

    if(!SetCommTimeouts(_port, &Cptimeouts))
    {
        printf("unable to set com_port time-out settings\n");
        CloseHandle(_port);
        _port=0;
        return(1);
    }

    return(0);
}


int SerialPort::Read(unsigned char *buf, int size, int to)
{
    DWORD n=0;
    DWORD dw = 0;
    COMMTIMEOUTS tou;
    GetCommTimeouts(_port, &tou);
    tou.ReadIntervalTimeout = to;
    tou.ReadTotalTimeoutMultiplier = 0;
    tou.ReadTotalTimeoutConstant = 1;
    tou.WriteTotalTimeoutMultiplier = 1;
    tou.WriteTotalTimeoutConstant = 1;
    SetCommTimeouts(_port, &tou);
    ReadFile(_port, buf, size, (LPDWORD)((void *)&n), NULL);
    if(n)
        printf("<- received %d bytes \r\n",n);
    return n;
}


int SerialPort::SendByte(unsigned char byte)
{
    int n;

    WriteFile(_port, &byte, 1, (LPDWORD)((void *)&n), NULL);

    if(n<0)  return(1);

    return(0);
}


int SerialPort::SendBuf(unsigned char *buf, int bytes, int to)
{
    DWORD n;
    int    left = bytes;
    int    written = 0;

    DWORD ct = GetTickCount() + to;
    do{

        if(WriteFile(_port, buf+written, left, (LPDWORD)((void *)&n), NULL)==FALSE)
            break;
        left-=n;
         ::msleep(64);
    }while(GetTickCount()<ct && left>0);

    if(left==0)
        printf("-> sent %d bytes \r\n",bytes);


    return left;
}


void SerialPort::CloseComport()
{
    CloseHandle(_port);
    _port=0;
}

/*
http://msdn.microsoft.com/en-us/library/windows/desktop/aa363258%28v=vs.85%29.aspx
*/

int SerialPort::IsDCDEnabled()
{
    int status;

    GetCommModemStatus(_port, (LPDWORD)((void *)&status));

    if(status&MS_RLSD_ON) return(1);
    else return(0);
}


int SerialPort::IsCTSEnabled()
{
    int status;

    GetCommModemStatus(_port, (LPDWORD)((void *)&status));

    if(status&MS_CTS_ON) return(1);
    else return(0);
}


int SerialPort::IsDSREnabled()
{
    int status;

    GetCommModemStatus(_port, (LPDWORD)((void *)&status));

    if(status&MS_DSR_ON) return(1);
    else return(0);
}


void SerialPort::enableDTR()
{
    EscapeCommFunction(_port, SETDTR);
}


void SerialPort::disableDTR()
{
    EscapeCommFunction(_port, CLRDTR);
}


void SerialPort::enableRTS()
{
    EscapeCommFunction(_port, SETRTS);
}


void SerialPort::disableRTS()
{
    EscapeCommFunction(_port, CLRRTS);
}

/*
https://msdn.microsoft.com/en-us/library/windows/desktop/aa363428%28v=vs.85%29.aspx
*/

void SerialPort::flushRX()
{
    PurgeComm(_port, PURGE_RXCLEAR | PURGE_RXABORT);
}


void SerialPort::flushTX()
{
    PurgeComm(_port, PURGE_TXCLEAR | PURGE_TXABORT);
}


void SerialPort::flushRXTX()
{
    PurgeComm(_port, PURGE_RXCLEAR | PURGE_RXABORT);
    PurgeComm(_port, PURGE_TXCLEAR | PURGE_TXABORT);
}


#endif


void SerialPort::cputs(const char *text)  /* sends a string to serial _port */
{
    while(*text != 0)
        SendByte(*(text++));
}

