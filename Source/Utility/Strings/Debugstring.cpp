/*
    Initial author: (https://github.com/)Convery for Ayria.se
    License: GPL 2.0
    Started: 2016-10-6
    Notes:
        A simple system to log messages to disk and stdout.
        The logfile is stored in LOGFILEDIR\\MODULENAME.log.
*/

/* TODO(Convery): Find out why GCC wont accept /Source as include dir */
#include "../../Configuration/Defines.h"
#include "Debugstring.h"
#include <cstdio>
#include <ctime>
#include <mutex>

// The full path to the logfile.
constexpr const char *Filepath = LOGFILEDIR MODULENAME ".log";
static std::mutex Guard;

void COAL::Debug::DeleteLogfile()
{
    std::remove(Filepath);
}
void COAL::Debug::AppendLogfileTimestamp(const char *Message)
{
    std::time_t Now = time(NULL);
    char Buffer [80];

    strftime (Buffer, 80, "%H:%M:%S", std::localtime(&Now));
    AppendLogfile(Message, Buffer);
}
void COAL::Debug::AppendLogfile(const char *Message, const char *Prefix)
{
    // Prevent multiple writes to the log.
    Guard.lock();
    {
        auto Filehandle = std::fopen(Filepath, "a");
        if (Filehandle)
        {
            if(Prefix) std::fprintf(Filehandle, "[%-8s] ", Prefix);
            std::fputs(Message, Filehandle);
            std::fputs("\n", Filehandle);
            std::fclose(Filehandle);
        }
    }
    Guard.unlock();

    // Duplicate the output to stdout.
#ifdef DEBUGTOSTREAM
    if(Prefix) std::fprintf(stdout, "[%-7s] ", Prefix);
    std::fputs(Message, stdout);
    std::fputs("\n", stdout);
#endif
}
