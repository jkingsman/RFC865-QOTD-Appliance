# Common builder stage for both targets
FROM alpine:3.19 AS builder

RUN apk add --no-cache gcc musl-dev
WORKDIR /app

COPY qotd_server.c .
COPY qotd_server_cowsay_fortune.c .

RUN gcc -O3 -static -o qotd_server qotd_server.c
RUN gcc -O3 -static -o qotd_server_cowsay_fortune qotd_server_cowsay_fortune.c

FROM scratch AS qotd_8ball
COPY --from=builder /app/qotd_server /qotd_server
EXPOSE 17/tcp 17/udp
ENTRYPOINT ["/qotd_server"]

FROM alpine:3.19 AS qotd_fortune_cowsay
RUN echo "@testing https://dl-cdn.alpinelinux.org/alpine/edge/testing" >> /etc/apk/repositories
RUN echo "@community http://dl-cdn.alpinelinux.org/alpine/edge/community" >> /etc/apk/repositories

RUN apk update && \
    apk add --no-cache \
    cowsay@testing \
    fortune@community

COPY --from=builder /app/qotd_server_cowsay_fortune /qotd_server_cowsay_fortune
EXPOSE 17/tcp 17/udp
ENTRYPOINT ["/qotd_server_cowsay_fortune"]
