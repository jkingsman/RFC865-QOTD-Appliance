# RFC865 Quote of the Day Protocol Server Appliance

Run a [Quote of the Day Protocol](https://datatracker.ietf.org/doc/html/rfc865) server, with a Dockerfile to set it up containerized. Now you can finally use DHCP option 8!

Note that the application and Dockerfile expect to run on port 17, which will require admin privileges to access a privileged port.

## Docker

Needs sudo to bind to privileged port.

```
sudo docker run -d -p 17:17/tcp -p 17:17/udp jkingsman/qotd-appliance
```

See https://hub.docker.com/repository/docker/jkingsman/qotd-appliance/general.

## Compilation

```
gcc -o qotd_server qotd_server.c
```

or

```
gcc -O3 -march=native -flto -pthread -fomit-frame-pointer -funroll-loops -Wall -pipe \
    -DNDEBUG -Wl,-O1 -Wl,--as-needed -o qotd_server qotd_server.c
```

if you're feeling aggressive.

## Execution

```
sudo ./qotd_server
```

## Interaction

```
# tcp
nc localhost 17

# udp
echo "" | nc -u localhost 17
```

## License

MIT License, Copyright (c) 2025 Jack Kingsman <jack@jackkingsman.me>

See `LICENSE.md` for full text.
