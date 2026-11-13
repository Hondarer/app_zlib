# 配布ソースは生成物であり、直接編集・整形しない。
# 第三者管理ライブラリは、OS やほかの製品の同名ライブラリと区別するため、名前に接尾辞 _app を付ける。
TARGET := zlib_app
LIB_TYPE = shared
ifdef PLATFORM_WINDOWS
    CFLAGS += /FI"$(MYAPP_DIR)/prod/include_internal/zlib_app_namespace.h"
else
    CFLAGS += -include "$(MYAPP_DIR)/prod/include_internal/zlib_app_namespace.h"
    LDFLAGS += -Wl,-soname,libzlib_app.so
endif
ifdef PLATFORM_WINDOWS
    # zutil.h / gzguts.h が ZLIB_INTERNAL を定義する。
    # see: https://github.com/madler/zlib/blob/v1.4/zconf.h
    DEFINES += ZLIB_DLL
else ifdef PLATFORM_LINUX
    DEFINES += ZLIB_API_VISIBILITY
    # upstream の構造体配置・整数変換・switch に由来する警告だけを抑制する。
    CFLAGS += -fvisibility=hidden -Wno-padded -Wno-cast-qual -Wno-conversion -Wno-sign-conversion -Wno-switch-default -Wno-switch-enum
endif
