/**
 * High Performance QOTD Server with Magic 8-Ball Fortunes in Cowsay
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
#define MAX_BUFFER_SIZE 512
#define NUM_FORTUNES 20

int tcp_socket = -1;
int udp_socket = -1;

const char *fortunes[NUM_FORTUNES] = {
    "It is certain.",
    "It is decidedly so.",
    "Without a doubt.",
    "Yes - definitely.",
    "You may rely on it.",
    "As I see it, yes.",
    "Most likely.",
    "Outlook good.",
    "Yes.",
    "Signs point to yes.",
    "Reply hazy, try again.",
    "Ask again later.",
    "Better not tell you now.",
    "Cannot predict now.",
    "Concentrate and ask again.",
    "Don't count on it.",
    "My reply is no.",
    "My sources say no.",
    "Outlook not so good.",
    "Very doubtful."};

void handle_signal(int sig)
{
  printf("[%ld] Shutting down QOTD server...\n", time(NULL));
  if (tcp_socket >= 0)
    close(tcp_socket);
  if (udp_socket >= 0)
    close(udp_socket);
  exit(EXIT_SUCCESS);
}

char *generate_cowsay(char *buffer, size_t buffer_size)
{
  int fortune_index = rand() % NUM_FORTUNES;
  const char *fortune = fortunes[fortune_index];

  const char *prefix = "8-ball says: ";
  size_t message_len = strlen(prefix) + strlen(fortune);

  char top_border[MAX_BUFFER_SIZE];
  char bottom_border[MAX_BUFFER_SIZE];
  memset(top_border, '_', message_len + 2);
  memset(bottom_border, '-', message_len + 2);
  top_border[message_len + 2] = '\0';
  bottom_border[message_len + 2] = '\0';

  snprintf(buffer, buffer_size,
           " %s\n"
           "< %s%s >\n"
           " %s\n"
           "        \\   ^__^\n"
           "         \\  (oo)\\_______\n"
           "            (__)\\       )\\/\\\n"
           "                ||----w |\n"
           "                ||     ||\n",
           top_border,
           prefix, fortune,
           bottom_border);

  return buffer;
}

int main()
{
  char quote_buffer[MAX_BUFFER_SIZE];
  struct sockaddr_in tcp_addr, udp_addr;
  int opt = 1;

  srand(time(NULL));
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

        // Generate and send quote
        generate_cowsay(quote_buffer, sizeof(quote_buffer));
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

        // Generate and send quote
        generate_cowsay(quote_buffer, sizeof(quote_buffer));
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
