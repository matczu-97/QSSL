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
    if (rc != 0)
    {
        printf("aes keys generation failed!");
        client_cleanup(client);
        return FALSE;
    }
    
    rc = generate_ecc_keys(&client->ecc_private_key, &client->ecc_public_key);
    if (rc != 0)
    {
        printf("ecc keys generation failed!");
        client_cleanup(client);
        return FALSE;
    }

    rc = generate_dilithium_keys(&client->dilithium_private_key, &client->dilithium_public_key);
    if (rc != 0)
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

int tls_client(Client* client, Server_Keys* ser_keys, int client_fd, struct sockaddr_in server_addr)
{
    size_t key_len = 0;
    char* server_message = NULL;
    
    // Recieve the RSA Key
    receive_and_send(client_fd, server_addr, &server_message, &key_len);
    printf("%s:\n", server_message);
    server_message = NULL;
    key_len = 0;
    receive_and_send(client_fd, server_addr, &server_message, &key_len);
    if (server_message) {
        // Deserialize the public key
        ser_keys->rsa_public_key = deserialize_rsa_key(server_message, key_len);
        if (ser_keys->rsa_public_key) {
            printf("ECC public key successfully deserialized!\n");
        }
        else {
            fprintf(stderr, "Failed to deserialize RSA public key\n");
            return FALSE;
        }
    }
    
    // Recieve the ECC Key
    server_message = NULL;
    key_len = 0;
    receive_and_send(client_fd, server_addr, &server_message, &key_len);
    printf("%s:\n", server_message);
    server_message = NULL;
    key_len = 0;
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
    server_message = NULL;
    key_len = 0;
    receive_and_send(client_fd, server_addr, &server_message, &key_len);
    printf("%s:\n", server_message);
    server_message = NULL;
    key_len = 0;
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
    server_message = NULL;
    key_len = 0;
    receive_and_send(client_fd, server_addr, &server_message, &key_len);
    printf("%s:\n", server_message);
    server_message = NULL;
    key_len = 0;
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
    send_and_receive(client_fd, server_addr, message_to_send, sizeof(message_to_send));
    send_and_receive(client_fd, server_addr, serialized_key, key_len);

    // Send the Dilithium Key
    message_to_send = "Dilithium Key";
    send_and_receive(client_fd, server_addr, message_to_send, sizeof(message_to_send));
    send_and_receive(client_fd, server_addr, client->dilithium_public_key, OQS_SIG_dilithium_2_length_public_key);

    printf("all public keys sent successfully\n");
    return TRUE;
}

int client_run() {
    WSADATA wsaData;
    int client_fd, rc;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    Server_Keys sk;
    Client client;

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
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Send message to server
    const char* message = "Hello, Server!";
    sendto(client_fd, message, strlen(message), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));

    rc = tls_client(&client, &sk, client_fd, server_addr);
    if (rc != TRUE)
    {
        printf("tls_client failed!, exiting...");
        return;
    }


    // Receive response from server
   /* int server_addr_len = sizeof(server_addr);
    int n = recvfrom(client_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&server_addr, &server_addr_len);
    if (n == SOCKET_ERROR) {
        printf("Receive failed: %d\n", WSAGetLastError());
        closesocket(client_fd);
        WSACleanup();
        return EXIT_FAILURE;
    }
    buffer[n] = '\0';
    printf("Server: %s\n", buffer);*/

    // Cleanup
    closesocket(client_fd);
    WSACleanup();
    return 0;
}