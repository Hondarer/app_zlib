# zlib

作業前に [作業規則](AGENTS.md) を確認してください。

このリポジトリ (ディレクトリ) は [c-modernization-kit](https://github.com/Hondarer/c-modernization-kit) の `app/zlib` として管理される、個別アプリです。  
[zlib](https://zlib.net/) を makefw でビルドするためのラッパーを提供します。  
c-modernization-kit のワークスペース内 (`framework/makefw` 等と組み合わせた make ビルド環境) で利用することを前提としたアプリであり、本ディレクトリ単体ではビルドできません。

## 固有の公開 API 名で OS のライブラリと分離します

OS の同名ライブラリと同じプロセスで関数解決が混ざることを防ぐため、公開関数・変数と関数呼び出しマクロに `zlib_app_` を付けます。  
たとえば `compress` の利用側は `zlib_app_compress` を呼び出します。  
型名、定数、構造体フィールドは配布ライブラリの定義を使います。  
旧名の alias と mock ラッパーは提供しないため、既存の利用側はソースを更新して再ビルド・再リンクしてください。  
`dlsym()` と `GetProcAddress()` に渡す文字列も固有名へ変更します。

## 提供する機能

zlib 1.4 の圧縮・展開、チェックサム、gzip ファイル API を取り込みます。  
`contrib/minizip` と公式サンプル コマンドは取り込みません。

| 環境 | 共有ライブラリ | リンク指定 |
|---|---|---|
| Linux/GCC | `prod/lib/libzlib_app.so` | `LIBS += zlib_app` |
| Windows/MSVC | `prod/lib/libzlib_app.dll` と import library `libzlib_app.lib` | `LIBS += zlib_app` |

Table: 環境別の zlib 共有ライブラリとリンク指定

静的な実ライブラリは生成しません。  
OS にインストールされた `libz` や `zlib1.dll` の置き換えは行いません。  
公開ヘッダーは `<zlib.h>` と `<zconf.h>` を使い、公開 API 名に固有の接頭辞を付けます。  
Windows の DLL 本体にだけ `ZLIB_DLL` を指定します。  
利用側では未定義のまま import library でリンクでき、`ZLIB_WINAPI` は定義しません。

ほかの app から利用する場合は、その app の `appdeps.mk` に `APP_DEPS += zlib` を記載し、対象の `makepart.mk` にリンク指定を追加します。  
公開ヘッダーは system include として提供し、外来ヘッダーの警告を利用側から分離します。

## ビルドと更新

```sh
make -C app/zlib
make -C app/zlib test
python3 app/zlib/bin_test/test_extract_package.py
```

`packages/` の公式アーカイブから、公開ヘッダーを `prod/include/`、本体と内部ヘッダーを `prod/libsrc/zlib/` に自動展開します。  
展開物は Git 管理対象外です。  
本体への変更は [パッチ](patches/README.md) に記録します。

アーカイブ、展開スクリプト、パッチが変更された場合と、展開物が欠落した場合は自動的に再展開します。  
内容が同一のファイルは更新しません。  
パッケージの取得元、ハッシュ、更新手順は [packages/README.md](packages/README.md) を参照してください。

## 利用サンプル

`prod/cbin/zlib_sample` (Windows では `.exe`) は、固定データをメモリ上で圧縮・展開し、元データと比較します。  
一致した場合はバージョンと処理前後のサイズを表示して 0、失敗した場合は非 0 を返します。  
実行時の探索パスはワークスペースの環境設定を使用します。  
app の追加後は `make sync-app-env` で更新します。

## API モック

テスト対象ソースを `TEST_SRCS` に指定し、mock をリンクするテストの `makepart.mk` に `DEFINES += MOCK_ZLIB_ENABLE_CALLER` を追加します。  
これにより呼び出し元情報の注入を有効にし、実ライブラリを使うテストには注入しません。

mock の宣言と注入マクロは API 表から生成し、mock ライブラリのビルド前に自動更新します。  
生成済みの `test/include/mock_zlib_overrides.h` は直接編集せず、通常 API 表と、可変長 API がある場合は専用の可変長 API 表を変更してください。

通常の API 呼び出しは差し替えヘッダーを通し、mock の先頭 3 引数へ呼び出し元のファイル名、行番号、関数名を渡します。  
`EXPECT_CALL` と `ON_CALL` でも、この 3 引数を実 API の引数より先に指定します。  
新しい固有名の関数アドレスを取得して関数ポインター経由で呼び出す場合も mock に到達しますが、呼び出し元は取得できないため、ファイル名と関数名は `"<unknown>"`、行番号は `0` になります。  
実委譲は app の固有シンボルだけを解決し、OS 側の旧名へ戻す探索は行いません。

単体テストでは `<mock_zlib.h>` と `LIBS += mock_zlib` を使用します。  
モックを生成しない場合と、個別の動作を設定していない場合は、`libzlib_app` の実関数へ委譲します。  
実ライブラリのロードとシンボル解決に失敗すると、理由を標準エラーへ出力して終了します。

```cpp
NiceMock<Mock_zlib> mock_zlib;
EXPECT_CALL(mock_zlib, zlib_app_compress(_, _, _, _, _, _, _)).WillOnce(Return(Z_MEM_ERROR));
```

実ライブラリとモックを同時にリンクしないでください。  
Windows ではモック本体と利用側の `ZLIB_DLL` を未定義にします。  
実委譲用ライブラリは Linux の `LD_LIBRARY_PATH`、Windows の `PATH` から読み込みます。  
同時に生成できる `Mock_zlib` は 1 個であり、生成・破棄・呼び出しを複数スレッドで並行実行しません。  
完全隔離では、対象が呼び出す関数すべてに動作を設定し、偽のストリームやファイル ハンドルを実関数へ渡さないでください。

### マクロと OS 差

| 呼び出し | モック対象 |
|---|---|
| `zlib_app_deflateInit`、`zlib_app_deflateInit2` | `zlib_app_deflateInit_`、`zlib_app_deflateInit2_` |
| `zlib_app_inflateInit`、`zlib_app_inflateInit2`、`zlib_app_inflateBackInit` | 末尾に `_` が付く実関数 |
| `zlib_app_zlib_version` | `zlib_app_zlibVersion`。オブジェクト形式の呼び出しマクロ |
| `zlib_app_gzprintf` | `zlib_app_gzvprintf` (書式文字列と `va_list`) |
| `zlib_app_gzgetc` | 実関数 `zlib_app_gzgetc`。モック ヘッダー内で同名マクロを解除 |

Table: zlib マクロ呼び出しとモック対象関数の対応一覧

テスト対象ソースが通常の `<zlib.h>` でコンパイルされた場合、`zlib_app_gzgetc` マクロは公開構造体を直接読み書きすることがあります。  
関数モックだけでその処理全体を差し替えることはできません。  
関数として呼び出す必要がある場合は `(zlib_app_gzgetc)(file)` と記述します。

Linux では `_LARGEFILE64_SOURCE=1` を指定し、64-bit offset API を含めて検証します。  
Windows のモックでは、本体がエクスポートする同 API の宣言を補います。  
Windows の通常 API も 64-bit offset を維持するため、公開ヘッダーへの [パッチ](patches/README.md#0002-windows-64-bit-offsetpatch) で `z_off_t` を `long long` にします。  
DLL、利用側、モックで同じヘッダーを使用し、`_FILE_OFFSET_BITS=64` による関数名の置換は使用しません。  
Windows 専用の `zlib_app_gzopen_w` は Windows の API 表と網羅テストだけに含めます。  
`Z_PREFIX`、`Z_SOLO`、呼び出し規約の変更はこの構成の対象外です。

## テスト

- `zlibTest`: メモリの往復、空・バイナリ、不正入力、バッファー不足、チェックサム、gzip ファイル、size_t 版 API
- `sampleTest`: サンプルの成功、圧縮・展開エラー、復元内容不一致
- `mockZlibTest`: 実委譲、既定動作、エラー差し替え、初期化マクロ、可変長引数
- `exportTest`: API 表、IDENT manifest、実ライブラリの全エクスポートの一致、関数シグネチャの一致

## ライセンス

zlib 本体のライセンスはアーカイブ内の `LICENSE` が正本です。  
展開後は `prod/libsrc/zlib/LICENSE` でも参照できます。  
本体の著作権表示とライセンス条件を維持してください。  
app 直下の `LICENSE` は手動作成したラッパー部分に適用します。
