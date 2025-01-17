#pragma once
#include "params.h"

#define BUFFER_SIZE  4096

void send_and_receive(int server_fd, struct sockaddr_in client_addr, const char* message, size_t len);

void receive_and_send(int server_fd, struct sockaddr_in client_addr, const char** message, size_t* len);