
#include <iostream>
#include <regex>
#include <QString>
#include "main.h"
#include "divais.h"
#include "serialcom.h"
#include "sshcom.h"
#include "tcpcom.h"
#ifdef __linux
#include <dlfcn.h>
#else
#include <windows.h>
#endif

#define SOMESEC         10000
#define SOMEMSECS       512
#define TRIMEXPECT      16384
#define RECBUFF         4096
#define READMAX         256

// tty://COM,115200,8N1
Device::Device(const char* cred, const char* proc):_pc(0),_ended(false),
                                      _alive(false),
                                      _tout(2000),
                                      _binary(false),
                                      _cancall(false),
                                      _priority(64),
                                      _eol(CRLF)
{
    if(proc && *proc)
        _outproc = proc;
    if(!strncmp(cred,"tty://",6))
    {
        _pc = new SerialComm(cred + 6);
        _alive = true;
        _cancall = false;
        if(this->start_thread())
        {
            std::cout << "Cannot start threads " << CRLF;
            ::exit(errno);
        }
        while(_cancall==false)
            ::msleep(16);
    }
}

Device::~Device()
{
    _pc->close();
    signal_to_stop();
    ::msleep(2000);
    _alive=false;
    while(!_ended )
        ::msleep(100);
    delete _pc;
}

int Device::_onReceived(const uint8_t* data, int len)
{
    std::string chunk;
    if(_binary)
        chunk = ::toString(data,len);
    else
        chunk = (const char*)data;

    if(_Debug)
        std::cout << chunk << CRLF;

    if(!_outproc.empty())
    {
        return this->_notifyScript(_outproc.c_str(), chunk.c_str())==0;
    }
    return 0;
}

int Device::_notifyScript(const char* foo, const char* chunk)
{
    Penv->acquire();
    int rvg = 0;
    Sqrat::Function func = Sqrat::RootTable().GetFunction(_SC(foo));
    if(!func.IsNull())
    {
        try{
            Sqrat::SharedPtr<int> rv = func.Evaluate<int>(
                        chunk);
           // rvg = *rv.Get();
        }
        catch(Sqrat::Exception ex)
        {
            std::cout << "Error: " << ex.Message() << std::flush;
        }
    }
    return 0;
}

int Device::_notifyWaiting()
{
    Sqrat::Function func = Sqrat::RootTable().GetFunction(_SC("waiting"));
    if(!func.IsNull())
    {
        try{
            Sqrat::SharedPtr<int> rv = func.Evaluate<int>(this);
            return *rv.Get();
        }
        catch(Sqrat::Exception ex)
        {
            std::cout << "Error: " << ex.Message() << CRLF;
        }
    }
    return 0;
}

int Device::sendkey(const char* key)
{
    if(_Debug)
        std::cout << FBLU(key) << CRLF;
    bstring bin = ::toBinary(key);
    _io.putOut(bin);
    _waitCompletion();
    return 0;
}

int Device::ctrl(char key)
{
    uint8_t buf[2]= {0};
    buf[0] = key-'A'+1;
    if(_Debug)
        std::cout << FBLU("Ctrl:" << key) << std::flush;
    _io.putOut(buf,1);
    _waitCompletion();
    return 0;
}

const char* Device::buffer()
{
    return _accum.c_str();
}

const char* Device::strstr(Sqrat::Array& strings)
{
    static std::string  ret;
    time_t              expire = tick_count() + _tout;

    _accum.clear();
    while(::tick_count() < expire && __alive && _notifyWaiting()==0)
    {
        if(_io.hasInput())
        {
            size_t length = 0;
            bstring buf = _io.getIn();
            _accum.append((const char*)buf.data());
            int ns  = strings.GetSize();
            for(int i=0;i<ns;i++)
            {
                const char* token = *(strings.GetValue<const char*>(i).Get());
                length+=::strlen(token);
                if(_accum.find(token)!=std::string::npos)
                {
                    ret=token;
                    expire=::tick_count();
                    break;
                }
            }
            if(_accum.length() > TRIMEXPECT)
            {
                std::string trunc = _accum.substr(_accum.length()-length);
                _accum = trunc;
            }
        }
        ::msleep(16);
    }
    return ret.c_str();
}


int Device::settimeout(int to)
{
    _tout = to;
    _pc->setTout(to);
    return 0;
}

