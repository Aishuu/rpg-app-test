#include "util.hpp"

#define KNRM  "\x1B[0m"
#define KERR  "\x1B[31m"
#define KWRN  "\x1B[33m"
#define KDBG  "\x1B[32m"

void error (const char * eMessage, ...) {
    va_list args;
    va_start(args, eMessage);
    fprintf (stderr, KERR "[ERR] ");
    vfprintf (stderr, eMessage, args);
    fprintf (stderr, "\n" KNRM);
    va_end(args);
    exit (EXIT_FAILURE);
}

void warning (const char * eMessage, ...) {
    va_list args;
    va_start(args, eMessage);
    fprintf (stderr, KWRN "[WRN] ");
    vfprintf (stderr, eMessage, args);
    fprintf (stderr, "\n" KNRM);
    va_end(args);
}

void debug (DebugLvl l, const char * eMessage, ...) {
    if (l > DBG_LVL)
        return;

    va_list args;
    va_start(args, eMessage);
    fprintf (stderr, KDBG "[DBG] ");
    vfprintf (stderr, eMessage, args);
    fprintf (stderr, "\n" KNRM);
    va_end(args);
}
