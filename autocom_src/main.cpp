

#include <unistd.h>
#include <string>
#include <deque>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "sqratext.h"
#include "sqwrap.h"
#include "sqratConst.h"
#include "icombase.h"
#include "divais.h"
#include "serialcom.h"
#include "sshcom.h"
#include "tcpcom.h"
#include "main.h"
#ifdef __linux
//#include <readline/readline.h>
//#include <readline/history.h>
#include <sys/ioctl.h>
#include <termios.h>
#endif


bool __alive;
SqEnv* Penv;
std::string kbhit();

bool    sexecute(std::vector<std::string> & prog);

int Run()
{
   while(sq_getvmstate(Penv->theVM()) != SQ_VMSTATE_SUSPENDED && __alive)
   {
       ::msleep(256);
   }
   return 0;
}

void Exit()
{
    __alive=false;
}

void XSleep(int t)
{
    ::msleep(t);
}

const char* num2str(int hex, int bytes)
{
    static char out[32];
    switch(bytes)
    {
        case 1:
            ::sprintf(out,"%02X", hex);
            break;
        case 2:
            ::sprintf(out,"%04X", hex);
            break;
        case 4:
            ::sprintf(out,"%08X", hex);
            break;
        case 8:
            ::sprintf(out,"%016X", hex);
            break;
    }
    if(_Debug)
        std::cout << FCYN(out)<< CRLF;

    return out;
}

const char* arr2str(Sqrat::Array a)
{
    static std::string out;

    int     sz = a.GetSize();
    uint8_t ptr[4096];
    char    duplet[4];

    out.clear();
    a.GetArray(ptr, sz);
    for(int i=0;i<sz;i++)
    {
        ::sprintf(duplet,"%02X",int(ptr[i]));
        out.append(duplet);
    }
    if(_Debug) std::cout << out << CRLF;
    return out.c_str();
}

int str2num(const char* str)
{
    int val = (int)strtol(str, NULL, 0);
    if(_Debug)
        std::cout << FCYN(val) <<std::hex <<", "<< FCYN(val)<<std::dec << CRLF;

    return val;
}

std::string  toString(const uint8_t* binary, int len)
{
    std::string chunk;
    char        duplet[4];

    for(int i=0;i<len;i++)
    {
        ::sprintf(duplet,"%02X",int(binary[i]));
        chunk.append(duplet);
    }
    return chunk;
}

bstring  toBinary(const char* string)
{
    bstring  binstr;
    int      val;
    char     tmp[4]={0};

    size_t l = ::strlen(string);
    for(size_t c=0;c<l;c+=2)
    {
        ::strncpy(tmp,&string[c],2);
        ::sscanf(tmp,"%02X",&val);
        binstr.append(1, char(val));
    }
    if(_Debug)
        std::cout << FCYN(string)<< CRLF;
    return binstr;
}

Sqrat::Array str2arr(const char* str)
{
    char         tmp[4]={0};
    int          val;
    size_t       sl = ::strlen(str);
    Sqrat::Array ra(Penv->theVM(),sl/2);

    for(size_t idx = 0 ; idx < sl; idx+=2)
    {
        ::strncpy(tmp, &str[idx],2);
        ::sscanf(tmp,"%02X",&val);
        ra.Insert(idx, uint8_t(val));
    }
    if(_Debug)
        std::cout << FCYN(str) << CRLF;
    return ra;
}


Sqrat::Array num2arr(int hex, int bytes)
{
    char out[32];

    switch(bytes)
    {
        case 1:
            ::sprintf(out,"%02X", hex);
            break;
        case 2:
            ::sprintf(out,"%04X", hex);
            break;
        case 4:
            ::sprintf(out,"%08X", hex);
            break;
        case 8:
            ::sprintf(out,"%016X", hex);
            break;
    }
    if(_Debug)
        std::cout << FCYN(out)<< CRLF;
    return str2arr(out);
}


int _Debug;

static void Debug(int deb)
{
    _Debug = deb;
}

static void Delete(Device* p)
{
    delete p;
}

