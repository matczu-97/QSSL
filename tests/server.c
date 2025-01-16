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

    generate_rsa_keys(&server->rsa_private_key, &server->rsa_public_key);
    generate_ecc_keys(&server->ecc_private_key, &server->ecc_public_key);
   int rc = generate_kyber_keys(&server->kyber_private_key, &server->kyber_public_key);
   if (rc != 0)
   {
       printf("kyber keys generation failed!");
       server_cleanup(server);
       return FALSE;
   }

    // Initialize function pointers
    server->cleanup = server_cleanup;

    server->is_initialized = TRUE;
    return TRUE;
}

int main() {
    WSADATA wsaData;
    int server_fd, rc;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    socklen_t client_addr_len = sizeof(client_addr);
    
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

    // Receive message from client
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int n = recvfrom(server_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_addr_len);
        if (n < 0) {
            perror("Receive failed");
            break;
        }

        buffer[n] = '\0';
        printf("Client: %s\n", buffer);

        // Send response to client
        const char* response = "Message received";
        sendto(server_fd, response, strlen(response), 0, (const struct sockaddr*)&client_addr, client_addr_len);
    }

    close(server_fd);
    return 0;
}