#ifndef UTIL_HPP
#define UTIL_HPP

#include <cstddef>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_PLAYERS                 10
#define MAX_NAME_SIZE               20
#define SERVER_RECONNECTION_DELAY   500000

#define VALID_NAME_CHAR(__c) ((__c >= 'A' && __c <= 'Z') || (__c >= 'a' && __c <= 'z') || (__c >= '0' && __c <= '9') || __c == '_' || __c == '*')

typedef char DebugLvl;
#define DBG_NONE    0
#define DBG_BASE    1
#define DBG_ALL     2

#define DBG_LVL     DBG_ALL

void error (const char * eMessage, ...);
void warning (const char * eMessage, ...);
void debug (DebugLvl l, const char * eMessage, ...);

#endif // UTIL_HPP
