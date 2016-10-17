/*
    Initial author: (https://github.com/)Convery for Ayria.se
    License: GPL 2.0
    Started: 2016-10-6
    Notes:
        A simple wrapper around common file operations.
        MSVC makes use of the C++17 TS.
*/

#pragma once
#include <vector>
#include <string>

namespace COAL
{
    namespace File
    {
        bool Write(const char *Filepath, const void *Databuffer, const size_t Datalength, const bool Append);
        bool Write(const char *Filepath, const std::string &Databuffer, const bool Append);
        bool Read(const char *Filepath, void *Databuffer, size_t *Datalength);
        bool Read(const char *Filepath, std::string *Databuffer);

        uint32_t Modified(const char *Filepath);
        size_t Size(const char *Filepath);
        bool Exists(const char *Filepath);

        bool Createdir(const char *Path);
        bool Touch(const char *Path);

        bool List(std::string Searchpath, std::vector<std::string> *Filenames, const char *Extension = nullptr);
        bool Listrecursive(std::string Searchpath, std::vector<std::string> *Filenames, const char *Extension = nullptr);
    }
}
