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

    client->aes_key_len = 0;
    client->is_initialized = FALSE;

    SAFE_AES_KEY_MEMSET(client->aes_key);
    EVP_cleanup();
    ERR_free_strings();
}

int client_init(Client* client, EC_KEY** ecc_public_key, EC_KEY** ecc_private_key)
{
    if (!client) {
        return FALSE;
    }
    if (client->is_initialized == TRUE)
    {
        return TRUE;
    }

    // create aes key
    if (!RAND_bytes(client->aes_key, AES_KEY_SIZE)) {
        handle_openssl_error();
    }

    client->ecc_private_key = *ecc_private_key;
    client->ecc_public_key = *ecc_public_key;

    // Initialize function pointers
    client->cleanup = client_cleanup;

    client->is_initialized = TRUE;
    return TRUE;
}

int client_run() {
    WSADATA wsaData;
    SOCKET client_fd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

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

    // Receive response from server
    int server_addr_len = sizeof(server_addr);
    int n = recvfrom(client_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&server_addr, &server_addr_len);
    if (n == SOCKET_ERROR) {
        printf("Receive failed: %d\n", WSAGetLastError());
        closesocket(client_fd);
        WSACleanup();
        return EXIT_FAILURE;
    }

    buffer[n] = '\0';
    printf("Server: %s\n", buffer);

    // Cleanup
    closesocket(client_fd);
    WSACleanup();
    return 0;
}