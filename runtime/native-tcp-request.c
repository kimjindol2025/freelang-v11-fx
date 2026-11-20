/* One literal TCP request, no application protocol. The runtime parent owns
 * its total deadline and stdin frame. DNS and sockets live only in this fresh
 * executable; no resolver is entered after fork in a multithreaded runtime. */
#define _GNU_SOURCE
#include <runtime.h>
#include "tcp-request-frame.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

typedef struct { unsigned char *data; size_t size, capacity; } RequestBytes;
static int64_t request_now(void) {
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now)) return -1;
    return (int64_t)now.tv_sec * 1000 + now.tv_nsec / 1000000;
}
static int request_read(void *data, size_t size) {
    size_t used = 0;
    while (used < size) {
        ssize_t n = read(STDIN_FILENO, (char *)data + used, size - used);
        if (n > 0) used += (size_t)n;
        else if (n < 0 && errno == EINTR) continue;
        else return -1;
    }
    return 0;
}
static int request_append(RequestBytes *out, const void *data, size_t size) {
    if (size > SIZE_MAX - out->size) return -1;
    size_t needed = out->size + size;
    if (needed > out->capacity) {
        size_t cap = out->capacity ? out->capacity : 65536;
        while (cap < needed) {
            if (cap > SIZE_MAX / 2) { cap = needed; break; }
            cap *= 2;
        }
        void *next = realloc(out->data, cap);
        if (!next) return -1;
        out->data = next; out->capacity = cap;
    }
    if (size) memcpy(out->data + out->size, data, size);
    out->size += size;
    return 0;
}

/* Non-final UTF-8 decoder writes retain an incomplete trailing character.
 * Structural prefix detection deliberately precedes Unicode validity checks,
 * as in the retained Node StringDecoder. Decode/trim uses the existing v11
 * string contract after excluding that pending suffix. */
