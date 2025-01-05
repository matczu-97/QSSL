#pragma once
#ifndef SERVER_H
#define SERVER_H
#include "params.h"

// Forward declare Server for use in function pointer types
typedef struct Server Server;

// Function pointer types for Server methods
typedef BOOL(*VerifySignatureFunc)(Server* server,
    unsigned char* encrypted_aes_key,
    size_t encrypted_key_len,
    unsigned char* signature,
    unsigned int sig_len);

typedef BOOL(*DecryptAesKeyFunc)(Server* server,
    unsigned char* encrypted_aes_key,
    size_t encrypted_key_len,
    unsigned char* aes_key,
    size_t* aes_key_len);

typedef BOOL(*DecryptDataFunc)(Server* server,
    unsigned char* encrypted_data,
    size_t encrypted_len,
    unsigned char* aes_key,
    unsigned char* iv,
    unsigned char* decrypted_data,
    size_t* decrypted_len);

typedef void (*CleanupFunc)(Server* server);

// Server struct with embedded function pointers
struct Server {
    // Data members
    RSA* rsa_private_key;    // For decrypting AES key
    RSA* rsa_public_key;     // For sharing with clients
    EC_KEY* ecc_private_key; // For signing
    EC_KEY* ecc_public_key;  // For verification
    BOOL is_initialized;
    
    // aes key to keep after discovered
    unsigned char aes_key[AES_KEY_SIZE]; 
    size_t aes_key_len;
    // decrypted data to decrypte 
    unsigned char decrypted_data[MAX_DATA_SIZE];
    size_t decrypted_len;

    // Method members (function pointers)
    VerifySignatureFunc verify_signature;
    DecryptAesKeyFunc decrypt_aes_key;
    DecryptDataFunc decrypt_data;
    CleanupFunc cleanup;
};

// Function declarations for actual implementations
static BOOL impl_verify_signature(Server* server,
    unsigned char* encrypted_aes_key,
    size_t encrypted_key_len,
    unsigned char* signature,
    unsigned int sig_len);

static BOOL impl_decrypt_aes_key(Server* server,
    unsigned char* encrypted_aes_key,
    size_t encrypted_key_len,
    unsigned char* aes_key,
    size_t* aes_key_len);

static BOOL impl_decrypt_data(Server* server ,
    unsigned char* encrypted_data,
    size_t encrypted_len,
    unsigned char* aes_key,
    unsigned char* iv,
    unsigned char* decrypted_data,
    size_t* decrypted_len);

static void impl_cleanup_server(Server* server);

// Constructor-like function
BOOL server_init(Server* server);


#ifdef SERVER_IMPLEMENTATION

static BOOL impl_verify_signature(Server* server,
    unsigned char* encrypted_aes_key,
    size_t encrypted_key_len,
    unsigned char* signature,
    unsigned int sig_len) {
    if (!server || !server->is_initialized) {
        return FALSE;
    }

    // Calculate hash of the encrypted AES key
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, encrypted_aes_key, encrypted_key_len);
    SHA256_Final(hash, &sha256);

    // Create signature object and set r,s values
    ECDSA_SIG* ecc_sig = ECDSA_SIG_new();
    if (!ecc_sig) {
        return FALSE;
    }

    BIGNUM* r = BN_bin2bn(signature, sig_len / 2, NULL);
    BIGNUM* s = BN_bin2bn(signature + sig_len / 2, sig_len / 2, NULL);

    if (!r || !s) {
        ECDSA_SIG_free(ecc_sig);
        BN_free(r);
        BN_free(s);
        return FALSE;
    }

    ECDSA_SIG_set0(ecc_sig, r, s);

    // Verify the signature using the public ECC key
    int result = ECDSA_do_verify(hash, SHA256_DIGEST_LENGTH, ecc_sig, server->ecc_public_key);
    ECDSA_SIG_free(ecc_sig);

    return (result == 1) ? TRUE : FALSE;
}

static BOOL impl_decrypt_aes_key(Server* server,
    unsigned char* encrypted_aes_key,
    size_t encrypted_key_len,
    unsigned char* aes_key,
    size_t* aes_key_len) {
    if (!server || !server->is_initialized) {
        return FALSE;
    }

    int result = RSA_private_decrypt(encrypted_key_len,
        encrypted_aes_key,
        aes_key,
        server->rsa_private_key,
        RSA_PKCS1_OAEP_PADDING);

    if (result == -1) {
        return FALSE;
    }

    *aes_key_len = result;
    return TRUE;
}

