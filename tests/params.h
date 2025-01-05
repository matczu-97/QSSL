#pragma once
#include <stdio.h>
#include <string.h>

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/obj_mac.h>
#include <openssl/err.h>

#define AES_KEY_SIZE 32  // 256 bits
#define AES_BLOCK_SIZE 16
#define MAX_DATA_SIZE 1024
#define RSA_KEY_SIZE 2048

typedef enum { FALSE = 0, TRUE = 1 } BOOL;

#if defined(OPENSSL_VERSION)

// declare once to allow implement server and client functions
#define CLIENT_IMPLEMENTATION
#define SERVER_IMPLEMENTATION


#else /* OPENSSL_VERSION_NUMBER */
#error "OpenSSL is required for this implementation"
#endif /* OPENSSL_VERSION_NUMBER */



static void handle_openssl_error() {
    exit(1);
}