static size_t request_decoded_prefix(const unsigned char *data, size_t size) {
    size_t i = size;
    unsigned buffered = 0;
    while (i && buffered < 4) {
        unsigned byte = data[--i]; buffered++;
        if (!(byte & 0x80)) return size;
        if ((byte & 0xc0) == 0xc0) {
            unsigned expected = (byte & 0xe0) == 0xc0 ? 2 :
                (byte & 0xf0) == 0xe0 ? 3 : (byte & 0xf8) == 0xf0 ? 4 : 0;
            return expected > buffered ? size - buffered : size;
        }
    }
    return size;
}
static int request_emit(RequestBytes *bytes) {
    size_t size = request_decoded_prefix(bytes->data, bytes->size);
    if (size > UINT32_MAX) return 1;
    fl_cli_runtime_begin();
    FLValue decoded = fl_v11_utf8_string(fl_strn_val((const char *)bytes->data, size));
    FLValue trimmed = fl_v11_builtin(fl_str_val("str-trim"), fl_vec_from(&decoded, 1));
    if (trimmed.tag != FL_STRING || !trimmed.obj) return 1;
    FLString *text = (FLString *)trimmed.obj;
    size_t sent = 0;
    while (sent < text->len) {
        ssize_t n = write(STDOUT_FILENO, text->data + sent, text->len - sent);
        if (n > 0) sent += (size_t)n;
        else if (n < 0 && errno == EINTR) continue;
        else return 1;
    }
    return 0;
}
static int request_socket(struct addrinfo *address, int64_t idle_deadline, int last) {
    int fd = socket(address->ai_family, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK,
                    address->ai_protocol);
    if (fd < 0) return -1;
    int one = 1;
    (void)setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof one);
    int connected = connect(fd, address->ai_addr, address->ai_addrlen);
    if (!connected) return fd;
    if (errno != EINPROGRESS) { close(fd); return -1; }
    int64_t started = request_now();
    if (started < 0) { close(fd); return -1; }
    int64_t deadline = last ? idle_deadline : started + 250;
    if (idle_deadline < deadline) deadline = idle_deadline;
    for (;;) {
        int64_t now = request_now();
        if (now < 0 || now >= deadline) { close(fd); return -1; }
        int64_t left = deadline - now;
        struct pollfd ready = {fd, POLLOUT, 0};
        int rc = poll(&ready, 1, left > INT_MAX ? INT_MAX : (int)left);
        if (rc < 0 && errno == EINTR) continue;
        if (rc < 0) { close(fd); return -1; }
        if (!rc) continue;
        int error = 0;
        socklen_t length = sizeof error;
        if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &length) || error) {
            close(fd); return -1;
        }
        return fd;
    }
}
static int request_same_address(const struct addrinfo *a, const struct addrinfo *b) {
    if (a->ai_family != b->ai_family) return 0;
    if (a->ai_family == AF_INET)
        return !memcmp(&((struct sockaddr_in *)a->ai_addr)->sin_addr,
                       &((struct sockaddr_in *)b->ai_addr)->sin_addr, sizeof(struct in_addr));
    return !memcmp(&((struct sockaddr_in6 *)a->ai_addr)->sin6_addr,
                   &((struct sockaddr_in6 *)b->ai_addr)->sin6_addr, sizeof(struct in6_addr)) &&
        ((struct sockaddr_in6 *)a->ai_addr)->sin6_scope_id ==
        ((struct sockaddr_in6 *)b->ai_addr)->sin6_scope_id;
}
static int request_connect(const char *host, unsigned port, uint64_t idle_ms) {
    int64_t before_lookup = request_now();
    if (before_lookup < 0) return -1;
    char service[16];
    snprintf(service, sizeof service, "%u", port);
    struct addrinfo hints = {0}, *list = NULL;
    hints.ai_family = AF_UNSPEC; hints.ai_socktype = SOCK_STREAM;
    /* Numeric lookup also recognizes IPv6 scope IDs. DNS uses the reference's
     * ADDRCONFIG hint and system resolver; its parent still bounds all work. */
    hints.ai_flags = AI_NUMERICHOST;
    if (getaddrinfo(*host ? host : "localhost", service, &hints, &list)) {
        hints.ai_flags = AI_ADDRCONFIG;
        if (getaddrinfo(*host ? host : "localhost", service, &hints, &list)) return -1;
    }
    size_t count = 0;
    for (struct addrinfo *a = list; a; a = a->ai_next)
        if (a->ai_family == AF_INET || a->ai_family == AF_INET6) count++;
    struct addrinfo **families[2] = {NULL,NULL};
    size_t sizes[2] = {0,0};
    int fd = -1, preferred = 0;
    if (!count || count > SIZE_MAX / sizeof(*families[0])) goto connect_end;
    families[0] = malloc(count * sizeof(*families[0]));
    families[1] = malloc(count * sizeof(*families[1]));
    if (!families[0] || !families[1]) goto connect_end;
    int first = 1;
    for (struct addrinfo *a = list; a; a = a->ai_next) {
        if (a->ai_family != AF_INET && a->ai_family != AF_INET6) continue;
        int family = a->ai_family == AF_INET6;
        if (first) { preferred = family; first = 0; }
        int duplicate = 0;
        for (size_t i = 0; i < sizes[family]; i++)
            if (request_same_address(a, families[family][i])) { duplicate = 1; break; }
        if (!duplicate) families[family][sizes[family]++] = a;
    }
    int64_t now = request_now();
    if (now < 0) goto connect_end;
    if (idle_ms && now - before_lookup >= (int64_t)idle_ms) goto connect_end;
    int64_t idle_deadline = idle_ms ? now + (int64_t)idle_ms : INT64_MAX;
    size_t offsets[2] = {0,0}, remaining = sizes[0] + sizes[1];
    int family = preferred;
    while (remaining) {
        if (offsets[family] == sizes[family]) family = 1-family;
        struct addrinfo *address = families[family][offsets[family]++];
        remaining--;
        fd = request_socket(address, idle_deadline, remaining == 0);
        if (fd >= 0) break;
        family = 1-family;
    }
connect_end:
    free(families[0]); free(families[1]); freeaddrinfo(list);
    return fd;
}
int main(int argc, char **argv) {
    (void)argv;
    if (argc != 1) return 1;
    unsigned char frame[FL_TCP_FRAME_SIZE];
    if (request_read(frame, sizeof frame) || memcmp(frame, FL_TCP_FRAME_MAGIC, 8)) return 1;
    size_t host_size = fl_tcp_frame_read_u32(frame + 8);
    size_t line_size = fl_tcp_frame_read_u32(frame + 12);
    unsigned port = fl_tcp_frame_read_u32(frame + 16);
    uint64_t idle_ms = fl_tcp_frame_read_u64(frame + 20);
    if (port > 65535 || idle_ms > INT_MAX || host_size == SIZE_MAX || line_size == SIZE_MAX) return 1;
    char *host = malloc(host_size + 1), *line = malloc(line_size + 1);
    if (!host || !line) { free(host); free(line); return 1; }
    if (request_read(host, host_size) || request_read(line, line_size) ||
        memchr(host, 0, host_size)) { free(host); free(line); return 1; }
    host[host_size] = 0;
    if (!line_size || line[line_size-1] != '\n') line[line_size++] = '\n';
    int fd = request_connect(host, port, idle_ms);
    free(host);
    if (fd < 0) { free(line); return 1; }
    size_t sent = 0;
    RequestBytes response = {0};
    int result = 1;
    int64_t active = request_now();
    if (active < 0) goto done;
    for (;;) {
        int64_t now = request_now();
        if (now < 0) break;
        int64_t remaining = idle_ms ? (int64_t)idle_ms - (now-active) : INT_MAX;
        if (remaining <= 0) { if (response.size) result = request_emit(&response); break; }
        struct pollfd ready = {fd, POLLIN | (sent < line_size ? POLLOUT : 0), 0};
        int rc = poll(&ready, 1, remaining > INT_MAX ? INT_MAX : (int)remaining);
        if (rc < 0 && errno == EINTR) continue;
        if (rc < 0) break;
        if (!rc) continue;
        if (ready.revents & (POLLERR | POLLNVAL)) break;
        if (ready.revents & (POLLIN | POLLHUP)) {
            unsigned char bytes[65536];
            ssize_t n = recv(fd, bytes, sizeof bytes, 0);
            if (n > 0) {
                active = request_now();
                if (active < 0 || request_append(&response, bytes, (size_t)n)) break;
                if (memchr(bytes, '\n', (size_t)n)) { result = request_emit(&response); break; }
            } else if (!n) break; /* EOF does not emit a partial no-LF response. */
            else if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK) break;
        }
        if ((ready.revents & POLLOUT) && sent < line_size) {
            ssize_t n = send(fd, line + sent, line_size - sent, MSG_NOSIGNAL);
            if (n > 0) { sent += (size_t)n; active = request_now(); if (active < 0) break; }
            else if (n < 0 && errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK) break;
            else if (!n) break;
        }
    }
done:
    close(fd); free(line); free(response.data);
    return result;
}
