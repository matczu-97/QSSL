#include "socket_functions.h"

void send_and_receive(int server_fd, struct sockaddr_in client_addr, const char* message, size_t len) {

    size_t sent_len = sendto(server_fd, message, len, 0, (const struct sockaddr*)&client_addr, sizeof(client_addr));
    if (sent_len < 0) {
        perror("sendto - send_and_receive");
    }
    else if ((size_t)sent_len != len) {
        fprintf(stderr, "Incomplete key sent: %zd/%zu bytes - send_and_receive\n", sent_len, len);
    }

    // waiting for response from client
    char buffer[BUFFER_SIZE];
    size_t recv_len = recvfrom(server_fd, buffer, sizeof(buffer), 0, NULL, NULL);
    if (recv_len < 0) {
        perror("recvfrom - send_and_receive");
        return;
    }
}

void receive_and_send(int server_fd, struct sockaddr_in client_addr, const char** message, size_t* len) {

    // waiting for message from client
    char buffer[BUFFER_SIZE];
    size_t recv_len = recvfrom(server_fd, buffer, sizeof(buffer), 0, NULL, NULL);
    if (recv_len < 0) {
        perror("recvfrom - receive_and_send");
        return;
    }
    // saving the message and len to parameters
    memcpy(*message, buffer, recv_len);
    *len = recv_len;

    char* response = "Ok";
    size_t sent_len = sendto(server_fd, response, sizeof(response), 0, (const struct sockaddr*)&client_addr, sizeof(client_addr));
    if (sent_len < 0) {
        perror("sendto - receive_and_send");
    }
    else if ((size_t)sent_len != sizeof(response)) {
        fprintf(stderr, "Incomplete key sent: %zd/%zu bytes - receive_and_send\n", sent_len, sizeof(response));
    }
}
