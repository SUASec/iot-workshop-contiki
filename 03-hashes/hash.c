/*
 * Copyright (c) 2022, Schmalkalden University of Applied Sciences
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/**
 * \file
 *      Example on how to create hashes using libjss
 * \author
 *      Tobias Tefke <t.tefke@stud.fh-sm.de>
 */

/* Contiki main file */
#include "contiki.h"

/* Crypto */
#include "sha.h"
#include "dev/sha256.h"

/* C libraries */
#include <stdio.h>
#include <string.h>

PROCESS(hash_p, "Hash");
AUTOSTART_PROCESSES(&hash_p);

void print_hash(unsigned char* hash, short len)
{
    short x;
    for(x = 0; x < len; x++)
        printf("%02x", hash[x]);
    printf("\n");
}

PROCESS_THREAD(hash_p, ev, data)
{
    PROCESS_BEGIN();

    char message_256[] = "Test string to hash";

    // SHA256
    short sha_256_len = 32; // 32 bytes = 256 bit
    unsigned char sha_256_val[sha_256_len];

    int result = sha_256(&message_256, sha_256_val);
    if (result == CRYPTO_SUCCESS) {
        puts("Created SHA256 hash:");
        print_hash(sha_256_val, sha_256_len);
        puts("Control value:");
        puts("0dd8a7fc5978ddc5b751276017d4ffd142a89c67241efd5fcc3ea6d40ec6c743");
    } else {
        puts("Could not create SHA256 hash");
    }
    
    puts("");

    // SHA512
    short sha_512_len = 64; //  64 bytes = 512 bit
    unsigned char sha_512_val[sha_512_len];
    const unsigned char* message_512 = (unsigned char*) message_256;
    sha_512(message_512, sha_512_val);
    puts("SHA512 hash:");
    print_hash(sha_512_val, sha_512_len);
    puts("Control value:");
    puts("656203ca58030af14b00eba92e7ea307e5acc8d80aac164f13162251e6c9f0e0caf71cb12ef67a44ced403c0a5135f5f4a7bf2c9ea8fdee271919a5ac0118b6e");

    PROCESS_END();
}