int Device::seteol(const char* cmd)
{
    _eol = cmd;
    return 0;
}

int Device::writeln(const char* cmd)
{
    bstring b;
    b.append((const uint8_t*)cmd, ::strlen(cmd));
    b.append((const uint8_t*)_eol.data(), _eol.length());
    _io.putOut(b);
    _waitCompletion();
    return 0;
}

int Device::puts(const char* line)
{
    bstring b;
    b.append((const uint8_t*)line, ::strlen(line));
    _io.putOut(b);
    _waitCompletion();
    return 0;
}

int Device::putsln(const char* line)
{
    return writeln(line);
}

int Device::writeml(const char* mlines)
{
    std::stringstream    s(mlines);
    std::string          line;

    while(getlinecrlf(s,line))
    {
        if(line[0]=='%') // send key
            sendkey(line.c_str()+1);
        else if(line[0]=='^') // send Ctrl Key
            ctrl(line[1]);
        else
            writeln(line.c_str());
    }
    _waitCompletion();
    return 0;
}

int Device::write(const char* cmd)
{
    if(_Debug) std::cout << FBLU(cmd) << CRLF;
    _io.putOut((uint8_t*)cmd, ::strlen(cmd));
    _waitCompletion();

    if(_Debug)
        std::cout << "SEND: " <<(const char*)cmd << CRLF;

    return 0;
}

void Device::_waitCompletion()
{
    do{
        ::msleep(SOMEMSECS);
        if(_notifyWaiting()==-1)
            break;
    }while(_io.hasOutput() && __alive);
}

int Device::setproc(const char* s)
{
    if(s)
        _outproc = s;
    else
        _outproc.clear();
    return 0;
}

int Device::strwait(const char* expr)
{
    return strwaitt(expr, _tout);
}

int Device::strwaitt(const char* expr, int to)
{
    if(_Debug) std::cout << "Expecting: " << expr << std::flush;
    time_t      expire;
    bool        infinite = false;

    _accum.clear();
    if(to==-1)
    {
        infinite = true;
        expire = ::tick_count() + SOMESEC;
    }
    else
        expire = ::tick_count() + to;

    while(::tick_count() < expire && __alive && _notifyWaiting()==0)
    {
        if(infinite)
            expire = ::tick_count()+SOMESEC;
        if(_io.hasInput())
        {
            bstring buf = _io.getIn();
            _accum.append((const char*)buf.data());
            if(expr[0] == '?')
            {
                std::regex self_regex(expr+1,
                                      std::regex_constants::ECMAScript |
                                      std::regex_constants::icase);
                if (std::regex_search(_accum, self_regex))
                {
                    return 0;
                }
            }
            else
            {
                if(_accum.find(expr) != std::string::npos)
                {
                    return 0;
                }
            }
            if(_accum.length() > TRIMEXPECT)
            {
                std::string trunc = _accum.substr(_accum.length()-::strlen(expr));
                _accum = trunc;
            }
        }
        ::msleep(SOMEMSECS);
    }
    std::cout << "..Expect: failed " << CRLF;
    return 1;
}

int Device::binwaitt(Sqrat::Array a, int to)
{
    int        sz = a.GetSize();
    uint8_t    ptr[256];
    time_t     expire;
    bool       infinite = false;
    _accum.clear();

    if(to==-1)
    {
        infinite = true;
        expire = ::tick_count() + SOMESEC;
    }
    else
        expire = ::tick_count() + to;

    a.GetArray(ptr, sz);
    while(::tick_count() < expire && __alive && _notifyWaiting()==0)
    {
        if(infinite)
            expire=::tick_count()+SOMESEC;
        _baccum.append(_io.getIn());
        if(!_baccum.empty())
        {
            if(_baccum.find(ptr) != bstring::npos)
            {
                return 0;
            }
        }
        ::msleep(100);
        if(_baccum.length() > TRIMEXPECT)
        {
            bstring trunc;
            trunc.append(_baccum.data(),_accum.length()-sz);
            _baccum = trunc;
        }
    }
    std::cout << FRED("..Expect: failed ") << CRLF;
    return 1;
}

int Device::binwait(Sqrat::Array a)
{
    return binwaitt(a, _tout);
}

