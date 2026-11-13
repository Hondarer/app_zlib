# zlib へのパッチ

展開後に makefw の `apply_patches.py` で番号順に適用します。  
展開されたファイルを直接編集せず、変更を unified diff として管理します。

## 0001-gcc-public-visibility.patch

Linux の共有ライブラリを `-fvisibility=hidden` で構築し、`ZLIB_API_VISIBILITY` 定義時に `ZEXTERN` で宣言した公開関数だけを公開します。  
内部関数と内部データの流出を防ぎ、公開関数と API モックの網羅性を照合できるようにします。  
圧縮アルゴリズム、型、関数シグネチャ、Windows の DLL 宣言は変更しません。  
パッチは `prod/include/zconf.h` に適用します。

## 0002-windows-64-bit-offset.patch

Windows の `z_off_t` の既定を `long long` とし、1.3.2 と同じ 64-bit offset を維持します。  
zlib 1.4 の既定値である `long` は Windows では 32 bit のため、通常の `gzseek`、`gztell`、チェックサム結合 API でも 64 bit の値を扱えるようにします。  
`_FILE_OFFSET_BITS=64` による通常名から `*64` 名への置換を避け、実ライブラリと API モックで両方の関数名を維持します。

パッチは `prod/include/zconf.h` に適用し、DLL、利用側、モックで同じ型定義を使用します。  
型の変更と公式の 64-bit offset 設定は [公式リリース情報](https://zlib.net/zlib.html) を参照してください。

## 0003-windows-64-bit-api.patch

`Z_WINLFS` が定義されるネイティブ Windows 環境では、`Z_LARGE64` を自動で有効にし、公開ヘッダーで `*64` API を宣言します。  
利用側で `_LARGEFILE64_SOURCE` を指定する必要はありません。  
Linux の有効化条件と通常の API 名は維持します。

パッチは `prod/include/zconf.h` に適用し、DLL、利用側、モックで同じ宣言を使用します。

## 0004-public-api-namespace-gzgetc.patch

`gzgetc` 関数の定義直前に、元名から固有名への内部対応マクロではなく、固有名の関数形式マクロを解除します。これにより、関数定義と関数形式マクロの双方が `zlib_app_gzgetc` を参照します。

パッチは `prod/libsrc/zlib/gzread.c` に適用します。
