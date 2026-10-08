/**
 *******************************************************************************
 *  @file           zlib_sample.c
 *  @brief          固定データを zlib で圧縮・展開し、復元結果を検証するサンプル コマンドを実装します。
 *  @author         Tetsuo Honda
 *  @date           2026/08/17
 *  @version        1.0.0
 *
 *  メモリー上で圧縮と展開を行い、復元データが元データと一致することを検証して、
 *  結果を標準出力へ表示します。
 *
 *  @copyright      Copyright (C) Tetsuo Honda. 2026. All rights reserved.
 *
 *******************************************************************************
 */

#include <stdio.h>
#include <string.h>

#include <zlib.h>

/**
 *  @brief          プログラムのエントリ ポイント。
 *
 *  @param[in]      argc コマンド ライン引数の個数 (使用しません)。
 *  @param[in]      argv コマンド ライン引数の文字列の配列 (使用しません。NULL を許容します)。
 *  @retval         0 復元したデータが元データと一致しました。
 *  @retval         1 圧縮、展開、データ比較、または結果の出力に失敗しました。
 *
 *  @par            スレッド セーフ
 *  起動時にメイン スレッドから呼び出します。
 */
int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    const Bytef source[] = "zlib sample: compress and restore";
    Bytef compressed[128] = {0};
    Bytef restored[sizeof(source)] = {0};
    uLongf compressed_size = sizeof(compressed);
    int ret = compress2(compressed, &compressed_size, source, sizeof(source), Z_BEST_COMPRESSION);
    if (ret != Z_OK)
    {
        fprintf(stderr, "Compression failed: %d\n", ret);
        return 1;
    }

    uLongf restored_size = sizeof(restored);
    ret = uncompress(restored, &restored_size, compressed, compressed_size);
    if (ret != Z_OK)
    {
        fprintf(stderr, "Decompression failed: %d\n", ret);
        return 1;
    }
    if (restored_size != sizeof(source) || memcmp(source, restored, sizeof(source)) != 0)
    {
        fprintf(stderr, "Restored data does not match\n");
        return 1;
    }
    ret = printf("zlib %s: %zu -> %lu -> %lu bytes, round trip OK\n", zlibVersion(), sizeof(source), compressed_size,
                 restored_size);
    if (ret < 0)
    {
        return 1;
    }
    return 0;
}
