#include "server.h"

void server_cleanup(Server* server)
{
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

    if (server->kyber_private_key) {
        OQS_MEM_cleanse(server->kyber_private_key, OQS_KEM_kyber_768_length_secret_key);
    }

    if (server->kyber_shared_secret) {
        OQS_MEM_cleanse(server->kyber_shared_secret, OQS_KEM_kyber_768_length_shared_secret);
    }

    if (server->dilithium_private_key) {
        OQS_MEM_cleanse(server->dilithium_private_key, OQS_SIG_dilithium_2_length_secret_key);
    }

    server->is_initialized = FALSE;
    EVP_cleanup();
    ERR_free_strings();
}

int server_init(Server* server) 
{
    if (!server) {
        return FALSE;
    }
    if (server->is_initialized == TRUE)
    {
        return TRUE;
    }

    int rc = generate_rsa_keys(&server->rsa_private_key, &server->rsa_public_key);
    if (rc != 0)
    {
        printf("rsa keys generation failed!");
        server_cleanup(server);
        return FALSE;
    }

    rc = generate_ecc_keys(&server->ecc_private_key, &server->ecc_public_key);
    if (rc != 0)
    {
        printf("ecc keys generation failed!");
        server_cleanup(server);
        return FALSE;
    }
   
    rc = generate_kyber_keys(&server->kyber_private_key, &server->kyber_public_key);
    if (rc != 0)
    {
        printf("kyber keys generation failed!");
        server_cleanup(server);
        return FALSE;
    } 

    rc = generate_dilithium_keys(&server->dilithium_private_key, &server->dilithium_public_key);
    if (rc != 0)
    {
        printf("dilithium keys generation failed!");
        server_cleanup(server);
        return FALSE;
    }

    // Initialize function pointers
    server->cleanup = server_cleanup;

    server->is_initialized = TRUE;
    return TRUE;
}

int tls_server(Server* server,Client_Keys* cl_keys, int server_fd, struct sockaddr_in client_addr)
{
    size_t key_len;
    char* serialized_key = NULL;

    // Serialize RSA public key
    serialized_key = serialize_rsa_key(server->rsa_public_key, &key_len);
    if (!serialized_key) {
        RSA_free(server->rsa_public_key);
        RSA_free(server->rsa_private_key);
        printf("failed to serilaze rsa key!\n");
        return FALSE;
    }

    // Send the RSA Key
    char* message_to_send = "RSA Key";
    send_and_receive(server_fd, client_addr, message_to_send,sizeof(message_to_send));
    send_and_receive(server_fd, client_addr, serialized_key, key_len);

    // Serialize ECC public key
    serialized_key = serialize_ecc_key(server->ecc_public_key, &key_len);
    if (!serialized_key) {
        EC_KEY_free(server->ecc_public_key);
        printf("failed to serilaze ecc key!\n");
        return FALSE;
    }

    // Send the ECC Key
    message_to_send = "ECC Key";
    send_and_receive(server_fd, client_addr, message_to_send, sizeof(message_to_send));
    send_and_receive(server_fd, client_addr, serialized_key, key_len);

    // Send the Kyber Key
    message_to_send = "Kyber Key";
    send_and_receive(server_fd, client_addr, message_to_send, sizeof(message_to_send));
    send_and_receive(server_fd, client_addr, server->kyber_public_key, OQS_KEM_kyber_768_length_secret_key);

    // Send the Dilithium Key
    message_to_send = "Dilithium Key";
    send_and_receive(server_fd, client_addr, message_to_send, sizeof(message_to_send));
    send_and_receive(server_fd, client_addr, server->dilithium_public_key, OQS_SIG_dilithium_2_length_public_key);

    printf("all public keys sent successfully\n");
    
    // Recieve the ECC Key
    char* client_message = NULL;
    key_len = 0;
    receive_and_send(server_fd, client_addr, &client_message, &key_len);
    printf("%s:\n", client_message);
    client_message = NULL;
    key_len = 0;
    receive_and_send(server_fd, client_addr, &client_message, &key_len);
    
    if (client_message) {
        cl_keys->ecc_public_key = deserialize_ecc_key(client_message, key_len);
        if (cl_keys->ecc_public_key) {
            printf("ECC public key successfully deserialized!\n");
        }
        else {
            fprintf(stderr, "Failed to deserialize ECC public key\n");
            return FALSE;
        }
    }

    // Recieve the Dilithium Key
    client_message = NULL;
    key_len = 0;
    receive_and_send(server_fd, client_addr, &client_message, &key_len);
    printf("%s:\n", client_message);
    client_message = NULL;
    key_len = 0;
    receive_and_send(server_fd, client_addr, &client_message, &key_len);
    
    if (key_len == OQS_SIG_dilithium_2_length_public_key) {
        // saving the key in the cl_keys struct
        memcpy(cl_keys->dilithium_public_key, client_message, key_len);
        printf("Dilithium public key successfully saved!\n");
    }
    else {
        fprintf(stderr, "Failed to save Dilithium public key\n");
        return FALSE;
    }

    printf("client public keys saved successfully\n");
    return TRUE;
}

int main() {
    WSADATA wsaData;
    int server_fd, rc;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    socklen_t client_addr_len = sizeof(client_addr);
    Client_Keys ck;
    Server server;
    rc = server_init(&server);
    if (rc != TRUE)
    {
        printf("server init failed!, exiting...");
        return;
    }

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed: %d\n", WSAGetLastError());
        return EXIT_FAILURE;
    }

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Bind socket to the address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (const struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is running on port %d\n", PORT);

    // Receive Hello message from client
    memset(buffer, 0, BUFFER_SIZE);
    int n = recvfrom(server_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_addr_len);
    if (n < 0) {
        perror("Receive failed");
        return;
    }

    buffer[n] = '\0';
    printf("Client: %s\n", buffer);

    // start tls handshake
    rc = tls_server(&server,&ck, server_fd, client_addr);
    if (rc != TRUE)
    {
        printf("tls_server failed!, exiting...");
        return;
    }

    /* here need to add the encryption process*/


    // Send response to client
    const char* response = "Message received";
    sendto(server_fd, response, strlen(response), 0, (const struct sockaddr*)&client_addr, client_addr_len);

    // Cleanup
    close(server_fd);
    WSACleanup();
    return 0;
}