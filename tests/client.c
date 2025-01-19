#include "client.h"

void client_cleanup(Client* client)
{
    if (!client) return;

    if (client->ecc_private_key) {
        EC_KEY_free(client->ecc_private_key);
        client->ecc_private_key = NULL;
    }

    if (client->ecc_public_key) {
        EC_KEY_free(client->ecc_public_key);
        client->ecc_public_key = NULL;
    }

    if (client->dilithium_private_key) {
        OQS_MEM_cleanse(client->dilithium_private_key, OQS_SIG_dilithium_2_length_secret_key);
    }

    client->is_initialized = FALSE;

    SAFE_AES_KEY_MEMSET(client->aes_key);
    EVP_cleanup();
    ERR_free_strings();
}

int client_init(Client* client)
{
    if (!client) {
        return FALSE;
    }
    if (client->is_initialized == TRUE)
    {
        return TRUE;
    }

    int rc = generate_aes_key(client->aes_key, AES_KEY_SIZE);
    if (rc != TRUE)
    {
        printf("aes keys generation failed!");
        client_cleanup(client);
        return FALSE;
    }
    
    rc = generate_ecc_keys(&client->ecc_private_key, &client->ecc_public_key);
    if (rc != TRUE)
    {
        printf("ecc keys generation failed!");
        client_cleanup(client);
        return FALSE;
    }

    rc = generate_dilithium_keys(&client->dilithium_private_key, &client->dilithium_public_key);
    if (rc != TRUE)
    {
        printf("dilithium keys generation failed!");
        client_cleanup(client);
        return FALSE;
    }

    // Initialize function pointers
    client->cleanup = client_cleanup;

    client->is_initialized = TRUE;
    return TRUE;
}

int public_key_exchange_client(Client* client, Server_Keys* ser_keys, int client_fd, struct sockaddr_in server_addr)
{
    size_t key_len = 0;
    char* server_message = NULL;
    
    // Recieve the RSA Key
    print_header_and_free(client_fd, server_addr, &server_message, &key_len);
    receive_and_send(client_fd, server_addr, &server_message, &key_len);
    if (server_message) {
        // Deserialize the public key
        ser_keys->rsa_public_key = deserialize_rsa_key(server_message, key_len);
        if (ser_keys->rsa_public_key) {
            printf("RSA public key successfully deserialized!\n");
        }
        else {
            fprintf(stderr, "Failed to deserialize RSA public key\n");
            return FALSE;
        }
    }
    
    // Recieve the ECC Key
    print_header_and_free(client_fd, server_addr, &server_message, &key_len);
    receive_and_send(client_fd, server_addr, &server_message, &key_len);
    if (server_message) {
        ser_keys->ecc_public_key = deserialize_ecc_key(server_message, key_len);
        if (ser_keys->ecc_public_key) {
            printf("ECC public key successfully deserialized!\n");
        }
        else {
            fprintf(stderr, "Failed to deserialize ECC public key\n");
            return FALSE;
        }
    }
    
    // Recieve the Kyber Key
    print_header_and_free(client_fd, server_addr, &server_message, &key_len);
    receive_and_send(client_fd, server_addr, &server_message, &key_len);
    if (key_len == OQS_KEM_kyber_768_length_secret_key) {
        // saving the key in the cl_keys struct
        memcpy(ser_keys->kyber_public_key, server_message, key_len);
        printf("Kyber public key successfully saved!\n");
    }
    else {
        fprintf(stderr, "Failed to save Kyber public key\n");
        return FALSE;
    }

    // Recieve the Dilithium Key
    print_header_and_free(client_fd, server_addr, &server_message, &key_len);
    receive_and_send(client_fd, server_addr, &server_message, &key_len);
    if (key_len == OQS_SIG_dilithium_2_length_public_key) {
        // saving the key in the cl_keys struct
        memcpy(ser_keys->dilithium_public_key, server_message, key_len);
        printf("Dilithium public key successfully saved!\n");
    }
    else {
        fprintf(stderr, "Failed to save Dilithium public key\n");
        return FALSE;
    }

    printf("server public keys saved successfully\n");

    char* serialized_key = NULL;
   
    // Serialize ECC public key
    serialized_key = serialize_ecc_key(client->ecc_public_key, &key_len);
    if (!serialized_key) {
        EC_KEY_free(client->ecc_public_key);
        printf("failed to serilaze ecc key!\n");
        return FALSE;
    }
    // Send the ECC Key
    char* message_to_send = "ECC Key";
    send_and_receive(client_fd, server_addr, message_to_send, strlen(message_to_send));
    send_and_receive(client_fd, server_addr, serialized_key, key_len);

    // Send the Dilithium Key
    message_to_send = "Dilithium Key";
    send_and_receive(client_fd, server_addr, message_to_send, strlen(message_to_send));
    send_and_receive(client_fd, server_addr, client->dilithium_public_key, OQS_SIG_dilithium_2_length_public_key);

    printf("all public keys sent successfully\n");
    return TRUE;
}

