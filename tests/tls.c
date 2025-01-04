#include <stdio.h>
#include <string.h>

#include "server.h"
#include "client.h"




int main() {
    // Initialize OpenSSL
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();

    Client client;
    Server server;

    if (!server_init(&server)) {
        fprintf(stderr, "Failed to initialize server\n");
        server.cleanup(&server);
        return 1;
    }

    // copy keys
    RSA* rsa_private_key, * rsa_public_key;
    EC_KEY* ecc_private_key, * ecc_public_key;

    rsa_private_key = server.rsa_private_key;
    rsa_public_key = server.rsa_public_key;
    ecc_private_key = server.ecc_private_key;
    ecc_public_key = server.ecc_public_key;


    if (!client_init(&client, &rsa_public_key, &ecc_private_key)) {
        fprintf(stderr, "Failed to initialize server\n");
        client.cleanup(&client);
        return 1;
    }


    // Original data
    const char* message = "This is a secret message that needs to be securely transmitted.";
    unsigned char data[MAX_DATA_SIZE];
    size_t data_len = strlen(message);
    memcpy(data, message, data_len);


    // Encrypt AES key with RSA
    unsigned char encrypted_aes_key[RSA_KEY_SIZE / 8];
    size_t encrypted_key_len;
    if (!client.encrypt_aes_key(&client,client.aes_key, AES_KEY_SIZE, encrypted_aes_key, &encrypted_key_len)) {
        handle_openssl_error();
    }

    // Sign encrypted AES key with ECC
    unsigned char signature[128];  // Large enough for ECDSA signature
    unsigned int sig_len;
    if (client.sign_aes_key(&client,encrypted_aes_key, encrypted_key_len,
        signature, &sig_len)) {
        handle_openssl_error();
    }

    // Encrypt data with AES
    unsigned char iv[AES_BLOCK_SIZE];
    unsigned char encrypted_data[MAX_DATA_SIZE + AES_BLOCK_SIZE];
    size_t encrypted_len;
    if (!client.encrypt_data(&client,data, data_len, client.aes_key, iv,
        encrypted_data, &encrypted_len)) {
        handle_openssl_error();
    }

    printf("Original message: %s\n", message);
    printf("Encrypted data length: %zu\n", encrypted_len);

    // Verify and decrypt
    unsigned char decrypted_data[MAX_DATA_SIZE];
    size_t decrypted_len;

    BOOL res = server.verify_signature(&server, encrypted_aes_key, encrypted_key_len, signature, sig_len);
    res &= server.decrypt_aes_key(&server, encrypted_aes_key, encrypted_key_len, server.aes_key, &server.aes_key_len);
    res &= server.decrypt_data(&server, encrypted_data, encrypted_len, server.aes_key, iv, server.decrypted_data, &server.decrypted_len);

    if (res)
    {
        server.decrypted_data[server.decrypted_len] = '\0';
        printf("Decrypted message: %s\n", server.decrypted_data);
    }
    else {
        printf("Verification or decryption failed!\n");
    }

   
    // Cleanup
    RSA_free(rsa_private_key);
    RSA_free(rsa_public_key);
    EC_KEY_free(ecc_private_key);
    EC_KEY_free(ecc_public_key);
    EVP_cleanup();
    ERR_free_strings();

    return 0;
}