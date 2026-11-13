/* 可変長 API は通常の関数 API 表と分けて管理する。 */
#ifndef MOCK_VARIADIC
    #error MOCK_VARIADIC must be defined
#endif

MOCK_VARIADIC(int, gzprintf, zlib_app_gzprintf, (gzFile file, const char *format), (file, format), (const char *caller_file, int caller_line, const char *caller_function, gzFile file, const char *format, va_list args), (caller_file, caller_line, caller_function, file, format, args))
