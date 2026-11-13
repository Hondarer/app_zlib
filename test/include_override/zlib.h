#ifndef MOCK_OVERRIDE_ZLIB_ZLIB_H
#define MOCK_OVERRIDE_ZLIB_ZLIB_H

#if !defined(MOCK_ZLIB_ENABLE_CALLER) || defined(MOCK_ZLIB_IN_OVERRIDE_REAL_HEADERS)
    #if defined(_WIN32)
        /* see: https://learn.microsoft.com/en-us/cpp/preprocessor/warning */
        #pragma warning(push, 0)
        #include "../../prod/include/zlib.h"
        #pragma warning(pop)
    #else
        #include_next <zlib.h>
    #endif
#else
    #define MOCK_ZLIB_IN_OVERRIDE_REAL_HEADERS
    #if defined(_WIN32)
        /* see: https://learn.microsoft.com/en-us/cpp/preprocessor/warning */
        #pragma warning(push, 0)
        #include "../../prod/include/zlib.h"
        #pragma warning(pop)
    #else
        #include_next <zlib.h>
    #endif
    #undef MOCK_ZLIB_IN_OVERRIDE_REAL_HEADERS
    #define MOCK_ZLIB_INJECT_CALLER
    #include <mock_zlib_overrides.h>
    #undef MOCK_ZLIB_INJECT_CALLER
#endif

#endif /* MOCK_OVERRIDE_ZLIB_ZLIB_H */
