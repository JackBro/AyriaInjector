/*
    Initial author: (https://github.com/)Convery for Ayria.se
    License: GPL 2.0
    Started: 2016-10-6
    Notes:
        Creates a new string from variadic arguments.
        va("%i", 42) == "42"
*/

#include "Variadicstring.h"
#include <cstdarg>
#include <array>
#include <mutex>

// The buffer-size is the total size for each specialized version.
#define BUFFER_SIZE 4096
#define SMALL_COUNT 16      // 16 buffers of 256 bytes each.
#define LARGE_COUNT 4       // 4 buffers of 1024 bytes each.

// The buffers for the output strings.
std::array<std::array<char, BUFFER_SIZE / SMALL_COUNT>, SMALL_COUNT> Smallbuffers;
std::array<std::array<char, BUFFER_SIZE / LARGE_COUNT>, LARGE_COUNT> Largebuffers;

const char *COAL::va(const char *Format, ...)
{
    char Resultbuffer[BUFFER_SIZE / LARGE_COUNT]{};
    std::va_list Varlist;
    int Resultlength;

    // Create a string from the arguments and truncate if needed.
    va_start(Varlist, Format);
    Resultlength = std::vsnprintf(Resultbuffer, BUFFER_SIZE / LARGE_COUNT, Format, Varlist);
    va_end(Varlist);

    // Send the string to the specialized version for storage.
    if (Resultlength > BUFFER_SIZE / SMALL_COUNT) return va_large(Resultbuffer);
    else return va_small(Resultbuffer);
}
const char *COAL::va_small(const char *Format, ...)
{
    static uint32_t Index = 0;
    static std::mutex Guard;
    std::va_list Varlist;
    char *Resultbuffer;

    // Prevent other threads from modifying the index.
    Guard.lock();
    {
        // Get a pointer to the preallocated buffer.
        Resultbuffer = Smallbuffers[Index].data();

        // Update the index and release the mutex.
        Index = (Index + 1) % SMALL_COUNT;
    }
    Guard.unlock();

    // Create a string from the arguments and truncate if needed.
    va_start(Varlist, Format);
    std::vsnprintf(Resultbuffer, BUFFER_SIZE / SMALL_COUNT, Format, Varlist);
    va_end(Varlist);

    return Resultbuffer;
}
const char *COAL::va_large(const char *Format, ...)
{
    static uint32_t Index = 0;
    static std::mutex Guard;
    std::va_list Varlist;
    char *Resultbuffer;

    // Prevent other threads from modifying the index.
    Guard.lock();
    {
        // Get a pointer to the preallocated buffer.
        Resultbuffer = Largebuffers[Index].data();

        // Update the index and release the mutex.
        Index = (Index + 1) % LARGE_COUNT;
    }
    Guard.unlock();

    // Create a string from the arguments and truncate if needed.
    va_start(Varlist, Format);
    std::vsnprintf(Resultbuffer, BUFFER_SIZE / LARGE_COUNT, Format, Varlist);
    va_end(Varlist);

    return Resultbuffer;
}
