#include "crypto_functions.h"

 BOOL verify_ecc (EC_KEY* verify_key, unsigned char* message, size_t message_len, unsigned char* signature, unsigned int sig_len) 
 {
    // Calculate hash of the encrypted AES key
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, message, message_len);
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
    int result = ECDSA_do_verify(hash, SHA256_DIGEST_LENGTH, ecc_sig, verify_key);
    ECDSA_SIG_free(ecc_sig);

    return (result == 1) ? TRUE : FALSE;
}

 BOOL sign_ecc (EC_KEY* sign_key, unsigned char* message, size_t message_len, unsigned char* signature, unsigned int* sig_len) 
 {
     unsigned char hash[SHA256_DIGEST_LENGTH];
     SHA256_CTX sha256;
     SHA256_Init(&sha256);
     SHA256_Update(&sha256, message, message_len);
     SHA256_Final(hash, &sha256);

     ECDSA_SIG* ecc_sig = ECDSA_do_sign(hash, SHA256_DIGEST_LENGTH, sign_key);
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

 BOOL rsa_encrypt(RSA* rsa_enc_key, unsigned char* message, size_t message_len, unsigned char* ciphertext, size_t* ciphertext_len)
 {
     int result = RSA_public_encrypt(message_len, message, ciphertext, rsa_enc_key, RSA_PKCS1_OAEP_PADDING);

     if (result == -1) {
         return FALSE;
     }

     *ciphertext_len = result;
     return TRUE;
 }

 BOOL rsa_decrypt(RSA * rsa_dec_key, unsigned char* ciphertext, size_t cipher_len, unsigned char* plaintext, size_t * plaintext_len)
 {
    int result = RSA_private_decrypt(cipher_len, ciphertext, plaintext, rsa_dec_key, RSA_PKCS1_OAEP_PADDING);

    if (result == -1) {
        return FALSE;
    }

    *plaintext_len = result;
    return TRUE;
 }

 BOOL aes_encrypt(unsigned char* enc_key, unsigned char* plaintext, size_t plaintext_len, unsigned char* iv,
     unsigned char* ciphertext, size_t* cipher_len)
 {
     if (!RAND_bytes(iv, AES_BLOCK_SIZE)) {
         return FALSE;
     }

     AES_KEY aes;
     if (AES_set_encrypt_key(enc_key, AES_KEY_SIZE * 8, &aes) < 0) {
         return FALSE;
     }

     size_t padded_len = ((plaintext_len + AES_BLOCK_SIZE - 1) / AES_BLOCK_SIZE) * AES_BLOCK_SIZE;

     unsigned char* padded_data = (unsigned char*)malloc(padded_len);
     if (!padded_data) {
         return FALSE;
     }

     memcpy(padded_data, plaintext, plaintext_len);
     size_t padding_len = padded_len - plaintext_len;
     memset(padded_data + plaintext_len, padding_len, padding_len);

     unsigned char temp_iv[AES_BLOCK_SIZE];
     memcpy(temp_iv, iv, AES_BLOCK_SIZE);

     for (size_t i = 0; i < padded_len; i += AES_BLOCK_SIZE) {
         AES_cbc_encrypt(padded_data + i, ciphertext + i,
             AES_BLOCK_SIZE, &aes, temp_iv, AES_ENCRYPT);
     }

     free(padded_data);
     *cipher_len = padded_len;
     return TRUE;
 }

BOOL aes_decrypt(unsigned char* dec_key, unsigned char* ciphertext, size_t cipher_len, unsigned char* iv,
    unsigned char* plaintext, size_t* plaintext_len) 
{
    AES_KEY aes;
    if (AES_set_decrypt_key(dec_key, AES_KEY_SIZE * 8, &aes) < 0) {
        return FALSE;
    }

    unsigned char temp_iv[AES_BLOCK_SIZE];
    memcpy(temp_iv, iv, AES_BLOCK_SIZE);

    // Decrypt all blocks
    for (size_t i = 0; i < cipher_len; i += AES_BLOCK_SIZE) {
        AES_cbc_encrypt(ciphertext + i, plaintext + i, AES_BLOCK_SIZE, &aes, temp_iv, AES_DECRYPT);
    }

    // Remove padding
    unsigned char padding_len = plaintext[cipher_len - 1];
    *plaintext_len = cipher_len - padding_len;

    return TRUE;
}