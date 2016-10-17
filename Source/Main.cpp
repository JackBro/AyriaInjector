/*
    Initial author: (https://github.com/)Convery for Ayria.se
    License: MIT
    Started: 2016-10-6
    Notes:
        Entrypoint for the application, windows takes wide strings.
*/

#include "STDInclude.h"
#include <Blackbone/src/BlackBone/Config.h>
#include <Blackbone/src/BlackBone/Process/Process.h>

#ifdef _MSC_VER
int wmain(int argc, wchar_t **argv)
{
    std::wstring Bootstrapmodule;
    std::wstring Startupargs;
    std::wstring Folder;
    std::wstring Exe;

    // Delete the old logfile.
    COAL::Debug::DeleteLogfile();

    // Load the startup arguments.
    if (argc > 2)
    {
        Folder = argv[1];
        Exe = argv[2];

        for (int i = 3; i < argc; ++i)
        {
            Startupargs += argv[i];
            Startupargs += ' ';
        }
    }
    else
    {
        VAPrint("Too few arguments passed. Usage: AyriaInjector.exe Folder Exe-name [Startupargs]");
        return 1;
    }

    // Create the new process.
    blackbone::Process Process;
    Process.CreateAndAttach(Folder + L'/' + Exe, true, true, Startupargs, Folder.c_str());

    // Figure out what kind of bootstrap we need.
    if (Process.core().native())
    {
        if (Process.core().isWow64())
            Bootstrapmodule = L"NativeBootstrap32.dll";
        else
            Bootstrapmodule = L"NativeBootstrap64.dll";

        // Map it into the process.
        Process.mmap().MapImage(Bootstrapmodule, blackbone::eLoadFlags::CreateLdrRef);
    }
    else
    {
        /*
            NOTE(Convery):
            There is currently no official managed bootstrapper.
            The third party versions (.NET and Java) both work as
            plugins and get loaded by the native bootstrap.
        */
    }

    // Resume the process.
    Process.Resume();

    return 0;
}

#else
int main(int argc, char **argv)
{
    std::wstring Bootstrapmodule;
    std::wstring StartupargsW;
    std::string StartupargsA;
    std::wstring FolderW;
    std::string FolderA;
    std::wstring ExeW;
    std::string ExeA;

    // Delete the old logfile.
    COAL::Debug::DeleteLogfile();

    // Load the startup arguments.
    if (argc > 2)
    {
        FolderA = argv[1];
        ExeA = argv[2];

        for (int i = 3; i < argc; ++i)
        {
            StartupargsA += argv[i];
            StartupargsA += ' ';
        }
    }
    else
    {
        VAPrint("Too few arguments passed. Usage: AyriaInjector.exe Folder Exe-name [Startupargs]");
        return 1;
    }

    // Convert ascii to unicode.
    StartupargsW = { StartupargsA.begin(), StartupargsA.end() };
    FolderW = { FolderA.begin(), FolderA.end() };
    ExeW = { ExeA.begin(), ExeA.end() };

    // Create the new process.
    blackbone::Process Process;
    Process.CreateAndAttach(FolderW + L'/' + ExeW, true, true, StartupargsW, FolderW.c_str());

    // Figure out what kind of bootstrap we need.
    if (Process.core().native())
    {
        if (Process.core().isWow64())
            Bootstrapmodule = L"NativeBootstrap32.dll";
        else
            Bootstrapmodule = L"NativeBootstrap64.dll";

        // Map it into the process.
        Process.mmap().MapImage(Bootstrapmodule);
    }
    else
    {
        /*
            NOTE(Convery):
            There is currently no official managed bootstrapper.
            The third party versions (.NET and Java) both work as
            plugins and get loaded by the native bootstrap.
        */
    }

    // Resume the process.
    Process.Resume();

    return 0;
}
#endif
