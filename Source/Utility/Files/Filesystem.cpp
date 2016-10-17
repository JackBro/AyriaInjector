/*
    Initial author: (https://github.com/)Convery for Ayria.se
    License: GPL 2.0
    Started: 2016-10-6
    Notes:
        A simple wrapper around common file operations.
        MSVC makes use of the C++17 TS.
*/

#include "Filesystem.h"
#include <algorithm>
#include <fstream>
#include <memory>
#include <thread>
#include <mutex>

#if __linux__
#define IOSMODE(x) (std::_Ios_Openmode)(x)
#else
#define IOSMODE(x) x
#endif

bool COAL::File::Write(const char *Filepath, const void *Databuffer, const size_t Datalength, const bool Append)
{
    bool Result;
    std::ofstream Filewriter(Filepath, IOSMODE(std::ios::binary | (Append ? std::ios::app : 0)));
    Result = !!Filewriter.write((const char *)Databuffer, Datalength);
    Filewriter.close();
    return Result;
}
bool COAL::File::Write(const char *Filepath, const std::string &Databuffer, const bool Append)
{
    return Write(Filepath, Databuffer.data(), Databuffer.size(), Append);
}
bool COAL::File::Read(const char *Filepath, void *Databuffer, size_t *Datalength)
{
    // Sanity checking to avoid time-wastage.
    if (!Exists(Filepath)) return false;

    // Open the file.
    size_t Readsize = std::min(Size(Filepath), *Datalength);
    std::ifstream Filereader(Filepath, std::ios::binary);

    // Read the file into the buffer.
    bool Result = !!Filereader.read((char *)Databuffer, Readsize);

    // Set the read length and cleanup.
    *Datalength = Readsize;
    Filereader.close();
    return Result;
}
bool COAL::File::Read(const char *Filepath, std::string *Databuffer)
{
    // Sanity checking to avoid time-wastage.
    if (!Exists(Filepath)) return false;

    // Create a temp buffer.
    size_t Filelength = Size(Filepath);
    auto Buffer = std::make_unique<char[]>(Filelength);

    // Read the file into the buffer.
    bool Result = Read(Filepath, Buffer.get(), &Filelength);

    // Append to the string.
    Databuffer->append(Buffer.get(), Filelength);

    return Result;
}

#if __linux__
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

