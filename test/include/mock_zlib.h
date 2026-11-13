#ifndef MOCK_ZLIB_H
#define MOCK_ZLIB_H

#include <zlib.h>
#include <testfw.h>
#include <mock_zlib_overrides.h>

// 関数形式マクロは関数定義と MOCK_METHOD の展開を妨げるため解除する。
#undef zlib_app_gzgetc

// Windows でも実ライブラリが公開する 64-bit API を宣言する。
// see: https://github.com/madler/zlib/blob/v1.3.2/gzguts.h
// see: https://github.com/madler/zlib/blob/v1.3.2/zutil.h
#ifndef Z_LARGE64
extern "C"
{
    ZEXTERN gzFile ZEXPORT zlib_app_gzopen64(const char *, const char *);
    ZEXTERN z_off64_t ZEXPORT zlib_app_gzseek64(gzFile, z_off64_t, int);
    ZEXTERN z_off64_t ZEXPORT zlib_app_gztell64(gzFile);
    ZEXTERN z_off64_t ZEXPORT zlib_app_gzoffset64(gzFile);
    ZEXTERN uLong ZEXPORT zlib_app_adler32_combine64(uLong, uLong, z_off64_t);
    ZEXTERN uLong ZEXPORT zlib_app_crc32_combine64(uLong, uLong, z_off64_t);
    ZEXTERN uLong ZEXPORT zlib_app_crc32_combine_gen64(z_off64_t);
}
#endif

inline constexpr char kLibZlibName[] = "libzlib_app" TESTFW_SHARED_LIBRARY_EXTENSION;

#define MOCK_ZLIB_RET(return_type, original_name, api_name, parameters, arguments, mock_parameters, mock_arguments, matchers) \
    extern return_type delegate_real_##api_name mock_parameters;
#define MOCK_ZLIB_VOID(return_type, original_name, api_name, parameters, arguments, mock_parameters, mock_arguments, matchers) \
    extern return_type delegate_real_##api_name mock_parameters;
#include <mock_zlib_api_table.h>
#undef MOCK_ZLIB_VOID
#undef MOCK_ZLIB_RET

class Mock_zlib
{
  public:
#define MOCK_ZLIB_RET(return_type, original_name, api_name, parameters, arguments, mock_parameters, mock_arguments, matchers)  MOCK_METHOD(return_type, api_name, mock_parameters);
#define MOCK_ZLIB_VOID(return_type, original_name, api_name, parameters, arguments, mock_parameters, mock_arguments, matchers) MOCK_METHOD(return_type, api_name, mock_parameters);
#include <mock_zlib_api_table.h>
#undef MOCK_ZLIB_VOID
#undef MOCK_ZLIB_RET

    Mock_zlib();
    virtual ~Mock_zlib();
};

extern Mock_zlib *mock_zlib_instance;

#endif /* MOCK_ZLIB_H */
