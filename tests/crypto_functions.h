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