#QT += core
QT -= gui

TARGET = autocom
CONFIG += console
DEFINES -= UNICODE
DEFINES += WINDOWS
QMAKE_LFLAGS += -static
CONFIG -= app_bundle
QMAKE_CXXFLAGS +=  -O0 -std=c++11
TEMPLATE = app



INCLUDEPATH += sq/include
INCLUDEPATH += sq/squirrel
INCLUDEPATH += sq/sqrat/include
INCLUDEPATH += sq/sqrat
INCLUDEPATH += sq/sqrat/include/sqrat

SOURCES += main.cpp \
    sqwrap.cpp \
    serialcom.cpp \
    tcpcom.cpp \
    sshcom.cpp \
    serialport.cpp \
    sqbytearr.cpp \
    divais.cpp \
    sq/sqstdlib/sqstdaux.cpp \
    sq/sqstdlib/sqstdblob.cpp \
    sq/sqstdlib/sqstdio.cpp \
    sq/sqstdlib/sqstdmath.cpp \
    sq/sqstdlib/sqstdrex.cpp \
    sq/sqstdlib/sqstdstream.cpp \
    sq/sqstdlib/sqstdstring.cpp \
    sq/sqstdlib/sqstdsystem.cpp \
    sq/squirrel/sqapi.cpp \
    sq/squirrel/sqbaselib.cpp \
    sq/squirrel/sqclass.cpp \
    sq/squirrel/sqcompiler.cpp \
    sq/squirrel/sqdebug.cpp \
    sq/squirrel/sqfuncstate.cpp \
    sq/squirrel/sqlexer.cpp \
    sq/squirrel/sqmem.cpp \
    sq/squirrel/sqobject.cpp \
    sq/squirrel/sqstate.cpp \
    sq/squirrel/sqtable.cpp \
    sq/squirrel/sqvm.cpp

HEADERS += \
    main.h \
    sqwrap.h \
    serialcom.h \
    tcpcom.h \
    sshcom.h \
    icombase.h \
    sqratext.h \
    osthread.h \
    serialport.h \
    icombase.h \
    sqbytearr.h \
    divais.h \
    sqbind.h \
    sq/sqstdlib/sqstdblobimpl.h \
    sq/sqstdlib/sqstdstream.h \
    sq/squirrel/sqarray.h \
    sq/squirrel/sqclass.h \
    sq/squirrel/sqclosure.h \
    sq/squirrel/sqcompiler.h \
    sq/squirrel/sqfuncproto.h \
    sq/squirrel/sqfuncstate.h \
    sq/squirrel/sqobject.h \
    sq/squirrel/sqopcodes.h \
    sq/squirrel/sqstate.h \
    sq/squirrel/sqstring.h \
    sq/squirrel/sqtable.h \
    sq/squirrel/squtils.h \
    sq/squirrel/sqvm.h




DISTFILES += \
    bin/pay.sss \
    bin/Script.sss \
    bin/payutil.sss \
    sqmethods.inc \
    bin/first.sss


