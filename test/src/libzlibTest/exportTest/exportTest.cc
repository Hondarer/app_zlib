#include <testfw.h>

#include <mock_zlib.h>

#include <set>
#include <string>
#include <type_traits>

static_assert(sizeof(z_off_t) == 8, "z_off_t は 64 bit である必要があります");
static_assert(sizeof(z_off64_t) == 8, "z_off64_t は 64 bit である必要があります");

#define MOCK_ZLIB_RET(return_type, original_name, api_name, real_parameters, real_arguments, mock_parameters, mock_arguments, matchers) \
    using expected_##original_name##_fn = return_type(*) real_parameters; \
    static_assert(std::is_same<decltype(&api_name), expected_##original_name##_fn>::value, #api_name " のシグネチャが不一致です");
#define MOCK_ZLIB_VOID(return_type, original_name, api_name, real_parameters, real_arguments, mock_parameters, mock_arguments, matchers) \
    using expected_##original_name##_fn = return_type(*) real_parameters; \
    static_assert(std::is_same<decltype(&api_name), expected_##original_name##_fn>::value, #api_name " のシグネチャが不一致です");
#include <mock_zlib_api_table.h>
#undef MOCK_ZLIB_VOID
#undef MOCK_ZLIB_RET

using expected_gzprintf_fn = int (*)(gzFile, const char *, ...);
static_assert(std::is_same<decltype(&zlib_app_gzprintf), expected_gzprintf_fn>::value, "gzprintf のシグネチャが不一致です");

static const char *const kExpectedExportNames[] = {
#define MOCK_ZLIB_RET(return_type, original_name, api_name, real_parameters, real_arguments, mock_parameters, mock_arguments, matchers)  #api_name,
#define MOCK_ZLIB_VOID(return_type, original_name, api_name, real_parameters, real_arguments, mock_parameters, mock_arguments, matchers) #api_name,
#include <mock_zlib_api_table.h>
#undef MOCK_ZLIB_VOID
#undef MOCK_ZLIB_RET
    "zlib_app_gzprintf",
};

// libzlib_app の期待シンボルと実ライブラリの全エクスポートが一致することの確認
TEST(exportTest, zlib_symbols_match_api_table)
{
    // Arrange
    std::set<std::string> expected(
        std::begin(kExpectedExportNames),
        std::end(kExpectedExportNames)); // [状態] - mock_zlib の API 表から期待する公開関数名を構築する。
#if defined(PLATFORM_WINDOWS)
    expected.insert(testing::identManifestSymbolName(
        "libzlib_app" TESTFW_SHARED_LIBRARY_EXTENSION)); // [状態] - IDENT manifest シンボル名を期待値へ追加する。
#endif                                                   /* PLATFORM_WINDOWS */
    std::string path = findWorkspaceRoot() + "/app/zlib/prod/lib/libzlib_app" +
                       TESTFW_SHARED_LIBRARY_EXTENSION; // [状態] - 検査対象を libzlib_app の動的ライブラリとする。

    // Pre-Assert

    // Act
    std::set<std::string> actual =
        testing::getActualExportNames(path); // [手順] - path を指定して testing::getActualExportNames() を呼び出す。

    // Assert
    // [サブ手順参照 名前=testing.expectExportNamesMatch]
    testing::expectExportNamesMatch(expected,
                                    actual); // libzlib_app のエクスポートに不足や想定外がないこと。
}