static BOOL impl_decrypt_data(Server* server,
    unsigned char* encrypted_data,
    size_t encrypted_len,
    unsigned char* aes_key,
    unsigned char* iv,
    unsigned char* decrypted_data,
    size_t* decrypted_len) {
    if (!server || !server->is_initialized) {
        return FALSE;
    }

    AES_KEY aes;
    if (AES_set_decrypt_key(aes_key, AES_KEY_SIZE * 8, &aes) < 0) {
        return FALSE;
    }

    unsigned char temp_iv[AES_BLOCK_SIZE];
    memcpy(temp_iv, iv, AES_BLOCK_SIZE);

    // Decrypt all blocks
    for (size_t i = 0; i < encrypted_len; i += AES_BLOCK_SIZE) {
        AES_cbc_encrypt(encrypted_data + i,
            decrypted_data + i,
            AES_BLOCK_SIZE,
            &aes,
            temp_iv,
            AES_DECRYPT);
    }

    // Remove padding
    unsigned char padding_len = decrypted_data[encrypted_len - 1];
    *decrypted_len = encrypted_len - padding_len;

    return TRUE;
}

static void impl_cleanup_server(Server* server) {
    if (!server) return;

    if (server->rsa_private_key) {
        RSA_free(server->rsa_private_key);
        server->rsa_private_key = NULL;
    }

    if (server->rsa_public_key) {
        RSA_free(server->rsa_public_key);
        server->rsa_public_key = NULL;
    }

    if (server->ecc_private_key) {
        EC_KEY_free(server->ecc_private_key);
        server->ecc_private_key = NULL;
    }

    if (server->ecc_public_key) {
        EC_KEY_free(server->ecc_public_key);
        server->ecc_public_key = NULL;
    }

    server->is_initialized = FALSE;
    EVP_cleanup();
    ERR_free_strings();
}


// Generate RSA key pair
void generate_rsa_keys(RSA** rsa_private_key, RSA** rsa_public_key) {
    BIGNUM* bn = BN_new();
    if (!BN_set_word(bn, RSA_F4)) {
        handle_openssl_error();
    }

    *rsa_private_key = RSA_new();
    if (!RSA_generate_key_ex(*rsa_private_key, RSA_KEY_SIZE, bn, NULL)) {
        handle_openssl_error();
    }

    // Extract public key
    *rsa_public_key = RSAPublicKey_dup(*rsa_private_key);
    if (*rsa_public_key == NULL) {
        handle_openssl_error();
    }

    BN_free(bn);
}

// Generate ECC key pair
void generate_ecc_keys(EC_KEY** ecc_private_key, EC_KEY** ecc_public_key) {
    *ecc_private_key = EC_KEY_new_by_curve_name(NID_secp256k1);
    if (!*ecc_private_key) {
        handle_openssl_error();
    }

    if (!EC_KEY_generate_key(*ecc_private_key)) {
        handle_openssl_error();
    }

    *ecc_public_key = EC_KEY_new_by_curve_name(NID_secp256k1);
    if (!*ecc_public_key) {
        handle_openssl_error();
    }

    const EC_POINT* pub_key = EC_KEY_get0_public_key(*ecc_private_key);
    if (!EC_KEY_set_public_key(*ecc_public_key, pub_key)) {
        handle_openssl_error();
    }
}



BOOL server_init(Server* server) {
    if (!server) {
        return FALSE;
    }

    generate_rsa_keys(&server->rsa_private_key,&server->rsa_public_key);
    generate_ecc_keys(&server->ecc_private_key,&server->ecc_public_key);

    // Initialize function pointers
    server->verify_signature = impl_verify_signature;
    server->decrypt_aes_key = impl_decrypt_aes_key;
    server->decrypt_data = impl_decrypt_data;
    server->cleanup = impl_cleanup_server;

    server->is_initialized = TRUE;
    return TRUE;
}

#endif /* SERVER_IMPLEMENTATION */
#endif /* SERVER_H */