#ifndef NOGGIT_SRC_NOGGIT_IOSTREAM_HPP
#define NOGGIT_SRC_NOGGIT_IOSTREAM_HPP

#include <iostream>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
#define COUT std::wcout
#define CERR std::wcerr
#define TEXT(STR) L ## STR
#else
#define COUT std::cout
#define CERR std::cerr
#define TEXT(STR) STR
#endif

#endif //NOGGIT_SRC_NOGGIT_IOSTREAM_HPP
