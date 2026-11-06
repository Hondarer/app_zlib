LIBS += zlib
# *64 API の宣言には _LARGEFILE64_SOURCE が必要 (Windows では自動定義されない)。
# see: https://github.com/madler/zlib/blob/v1.4/zconf.h
DEFINES += _LARGEFILE64_SOURCE
