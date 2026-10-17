#include <testfw.h>
#include <mock_zlib.h>

#include <cstdio>

class mockZlibTest : public Test
{
  protected:
    // [サブ手順 名前=mockZlibTest.TearDown]
    void TearDown() override
    {
        std::remove("zlib_printf_test.gz");
    }
    // [サブ手順終了]
};

// モック未生成時に zlib の処理へ正しく委譲されることの確認
TEST_F(mockZlibTest, delegates_without_mock)
{
    // Arrange
    const Bytef source[] = "123456789"; // [状態] - モックを生成せず既知の入力を用意する。

    // Pre-Assert

    // Act
    const uLong crc =
        crc32(0, source, 9); // [手順] - zlib の処理へ委譲されるように、0、source、9 を指定して crc32() を呼び出す。
    const char *actual_ret_version = zlibVersion(); // [手順] - zlibVersion() を呼び出す。

    // Assert
    EXPECT_EQ(
        0xcbf43926UL,
        crc); // [確認_正常系] - crc32() の戻り値が 0xcbf43926UL であること。(入力 "123456789" の CRC-32 であることの確認)
    EXPECT_STREQ(ZLIB_VERSION,
                 actual_ret_version); // [確認_正常系] - zlibVersion() の戻り値が ZLIB_VERSION であること。
}
// [サブ手順参照 名前=mockZlibTest.TearDown]

// モック生成後も未指定の呼び出しが zlib の処理へ委譲されることの確認
TEST_F(mockZlibTest, delegates_unspecified_calls)
{
    // Arrange
    NiceMock<Mock_zlib> mock_zlib;      // [状態] - 個別の動作を指定せずモックを生成する。
    const Bytef source[] = "123456789"; // [状態] - 既知の入力を用意する。

    // Pre-Assert

    // Act
    const uLong crc = crc32(
        0, source,
        9); // [手順] - 既定動作の Mock_zlib を介して zlib の処理へ委譲されるように、0、source、9 を指定して crc32() を呼び出す。

    // Assert
    EXPECT_EQ(
        0xcbf43926UL,
        crc); // [確認_正常系] - crc32() の戻り値が 0xcbf43926UL であること。(入力 "123456789" の CRC-32 であることの確認)
}
// [サブ手順参照 名前=mockZlibTest.TearDown]

// 初期化マクロ経由でモックのエラー戻り値を上書きできることの確認
TEST_F(mockZlibTest, overrides_error_and_init_macro)
{
    // Arrange
    NiceMock<Mock_zlib> mock_zlib; // [状態] - 既定動作の Mock_zlib を生成する。
    z_stream stream = {};          // [状態] - マクロが渡すストリームを用意する。

    // Pre-Assert
    EXPECT_CALL(mock_zlib, deflateInit_(&stream, Z_DEFAULT_COMPRESSION, StrEq(ZLIB_VERSION), sizeof(z_stream)))
        .WillOnce(Return(
            Z_MEM_ERROR)); // [Pre-Assert確認_異常系] - deflateInit_() が &stream、Z_DEFAULT_COMPRESSION、ZLIB_VERSION と等しい文字列、sizeof(z_stream) を指定して 1 回呼び出されること。
                           // [Pre-Assert手順] - deflateInit_() からメモリ不足 (Z_MEM_ERROR) を返す。

    // Act
    const int ret = deflateInit(
        &stream, Z_DEFAULT_COMPRESSION); // [手順] - Z_DEFAULT_COMPRESSION を指定して deflateInit() を呼び出す。

    // Assert
    EXPECT_EQ(Z_MEM_ERROR, ret); // [確認_異常系] - deflateInit() の戻り値が Z_MEM_ERROR であること。
}
// [サブ手順参照 名前=mockZlibTest.TearDown]

// gzprintf の可変長引数が gzvprintf へ正しく渡されることの確認
TEST_F(mockZlibTest, printf_passes_variadic_arguments)
{
    // Arrange
    NiceMock<Mock_zlib> mock_zlib; // [状態] - 実ファイルを使わず書式化を観測する。
    char output[32] = {};          // [状態] - 書式化結果の格納先を用意する。

    // Pre-Assert
    EXPECT_CALL(mock_zlib, gzvprintf(nullptr, StrEq("%s:%d"), _))
        .WillOnce(
            [&output](gzFile, const char *format, va_list args)
            {
                return std::vsnprintf(output, sizeof(output), format, args);
            }); // [Pre-Assert確認_正常系] - gzvprintf() が nullptr、"%s:%d" と等しい文字列を指定して 1 回呼び出されること。
                // [Pre-Assert手順] - gzvprintf() から、書式と可変長引数で output を書き、書き込んだ長さを返す。

    // Act
    const int ret =
        gzprintf(nullptr, "%s:%d", "value", 42); // [手順] - 異なる型の可変長引数を指定して gzprintf() を呼び出す。

    // Assert
    EXPECT_EQ(8, ret); // [確認_正常系] - gzprintf() の戻り値が 8 であること。("value:42" の文字数であることの確認)
    EXPECT_STREQ("value:42",
                 output); // [確認_正常系] - output が "value:42" であること。(渡した書式と引数の結果であることの確認)
}
// [サブ手順参照 名前=mockZlibTest.TearDown]

// gzprintf が実ファイルに対して正常に書式化出力できることの確認
TEST_F(mockZlibTest, printf_delegates_to_real_file)
{
    // Arrange
    char output[32] = {}; // [状態] - モック未生成で実ファイルへの委譲を検証する。

    // Pre-Assert

    // Act
    gzFile writer = gzopen("zlib_printf_test.gz", "wb"); // [手順] - 書き込み用の "wb" を指定して gzopen() を呼び出す。
    ASSERT_NE(nullptr, writer);
    // [状態確認] - 書き込み用の gzopen() の戻り値が nullptr 以外であること。
    const int ret =
        gzprintf(writer, "%s:%d", "value",
                 42); // [手順] - zlib の処理へ委譲されるように、可変長引数を指定して gzprintf() を呼び出す。
    const int close_ret = gzclose(writer);
    gzFile reader = gzopen("zlib_printf_test.gz", "rb"); // [手順] - 読み込み用の "rb" を指定して gzopen() を呼び出す。
    ASSERT_NE(nullptr, reader);
    // [状態確認] - 読み込み用の gzopen() の戻り値が nullptr 以外であること。
    const int size = gzread(reader, output, sizeof(output));
    const int read_close_ret = gzclose(reader);

    // Assert
    EXPECT_EQ(8, ret); // [確認_正常系] - gzprintf() の戻り値が 8 であること。("value:42" の文字数であることの確認)
    EXPECT_EQ(Z_OK, close_ret);      // [確認_正常系] - 書き込み後の gzclose() の戻り値が Z_OK であること。
    EXPECT_EQ(Z_OK, read_close_ret); // [確認_正常系] - 読み込み後の gzclose() の戻り値が Z_OK であること。
    EXPECT_EQ(8, size); // [確認_正常系] - gzread() の戻り値が 8 であること。("value:42" の文字数であることの確認)
    EXPECT_STREQ("value:42",
                 output); // [確認_正常系] - output が "value:42" であること。(実ファイルに書式化結果が残ったことの確認)
}
// [サブ手順参照 名前=mockZlibTest.TearDown]
