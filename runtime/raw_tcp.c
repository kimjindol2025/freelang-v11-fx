/* Single-process raw TCP event loop. fl-yield dispatches callbacks on its caller. */
#define _GNU_SOURCE
#include "runtime.h"
#include <arpa/inet.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define RAW_CLIENTS 64
typedef struct {
    int fd;
    unsigned long id;
    char address[NI_MAXHOST];
    int port;
} RawClient;

static int raw_server = -1;
static FLValue raw_handler;
typedef FLValue (*RawHandler4)(FLValue, FLValue, FLValue, FLValue);
static RawHandler4 raw_named_handler;
static RawClient raw_clients[RAW_CLIENTS];
static unsigned long raw_next_id = 1;

static const char* raw_text(FLValue value) {
    return value.tag == FL_STRING && value.obj ? ((FLString*)value.obj)->data : "";
}

static void raw_name(unsigned long id, char out[32]) {
    snprintf(out, 32, "conn_%lu", id);
}

static RawClient* raw_find(FLValue value) {
    const char* name = raw_text(value);
    if (strncmp(name, "conn_", 5) != 0) return NULL;
    char* end;
    unsigned long id = strtoul(name + 5, &end, 10);
    if (!id || *end) return NULL;
    for (int i = 0; i < RAW_CLIENTS; i++)
        if (raw_clients[i].fd >= 0 && raw_clients[i].id == id) return &raw_clients[i];
    return NULL;
}

static void raw_emit(const char* event, RawClient* client, const char* payload) {
    char name[32];
    raw_name(client->id, name);
    FLValue handler = raw_handler;
    fl_heap_retain(handler);
    fl_arena_begin();
    FLValue args[5] = {
        fl_str_val(event), fl_str_val(name), fl_str_val(payload),
        fl_str_val(client->address), fl_int(client->port)
    };
    if (handler.tag == FL_FN) {
        fl_fn_call(handler, 5, args);
    } else {
        raw_named_handler(args[0], args[1], args[2], args[3]);
    }
    fl_arena_end();
    fl_heap_release(handler);
}

static void raw_close(RawClient* client) {
    if (client->fd < 0) return;
    int fd = client->fd;
    client->fd = -1;
    raw_emit("close", client, "");
    close(fd);
}

FLValue fl_tcp_server_stop(void) {
    for (int i = 0; i < RAW_CLIENTS; i++) raw_close(&raw_clients[i]);
    if (raw_server >= 0) close(raw_server);
    raw_server = -1;
    if (raw_handler.tag >= FL_STRING) fl_heap_release(raw_handler);
    raw_handler = fl_nil();
    raw_named_handler = NULL;
    return fl_str_val("ok");
}

FLValue fl_tcp_server_raw(FLValue port, FLValue handler, FLValue host) {
    if (raw_server >= 0 || port.tag != FL_INT || port.i < 1 || port.i > 65535 ||
        (handler.tag != FL_FN && handler.tag != FL_STRING)) return fl_str_val("error");
    if (handler.tag == FL_STRING) {
        char symbol[256];
        const char* source = raw_text(handler);
        size_t i = 0;
        for (; source[i] && i < sizeof(symbol) - 1; i++)
            symbol[i] = source[i] == '-' ? '_' : source[i];
        symbol[i] = 0;
        raw_named_handler = (RawHandler4)dlsym(RTLD_DEFAULT, symbol);
        if (!raw_named_handler) return fl_str_val("error");
    }
    char service[16];
    snprintf(service, sizeof(service), "%lld", (long long)port.i);
    struct addrinfo hints = {.ai_family = AF_UNSPEC, .ai_socktype = SOCK_STREAM};
    struct addrinfo* addresses = NULL;
    if (getaddrinfo(raw_text(host), service, &hints, &addresses)) return fl_str_val("error");
    int fd = -1;
    for (struct addrinfo* item = addresses; item; item = item->ai_next) {
        fd = socket(item->ai_family, item->ai_socktype, item->ai_protocol);
        if (fd < 0) continue;
        int reuse = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
        if (bind(fd, item->ai_addr, item->ai_addrlen) == 0 && listen(fd, RAW_CLIENTS) == 0) break;
        close(fd);
        fd = -1;
    }
    freeaddrinfo(addresses);
    if (fd < 0 || fcntl(fd, F_SETFL, O_NONBLOCK) < 0) {
        if (fd >= 0) close(fd);
        return fl_str_val("error");
    }
    raw_server = fd;
    raw_handler = fl_heap_copy(handler);
    for (int i = 0; i < RAW_CLIENTS; i++) raw_clients[i].fd = -1;
    return fl_str_val("ok");
}

