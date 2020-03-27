#ifndef MAIN_H_
#define MAIN_H_

#include <stdio.h>
#include <ctype.h>
#include <iostream>
#include <string>
#include <QCoreApplication>
#include <QTextStream>
#include <QTimer>
#include "sqwrap.h"


typedef std::basic_string<uint8_t>  bstring;

std::string  toString(const uint8_t* binary, int len);
bstring  toBinary(const char* string);


extern SqEnv* Penv;
extern bool __alive;
extern int _Debug;
#endif