int main(int argc, char *argv[])
{
//    QCoreApplication a(argc, argv);
    SqEnv sq;

    __alive=true;
    sq.acquire();
    Penv = &sq;
    Sqrat::ConstTable(sq.theVM())
            .Const("ESC", "1B")
            .Const("F1", "1B4F50")
            .Const("UP", "1B5B41")
            .Const("DOWN", "1B5B42")
            .Const("LEFT", "1B5B44")
            .Const("RIGHT", "1B5B43")
            .Const("DEL", "1B5B33")
            .Const("INS", "1B5B32")
            .Const("END", "1B4F46")
            .Const("HOME", "1B4F48")
            .Const("ENTER", "0D")
            .Const("RETURN", "0A")
            .Const("CONTINUE", 0)
            .Const("EXIT",    -1)
            .Const("PROCESSED", 1)
            .Const("SYNC", 1)
            .Const("ASYNC", 2);

    Sqrat::RootTable(sq.theVM());

    Sqrat::RootTable(sq.theVM()).Func("debug", &Debug);
    Sqrat::RootTable(sq.theVM()).Func("exit", &Exit);
    Sqrat::RootTable(sq.theVM()).Func("delete", &Delete);
    Sqrat::RootTable(sq.theVM()).Func("sleep", &XSleep);
    Sqrat::RootTable(sq.theVM()).Func("str2num", &str2num);   // "0x12" - > 18
    Sqrat::RootTable(sq.theVM()).Func("arr2str", &arr2str);   // [1,3,4] - > 010204
    Sqrat::RootTable(sq.theVM()).Func("str2arr", &str2arr);   // "01020304" - > [1,2,4,4]
    Sqrat::RootTable(sq.theVM()).Func("num2str", &num2str);   // 5,2 - > 05,  5,4 - > 0005
    Sqrat::RootTable(sq.theVM()).Func("num2arr", &num2arr);   // 512,4 - > [0,0,1,ff]



    Sqrat::Class<Device> cls(sq.theVM(), _SC("Device"));
    cls.Ctor<const char*, const char*>();
    cls.Func(_SC("seteol"), &Device::seteol);
    cls.Func(_SC("putsln"), &Device::putsln);
    cls.Func(_SC("puts"), &Device::puts);
    //cls.Func(_SC("write"), &Device::write);

    cls.Func(_SC("buffer"), &Device::buffer);
    cls.Func(_SC("gets"), &Device::gets);
    cls.Func(_SC("read"), &Device::read);
    cls.Func(_SC("strwait"), &Device::strwait);
    cls.Func(_SC("strwaitt"), &Device::strwaitt);
    cls.Func(_SC("binwaitt"), &Device::binwaitt);
    cls.Func(_SC("binwait"), &Device::binwait);
    cls.Func(_SC("any"), &Device::any);
    cls.Func(_SC("strstr"), &Device::strstr);

    cls.Overload<Sqrat::Array (Device::*)(Sqrat::Array)>(_SC("writeread"), &Device::writeread);

    cls.Func(_SC("timeout"), &Device::settimeout);
    cls.Func(_SC("setbinary"), &Device::setbinary);
    cls.Func(_SC("setproc"), &Device::setproc);
    cls.Func(_SC("putc"), &Device::ctrl);
    cls.Func(_SC("putctrl"), &Device::sendkey);
    cls.Func(_SC("close"), &Device::close);
    Sqrat::RootTable().Bind(_SC("Device"), cls);

    if(argc == 2)
    {
        try{
            MyScript scr = sq.compile_script(argv[1]);
            scr.run_script();
        }
        catch(Sqrat::Exception ex)
        {
            std::cout << FRED(ex.Message().c_str()) << CRLF;
        }
    }

    return Run();
}

bool sexecute(std::vector<std::string> & aprog)
{
    std::string prog;
    std::vector<std::string>::iterator b = aprog.begin();
    for(;b!=aprog.end();b++)
        prog.append(*b);

    sq_compilebuffer(Penv->theVM(), prog.c_str(), SQInteger(prog.length()), "", SQTrue);

    sq_pushroottable(Penv->theVM());
    if (SQ_FAILED(sq_call(Penv->theVM(), 1, SQFalse, SQFalse)))
    {
        const SQChar* errMsg = "Unknown error.";
        sq_getlasterror(Penv->theVM());
        if (sq_gettype(Penv->theVM(), -1) == OT_STRING)
            sq_getstring(Penv->theVM(), -1, &errMsg);
        std::cout << FRED("ERROR:" <<  errMsg) << CRLF;
        return false;
    }
    else
        return true;
}

