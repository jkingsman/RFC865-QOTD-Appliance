/**
 * QOTD Server with Dynamic Command Execution
 * Implements RFC 865 on TCP/UDP port 17
 * MIT License (c) 2025 Jack Kingsman <jack@jackkingsman.me>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

#define QOTD_PORT 17
#define BACKLOG 10
// this should be 512 to behave the best since RFC 865 says 512 but... eh. "should not".
#define MAX_BUFFER_SIZE 1024
#define QUOTE_COMMAND "fortune | cowsay"

int tcp_socket = -1;
int udp_socket = -1;

void handle_signal(int sig)
{
  printf("[%ld] Shutting down QOTD server...\n", time(NULL));
  if (tcp_socket >= 0)
    close(tcp_socket);
  if (udp_socket >= 0)
    close(udp_socket);
  exit(EXIT_SUCCESS);
}

char *get_command_output(char *buffer, size_t buffer_size)
{
  FILE *pipe = popen(QUOTE_COMMAND, "r");
  if (!pipe)
  {
    snprintf(buffer, buffer_size, "Error: Command execution failed\n");
    return buffer;
  }

  size_t total_read = 0;
  size_t bytes_read;

  while (total_read < buffer_size - 1 &&
         (bytes_read = fread(buffer + total_read, 1, buffer_size - total_read - 1, pipe)) > 0)
  {
    total_read += bytes_read;
  }

  buffer[total_read] = '\0';
  pclose(pipe);
  return buffer;
}

int main()
{
  setbuf(stdout, NULL);

  char quote_buffer[MAX_BUFFER_SIZE];
  struct sockaddr_in tcp_addr, udp_addr;
  int opt = 1;

  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);

  // wear your socks
  if ((tcp_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("TCP socket creation failed");
    exit(EXIT_FAILURE);
  }

  if ((udp_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    perror("UDP socket creation failed");
    close(tcp_socket);
    exit(EXIT_FAILURE);
  }

  if (setsockopt(tcp_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0 ||
      setsockopt(udp_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
  {
    perror("setsockopt failed");
    close(tcp_socket);
    close(udp_socket);
    exit(EXIT_FAILURE);
  }

  memset(&tcp_addr, 0, sizeof(tcp_addr));
  tcp_addr.sin_family = AF_INET;
  tcp_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  tcp_addr.sin_port = htons(QOTD_PORT);
  memcpy(&udp_addr, &tcp_addr, sizeof(tcp_addr));

  // tcp bind
  if (bind(tcp_socket, (struct sockaddr *)&tcp_addr, sizeof(tcp_addr)) < 0)
  {
    perror("TCP bind failed");
    close(tcp_socket);
    close(udp_socket);
    exit(EXIT_FAILURE);
  }

  if (listen(tcp_socket, BACKLOG) < 0)
  {
    perror("TCP listen failed");
    close(tcp_socket);
    close(udp_socket);
    exit(EXIT_FAILURE);
  }

  // udp bind
  if (bind(udp_socket, (struct sockaddr *)&udp_addr, sizeof(udp_addr)) < 0)
  {
    perror("UDP bind failed");
    close(tcp_socket);
    close(udp_socket);
    exit(EXIT_FAILURE);
  }

  // non-blocking socks
  int flags;
  flags = fcntl(tcp_socket, F_GETFL, 0);
  fcntl(tcp_socket, F_SETFL, flags | O_NONBLOCK);
  flags = fcntl(udp_socket, F_GETFL, 0);
  fcntl(udp_socket, F_SETFL, flags | O_NONBLOCK);

  printf("[%ld] QOTD server started on port %d (TCP and UDP)\n", time(NULL), QOTD_PORT);
  printf("[%ld] Using command: %s\n", time(NULL), QUOTE_COMMAND);

  // main loop
  while (1)
  {
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(tcp_socket, &read_fds);
    FD_SET(udp_socket, &read_fds);

    int max_fd = (tcp_socket > udp_socket) ? tcp_socket : udp_socket;

    if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0)
    {
      if (errno == EINTR)
        continue;
      perror("select failed");
      break;
    }

    // tcp
    if (FD_ISSET(tcp_socket, &read_fds))
    {
      struct sockaddr_in client_addr;
      socklen_t client_len = sizeof(client_addr);

      int client_fd = accept(tcp_socket, (struct sockaddr *)&client_addr, &client_len);
      if (client_fd >= 0)
      {
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        printf("[%ld] TCP connection from %s:%d\n", time(NULL), client_ip, ntohs(client_addr.sin_port));

        // send quote
        get_command_output(quote_buffer, sizeof(quote_buffer));
        send(client_fd, quote_buffer, strlen(quote_buffer), 0);
        close(client_fd);
      }
      else if (errno != EAGAIN && errno != EWOULDBLOCK)
      {
        perror("TCP accept failed");
      }
    }

    // udp
    if (FD_ISSET(udp_socket, &read_fds))
    {
      struct sockaddr_in client_addr;
      socklen_t client_len = sizeof(client_addr);
      char recv_buffer[MAX_BUFFER_SIZE];

      ssize_t recv_size = recvfrom(udp_socket, recv_buffer, sizeof(recv_buffer), 0,
                                   (struct sockaddr *)&client_addr, &client_len);

      if (recv_size >= 0)
      {
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        printf("[%ld] UDP datagram from %s:%d\n", time(NULL), client_ip, ntohs(client_addr.sin_port));

        // send quote
        get_command_output(quote_buffer, sizeof(quote_buffer));
        sendto(udp_socket, quote_buffer, strlen(quote_buffer), 0,
               (struct sockaddr *)&client_addr, client_len);
      }
      else if (errno != EAGAIN && errno != EWOULDBLOCK)
      {
        perror("UDP recvfrom failed");
      }
    }
  }

  close(tcp_socket);
  close(udp_socket);

  return EXIT_SUCCESS;
}
