#include <testfw.h>
#include <zlib.h>

#include <array>
#include <cstdio>
#include <fstream>
#include <memory>
#include <vector>

class zlibTest : public Test
{
  protected:
    // [サブ手順 名前=zlibTest.TearDown]
    void TearDown() override
    {
        std::remove("zlib_roundtrip_test.gz");
    }
    // [サブ手順終了]
};

// 通常 API と 64-bit API が 32 bit を超える長さで同じ CRC-32 を返すことの確認
TEST_F(zlibTest, combine_preserves_64_bit_length)
{
    // Arrange
    const z_off64_t length = 0x100000001LL; // [状態] - 32 bit を超える結合対象の長さを用意する。

    // Pre-Assert

    // Act
    const uLong actual = zlib_app_crc32_combine(0x12345678UL, 0x87654321UL,
                                       length); // [手順] - 通常 API に 32 bit を超える長さを渡す。
    const uLong expected = zlib_app_crc32_combine64(0x12345678UL, 0x87654321UL,
                                           length); // [手順] - 同じ長さを 64-bit API に渡す。

    // Assert
    EXPECT_EQ(expected, actual); // [確認_正常系] - 通常 API が長さを切り詰めず 64-bit API と同じ結果を返すこと。
}
// [サブ手順参照 名前=zlibTest.TearDown]

// 空データ・バイナリ・反復データを含むメモリ上の圧縮・展開ラウンド トリップが成功することの確認
TEST_F(zlibTest, memory_roundtrip_including_empty_and_binary)
{
    // Arrange
    const std::vector<std::vector<Bytef>> inputs = {
        {}, {0, 1, 0, 127, 128, 255}, std::vector<Bytef>(4096, 65)}; // [状態] - 空・バイナリ・反復データを用意する。
    std::vector<std::vector<Bytef>> actual_restored;

    // Pre-Assert

    // Act
    for (const auto &source : inputs)
    {
        const Bytef empty = 0;
        const Bytef *input = &empty;
        if (!source.empty())
        {
            input = source.data();
        }
        const uLong input_size = static_cast<uLong>(source.size());
        std::vector<Bytef> compressed(zlib_app_compressBound(input_size));
        uLongf compressed_size = static_cast<uLongf>(compressed.size());
        std::vector<Bytef> restored(source.size() + 1);
        uLongf restored_size = static_cast<uLongf>(restored.size());

        const int compress_ret = zlib_app_compress(compressed.data(), &compressed_size, input,
                                          input_size); // [手順] - 圧縮対象のメモリを指定して zlib_app_compress() を呼び出す。
        const int restore_ret = zlib_app_uncompress(restored.data(), &restored_size, compressed.data(),
                                           compressed_size); // [手順] - 圧縮結果を指定して zlib_app_uncompress() を呼び出す。

        ASSERT_EQ(Z_OK, compress_ret); // [確認_正常系 回数=3] - zlib_app_compress() の戻り値が Z_OK であること。
        ASSERT_EQ(Z_OK, restore_ret);  // [確認_正常系 回数=3] - zlib_app_uncompress() の戻り値が Z_OK であること。
        restored.resize(restored_size);
        actual_restored.push_back(restored);
    }

    // Assert
    for (size_t i = 0; i < inputs.size(); ++i)
    {
        EXPECT_EQ(inputs[i],
                  actual_restored[i]); // [確認_正常系 回数=3] - actual_restored[i] が inputs[i] と一致すること。
    }
}
// [サブ手順参照 名前=zlibTest.TearDown]

// 不正な圧縮形式および出力領域不足が正しく検出されることの確認
TEST_F(zlibTest, invalid_data_and_short_buffer)
{
    // Arrange
    const Bytef invalid[] = {0xff, 0xff, 0xff}; // [状態] - zlib 形式ではないデータを用意する。
    Bytef output[16] = {};
    uLongf size = sizeof(output);
    uLongf short_size = 1;

    // Pre-Assert

    // Act
    const int invalid_ret =
        zlib_app_uncompress(output, &size, invalid, sizeof(invalid)); // [手順] - 不正データを指定して zlib_app_uncompress() を呼び出す。

    // Assert
    EXPECT_EQ(Z_DATA_ERROR,
              invalid_ret); // [確認_異常系] - 不正データに対する zlib_app_uncompress() の戻り値が Z_DATA_ERROR であること。

    // Act_2
    const int short_ret = zlib_app_compress(output, &short_size, invalid,
                                   sizeof(invalid)); // [手順] - 出力領域を 1 バイトに制限して zlib_app_compress() を呼び出す。

    // Assert_2
    EXPECT_EQ(
        Z_BUF_ERROR,
        short_ret); // [確認_異常系] - 出力領域を 1 バイトに制限した zlib_app_compress() の戻り値が Z_BUF_ERROR であること。
}
// [サブ手順参照 名前=zlibTest.TearDown]

