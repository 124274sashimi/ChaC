#include <stdint.h>
#include <stdio.h>

#define ROTL(a,b) (((a) << (b)) | ((a) >> (32 - (b))))
#define QR(a, b, c, d) (             \
        a += b, d ^= a, d = ROTL(d, 16), \
        c += d, b ^= c, b = ROTL(b, 12), \
        a += b, d ^= a, d = ROTL(d,  8), \
        c += d, b ^= c, b = ROTL(b,  7))
#define ROUNDS 20

// binary representation of "ChaC"
#define CONST 0b01000011011010000110000101000011

void keystream(uint64_t out[8], uint64_t const in[8]) {

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

void print_block(FILE *file, uint64_t const in[8]) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 32; j++) {
            fprintf(file, "%c", ((in[i] >> (31-j)) & 1) ? '0' : '1');
            //printf("%c", ((in[i] >> (31-j)) & 1) ? '0' : '1');
        }
        //printf(" ");
        //if (i%4 == 3) { printf("\n"); }
    }
}

void copy_block(uint64_t dest[8], uint64_t const src[8]) {
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

int main() {
    /*uint32_t key[4], nonce[2];*/
    /*uint32_t count;*/
    uint64_t block[8];


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

    //printf("-----------------------------------------------------------------------------------------------------------------------------------\n");
    //
    FILE *file = fopen("nist_data.txt", "a");

    uint64_t res[8], last_res[8];
    copy_block(last_res, block);
    //printf("sizeof(block)=%zu, sizeof(res)=%zu, sizeof(last_res)=%zu\n",
    for (int i = 0; i < 4000; i++) { // Get 1 million characters to test randomness
        keystream(res, block);
        print_block(file, res);
        //printf("%d\n\n", block_diffs(res, last_res));
        copy_block(last_res, res);
        block[4]++; // count
    }

    fclose(file);

}