int handshake_client(Client* client, Server_Keys* ser_keys,const int client_fd, struct sockaddr_in server_addr, unsigned char* session_key)
{
    int rc;
    unsigned int ecc_signature_len = 0;
    size_t encrypted_len = 0, dil_sign_len = 0;
    char* message_to_send = NULL;
    unsigned char encrypted_key[RSA_KEY_SIZE / 8], ecc_sig[64], dil_sign[OQS_SIG_dilithium_2_length_signature];
    uint8_t encapsulated_message[OQS_KEM_kyber_768_length_ciphertext], shared_secret[OQS_KEM_kyber_768_length_shared_secret];  

    // Encrypt AES key using RSA
    rc = rsa_encrypt(ser_keys->rsa_public_key, client->aes_key, AES_KEY_SIZE, encrypted_key, &encrypted_len);
    if (rc != TRUE) {
        printf("failed to encrypt aes key!\n");
        return FALSE;
    }

    // Sign the Key using ECC
    rc = ecc_sign(client->ecc_private_key, encrypted_key, encrypted_len, ecc_sig, &ecc_signature_len);
    if (rc != TRUE) {
        printf("failed to sign the key using ECC!\n");
        return FALSE;
    }

    // Send the Encrypted Key
    message_to_send = "Encrypted AES Key";
    send_and_receive(client_fd, server_addr, message_to_send, strlen(message_to_send));
    send_and_receive(client_fd, server_addr, encrypted_key, encrypted_len);
    // Send the Encrypted Key Signature
    message_to_send = "Signature for AES Key";
    send_and_receive(client_fd, server_addr, message_to_send, strlen(message_to_send));
    send_and_receive(client_fd, server_addr, ecc_sig, ecc_signature_len);

    // Encapsulate data using Kyber
    rc = kyber_encapsulate(encapsulated_message, shared_secret, ser_keys->kyber_public_key);
    if (rc != TRUE) {
        printf("failed to encapsulate with kyber!\n");
        return FALSE;
    }

    // Sign the Key using Dilithium
    rc = dilithium_sign(client->dilithium_private_key, encapsulated_message, OQS_KEM_kyber_768_length_ciphertext, dil_sign, &dil_sign_len);
    if (rc != TRUE) {
        printf("failed to sign the key using Dilithium!\n");
        return FALSE;
    }

    // Send the Encapsulated message
    message_to_send = "Encapsulated Kyber Data";
    send_and_receive(client_fd, server_addr, message_to_send, strlen(message_to_send));
    send_and_receive(client_fd, server_addr, encapsulated_message, OQS_KEM_kyber_768_length_ciphertext);
    // Send the Dilithium Signature
    message_to_send = "Dilithium Signature";
    send_and_receive(client_fd, server_addr, message_to_send, strlen(message_to_send));
    send_and_receive(client_fd, server_addr, dil_sign, dil_sign_len);

    printf("transfer of session key completed\n");

    // create the session key using xor between both keys
    xor(client->aes_key,shared_secret,session_key,AES_KEY_SIZE);
    printf("Session key is Ready to use\n");

    return TRUE;
}

int main() {
    WSADATA wsaData;
    int client_fd, rc;
    struct sockaddr_in server_addr;
    unsigned char buffer[BUFFER_SIZE], session_key[AES_KEY_SIZE];
    Server_Keys sk;
    Client client;

    rc = client_init(&client);
    if (rc != TRUE)
    {
        printf("client init failed!, exiting...");
        return;
    }

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed: %d\n", WSAGetLastError());
        return EXIT_FAILURE;
    }

    // Create socket
    if ((client_fd = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
        printf("Socket creation failed: %d\n", WSAGetLastError());
        WSACleanup();
        return EXIT_FAILURE;
    }

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(LOCALHOST);

    // Send message to server
    const char* message = "Hello, Server!";
    sendto(client_fd, message, strlen(message), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));

    // Transfer public keys between client and server
    rc = public_key_exchange_client(&client, &sk, client_fd, server_addr);
    if (rc != TRUE)
    {
        printf("public_key_exchange_client failed!, exiting...");
        return;
    }

    // Create the session key and send it
    rc = handshake_client(&client, &sk, client_fd, server_addr, session_key);
    if (rc != TRUE)
    {
        printf("handshake_client failed!, exiting...");
        return;
    }

    rc = write_key_file("shared_client.bin", session_key, AES_KEY_SIZE);
    if (rc != TRUE)
    {
        printf("failed to write to file, exiting...");
        return;
    }

    // loop for safe communication
    while (1)
    {
     
    }

    // Cleanup
    client.cleanup(&client);
    closesocket(client_fd);
    WSACleanup();
    system("PAUSE");
    return 0;
}