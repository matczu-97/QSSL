#pragma once
#ifndef CLIENT_H
#define CLIENT_H

#include "params.h"

// Forward declare Client for use in function pointer types
typedef struct Client Client;

// Function pointer types for Client methods
typedef BOOL(*SignAesKeyFunc)(Client* client,
    unsigned char* encrypted_aes_key,
    size_t encrypted_key_len,
    unsigned char* signature,
    unsigned int* sig_len);

typedef BOOL(*EncryptAesKeyFunc)(Client* client,
    unsigned char* aes_key,
    size_t aes_key_len,
    unsigned char* encrypted_aes_key,
    size_t* encrypted_key_len);

typedef BOOL(*EncryptDataFunc)(Client* client,
    unsigned char* data,
    size_t data_len,
    unsigned char* aes_key,
    unsigned char* iv,
    unsigned char* encrypted_data,
    size_t* encrypted_len);

typedef void (*CleanupFunc)(Client* client);

// Client struct with embedded function pointers
struct Client {
    // Data members
    RSA* rsa_public_key;
    EC_KEY* ecc_private_key;
    BOOL is_initialized;

    // Generate AES key
    unsigned char aes_key[AES_KEY_SIZE];

    // Method members (function pointers)
    SignAesKeyFunc sign_aes_key;
    EncryptAesKeyFunc encrypt_aes_key;
    EncryptDataFunc encrypt_data;
    CleanupFunc cleanup;
};

// Function declarations for actual implementations
static BOOL impl_sign_aes_key(Client* client,
    unsigned char* encrypted_aes_key,
    size_t encrypted_key_len,
    unsigned char* signature,
    unsigned int* sig_len);

static BOOL impl_encrypt_aes_key(Client* client,
    unsigned char* aes_key,
    size_t aes_key_len,
    unsigned char* encrypted_aes_key,
    size_t* encrypted_key_len);

static BOOL impl_encrypt_data(Client* client,
    unsigned char* data,
    size_t data_len,
    unsigned char* aes_key,
    unsigned char* iv,
    unsigned char* encrypted_data,
    size_t* encrypted_len);

static void impl_cleanup(Client* client);

// Constructor-like function
BOOL client_init(Client* client);



static BOOL impl_sign_aes_key(Client* client,
    unsigned char* encrypted_aes_key,
    size_t encrypted_key_len,
    unsigned char* signature,
    unsigned int* sig_len) {
    if (!client || !client->is_initialized) {
        return FALSE;
    }

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, encrypted_aes_key, encrypted_key_len);
    SHA256_Final(hash, &sha256);

    ECDSA_SIG* ecc_sig = ECDSA_do_sign(hash, SHA256_DIGEST_LENGTH,
        client->ecc_private_key);
    if (!ecc_sig) {
        return FALSE;
    }

    const BIGNUM* r, * s;
    ECDSA_SIG_get0(ecc_sig, &r, &s);

    *sig_len = BN_num_bytes(r) + BN_num_bytes(s);
    BN_bn2bin(r, signature);
    BN_bn2bin(s, signature + BN_num_bytes(r));

    ECDSA_SIG_free(ecc_sig);
    return TRUE;
}

static BOOL impl_encrypt_aes_key(Client* client,
    unsigned char* aes_key,
    size_t aes_key_len,
    unsigned char* encrypted_aes_key,
    size_t* encrypted_key_len) {
    if (!client || !client->is_initialized) {
        return FALSE;
    }

    int result = RSA_public_encrypt(aes_key_len, aes_key,
        encrypted_aes_key,
        client->rsa_public_key,
        RSA_PKCS1_OAEP_PADDING);

    if (result == -1) {
        return FALSE;
    }

    *encrypted_key_len = result;
    return TRUE;
}

static BOOL impl_encrypt_data(Client* client,
    unsigned char* data,
    size_t data_len,
    unsigned char* aes_key,
    unsigned char* iv,
    unsigned char* encrypted_data,
    size_t* encrypted_len) {
    if (!client || !client->is_initialized) {
        return FALSE;
    }

    if (!RAND_bytes(iv, AES_BLOCK_SIZE)) {
        return FALSE;
    }

    AES_KEY aes;
    if (AES_set_encrypt_key(aes_key, AES_KEY_SIZE * 8, &aes) < 0) {
        return FALSE;
    }

    size_t padded_len = ((data_len + AES_BLOCK_SIZE - 1) / AES_BLOCK_SIZE)
        * AES_BLOCK_SIZE;
    unsigned char* padded_data = (unsigned char*)malloc(padded_len);
    if (!padded_data) {
        return FALSE;
    }

    memcpy(padded_data, data, data_len);
    size_t padding_len = padded_len - data_len;
    memset(padded_data + data_len, padding_len, padding_len);

    unsigned char temp_iv[AES_BLOCK_SIZE];
    memcpy(temp_iv, iv, AES_BLOCK_SIZE);

    for (size_t i = 0; i < padded_len; i += AES_BLOCK_SIZE) {
        AES_cbc_encrypt(padded_data + i, encrypted_data + i,
            AES_BLOCK_SIZE, &aes, temp_iv, AES_ENCRYPT);
    }

    free(padded_data);
    *encrypted_len = padded_len;
    return TRUE;
}

static void impl_cleanup_client(Client* client) {
    if (!client) return;

    if (client->rsa_public_key) {
        RSA_free(client->rsa_public_key);
        client->rsa_public_key = NULL;
    }

    if (client->ecc_private_key) {
        EC_KEY_free(client->ecc_private_key);
        client->ecc_private_key = NULL;
    }

    client->is_initialized = FALSE;
    EVP_cleanup();
    ERR_free_strings();
}

BOOL client_init(Client* client, RSA** rsa_public_key, EC_KEY** ecc_private_key) {
    if (!client) {
        return FALSE;
    }
    if (client->is_initialized == TRUE)
    {
        return TRUE;
    }

    // initialize aes key
    if (!RAND_bytes(client->aes_key, AES_KEY_SIZE)) {
        handle_openssl_error();
    }

    client->rsa_public_key = *rsa_public_key;
    client->ecc_private_key = *ecc_private_key;

    // Initialize function pointers
    client->sign_aes_key = impl_sign_aes_key;
    client->encrypt_aes_key = impl_encrypt_aes_key;
    client->encrypt_data = impl_encrypt_data;
    client->cleanup = impl_cleanup_client;

    client->is_initialized = TRUE;
    return TRUE;
}


#endif /* CLIENT_H */