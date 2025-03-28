#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

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
            out[i*8 + j + 8] = x[j] ^ in[j];
        }
    }
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

void xor(uint32_t buffer[8], uint32_t keystream[8]) {
    for (int i = 0; i < 8; i++) {
        buffer[i] ^= keystream[i];
    }
}


int main(int argc, char *argv[]) {
    uint32_t key[4] = { 0, 0, 0, 0 }; // Replace with your key
    
    uint32_t nonce[2] = { 0, 0 };

    if (argc < 3) {
        fprintf(stderr, "Usage: %s [-n nonce] input_file output_file\n", argv[0]);
        return 1;
    }

    // input parsing :()
    int opt;
    while ((opt = getopt(argc, argv, "hn:")) != -1) {
        switch (opt) {
            case 'n':
                ; // make the compiler happy
                char *endptr;  // To detect invalid input
                uint64_t hex_value = strtol(optarg, &endptr, 16);

                if (*endptr != '\0') {
                    printf("Invalid hexadecimal input.\n");
                } else {
                    printf("Using a nonce of 0x%llx\n", hex_value);
                }

                nonce[0] = hex_value >> 32;
                nonce[1] = hex_value;
                break;
            case 'h':
            default:
                fprintf(stderr, "Usage: %s [-n nonce] input_file output_file\n", argv[0]);
                return 1;
        }
    }

    if (argc - optind != 2) {
        fprintf(stderr, "Usage: %s [-n nonce] input_file output_file\n", argv[0]);
        return 1;
    }

    // Open file to encrypt
    FILE *inputFile = fopen(argv[optind], "r");

    if (inputFile == NULL) {
        printf("Error opening the input file");
        return 1;
    }

    // Open file to decrypt
    FILE *outputFile = fopen(argv[optind+1], "w");

    if (outputFile == NULL) {
        printf("Error opening the output");
        return 1;
    }


    // Init values for block
    uint32_t block[8];

    block[0] = key[0];
    block[1] = key[1];
    block[2] = key[2];
    block[3] = key[3];

    block[4] = 0; // count
    block[5] = nonce[0];
    block[6] = nonce[1];
    block[7] = CONST;

    // Perform ChaC!
    size_t bytes_read = 0;
    int sub_block = 0;
    unsigned char buffer[32];
    uint32_t stream[32];

    while ((bytes_read = fread(buffer, sizeof(unsigned char), 32, inputFile)) > 0) {
        for (int i = bytes_read; i < 32; i++) { // clear remaining bytes
            buffer[i] = 0;
            printf("a");
        }

        if ((sub_block % 4) == 0) {
            sub_block = 0;
            keystream(stream, block); 
            block[4]++; // count
        }

        xor((uint32_t *) buffer, stream+(sub_block * 8));

        fwrite(buffer, sizeof(unsigned char), 32, outputFile);
        sub_block++;
    }

    fclose(inputFile);
    fclose(outputFile);
    return 0;
}
