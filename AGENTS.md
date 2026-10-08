# AGENTS.md

## 対象

このリポジトリは、zlib の公式配布アーカイブをワークスペースへ取り込むラッパーであり、zlib を利用するプログラムの単体テスト向け API モックも含みます。

## 参照先

作業に関係する文書の該当節を参照してください。

- [README.md](README.md)
- パッケージの展開と更新では [packages/README.md](packages/README.md)
- パッチや本体への変更では [patches/README.md](patches/README.md)

## 注意点

- `packages/` から展開された zlib 本体を直接編集しないでください。
- Windows の mock 利用側では `ZLIB_DLL` を定義せず、リンク先では `mock_zlib` を使用し、実ライブラリを同時リンクしないでください。
- mock の API 表を変更した場合は、公開関数の網羅テストと README.md の利用方法を確認してください。
- 展開スクリプトを変更した場合は、`python3 bin_test/test_extract_package.py` を実行してください。
