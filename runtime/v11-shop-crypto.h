/* Shop authentication primitives. Include after v11-auth.h in v11-compat.h.
 * Strings cross the UTF-16/UTF-8 boundary using the existing text helpers;
 * cryptography is supplied by OpenSSL, never by crypto-stub.c.
 * hash-eq intentionally accepts complete 32-byte SHA-256 hex digests only.
 */
#ifndef FL_V11_SHOP_CRYPTO_H
#define FL_V11_SHOP_CRYPTO_H

#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>
#include <time.h>

#define VSC_RANDOM_MAX (1024u * 1024u)

static FLValue vsc_error(const char *op, const char *kind, const char *why) {
    char message[192];
    snprintf(message, sizeof(message), "%s: %s", op, why);
    fl_throw(fl_make_error(kind, message));
    return fl_nil();
}

static FLValue vsc_random(FLValue count) {
    if (!t_numeric(count))
        return vsc_error("random_bytes", "TypeError", "integer byte count required");
    double number = t_double(count);
    if (!isfinite(number) || number < 0 || number > VSC_RANDOM_MAX || floor(number) != number)
        return vsc_error("random_bytes", "RangeError", "byte count outside native resource limit");
    size_t length = (size_t)number;
    if (!length) return fl_str_val("");
    unsigned char *bytes = malloc(length);
    char *hex = malloc(length * 2 + 1);
    if (!bytes || !hex) {
        free(bytes); free(hex);
        return vsc_error("random_bytes", "RuntimeError", "allocation failed");
    }
    if (RAND_bytes(bytes, (int)length) != 1) {
        OPENSSL_cleanse(bytes, length); free(bytes); free(hex); ERR_clear_error();
        return vsc_error("random_bytes", "RuntimeError", "secure random failed");
    }
    va_hex(bytes, length, hex);
    OPENSSL_cleanse(bytes, length); free(bytes);
    FLValue result = fl_str_val(hex);
    OPENSSL_cleanse(hex, length * 2); free(hex);
    return result;
}

static int vsc_digest(FLValue value, unsigned char out[32]) {
    if (value.tag != FL_STRING) return 0;
    FLString *text = (FLString *)value.obj;
    if (text->len != 64) return 0;
    unsigned invalid = 0;
    for (size_t i = 0; i < 32; i++) {
        int high = va_nibble((unsigned char)text->data[i * 2]);
        int low = va_nibble((unsigned char)text->data[i * 2 + 1]);
        invalid |= (unsigned)(high < 0 || low < 0);
        out[i] = (unsigned char)(((unsigned)high & 15u) * 16u + ((unsigned)low & 15u));
    }
    return invalid == 0;
}

static FLValue vsc_equal(FLValue left, FLValue right) {
    unsigned char a[32] = {0}, b[32] = {0};
    int valid_a = vsc_digest(left, a), valid_b = vsc_digest(right, b);
    int same = CRYPTO_memcmp(a, b, sizeof(a)) == 0;
    OPENSSL_cleanse(a, sizeof(a)); OPENSSL_cleanse(b, sizeof(b));
    return fl_bool(valid_a && valid_b && same);
}

static FLValue vsc_hmac(FLValue key, FLValue message) {
    if (key.tag != FL_STRING || message.tag != FL_STRING)
        return vsc_error("hmac_sha256", "TypeError", "string key and message required");
    unsigned char digest[32]; char hex[65];
    /* va_mac takes message first; the public API takes key first. */
    va_mac(message, key, digest, "hmac_sha256");
    va_hex(digest, sizeof(digest), hex);
    OPENSSL_cleanse(digest, sizeof(digest));
    FLValue result = fl_str_val(hex);
    OPENSSL_cleanse(hex, sizeof(hex));
    return result;
}

static FLValue vsc_now_iso(void) {
    struct timespec stamp;
    struct tm utc;
    if (clock_gettime(CLOCK_REALTIME, &stamp) != 0 ||
        !gmtime_r(&stamp.tv_sec, &utc))
        return vsc_error("now_iso", "RuntimeError", "UTC clock failed");
    if (utc.tm_year < -1900 || utc.tm_year > 8099 ||
        stamp.tv_nsec < 0 || stamp.tv_nsec >= 1000000000L)
        return vsc_error("now_iso", "RangeError", "UTC clock outside ISO year range");
    char text[32];
    int length = snprintf(text, sizeof(text), "%04d-%02d-%02dT%02d:%02d:%02d.%03ldZ",
        utc.tm_year + 1900, utc.tm_mon + 1, utc.tm_mday,
        utc.tm_hour, utc.tm_min, utc.tm_sec, stamp.tv_nsec / 1000000L);
    if (length != 24)
        return vsc_error("now_iso", "RuntimeError", "UTC formatting failed");
    return fl_str_val(text);
}

/* Root integrates only these four normalized operation names into the public
 * catalog and dispatcher. A wrong arity never silently drops arguments. */
static FLValue fl_v11_shop_crypto(FLValue name, FLValue argv) {
    const char *op = ((FLString *)name.obj)->data;
    FLVector *args = (FLVector *)argv.obj;
    size_t count = args->len;
    if (!strcmp(op, "random-bytes")) {
        if (count != 1) return vsc_error("random_bytes", "TypeError", "one argument required");
        return vsc_random(args->data[0]);
    }
    if (!strcmp(op, "hash-eq")) {
        if (count != 2) return vsc_error("hash_eq", "TypeError", "two arguments required");
        return vsc_equal(args->data[0], args->data[1]);
    }
    if (!strcmp(op, "hmac-sha256")) {
        if (count != 2) return vsc_error("hmac_sha256", "TypeError", "two arguments required");
        return vsc_hmac(args->data[0], args->data[1]);
    }
    if (!strcmp(op, "now-iso")) {
        if (count != 0) return vsc_error("now_iso", "TypeError", "no arguments allowed");
        return vsc_now_iso();
    }
    return vsc_error("shop_crypto", "RuntimeError", "unknown operation");
}

#endif
