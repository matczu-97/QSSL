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

    BOOL is_initialized;

    ServerCleanupFunc cleanup;
};