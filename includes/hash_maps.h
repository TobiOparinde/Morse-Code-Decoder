typedef struct {
    char* morse;
    char letter;
} MorseMapToLetter;

typedef struct {
    char* morse;
    uint8_t segments;
} MorseMapToSegments;

MorseMapToLetter morseToLetter[] = {
    {".-", 'A'}, {"-...", 'B'}, {"-.-.", 'C'}, {"-..", 'D'},
    {".", 'E'}, {"..-.", 'F'}, {"--.", 'G'}, {"....", 'H'},
    {"..", 'I'}, {".---", 'J'}, {"-.-", 'K'}, {".-..", 'L'},
    {"--", 'M'}, {"-.", 'N'}, {"---", 'O'}, {".--.", 'P'},
    {"--.-", 'Q'}, {".-.", 'R'}, {"...", 'S'}, {"-", 'T'},
    {"..-", 'U'}, {"...-", 'V'}, {".--", 'W'}, {"-..-", 'X'},
    {"-.--", 'Y'}, {"--..", 'Z'}, {"", '!'}, {NULL, 0}  // End marker
};

MorseMapToSegments morseToDisplay[] = {
    {".-", 0b11101110}, {"-...", 0b00111110}, {"-.-.", 0b10011100}, {"-..", 0b01111010},
    {".", 0b10011110}, {"..-.", 0b10001110}, {"--.", 0b11110110}, {"....", 0b01101110},
    {"..", 0b00001100}, {".---", 0b01111000}, {"-.-", 0b01101110}, {".-..", 0b00011100},
    {"--", 0b11101100}, {"-.", 0b00101010}, {"---", 0b11111100}, {".--.", 0b11001110},
    {"--.-", 0b11100110}, {".-.", 0b00001010}, {"...", 0b10110110}, {"-", 0b00011110},
    {"..-", 0b01111100}, {"...-", 0b00111000}, {".--", 0b01010100}, {"-..-", 0b01101110},
    {"-.--", 0b01110110}, {"--..", 0b11011010}, {NULL, 0}  // End marker
};

char decodeMorseToPrint(char* code) {
    for (int i = 0; morseToLetter[i].morse != NULL; i++) {
        if (strcmp(morseToLetter[i].morse, code) == 0) {
            return morseToLetter[i].letter;
        }
    }
    return '?';  // Return '?' if no match is found
}

uint8_t decodeMorseToDisplay(char* code) {
    for (int i = 0; morseToDisplay[i].morse != NULL; i++) {
        if (strcmp(morseToDisplay[i].morse, code) == 0) {
            return morseToDisplay[i].segments;
        }
    }
    return '?';  // Return '?' if no match is found
}
