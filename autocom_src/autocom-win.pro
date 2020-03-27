QT += core serialport
QT -= gui


TARGET = autocom
CONFIG += console
CONFIG -= app_bundle
DEFINES -= UNICODE
DEFINES += WINDOWS
QMAKE_CXXFLAGS +=  -O0 -std=c++11 -Wno-unused-parameter -Wno-missing-field-initializers
QMAKE_CFLAGS +=  -Wno-unused-parameter -Wno-missing-field-initializers
TEMPLATE = app

INCLUDEPATH += sq/include
INCLUDEPATH += sq/squirrel
INCLUDEPATH += sq/sqrat/include
INCLUDEPATH += sq/sqrat

SOURCES += main.cpp \
    sqwrap.cpp \
    serialcom.cpp \
    tcpcom.cpp \
    sshcom.cpp \
    combase.cpp \
    proxy.cpp

HEADERS += \
    main.h \
    sqwrap.h \
    serialcom.h \
    tcpcom.h \
    sshcom.h \
    combase.h \
    sqratext.h \
    proxy.h \
    sq/sqstdlib/sqstdstream.h


SOURCES += \
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

#C:\Users\marius\Documents\CPP\lsss\sq\winlibR\release



