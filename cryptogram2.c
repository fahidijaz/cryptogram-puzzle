#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

void shuffle(char key[]);
void initialization();
bool updateState(char input[]);
void displayWorld();

char puzzle[100];
char encryptedString[100];
char playerKey[26] = {'\0'};
char encryptionKey[26] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};

int main() {
    initialization();
    displayWorld();
    char input[10]; // accommodate longer inputs
    while (true) {
        printf("Enter a pair of characters or 'quit' to quit: ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0'; // remove newline character
        if (updateState(input)) {
            break; // exit loop 
        }
        displayWorld();
    }
    return 0;
}

void shuffle(char key[]) {
    int i, j;
    char temp;
    for (i = 25; i > 0; i--) {
        j = rand() % (i + 1);
        temp = key[i];
        key[i] = key[j];
        key[j] = temp;
    }
}

void initialization() {
    char* puzzleString = "Cryptogram Puzzle";
    strcpy(puzzle, puzzleString);

    // generate encryption key 
    srand(time(NULL));
    shuffle(encryptionKey);

    // build encrypted string
    int len = strlen(puzzleString);
    for (int i = 0; i < len; i++) {
        if (isalpha(puzzleString[i])) {
            char encryptedChar = encryptionKey[toupper(puzzleString[i]) - 'A'];
            encryptedString[i] = encryptedChar;
        } else {
            encryptedString[i] = puzzleString[i];
        }
    }
    encryptedString[len] = '\0';
}

bool updateState(char input[]) {
    if (strcmp(input, "quit") == 0) {
        printf("Quitting...\n");
        return true;
    }
    if (strlen(input) == 2 && isalpha(input[0]) && isalpha(input[1])) {
        int index = toupper(input[0]) - 'A';
        playerKey[index] = input[1];
    } else {
        printf("Invalid input, enter a pair of characters or 'quit' to quit.\n");
    }
    return false;
}

void displayWorld() {
    printf("Encrypted: %s\n", encryptedString);
    printf("Decrypted: ");
    for (int i = 0; i < strlen(encryptedString); i++) {
        if (isalpha(encryptedString[i])) {
            char keyChar = playerKey[toupper(encryptedString[i]) - 'A'];
            if (keyChar == '\0') {
                printf("_");
            } else {
                printf("%c", keyChar);
            }
        } else {
            printf("%c", encryptedString[i]);
        }
    }
    printf("\n");
}
