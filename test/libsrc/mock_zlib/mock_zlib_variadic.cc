#include <mock_zlib.h>
#include <cstdarg>

extern "C" int mock_zlib_app_gzprintf_variadic(const char *caller_file, int caller_line, const char *caller_function,
                                               gzFile file, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    const int result = mock_zlib_app_gzvprintf(caller_file, caller_line, caller_function, file, format, args);
    va_end(args);
    return result;
}
#ifndef _WIN32
MOCK_WEAK_IMPL(int, zlib_app_gzprintf, gzFile file, const char *format, ...)
#else
extern "C" int ZEXPORTVA zlib_app_gzprintf(gzFile file, const char *format, ...)
#endif
{
    va_list args;
    va_start(args, format);
    const int result = mock_zlib_app_gzvprintf("<unknown>", 0, "<unknown>", file, format, args);
    va_end(args);
    return result;
}
