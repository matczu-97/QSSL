#include "server.h"

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

    // Initialize function pointers
    server->cleanup = server_cleanup;

    server->is_initialized = TRUE;
    return TRUE;
}

int server_run() {
    WSADATA wsaData;
    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    socklen_t client_addr_len = sizeof(client_addr);

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