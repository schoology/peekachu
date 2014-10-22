#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <argp.h>
#include <libmemcached/memcached.h>

#include "fastlz.h"
#include "peekachu.h"

memcached_st *si_connect(const char *host, int port) {
    char conn[MAX_CONNECTION_STRING];
    int conn_port = 11211;

    if (port > 0) {
        conn_port = port;
    }
    sprintf(conn, "--SERVER=%s:%d", host, conn_port);

    return memcached(conn, strlen(conn));
}

size_t si_set(memcached_st *mc, const char *key, const char *value, size_t value_len, int compress) {
    if (compress == 1) {
        char *compressed;
        int compressed_len;

        compressed_len = si_compress(value, value_len, &compressed);
        memcached_set(mc, key, strlen(key), compressed, compressed_len, (time_t)0, (uint32_t)0);
        
        return compressed_len;
    }

    memcached_set(mc, key, strlen(key), value, value_len, (time_t)0, (uint32_t)0);

    return value_len;
}

void si_get(memcached_st *mc, const char *key, int decompress, char **raw, size_t *raw_len, char **value, size_t *value_len) {
    uint32_t flags;
    memcached_return_t err;

    (*raw) = memcached_get(mc, key, strlen(key), raw_len, &flags, &err);

    if (decompress == 1) {
        (*value_len) = si_decompress(*raw, *raw_len, value);
    }
}

int si_compress(const char *input, int input_len, char **output) {
    int len;
    uint32_t size_header = input_len;
    char *compressed;
    uint32_t output_len = sizeof(uint32_t) + (uint32_t)((input_len*1.05) + 2);
    compressed = malloc(output_len);

    memcpy(compressed, &size_header, sizeof(uint32_t));
    len = fastlz_compress(input, input_len, (compressed + sizeof(uint32_t)));
    
    if(len > 0) {
       len += sizeof(uint32_t);
       compressed[len] = 0;
       (*output) = compressed;

       return len;
    }
    else {
        free(output);
    }

    return 0;
}

int si_decompress(const char *input, int input_len, char **output) {
    uint32_t len;

    if (input_len > sizeof(uint32_t)) {
        memcpy(&len, input, sizeof(uint32_t));
        if (len > 0) {
            char *decompressed;

            input += sizeof(uint32_t);
            input_len -= sizeof(uint32_t);
            decompressed = malloc(len + 1);

            if (len == fastlz_decompress(input, input_len, decompressed, len)) {
                decompressed[len] = 0;
                (*output) = decompressed;
                return len;
            }
            else {
                free(decompressed);
            }
        }
    }

    return 0;
}

void compression_test() {
    char *data = "IamTextIamTextIamTextIamTextIamText";
    char *comp;
    char *decomp;
    int len;
    len = si_compress(data, strlen(data), &comp);

    printf("\n-- COMPRESSION_TEST --\n");

    if (len <= 0) {
        printf("Error compressing: %d\n", len);
        return;
    }
    else {
        int i;
        printf("Compressed Data:\n");
        printf("len: %d\n", len);
        for (i = 0; i < len; ++i) {
            printf("%c", comp[i]);
        }
        printf("\n");
    }

    len = si_decompress(comp, len, &decomp);

    if (len <= 0) {
        printf("Error decompressing: %d\n", len);
        return;
    }
    else {
        int i;
        printf("Decompressed Data:\n");
        for (i = 0; i < len; ++i) {
            printf("%c", decomp[i]);
        }
        printf("\n");
    }

    free(comp);
    free(decomp);
}

void full_test() {
    const char *key = "footext";
    const char *sent = "I am some text that the footext key will text text text text store text.";
    size_t sent_len;
    char *raw;
    size_t raw_len;
    char *value;
    size_t value_len;

    printf("\n-- FULL_TEST --\n");

    memcached_st *mc = si_connect("localhost", -1);

    sent_len = si_set(mc, key, sent, strlen(sent), 1);
    printf("%d compressed to %d before sending\n", strlen(sent), sent_len);

    si_get(mc, key, 1, &raw, &raw_len, &value, &value_len);
    printf("%s\n", value);
}

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct si_context_t *ctx = state->input;

    switch(key) {
        case 'h':
            ctx->host = arg;
            break;

        case 'p':
            ctx->port = atoi(arg);
            break;

        case 'z':
            ctx->compression = 1;
            break;

        case 'f':
            ctx->file = arg;
            break;

        case 'r':
            ctx->raw = 1;
            break;

        case ARGP_KEY_ARG:
            if (state->arg_num >= 1)
                argp_usage(state);

            ctx->key = arg;
            break;

        case ARGP_KEY_END:
            if (state->arg_num < 1)
                argp_usage(state);
            break;

        default:
            return ARGP_ERR_UNKNOWN;
    }   

    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, program_doc };

/* Prints a string character by character up to len. */
void printchars(const char *input, int len) {
    int i;
    for(i = 0; i < len; ++i) {
        printf("%c", input[i]);
    }
}

/* Peeks at a key's value in memcache. */
int si_peek(struct si_context_t *ctx) {
    char *raw;
    size_t raw_len;
    char *value;
    size_t value_len;

    memcached_st *mc = si_connect(ctx->host, ctx->port);
    si_get(mc, ctx->key, ctx->compression, &raw, &raw_len, &value, &value_len);

    if (raw == NULL) {
        printf("No value for key: %s\n", ctx->key);
        return 0;
    }

    if (ctx->raw) {
        printf("\n|----- BEGIN RAW VALUE -----|\n");
        printchars(raw, raw_len); 
        printf("\n|-----  END RAW VALUE  -----|\n");
        free(raw);
    }

    if (value == NULL) {
        printf("Value doesn't seem to be compressed...you sure it should be?\n");
    }
    else {
        printf("\n|----- BEGIN VALUE -----|\n");
        printchars(value, value_len);
        printf("\n|-----  END VALUE  -----|\n");
        free(value);
    }

    return 1;
}

/* Reads the value to send to memcache from a file (or stdin). */
int si_read(const char *file, char **value) {
    FILE *f = fopen(file, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *data = malloc(fsize + 1);
    fread(data, fsize, 1, f);
    fclose(f);

    data[fsize] = 0;
    (*value) = data;

    return (int)fsize;
}

/* Pushes a value up to memcache. */
int si_push(struct si_context_t *ctx) {
    char *raw;
    int raw_len = si_read(ctx->file, &raw);

    if (ctx->raw) {
        printf("\n|----- BEGIN RAW VALUE -----|\n");
        printchars(raw, raw_len); 
        printf("\n|-----  END RAW VALUE  -----|\n");
    }
    memcached_st *mc = si_connect(ctx->host, ctx->port);

    return si_set(mc, ctx->key, raw, raw_len, ctx->compression);
}

int main(int argc, char **argv) {

    struct si_context_t ctx;
    ctx.host = "localhost";
    ctx.port = 11211;
    ctx.key = 0;
    ctx.compression = 0;
    ctx.file = 0;
    ctx.raw = 0;

    argp_parse(&argp, argc, argv, 0, 0, &ctx);

    if (ctx.file != NULL) {
        si_push(&ctx);
    }
    else {
        si_peek(&ctx);
    }
    
    return 0;
}
