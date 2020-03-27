#QT += core
QT -= gui

TARGET = autocom
CONFIG += console
CONFIG -= app_bundle
QMAKE_CXXFLAGS +=  -O0 -std=c++14
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
    divais.cpp

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
    divais.h



LIBS += -ldl -lpthread


DISTFILES += \
    bin/flash.sss \
    bin/pay.sss \
    bin/Script.sss \
    bin/payutil.sss




unix:!macx: LIBS += -L$$PWD/lib/ -lsquirrel

INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.

unix:!macx: PRE_TARGETDEPS += $$PWD/lib/libsquirrel.a

unix:!macx: LIBS += -L$$PWD/lib/ -lsqstdlib

INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.

unix:!macx: PRE_TARGETDEPS += $$PWD/lib/libsqstdlib.a



