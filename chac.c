#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define ROTL(a,b) (((a) << (b)) | ((a) >> (32 - (b))))
#define QR(a, b, c, d) (             \
        a += b, d ^= a, d = ROTL(d, 16), \
        c += d, b ^= c, b = ROTL(b, 12), \
        a += b, d ^= a, d = ROTL(d,  8), \
        c += d, b ^= c, b = ROTL(b,  7))
#define ROUNDS 20

// binary representation of "ChaC"
#define CONST 0b01000011011010000110000101000011

void keystream(uint32_t out[32], uint32_t const in[8]) {

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

    for (int i = 0; i < 3; i++) { // Cheap rotations
        QR(x[0], x[1], x[2], x[3]);
        QR(x[4], x[5], x[6], x[7]);

        // Diagonals
        QR(x[0], x[5], x[2], x[7]);
        QR(x[1], x[6], x[3], x[4]);   

        for (int j = 0; j < 8; j++) {
            out[i*8 + j] = x[j] ^ in[j];
        }
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

void add256Bits(unsigned char* buffer, size_t position, uint32_t* data, size_t bufferSize) {

    const size_t CHUNK_SIZE_BYTES = 32;
    if (position + CHUNK_SIZE_BYTES <= bufferSize) {
        memcpy(buffer + position, data, CHUNK_SIZE_BYTES);
    } else {
        fprintf(stderr, "Buffer overflow prevented\n");
    }
}
void crypt(unsigned char* message, unsigned char* keystream, unsigned char* store, int32_t length ){
    for(int i = 0; i < length; i++) {
        store[i] = message[i] ^ keystream[i];
    }
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

    // Open file to encrypt
    FILE *inputFile = fopen(argv[2], "r");
    fseek(inputFile, 0, SEEK_END);
    long fileSize = ftell(inputFile);
    fseek(inputFile, 0, SEEK_SET);
    int32_t totalBytes = ceil((double)fileSize / 128)*128;
    unsigned char* buffer = (unsigned char*)malloc(totalBytes);
    memset(buffer, 0, totalBytes);

    uint32_t res[32], last_res[32];
    copy_block(last_res, block);

    // Generate keystream, save to buffer
    for (int i = 0; i < totalBytes/128; i++) {
        keystream(res, block);
        copy_block(last_res, res);
        copy_block(last_res+8, res);
        copy_block(last_res+16, res);
        copy_block(last_res+24, res);
        add256Bits(buffer, i*128,res,totalBytes);
        block[4]++; // count
    }

    unsigned char* fileContent = (unsigned char *) malloc(totalBytes); //Message
    size_t message = fread(fileContent, 1, fileSize, inputFile);
    fclose(inputFile);

    unsigned char *cyphertext = (unsigned char *) malloc(totalBytes + 0); // Output Encryption File
    unsigned char *decryption = (unsigned char *) malloc(totalBytes + 0); // Output Encryption File

    printf("Original text:\n%s\n", fileContent);

    // Encrypt Text
    crypt(fileContent, buffer, cyphertext, totalBytes);

    printf("%s", "Cyphertext:\n");
    printf("%s\n\n", cyphertext);

    //Decrypt text
    crypt(cyphertext, buffer, decryption, totalBytes);
    printf("Decryption:\n%s\n", decryption);


}
