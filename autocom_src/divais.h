#ifndef SHELL_H
#define SHELL_H

#include <vector>
#include "sqwrap.h"
#include "icombase.h"
#include "main.h"
class Comm;

typedef Comm* (*pCreCom)(const char*);
typedef void (*pDesCom)(Comm*);

class IoData
{
public:

    void putOut(const bstring& s)
    {
        AutoLock l(&_tmut);
        _out.append(s.data(), s.length());
    }

    bstring getOut()
    {
        bstring out;
        AutoLock l(&_tmut);
        if(!_out.empty())
        {
            out.append(_out.data(), _out.length());
            _out.clear();
        }
        return out;
    }

    void putIn(const uint8_t* s, size_t len)
    {
        AutoLock l(&_tmut);
        _in.append(s,len);
    }

    void putOut(const uint8_t* s, size_t len)
    {
        AutoLock l(&_tmut);
        _out.append(s,len);
    }

    void putIn(const bstring& s)
    {
        AutoLock l(&_tmut);
        _in.append(s.data(), s.length());
    }

    bstring getIn()
    {
        bstring out;
        AutoLock l(&_tmut);
        if(!_in.empty())
        {
            out.append(_in.data(), _in.length());
            _in.clear();
        }
        return out;
    }

    size_t  outLen()const
    {
        AutoLock l(&_tmut);
        return !_out.length();
    }

    size_t  inLen()const
    {
        AutoLock l(&_tmut);
        return !_in.length();
    }

    size_t  hasInput()const
    {
        AutoLock l(&_tmut);
        return !_in.empty();
    }

    size_t  hasOutput()const
    {
        AutoLock l(&_tmut);
        return !_out.empty();
    }


    bstring  _in;
    bstring  _out;
    umutex   _tmut;
    std::string  _accum;
};


class Device : public OsThread
{
public:
    Device(const char* cred, const char* messageloop);
    ~Device();
    void thread_main();

    const char* strstr(Sqrat::Array& strings);
    int settimeout(int tos);
    int writeln(const char* cmd);
    int iomode(int);
    int seteol(const char* cmd);
    int write(const char*);
    int puts(const char*);
    int putsln(const char*);
    const char* buffer();
    int writeml(const char*);
    int write(Sqrat::Array a);
    int strwaitt(const char*,int to);
    int strwait(const char*);
    int binwaitt(Sqrat::Array a, int to);
    int binwait(Sqrat::Array a);
    int any(int to);
    int noout(int to, int max);
    Sqrat::Array read(int max);
    int setbinary(int t){_binary=t; return 0;}
    Sqrat::Array writeread(Sqrat::Array a);
    Sqrat::Array writeread(Sqrat::Array a, int);
    const char* writeread(const char* s);
    const char* writeread(const char* s, int);

    const char* gets();
    int sendkey(const char*);
    int ctrl(char);
    int setproc(const char*);
    int close();
    void setpriority(int pr);

protected:
    int _onReceived(const uint8_t* data, int len);
    int _notifyScript(const char* foo, const char* chunk);
    bstring  _toBinary(const char* string);
    std::string _toString(const uint8_t* binary, int len);
    void _waitCompletion();
    int  _notifyWaiting();
private:
    std::string  _outproc;
    std::string  _outret;
    bstring  _boutret;

    bstring     _tmp;
    IComm*      _pc;
    bool        _ended;
    bool        _alive;
    int         _tout;
    int         _binary;
    bool        _cancall;
    int         _priority;
    std::string _eol;
    bstring     _baccum;
    std::string     _accum;
    IoData      _io;
};

inline std::istream& getlinecrlf(std::istream& is, std::string& t)
{
    t.clear();
    std::istream::sentry se(is, true);
    std::streambuf* sb = is.rdbuf();

    for(;;) {
        int c = sb->sbumpc();
        switch (c) {
        case '\n':
            return is;
        case '\r':
            if(sb->sgetc() == '\n')
                sb->sbumpc();
            return is;
        case EOF:
            // Also handle the case when the last line has no line ending
            if(t.empty())
                is.setstate(std::ios::eofbit);
            return is;
        default:
            t += (char)c;
        }
    }
}


#endif // SHELL_H
