/* 
 * peekachu - Utility to peek at values stored in memcache. Support compression via FastLZ.
 *
 * Copyright (C) 2014 Schoology, Inc (sysadmins@schoology.com)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef PEEKACHU_H
#define PEEKACHU_H

#define PEEKACHU_VERSION 0x000100

#define PEEKACHU_VERSION_MAJOR    0
#define PEEKACHU_VERSION_MINOR    1
#define PEEKACHU_VERSION_REVISION 0

#if defined (__cplusplus)
extern "C" {
#endif

/* Constants */
#define MAX_CONNECTION_STRING 2048

/* 
 * Connects to the given memcache host.
 *
 * host - Hostname of the server to connect to.
 * port - When positive, non-zero value is passed used as connection port.
 *        If negative or zero is passed, then default of 11211 is used.
 *
 * returns connection to memcache server.
 *
 */
memcached_st *si_connect(const char *host, int port);

/* 
 * Sets a key to a value, optionally compressing it.
 *
 * mc        - Connection to memcache server.
 * key       - Key to set the value of.
 * value     - Raw (i.e. uncompressed) value of the key.
 * value_len - Length, in characters, of value.
 * compress  - 1 to compress value before sending to memcache, any other value will not compress.
 *
 * returns size_t representing the size of data sent to memcache. 
 * 
 */
size_t si_set(memcached_st *mc, const char *key, const char *value, size_t value_len, int compress);

/* 
 * Get the value of a key from memcache.
 *
 * mc         - Connection to memcache server.
 * key        - Key to get the value of.
 * decompress - Set to 1 to attempt decompression of raw, any other value and decompression will not occur.
 * raw        - Set to the raw value stored by memcache (i.e. may or may not be compressed).
 *              This value is the same regardless of the value of "decompress". 
 * raw_len    - Set to the size of the raw value.
 * value      - NULL if "raw" was not compressed, or if an error occured during decompression.
 *              Otherwise, this will be set to the decompressed value of "raw".
 * value_len  - Set to the size of "value", or 0 (zero) if "value" is NULL.
 *
 */
void si_get(memcached_st *mc, const char *key, int decompress, char **raw, size_t *raw_len, char **value, size_t *value_len);

/*
 * Implements the compression scheme used by the the PHP Memcached extension.
 *
 * input      - Value to be compressed.
 * input_len  - Integer represention of the size of the input.
 * output     - Set to the compressed data.
 *
 * returns the size of the compressed data (i.e. size of "output").
 */
int si_compress(const char *input, int input_len, char **output);

/*
 * Implements the decompression scheme used by the PHP Memcached extension.
 *
 * input     - Compressed data.
 * input_len - Integer representing the size of the compressed data.
 * output    - Set to the resulting decompressed data, or NULL on error/size mismatch.
 *
 * returns the size of the decompressed data (i.e. size of "output") or 0 (zero) on error.
 *
 */
int si_decompress(const char *input, int input_len, char **output);

/* Command Line Argument Parsing */
const char *argp_program_version = "peekachu-0.1.0";
const char *argp_program_bug_address = "<sysadmins@schoology.com>";

static char program_doc[] =
    "peekachu -- Allows users to peek at compressed/raw values in memcache.";

static char args_doc[] = "KEY";

static struct argp_option options[] = {
    { "host",        'h', "HOST", 0, "Hostname of the memcache server to connect to." },
    { "port",        'p', "PORT", 0, "Port number of the memcache server to connect to." },
    { "compression", 'z', 0,      0, "Enabled compression/decompression." },
    { "file",        'f', "FILE", 0, "File to get key's value from. Causes key to be set. Use - to read from stdin, otherwise path to FILE." },
    { "raw",         'r', 0,      0, "Forces peekachu to always write the raw key value to stdout." },
    { 0 }
};

struct si_context_t {
    char *host;
    int port;
    char *key;
    int compression;
    char *file;
    int raw;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state);

#if defined (__cplusplus)
}
#endif

#endif /* PEEKACHU_H */
