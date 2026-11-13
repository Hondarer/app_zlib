LIBS += mock_zlib
DEFINES += MOCK_ZLIB_ENABLE_CALLER
TEST_SRCS += $(MYAPP_DIR)/test/src/libzlibTest/mockZlibTest/caller_info.c \
    $(MYAPP_DIR)/test/src/libzlibTest/mockZlibTest/caller_info_cpp.cc