int Device::any(int to)
{
    time_t  expire;
    bool    infinite = false;

    if(to==-1)
    {
        infinite = true;
        expire = ::tick_count() + SOMESEC;
    }
    else
        expire = ::tick_count() + to;

    while(::tick_count() < expire && !_io.hasInput() && __alive)
    {
        if(infinite)
            expire = ::tick_count() + SOMESEC;
        ::msleep(SOMEMSECS);
    }
    ::msleep(SOMEMSECS);
    if(!_io.hasInput())
    {
        if(_Debug)
            std::cout << FRED(" Expecting timout. No response") << CRLF;
        return 1;
    }
    return 0;
}

int Device::close()
{
    _pc->close();
    return 0;
}

Sqrat::Array Device::read(int max)
{
    if(_io.hasInput())
    {
        max = _io.inLen();
    }
    if(max)
    {
        Sqrat::Array ra(Penv->theVM(), max);
        bstring in;

        _binary = true;
        if(_io.hasInput())
        {
            in = _io.getIn();
            for(size_t idx = 0 ; idx < in.size(); idx++)
            {
                ra.Insert(0, uint8_t(in.at(idx)));
            }
            if(_Debug)
            {
                std::cout << FCYN("Rec: " << ::toString((const uint8_t*)in.data(), in.length())) << CRLF;
            }
        }
        return ra;
    }
    return Sqrat::Array(Penv->theVM(), 0);
}


const char* Device::gets()
{
    _boutret = _io.getIn();
    return (const char*)_boutret.data();
}


int Device::write(Sqrat::Array a)
{
    int     sz = a.GetSize();
    uint8_t ptr[256];
    _binary = true;
    a.GetArray(ptr, sz);
    if(_Debug)
    {
        std::string txt = ::toString(ptr, sz);
        std::cout << FBLU(txt) << CRLF;
    }
    bstring  binstr;
    binstr.append(ptr,sz);
    _io.putOut(binstr);
    this->_waitCompletion();
    return 0;
}

Sqrat::Array Device::writeread(Sqrat::Array a)
{
    return writeread(a, _tout);
}

Sqrat::Array Device::writeread(Sqrat::Array a, int to)
{
    this->write(a);
    this->any(to);
    return this->read(READMAX);
}

const char* Device::writeread(const char* s)
{
    return writeread(s, _tout);
}

const char* Device::writeread(const char* s, int to)
{
    this->write(s);
    any(to);
    return this->gets();
}

/**
 * @brief Device::noout
 * wait until for to there is no activity
 * @param to
 * @return
 */
int Device::noout(int to, int max)
{
    size_t so = 0,nsz;
    time_t tf = ::tick_count() + max;

    nsz = _io.outLen();
    do{
        ::msleep(to);
        if(nsz != so)
           so = nsz;
        else
            break;
        nsz = _io.outLen();
    }while(__alive && ::tick_count() < tf && _notifyWaiting()==0);

    return 0;
}

void Device::setpriority(int pr)
{
    _priority = pr;
}

void Device::thread_main()
{
    try{
        if(_pc->open()==0)
        {
            int      rv = 0;
            int      lop;
            bstring  temp;
            unsigned char   shot[RECBUFF];

            _cancall = true;
            while(_alive)// && _pc->isOpen())
            {
                rv = 0;
                if(_io.hasOutput())
                {
                    temp = _io.getOut();
                    rv = _pc->sendTo(temp.data(), temp.length());
                    if(rv == -1)
                    {
                        std::cout << FRED("Send failed. Abandoning comm")<<CRLF;
                        throw -1;
                    }
                    if(_Debug){
                        std::cout << "<-["<< temp.c_str() <<"]\r\n";
                    }
                }
                ::msleep(SOMEMSECS);

                int bytes  = _pc->recFrom(shot,sizeof(shot)-1);
                if(bytes == -1)
                {
                    std::cout << FRED("Connection closed ") <<CRLF;
                    throw -1;
                }
                else if(bytes > 0)
                {
                    shot[bytes] = 0;
                    if(_Debug){
                        std::cout << "->["<<shot<<"]\n" << std::flush;
                    }

                     _io.putIn(shot, bytes);
                   this->_onReceived(shot, bytes);
                }
                ::msleep(SOMEMSECS);
                if(_Debug && lop++%32==0)
                    printf(".\n");
            }
            _pc->close();
        }
    }catch(int i)
    {
        std::cout << "IO Pooling ended" <<CRLF;
    }
    if(_pc->isOpen())
        _pc->close();
    _ended=true;
}
