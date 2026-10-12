#include <testfw.h>
#include <mock_zlib.h>

// 圧縮・展開のラウンドトリップが正常に成功することの確認
TEST(sampleTest, successful_roundtrip)
{
    // Arrange

    // Pre-Assert

    // Act
    const int ret = __real_main(0, nullptr); // [手順] - 実ライブラリへ委譲してサンプルを実行する。

    // Assert
    EXPECT_EQ(0, ret); // [確認_正常系] - __real_main() の戻り値が 0 であること。
}

// 圧縮エラー発生時にサンプルがエラー終了することの確認
TEST(sampleTest, compression_error)
{
    // Arrange
    NiceMock<Mock_zlib> mock_zlib; // [状態] - 既定動作の Mock_zlib を生成する。

    // Pre-Assert
    EXPECT_CALL(mock_zlib, compress2(_, _, _, _, Z_BEST_COMPRESSION))
        .WillOnce(Return(
            Z_MEM_ERROR)); // [Pre-Assert確認_異常系] - compress2() が Z_BEST_COMPRESSION を指定して 1 回呼び出されること。
                           // [Pre-Assert手順] - メモリー不足を返す。
    EXPECT_CALL(mock_zlib, uncompress(_, _, _, _))
        .Times(0); // [Pre-Assert確認_異常系] - uncompress() が呼び出されないこと。(圧縮失敗後は展開しないことの確認)

    // Act
    const int ret = __real_main(0, nullptr); // [手順] - 圧縮エラー時のサンプルを実行する。

    // Assert
    EXPECT_EQ(1, ret); // [確認_異常系] - __real_main() の戻り値が 1 であること。(圧縮エラーで終了したことの確認)
}

// 展開エラー発生時にサンプルがエラー終了することの確認
TEST(sampleTest, decompression_error)
{
    // Arrange
    NiceMock<Mock_zlib> mock_zlib; // [状態] - 既定動作の Mock_zlib を生成する。

    // Pre-Assert
    EXPECT_CALL(mock_zlib, uncompress(_, _, _, _))
        .WillOnce(Return(Z_DATA_ERROR)); // [Pre-Assert確認_異常系] - uncompress() が 1 回呼び出されること。
                                         // [Pre-Assert手順] - データ不正を返す。

    // Act
    const int ret = __real_main(0, nullptr); // [手順] - 展開エラー時のサンプルを実行する。

    // Assert
    EXPECT_EQ(1, ret); // [確認_異常系] - __real_main() の戻り値が 1 であること。(展開エラーで終了したことの確認)
}

// 復元データが不一致の場合にエラー終了することの確認
TEST(sampleTest, restored_data_mismatch)
{
    // Arrange
    NiceMock<Mock_zlib> mock_zlib; // [状態] - 既定動作の Mock_zlib を生成する。

    // Pre-Assert
    EXPECT_CALL(mock_zlib, uncompress(_, _, _, _))
        .WillOnce(Return(Z_OK)); // [Pre-Assert確認_異常系] - uncompress() が 1 回呼び出されること。
                                 // [Pre-Assert手順] - バッファーを更新せず成功を返す。

    // Act
    const int ret = __real_main(0, nullptr); // [手順] - 復元データが一致しないサンプルを実行する。

    // Assert
    EXPECT_EQ(1, ret); // [確認_異常系] - __real_main() の戻り値が 1 であること。(復元内容の不一致で終了したことの確認)
}
