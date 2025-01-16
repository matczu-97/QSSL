#pragma once
#include "params.h"

typedef BOOL(*VerifyECCSignature)(EC_KEY* verify_key, unsigned char* message, size_t message_len, unsigned char* signature, unsigned int sig_len);

typedef BOOL(*SignECCSignature)(EC_KEY* sign_key, unsigned char* message, size_t message_len, unsigned char* signature, unsigned int* sig_len);

typedef BOOL(*RSAEncrypt)(RSA* rsa_enc_key, unsigned char* message, size_t message_len, unsigned char* ciphertext, size_t* ciphertext_len);

typedef BOOL(*RSADecrypt)(RSA* rsa_dec_key, unsigned char* ciphertext, size_t cipher_len, unsigned char* plaintext, size_t* plaintext_len);

typedef BOOL(*AESEncrypt)(unsigned char* enc_key, unsigned char* plaintext, size_t plaintext_len, unsigned char* iv,
    unsigned char* ciphertext, size_t* cipher_len);

typedef BOOL(*AESDecrypt)(unsigned char* dec_key, unsigned char* ciphertext, size_t cipher_len, unsigned char* iv,
    unsigned char* plaintext, size_t* plaintext_len);

BOOL verify_ecc(EC_KEY* verify_key, unsigned char* message, size_t message_len, unsigned char* signature, unsigned int sig_len);

BOOL sign_ecc(EC_KEY* sign_key, unsigned char* message, size_t message_len, unsigned char* signature, unsigned int* sig_len);

BOOL rsa_encrypt(RSA* rsa_enc_key, unsigned char* message, size_t message_len, unsigned char* ciphertext, size_t* ciphertext_len);

BOOL rsa_decrypt(RSA* rsa_dec_key, unsigned char* ciphertext, size_t cipher_len, unsigned char* plaintext, size_t* plaintext_len);

BOOL aes_encrypt(unsigned char* enc_key, unsigned char* plaintext, size_t plaintext_len, unsigned char* iv,
    unsigned char* ciphertext, size_t* cipher_len);

BOOL aes_decrypt(unsigned char* dec_key, unsigned char* ciphertext, size_t cipher_len, unsigned char* iv,
    unsigned char* plaintext, size_t* plaintext_len);

void generate_rsa_keys(RSA** rsa_private_key, RSA** rsa_public_key);

void generate_ecc_keys(EC_KEY** ecc_private_key, EC_KEY** ecc_public_key);

int generate_kyber_keys(uint8_t* kyber_secret_key, uint8_t* kyber_public_key);

int kyber_encapsulate(uint8_t* encapsulated_data, uint8_t* shared_secret, uint8_t* kyber_public_key);

int kyber_decapsulate(uint8_t* encapsulated_data, uint8_t* shared_secret, uint8_t* kyber_secret_key);