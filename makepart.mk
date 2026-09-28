# make 読み込み時に展開し、子ディレクトリのソース探索より先に準備する。
# see: framework/makefw/docs/makeparts.md
# 同じ make 実行内の子 make では、アーカイブ・パッチ・展開スクリプトが変わらないため、
# 確認済みの app ディレクトリを ZLIB_EXTRACT_DONE で引き継ぎ、Python の起動を 1 回に抑える。
ifndef MAKEFW_SYNC_EVAL
    ifneq ($(ZLIB_EXTRACT_DONE),$(MYAPP_DIR))
        ZLIB_EXTRACT_STATUS := $(shell python3 "$(MYAPP_DIR)/bin_internal/extract_package.py" --app-dir "$(MYAPP_DIR)" --makefw-home "$(MAKEFW_HOME)" >&2; echo $$?)
        ifneq ($(ZLIB_EXTRACT_STATUS),0)
            $(error zlib パッケージの準備に失敗しました。上記のメッセージを確認してください)
        endif
        export ZLIB_EXTRACT_DONE := $(MYAPP_DIR)
    endif
endif

ifdef PLATFORM_LINUX
    # 64-bit offset API と unistd.h の宣言を本体・テストで統一する。
    # see: https://github.com/madler/zlib/blob/v1.3.2/zconf.h
    DEFINES += _LARGEFILE64_SOURCE=1
endif
