/* Native-only embedding tool. Reads the owned FreeLang source, writes the
 * generated C header to stdout, and includes its exact SHA-256. */
#include <stdio.h>
#include <stdlib.h>
#include <openssl/evp.h>

int main(int argc, char **argv) {
    if (argc != 2) { fputs("usage: embed-browser-island-core source.fl\n", stderr); return 2; }
    FILE *input = fopen(argv[1], "rb");
    if (!input) { fputs("embed: source open failed\n", stderr); return 1; }
    unsigned char *bytes = malloc(4u * 1024u * 1024u);
    if (!bytes) { fclose(input); return 1; }
    size_t length = fread(bytes, 1, 4u * 1024u * 1024u, input);
    int invalid = ferror(input) || !length || !feof(input);
    if (fclose(input)) invalid = 1;
    if (invalid) { free(bytes); fputs("embed: empty, oversized or unreadable source\n", stderr); return 1; }
    unsigned char hash[EVP_MAX_MD_SIZE]; unsigned hash_length = 0;
    EVP_MD_CTX *context = EVP_MD_CTX_new();
    int ok = context && EVP_DigestInit_ex(context, EVP_sha256(), NULL) == 1 &&
        EVP_DigestUpdate(context, bytes, length) == 1 &&
        EVP_DigestFinal_ex(context, hash, &hash_length) == 1 && hash_length == 32;
    EVP_MD_CTX_free(context);
    if (!ok) { free(bytes); fputs("embed: SHA-256 failed\n", stderr); return 1; }
    puts("/* Generated from self/browser-island-core.fl; do not edit. */");
    puts("static const unsigned char fl_browser_island_core[] = {");
    for (size_t index = 0; index < length; index++) {
        if (index % 12 == 0) fputs("  ", stdout);
        printf("0x%02x%s", bytes[index], index + 1 == length ? "" : ", ");
        if (index % 12 == 11 || index + 1 == length) putchar('\n');
    }
    puts("};");
    fputs("static const char fl_browser_island_core_sha256[] = \"", stdout);
    for (unsigned index = 0; index < hash_length; index++) printf("%02x", hash[index]);
    puts("\";");
    free(bytes);
    if (fflush(stdout) || ferror(stdout)) { fputs("embed: output failed\n", stderr); return 1; }
    return 0;
}
