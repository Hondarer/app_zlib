#include <zlib.h>

const char *namespace_test_caller_file(void);
int namespace_test_caller_line(void);
int namespace_test_direct_call(z_streamp stream);
int namespace_test_pointer_call(z_streamp stream);

const char *namespace_test_caller_file(void)
{
    return __FILE__;
}
/* 呼び出し行の照合に使うため、次の 2 定義を隣接させます。 */
// clang-format off
int namespace_test_caller_line(void) { return __LINE__ + 1; }
int namespace_test_direct_call(z_streamp stream) { return zlib_app_deflate(stream, Z_NO_FLUSH); }
// clang-format on
int namespace_test_pointer_call(z_streamp stream)
{
    int (*api)(z_streamp, int) = &zlib_app_deflate;
    return api(stream, Z_NO_FLUSH);
}

int namespace_test_variadic_line(void);
int namespace_test_variadic_call(void);
/* 呼び出し行の照合に使うため、次の 2 定義を隣接させます。 */
// clang-format off
int namespace_test_variadic_line(void) { return __LINE__ + 1; }
int namespace_test_variadic_call(void) { return zlib_app_gzprintf(NULL, "%s", "value"); }
// clang-format on
