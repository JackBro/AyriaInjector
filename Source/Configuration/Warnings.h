/*
    Initial author: (https://github.com/)Convery for Ayria.se
    License: GPL 2.0
    Started: 2016-10-6
    Notes:
        This file disables common warnings to allow /w4
*/

#pragma once

// Visual Studio only.
#ifdef _MSC_BUILD

// Warning about constant overflows, signed type being used as unsigned.
#pragma warning(disable: 4307)

// Warning about unused parameters.
#pragma warning(disable: 4100)

// Warning about symbols being redeclared, release | debug mismatch.
#pragma warning(disable: 4565)

// Warning about comments in type declarations.
#pragma warning(disable: 4138)

// POSIX name for the function has been deprecated.
#pragma warning(disable: 4996)
#else

#endif
