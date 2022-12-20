set(wolfssl_VERSION "5.5.3")
file(REAL_PATH "wolfssl-${wolfssl_VERSION}" wolfssl_THIRDPARTY_ROOT)

idf_component_register(
    SRC_DIRS
        "${wolfssl_THIRDPARTY_ROOT}/src"
        "${wolfssl_THIRDPARTY_ROOT}/wolfcrypt/src"
        # "${wolfssl_THIRDPARTY_ROOT}/wolfcrypt/src/port/Espressif"
    EXCLUDE_SRCS
        "${wolfssl_THIRDPARTY_ROOT}/src/bio.c"
        "${wolfssl_THIRDPARTY_ROOT}/wolfcrypt/src/aes_asm.S"
        "${wolfssl_THIRDPARTY_ROOT}/wolfcrypt/src/aes_gcm_asm.S"
        "${wolfssl_THIRDPARTY_ROOT}/wolfcrypt/src/chacha_asm.S"
        "${wolfssl_THIRDPARTY_ROOT}/wolfcrypt/src/fe_x25519_asm.S"
        "${wolfssl_THIRDPARTY_ROOT}/wolfcrypt/src/poly1305_asm.S"
        "${wolfssl_THIRDPARTY_ROOT}/wolfcrypt/src/sha256_asm.S"
        "${wolfssl_THIRDPARTY_ROOT}/wolfcrypt/src/sha512_asm.S"
        "${wolfssl_THIRDPARTY_ROOT}/wolfcrypt/src/sp_x86_64_asm.S"
    INCLUDE_DIRS "." "include" "${wolfssl_THIRDPARTY_ROOT}"
)

# list(APPEND WOLFSSL_COMPILE_OPTIONS -DWOLFSSL_ESPIDF -DWOLFSSL_ESPWROOM32)

idf_build_set_property(COMPILE_OPTIONS "${WOLFSSL_COMPILE_OPTIONS}" APPEND)
