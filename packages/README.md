# packages

[公式配布ページ](https://zlib.net/) から取得した `zlib-1.4.tar.gz` を同梱します。

- 取得元: <https://zlib.net/zlib-1.4.tar.gz>
- バージョン: 1.4
- SHA-256: `02dd159783677a749c04adbdd02b55b8a2bcc7e054f51b76b549199a916ad584`
- ライセンス: アーカイブ内の `LICENSE`

```sh
curl -L -o app/zlib/packages/zlib-1.4.tar.gz https://zlib.net/zlib-1.4.tar.gz
sha256sum app/zlib/packages/zlib-1.4.tar.gz
```

更新時は公式掲載のハッシュと照合し、古いアーカイブを新しいものに置き換え、この文書のバージョンとハッシュも更新します。  
アーカイブは常に 1 個を配置します。  
複数ある場合はファイル名のバージョンが最新のものを選び、未採用のファイルを警告します。  
バージョンを判定できない名前が混在する場合は更新日時で選択します。  
アーカイブがない場合は配置方法を表示してビルドを停止します。

更新後は `make -C app/zlib clean` で旧ヘッダーを使ったビルド成果物を削除し、`make -C app/zlib test` で公開 API の変更と実動作を確認します。  
公開関数が変わった場合は API 表と網羅テストも更新します。  
展開元に必要なファイルがない場合は、展開対象一覧を新バージョンの構成に合わせます。
