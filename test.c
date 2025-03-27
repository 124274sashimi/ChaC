#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void printBinary(char c, FILE *output) {
    for (int i = 7; i >= 0; i--) {
        fputc((c >> i) & 1 ? '1' : '0', output);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3 || strcmp(argv[1], "--file") != 0) {
        printf("Usage: %s --file <filename>\n", argv[0]);
        return 1;
    }

    FILE *inputFile = fopen(argv[2], "r");
    if (inputFile == NULL) {
        printf("Error: Could not open input file '%s'\n", argv[2]);
        return 1;
    }

    char outputFileName[256];
    snprintf(outputFileName, sizeof(outputFileName), "%s.bin", argv[2]);
    FILE *outputFile = fopen(outputFileName, "w"); // Text mode for readability
    if (outputFile == NULL) {
        fclose(inputFile);
        return 1;
    }

    int c;
    while ((c = fgetc(inputFile)) != EOF) {
        printBinary((char)c, outputFile);
        fputc(' ', outputFile); // Space between bytes for readability
    }

    fclose(inputFile);
    fclose(outputFile);
    printf("Converted '%s' to binary format in '%s'\n", argv[2], outputFileName);
    return 0;
}
