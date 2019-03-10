LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE       := mbedTLS
LOCAL_CFLAGS       := -O3 -fshort-wchar -ffast-math -fomit-frame-pointer -I../lib/include
LOCAL_CPPFLAGS     := -O3 -fshort-wchar -ffast-math -fomit-frame-pointer -I../lib/include
LOCAL_CPP_FEATURES := # rtti exceptions
LOCAL_ARM_NEON     := true # force NEON usage for all files
LOCAL_SRC_FILES    := \
   ../../lib/library/aes.c \
   ../../lib/library/aesni.c \
   ../../lib/library/arc4.c \
   ../../lib/library/aria.c \
   ../../lib/library/asn1parse.c \
   ../../lib/library/asn1write.c \
   ../../lib/library/base64.c \
   ../../lib/library/bignum.c \
   ../../lib/library/blowfish.c \
   ../../lib/library/camellia.c \
   ../../lib/library/ccm.c \
   ../../lib/library/certs.c \
   ../../lib/library/chacha20.c \
   ../../lib/library/chachapoly.c \
   ../../lib/library/cipher.c \
   ../../lib/library/cipher_wrap.c \
   ../../lib/library/cmac.c \
   ../../lib/library/ctr_drbg.c \
   ../../lib/library/debug.c \
   ../../lib/library/des.c \
   ../../lib/library/dhm.c \
   ../../lib/library/ecdh.c \
   ../../lib/library/ecdsa.c \
   ../../lib/library/ecjpake.c \
   ../../lib/library/ecp.c \
   ../../lib/library/ecp_curves.c \
   ../../lib/library/entropy.c \
   ../../lib/library/entropy_poll.c \
   ../../lib/library/error.c \
   ../../lib/library/gcm.c \
   ../../lib/library/havege.c \
   ../../lib/library/hkdf.c \
   ../../lib/library/hmac_drbg.c \
   ../../lib/library/md.c \
   ../../lib/library/md_wrap.c \
   ../../lib/library/md2.c \
   ../../lib/library/md4.c \
   ../../lib/library/md5.c \
   ../../lib/library/memory_buffer_alloc.c \
   ../../lib/library/net_sockets.c \
   ../../lib/library/nist_kw.c \
   ../../lib/library/oid.c \
   ../../lib/library/padlock.c \
   ../../lib/library/pem.c \
   ../../lib/library/pk.c \
   ../../lib/library/pk_wrap.c \
   ../../lib/library/pkcs11.c \
   ../../lib/library/pkcs12.c \
   ../../lib/library/pkcs5.c \
   ../../lib/library/pkparse.c \
   ../../lib/library/pkwrite.c \
   ../../lib/library/platform.c \
   ../../lib/library/platform_util.c \
   ../../lib/library/poly1305.c \
   ../../lib/library/ripemd160.c \
   ../../lib/library/rsa.c \
   ../../lib/library/rsa_internal.c \
   ../../lib/library/sha1.c \
   ../../lib/library/sha256.c \
   ../../lib/library/sha512.c \
   ../../lib/library/ssl_cache.c \
   ../../lib/library/ssl_ciphersuites.c \
   ../../lib/library/ssl_cli.c \
   ../../lib/library/ssl_cookie.c \
   ../../lib/library/ssl_srv.c \
   ../../lib/library/ssl_ticket.c \
   ../../lib/library/ssl_tls.c \
   ../../lib/library/threading.c \
   ../../lib/library/timing.c \
   ../../lib/library/version.c \
   ../../lib/library/version_features.c \
   ../../lib/library/x509.c \
   ../../lib/library/x509_create.c \
   ../../lib/library/x509_crl.c \
   ../../lib/library/x509_crt.c \
   ../../lib/library/x509_csr.c \
   ../../lib/library/x509write_crt.c \
   ../../lib/library/x509write_csr.c \
   ../../lib/library/xtea.c

include $(BUILD_STATIC_LIBRARY)
