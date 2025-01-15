#pragma once
#include "crypto_functions.h"

#define PORT 8080
#define BUFFER_SIZE 4096

// Forward declarations of structs
typedef struct Client Client;

// Function pointer typedefs
typedef void (*ClientCleanupFunc)(Client* client);

struct Client {
    // Data members
    EC_KEY* ecc_private_key; // For signing
    EC_KEY* ecc_public_key;  // For verification
    /*Dillitium here*/
    BOOL is_initialized;

    // Generate AES key
    unsigned char aes_key[AES_KEY_SIZE];
    size_t aes_key_len;

    ClientCleanupFunc cleanup;
};