// 既知の入力に対して CRC-32 および Adler-32 チェックサムが正しく計算されることの確認
TEST_F(zlibTest, checksum_known_vector)
{
    // Arrange
    const Bytef source[] = "123456789"; // [状態] - チェックサムの既知の入力を用意する。

    // Pre-Assert

    // Act
    const uLong crc = zlib_app_crc32(0, source, 9);     // [手順] - "123456789" を指定して zlib_app_crc32() を呼び出す。
    const uLong adler = zlib_app_adler32(1, source, 9); // [手順] - "123456789" を指定して zlib_app_adler32() を呼び出す。

    // Assert
    EXPECT_EQ(0xcbf43926UL, crc);   // [確認_正常系] - zlib_app_crc32() の戻り値が 0xcbf43926UL であること。
    EXPECT_EQ(0x091e01deUL, adler); // [確認_正常系] - zlib_app_adler32() の戻り値が 0x091e01deUL であること。
}
// [サブ手順参照 名前=zlibTest.TearDown]

// gzip ファイルの圧縮書き込みおよび展開読み込みのラウンド トリップが成功することの確認
TEST_F(zlibTest, gzip_file_roundtrip)
{
    // Arrange
    const std::array<Bytef, 6> source = {0, 255, 1, 2, 0, 128}; // [状態] - NUL を含むデータを用意する。
    std::array<Bytef, 6> restored = {};

    // Pre-Assert

    // Act
    gzFile writer =
        zlib_app_gzopen("zlib_roundtrip_test.gz", "wb"); // [手順] - 書き込み用の "wb" を指定して zlib_app_gzopen() を呼び出す。
    ASSERT_NE(nullptr, writer);
    // [確認_正常系] - 書き込み用の zlib_app_gzopen() の戻り値が nullptr 以外であること。
    const int written =
        zlib_app_gzwrite(writer, source.data(),
                static_cast<unsigned>(source.size())); // [手順] - バイナリを指定して zlib_app_gzwrite() を呼び出す。
    const int write_close_ret = zlib_app_gzclose(writer);
    gzFile reader =
        zlib_app_gzopen("zlib_roundtrip_test.gz", "rb"); // [手順] - 読み込み用の "rb" を指定して zlib_app_gzopen() を呼び出す。
    ASSERT_NE(nullptr, reader);
    // [確認_正常系] - 読み込み用の zlib_app_gzopen() の戻り値が nullptr 以外であること。
    const int read_size = zlib_app_gzread(
        reader, restored.data(),
        static_cast<unsigned>(restored.size())); // [手順] - 読み込み先のバッファーを指定して zlib_app_gzread() を呼び出す。
    const int read_close_ret = zlib_app_gzclose(reader);

    // Assert
    EXPECT_EQ(6, written);            // [確認_正常系] - zlib_app_gzwrite() の戻り値が 6 であること。
    EXPECT_EQ(6, read_size);          // [確認_正常系] - zlib_app_gzread() の戻り値が 6 であること。
    EXPECT_EQ(Z_OK, write_close_ret); // [確認_正常系] - 書き込み後の zlib_app_gzclose() の戻り値が Z_OK であること。
    EXPECT_EQ(Z_OK, read_close_ret);  // [確認_正常系] - 読み込み後の zlib_app_gzclose() の戻り値が Z_OK であること。
    EXPECT_EQ(source, restored);      // [確認_正常系] - restored が source と一致すること。
}
// [サブ手順参照 名前=zlibTest.TearDown]

// size_t 版 API による圧縮・展開のラウンド トリップが成功することの確認
TEST_F(zlibTest, size_t_api_roundtrip)
{
    // Arrange
    const Bytef source[] = "zlib 1.3.2"; // [状態] - size_t 版 API の入力を用意する。
    Bytef output[128] = {};
    Bytef restored[sizeof(source)] = {};
    z_size_t output_size = sizeof(output);
    z_size_t restored_size = sizeof(restored);

    // Pre-Assert

    // Act
    const int compress_ret =
        zlib_app_compress_z(output, &output_size, source, sizeof(source)); // [手順] - size_t 版の zlib_app_compress_z() を呼び出す。
    const int restore_ret =
        zlib_app_uncompress_z(restored, &restored_size, output, output_size); // [手順] - size_t 版の zlib_app_uncompress_z() を呼び出す。

    // Assert
    EXPECT_EQ(Z_OK, compress_ret);            // [確認_正常系] - zlib_app_compress_z() の戻り値が Z_OK であること。
    EXPECT_EQ(Z_OK, restore_ret);             // [確認_正常系] - zlib_app_uncompress_z() の戻り値が Z_OK であること。
    EXPECT_EQ(sizeof(source), restored_size); // [確認_正常系] - restored_size が sizeof(source) と一致すること。
    EXPECT_THAT(restored,
                ElementsAreArray(source)); // [確認_正常系] - restored の各要素が source の各要素と順に一致すること。
}
// [サブ手順参照 名前=zlibTest.TearDown]

