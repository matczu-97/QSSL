#pragma once
#include "crypto_functions.h"

#define PORT 8080
#define BUFFER_SIZE 4096


// Forward declarations of structs
typedef struct Server Server;

// Function pointer typedefs
typedef void (*ServerCleanupFunc)(Server* server);

struct Server {
    // Data members
    RSA* rsa_private_key;
    RSA* rsa_public_key;
    EC_KEY* ecc_private_key;
    EC_KEY* ecc_public_key;
    /*Dillitium and Kyber here*/
    uint8_t kyber_private_key[OQS_KEM_kyber_768_length_secret_key];
    uint8_t kyber_public_key[OQS_KEM_kyber_768_length_secret_key];
    uint8_t kyber_shared_secret[OQS_KEM_kyber_768_length_shared_secret];

    BOOL is_initialized;

    ServerCleanupFunc cleanup;
};
