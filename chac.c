#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ROTL(a,b) (((a) << (b)) | ((a) >> (32 - (b))))
#define QR(a, b, c, d) (             \
        a += b, d ^= a, d = ROTL(d, 16), \
        c += d, b ^= c, b = ROTL(b, 12), \
        a += b, d ^= a, d = ROTL(d,  8), \
        c += d, b ^= c, b = ROTL(b,  7))
#define ROUNDS 20

// binary representation of "ChaC"
#define CONST 0b01000011011010000110000101000011

void keystream(uint32_t out[8], uint32_t const in[8]) {

    uint32_t x[16];

    // Copy input block to output
    for (int i = 0; i < 8; i++) {
        x[i] = in[i];
    }

    for (int i = 0; i < ROUNDS; i+=2) {
        // Columns
        QR(x[0], x[1], x[2], x[3]);
        QR(x[4], x[5], x[6], x[7]);

        // Diagonals
        QR(x[0], x[5], x[2], x[7]);
        QR(x[1], x[6], x[3], x[4]);
    }

    for (int i = 0; i < 8; i++) {
        out[i] = x[i] ^ in[i];

    }

    // TODO: Generate the next three 256-bit keystreams
}

void print_block(uint32_t const in[8]) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 32; j++) {
            printf("%c", ((in[i] >> (31-j)) & 1) ? '0' : '1');
        }
        printf(" ");
        //if (i%4 == 3) { printf("\n"); }
    }
}

void copy_block(uint32_t dest[8], uint32_t const src[8]) {
    for (int i = 0; i < 8; i++) {
        dest[i] = src[i];
    }
}

int block_diffs(uint32_t a[8], uint32_t b[8]) {
    int diffs = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 32; j++) {
            diffs += ((a[i] >> j) & 1) ^ ((b[i] >> j) & 1);
        }
    }
    return diffs;
}

int main(int argc, char *argv[]) {
    /*uint32_t key[4], nonce[2];*/
    /*uint32_t count;*/
    uint32_t block[8];


    uint32_t key[4] = { 0, 0, 0, 0 };
    uint32_t nonce[2] = { 0, 0 };
    uint32_t count = 0;

    block[0] = key[0];
    block[1] = key[1];
    block[2] = key[2];
    block[3] = key[3];

    block[4] = count;
    block[5] = nonce[0];
    block[6] = nonce[1];
    block[7] = CONST;

    //print_block(block);

    FILE *inputFile = fopen(argv[2], "r");
    fseek(inputFile, 0, SEEK_END);
    long fileSize = ftell(inputFile)*8;
    printf("%s", "File Size:");
    printf("%ld", fileSize);
    printf("%s", "\n");

    int32_t *buffer = (int32_t *)malloc(fileSize);

    uint32_t res[8], last_res[8];
    copy_block(last_res, block);


    for (int i = 0; i < fileSize/256; i++) {
        keystream(res, block);
        print_block(res);
        copy_block(last_res, res);

        uintptr_t shift = (uintptr_t)res;
        int32_t index = shift >> 1;

        printf("%s", "index:");
        printf("%d", index);
        printf("%s", "\n");

        int32_t size = 32;
        memcpy(buffer, &index, size);
        printf("%s", "\n");
        print_block(buffer);

        block[4]++; // count
    }
    size_t bytesRead = fread(buffer, 1, fileSize, inputFile);

    fclose(inputFile);

}