// 展開先バッファー不足時に Z_BUF_ERROR が返されることの確認
TEST_F(zlibTest, decompression_buffer_too_small)
{
    // Arrange
    const Bytef source[] = "restored data exceeds one byte"; // [状態] - 展開先より大きいデータを用意する。
    Bytef compressed[128] = {};
    uLongf compressed_size = sizeof(compressed);
    ASSERT_EQ(Z_OK, zlib_app_compress(compressed, &compressed_size, source,
                             sizeof(source))); // [状態確認] - zlib_app_compress() の戻り値が Z_OK であること。
    Bytef restored[1] = {};
    uLongf restored_size = sizeof(restored); // [状態] - 展開先を 1 バイトに制限する。

    // Pre-Assert

    // Act
    const int ret = zlib_app_uncompress(restored, &restored_size, compressed,
                               compressed_size); // [手順] - 狭い展開先を指定して zlib_app_uncompress() を呼び出す。

    // Assert
    EXPECT_EQ(Z_BUF_ERROR, ret); // [確認_異常系] - zlib_app_uncompress() の戻り値が Z_BUF_ERROR であること。
}
// [サブ手順参照 名前=zlibTest.TearDown]

// 非圧縮ファイルの EOF より先へ移動し、実 OS の 64 bit ファイル位置を確認する。
TEST_F(zlibTest, file_offsets_beyond_32_bits)
{
    // Arrange
    std::ofstream seed("zlib_roundtrip_test.gz", std::ios::binary);
    ASSERT_TRUE(seed.is_open());                        // [状態確認] - 入力ファイルを作成できること。
    ASSERT_TRUE(static_cast<bool>(seed.write("x", 1))); // [状態確認] - 非圧縮データを 1 byte 書き込めること。
    seed.close();
    std::unique_ptr<gzFile_s, decltype(&zlib_app_gzclose)> file(zlib_app_gzopen("zlib_roundtrip_test.gz", "rb"), &zlib_app_gzclose);
    ASSERT_NE(nullptr, file);                 // [状態確認] - 読み込み用ハンドルを取得できること。
    ASSERT_EQ(1, zlib_app_gzdirect(file.get()));       // [状態確認] - 非圧縮ファイルの直接読み取りになっていること。
    const z_off64_t position = 0x100000011LL; // [状態] - 4 GiB を超える位置を用意する。

    // Pre-Assert

    // Act
    const z_off_t seek = zlib_app_gzseek(file.get(), position, SEEK_SET); // [手順] - 通常 API で 4 GiB を超える位置へ移動する。
    const z_off_t tell = zlib_app_gztell(file.get());                     // [手順] - 論理位置を取得する。
    const z_off_t offset = zlib_app_gzoffset(file.get());                 // [手順] - OS のファイル位置を取得する。
    const z_off64_t back = zlib_app_gzseek64(file.get(), -0x80000000LL, SEEK_CUR); // [手順] - 64-bit API で 2 GiB 戻る。
    const z_off64_t tell64 = zlib_app_gztell64(file.get());                        // [手順] - 64-bit API で論理位置を取得する。
    const z_off64_t offset64 = zlib_app_gzoffset64(file.get()); // [手順] - 64-bit API で OS のファイル位置を取得する。

    // Assert
    EXPECT_EQ(position, seek);         // [確認_正常系] - 通常 API の移動結果が 4 GiB を超えても切り詰められないこと。
    EXPECT_EQ(position, tell);         // [確認_正常系] - 通常 API の論理位置が一致すること。
    EXPECT_EQ(position, offset);       // [確認_正常系] - 通常 API の OS ファイル位置が一致すること。
    EXPECT_EQ(0x80000011LL, back);     // [確認_正常系] - 負の相対移動後も 2 GiB を超える位置が保持されること。
    EXPECT_EQ(0x80000011LL, tell64);   // [確認_正常系] - 64-bit API の論理位置が一致すること。
    EXPECT_EQ(0x80000011LL, offset64); // [確認_正常系] - 64-bit API の OS ファイル位置が一致すること。
}
// [サブ手順参照 名前=zlibTest.TearDown]
