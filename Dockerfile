FROM alpine:3.19 AS builder

RUN apk add --no-cache gcc musl-dev
WORKDIR /app
COPY qotd_server.c .
RUN gcc -O3 -static -o qotd_server qotd_server.c

FROM scratch
COPY --from=builder /app/qotd_server /qotd_server
EXPOSE 17/tcp 17/udp
ENTRYPOINT ["/qotd_server"]