uint32_t COAL::File::Modified(const char *Filepath)
{
    struct stat Attribute;
    stat(Filepath, &Attribute);
    return uint32_t(Attribute.st_mtime);
}
bool COAL::File::Createdir(const char *Path)
{
    std::string Filepath(Path);
    auto Position = Filepath.find_first_of("/");
    bool Result = false;

    while (Position)
    {
        Result = mkdir(Filepath.substr(0, Position).c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        Position = Filepath.find_first_of("/", Position + 1);
    }

    return Result;
}
bool COAL::File::List(std::string Searchpath, std::vector<std::string> *Filenames, const char *Extension)
{
    struct stat Fileinfo;
    dirent *Filedata;
    std::string Path;
    DIR *Filehandle;

    // Append trailing slash, asterisk and extension.
    if (Searchpath.back() != '\\') Searchpath.append("\\");
    Path = Searchpath;
    Searchpath.append("*");
    if (Extension) Searchpath.append(".");
    if (Extension) Searchpath.append(Extension);

    // Iterate through the directory.
    Filehandle = opendir(Searchpath.c_str());
    while ((Filedata = readdir(Filehandle)))
    {
        // Respect hidden files and folders.
        if (Filedata->d_name[0] == '.')
            continue;

        // Get extended fileinfo.
        std::string Filepath = Path + "/" + Filedata->d_name;
        if (stat(Filepath.c_str(), &Fileinfo) == -1) continue;

        // Add the file to the list.
        if (!(Fileinfo.st_mode & S_IFDIR))
            Filenames->push_back(Filedata->d_name);
    }
    closedir(Filehandle);

    return !!Filenames->size();
}
bool COAL::File::Listrecursive(std::string Searchpath, std::vector<std::string> *Filenames, const char *Extension)
{
    std::vector<std::thread> Workers;
    static std::mutex Guard;
    struct stat Fileinfo;
    dirent *Filedata;
    std::string Path;
    DIR *Filehandle;

    // Append trailing slash, asterisk and extension.
    if (Searchpath.back() != '\\') Searchpath.append("\\");
    Path = Searchpath;
    Searchpath.append("*");
    if (Extension) Searchpath.append(".");
    if (Extension) Searchpath.append(Extension);

    // Iterate through the directory.
    Filehandle = opendir(Searchpath.c_str());
    while ((Filedata = readdir(Filehandle)))
    {
        // Respect hidden files and folders.
        if (Filedata->d_name[0] == '.')
            continue;

        // Start a new thread for directories.
        if (Fileinfo.st_mode & S_IFDIR)
        {
            Workers.push_back(std::thread(Listrecursive, Path + Filedata->d_name, Filenames, Extension));
            continue;
        }

        // Append the full path.
        Guard.lock();
        {
            Filenames->push_back(Path + Filedata->d_name);
        }
        Guard.unlock();
    }
    closedir(Filehandle);

    // Wait for all threads to finish.
    for (std::thread &Thread : Workers)
        if (Thread.joinable())
            Thread.join();

    return !!Filenames->size();
}
#else
    #include <Windows.h>
    #include <direct.h>

    // msvc 14
    #if _MSC_VER >= 1900
    #include <experimental\filesystem>

    uint32_t COAL::File::Modified(const char *Filepath)
    {
        // Sanity checking to avoid time-wastage.
        if (!Exists(Filepath)) return 0;

        auto Filetime = std::experimental::filesystem::last_write_time(Filepath);
        return uint32_t(decltype(Filetime)::clock::to_time_t(Filetime));
    }
    size_t COAL::File::Size(const char *Filepath)
    {
        return size_t(std::experimental::filesystem::file_size(Filepath));
    }
    bool COAL::File::Exists(const char *Filepath)
    {
        return std::experimental::filesystem::exists(Filepath);
    }
    bool COAL::File::Createdir(const char *Path)
    {
        return std::experimental::filesystem::create_directories(Path);
    }
    #else
    
    uint32_t COAL::File::Modified(const char *Filepath)
    {
        HANDLE Filehandle = CreateFileA(Filepath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        FILETIME ModifiedTime;

        GetFileTime(Filehandle, NULL, NULL, &ModifiedTime);
        CloseHandle(Filehandle);

        return uint32_t((*(uint64_t *)&ModifiedTime / 10000000 - 11644473600LL));
    }
    size_t COAL::File::Size(const char *Filepath)
    {
        std::ifstream Filehandle(Filepath, std::ios::binary);
        std::streamsize Size = 0;

        Filehandle.seekg(0, std::ios::end);
        Size = Filehandle.tellg();
        Filehandle.seekg(0, std::ios::beg);

        if (Size == -1) return 0;
        else return size_t(Size);
    }
    bool COAL::File::Exists(const char *Filepath)
    {
        std::ifstream Filehandle(Filepath, std::ios::binary);
        std::streamsize Size = 0;

        Filehandle.seekg(0, std::ios::end);
        Size = Filehandle.tellg();
        Filehandle.seekg(0, std::ios::beg);

        return Size != -1;
    }
    bool COAL::File::Createdir(const char *Path)
    {
        std::string Filepath(Path);
        auto Position = Filepath.find_first_of("/");
        bool Result = false;

        while (Position)
        {
            Result = !!_mkdir(Filepath.substr(0, Position).c_str());
            Position = Filepath.find_first_of("/", Position + 1);
        }

        return Result;
    }
    #endif

    bool COAL::File::Touch(const char *Path)
    {
        return Write(Path, "", 0, true);
    }
    bool COAL::File::List(std::string Searchpath, std::vector<std::string> *Filenames, const char *Extension)
    {
        WIN32_FIND_DATAA Filedata;
        HANDLE Filehandle;

        // Append trailing slash, asterisk and extension.
        if (Searchpath.back() != '\\') Searchpath.append("\\");
        Searchpath.append("*");
        if (Extension) Searchpath.append(".");
        if (Extension) Searchpath.append(Extension);

        // Find the first extension.
        Filehandle = FindFirstFileA(Searchpath.c_str(), &Filedata);
        if (Filehandle == (void *)ERROR_INVALID_HANDLE || Filehandle == (void *)INVALID_HANDLE_VALUE)
            return false;

        do
        {
            // Respect hidden files.
            if (Filedata.cFileName[0] == '.')
                continue;

            // Add the file to the list.
            if (!(Filedata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                Filenames->push_back(Filedata.cFileName);

        } while (FindNextFileA(Filehandle, &Filedata));

        return !!Filenames->size();
    }
    bool COAL::File::Listrecursive(std::string Searchpath, std::vector<std::string> *Filenames, const char *Extension)
    {
        static std::mutex Threadguard;
        std::vector<std::thread> Workers;
        WIN32_FIND_DATAA Filedata;
        HANDLE Filehandle;
        std::string Path;

        // Append trailing slash, asterisk and extension.
        if (Searchpath.back() != '\\') Searchpath.append("\\");
        Path = Searchpath;
        Searchpath.append("*");
        if (Extension) Searchpath.append(".");
        if (Extension) Searchpath.append(Extension);

        // Find the first extension.
        Filehandle = FindFirstFileA(Searchpath.c_str(), &Filedata);
        if (Filehandle == (void *)ERROR_INVALID_HANDLE || Filehandle == (void *)INVALID_HANDLE_VALUE)
            return false;

        do
        {
            // Respect hidden files and folders.
            if (Filedata.cFileName[0] == '.') continue;

            // Start a new thread for directories.
            if (Filedata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                Workers.push_back(std::thread(Listrecursive, Path + Filedata.cFileName, Filenames, Extension));
                continue;
            }

            // Append the full path.
            Threadguard.lock();
            {
                Filenames->push_back(Path + Filedata.cFileName);
            }
            Threadguard.unlock();

        } while (FindNextFileA(Filehandle, &Filedata));

        // Wait for all threads to finish.
        for (std::thread &Thread : Workers)
            if (Thread.joinable())
                Thread.join();

        return !!Filenames->size();
    }
#endif
