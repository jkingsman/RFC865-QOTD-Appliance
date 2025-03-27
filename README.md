# RFC865 Quote of the Day Protocol Server Appliance

Run a [Quote of the Day Protocol](https://datatracker.ietf.org/doc/html/rfc865) server, with a Dockerfile to set it up containerized. Now you can finally use DHCP option 8!

Note that the application and Dockerfile expect to run on port 17, which will require admin privileges to access a privileged port.

One target is for a simple 8ball-style fortune, and the other is a `fortune | cowsay` output.

## Quickstart

Needs sudo to bind to privileged port.

```bash
sudo docker run -d -p 17:17/tcp -p 17:17/udp jkingsman/qotd-appliance
```

See https://hub.docker.com/repository/docker/jkingsman/qotd-appliance/general.

## Docker

### Docker Compilation

Build and run directly with

```bash
# 8ball version
docker build --target qotd_8ball -t qotd_8ball .

# cowsay fortune version
docker build --target qotd_fortune_cowsay -t qotd_fortune_cowsay .
```

### Docker Execution

```bash
sudo docker run -p 17:17/tcp -p 17:17/udp --name qotd_8ball_container qotd_8ball
sudo docker run -p 17:17/tcp -p 17:17/udp --name qotd_fortune_cowsay_container qotd_fortune_cowsay
```

## Local
### Local Compilation

```bash
# 8ball version
gcc -o qotd_server qotd_server.c

# cowsay fortune version
gcc -o qotd_server qotd_server_cowsay_fortune.c
```

if you're feeling aggressive.

## Local Execution

```bash
sudo ./qotd_server
```

## Interaction

```bash
# tcp
nc localhost 17

# udp
echo "" | nc -u localhost 17
```

## License

MIT License, Copyright (c) 2025 Jack Kingsman <jack@jackkingsman.me>

See `LICENSE.md` for full text.
