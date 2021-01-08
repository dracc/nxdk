MBEDTLS_DIR = $(NXDK_DIR)/lib/mbedtls/mbedtls
MBEDTLS_SRCS = $(MBEDTLS_DIR)/library/debug.c \
               $(MBEDTLS_DIR)/library/ssl_cache.c \
               $(MBEDTLS_DIR)/library/ssl_ciphersuites.c \
               $(MBEDTLS_DIR)/library/ssl_cli.c \
               $(MBEDTLS_DIR)/library/ssl_cookie.c \
               $(MBEDTLS_DIR)/library/ssl_msg.c \
               $(MBEDTLS_DIR)/library/ssl_srv.c \
               $(MBEDTLS_DIR)/library/ssl_ticket.c \
               $(MBEDTLS_DIR)/library/ssl_tls.c \
               $(MBEDTLS_DIR)/library/ssl_tls13_keys.c \
               $(MBEDTLS_DIR)/library/timing.c \


MBEDTLS_OBJS = $(addsuffix .obj, $(basename $(MBEDTLS_SRCS)))

NXDK_CFLAGS += -I$(MBEDTLS_DIR) \
               -I$(MBEDTLS_DIR)/library \
               -I$(MBEDTLS_DIR)/include \
               -I$(MBEDTLS_DIR)/include/mbedtls

$(NXDK_DIR)/lib/libmbedtls.lib: $(MBEDTLS_OBJS)

main.exe: $(NXDK_DIR)/lib/libmbedtls.lib

CLEANRULES += clean-mbedtls

.PHONY: clean-mbedtls
clean-mbedtls:
	$(VE)rm -f $(MBEDTLS_OBJS) $(NXDK_DIR)/lib/libmbedtls.lib