FLValue fl_tcp_write(FLValue connection, FLValue data) {
    RawClient* client = raw_find(connection);
    if (!client || data.tag != FL_STRING) return fl_str_val("error");
    FLString* text = (FLString*)data.obj;
    size_t sent = 0;
    while (sent < text->len) {
        ssize_t n = send(client->fd, text->data + sent, text->len - sent, MSG_NOSIGNAL);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) return fl_str_val("error");
        sent += (size_t)n;
    }
    return fl_str_val("ok");
}

FLValue fl_tcp_drop(FLValue connection) {
    RawClient* client = raw_find(connection);
    if (!client) return fl_str_val("not-found");
    raw_close(client);
    return fl_str_val("ok");
}

FLValue fl_tcp_peer_port(FLValue connection) {
    RawClient* client = raw_find(connection);
    return client ? fl_int(client->port) : fl_nil();
}

FLValue fl_tcp_yield(FLValue timeout) {
    int milliseconds = timeout.tag == FL_INT && timeout.i > 0 ?
        (timeout.i > 60000 ? 60000 : (int)timeout.i) : 0;
    struct pollfd fds[RAW_CLIENTS + 1];
    RawClient* slots[RAW_CLIENTS + 1];
    int count = 0;
    if (raw_server >= 0) {
        fds[count] = (struct pollfd){.fd = raw_server, .events = POLLIN};
        slots[count++] = NULL;
    }
    for (int i = 0; i < RAW_CLIENTS; i++) if (raw_clients[i].fd >= 0) {
        fds[count] = (struct pollfd){.fd = raw_clients[i].fd, .events = POLLIN};
        slots[count++] = &raw_clients[i];
    }
    int ready = poll(fds, (nfds_t)count, milliseconds);
    if (ready <= 0) return fl_int(0);
    int dispatched = 0;
    for (int i = 0; i < count; i++) {
        if (!fds[i].revents) continue;
        if (!slots[i]) {
            struct sockaddr_storage peer;
            socklen_t length = sizeof(peer);
            int fd = accept(raw_server, (struct sockaddr*)&peer, &length);
            if (fd < 0) continue;
            int slot = -1;
            for (int j = 0; j < RAW_CLIENTS; j++)
                if (raw_clients[j].fd < 0) { slot = j; break; }
            if (slot < 0) { close(fd); continue; }
            RawClient* client = &raw_clients[slot];
            client->fd = fd;
            client->id = raw_next_id++;
            char service[NI_MAXSERV];
            if (getnameinfo((struct sockaddr*)&peer, length, client->address,
                            sizeof(client->address), service, sizeof(service),
                            NI_NUMERICHOST | NI_NUMERICSERV)) {
                raw_close(client);
                continue;
            }
            client->port = atoi(service);
            struct timeval deadline = {.tv_sec = 2};
            setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &deadline, sizeof(deadline));
            raw_emit("connect", client, "");
            dispatched++;
        } else {
            RawClient* client = slots[i];
            if (client->fd < 0) continue;
            char data[4097];
            ssize_t n = recv(client->fd, data, sizeof(data) - 1, 0);
            if (n > 0) {
                data[n] = 0; /* shortcut: text payloads only; add byte buffers for binary protocols. */
                raw_emit("data", client, data);
                dispatched++;
            } else if (n == 0 || (errno != EINTR && errno != EAGAIN)) {
                raw_close(client);
                dispatched++;
            }
        }
    }
    return fl_int(dispatched);
}
