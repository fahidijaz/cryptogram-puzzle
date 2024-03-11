#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

//hold quote text
struct Quote {
    char* phrase;
    char* author;
    struct Quote* next;
};
typedef struct Quote Quote;

// variables
Quote* quoteList = NULL;
int numQuotes = 0;
char playerKey[26] = {'\0'};
Quote* currentPuzzle = NULL;

// Function prototypes
void shuffle(char key[]);
void initialization();
void loadPuzzles();
Quote* getPuzzle();
bool updateState(char input[]);
bool displayWorld();
void teardown();

int main() {
    srand(time(NULL));
    char choice;
    do {
        initialization();
        bool puzzleCompleted = false;
        while (!puzzleCompleted) {
            puzzleCompleted = displayWorld();
            if (!puzzleCompleted) {
                char input[10]; // accommodate longer inputs
                printf("Enter a pair of characters or 'quit' to quit: ");
                fgets(input, sizeof(input), stdin);
                input[strcspn(input, "\n")] = '\0'; // remove newline character
                if (updateState(input)) {
                    teardown();
                    return 0; // exit program 
                }
            }
        }
        printf("You solved the puzzle!\n");
        printf("Would you like to play again? (y/n): ");
        scanf(" %c", &choice); //ask to play again
        getchar(); // consume newline character
        teardown(); // free mem
    } while (choice == 'y' || choice == 'Y');

    return 0;
}

//shuffle the decryption key
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

//initialize game
void initialization() {
    loadPuzzles();

    // Reset playerKey
    for (int i = 0; i < 26; i++) {
        playerKey[i] = '\0';
    }

    // Get a new puzzle
    currentPuzzle = getPuzzle();
}

//free allocated memory
void teardown() {
    Quote* current = quoteList;
    while (current != NULL) {
        Quote* next = current->next;
        free(current->phrase);
        free(current->author);
        free(current);
        current = next;
    }
    quoteList = NULL;
    numQuotes = 0;
}

//load puzzle from file
void loadPuzzles() {
    FILE *file = fopen("quotes.txt", "r");
    if (file == NULL) {
        printf("Error: Unable to open file.\n");
        exit(1);
    }

    char line[1000];
    Quote* lastQuote = NULL;
    while (fgets(line, sizeof(line), file) != NULL) {
        if (strlen(line) < 3) {
            //if blank line then quote ends
            if (lastQuote != NULL) {
                lastQuote->next = (Quote*)malloc(sizeof(Quote));
                lastQuote = lastQuote->next;
            } else {
                quoteList = (Quote*)malloc(sizeof(Quote));
                lastQuote = quoteList;
            }
            lastQuote->next = NULL;
            numQuotes++;
        } else if (line[0] == '-') {
            // Author line
            if (lastQuote != NULL) {
                lastQuote->author = strdup(line + 2); // Skip "- "
                lastQuote->author[strcspn(lastQuote->author, "\r\n")] = '\0'; // Remove newline character
            }
        } else {
            // Phrase line
            if (lastQuote != NULL) {
                if (lastQuote->phrase == NULL) {
                    lastQuote->phrase = strdup(line);
                } else {
                    char* temp = realloc(lastQuote->phrase, strlen(lastQuote->phrase) + strlen(line) + 1);
                    if (temp == NULL) {
                        printf("Error: Memory allocation failed.\n");
                        exit(1);
                    }
                    lastQuote->phrase = temp;
                    strcat(lastQuote->phrase, line);
                }
            }
        }
    }

    fclose(file);
}

//gets randomly seeded puzzle
Quote* getPuzzle() {
    if (numQuotes == 0) {
        loadPuzzles();
    }
    int randomIndex = rand() % numQuotes;

    Quote* currentQuote = quoteList;
    for (int i = 0; i < randomIndex; i++) {
        currentQuote = currentQuote->next;
    }
    return currentQuote;
}

//updates the decryption key
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

//display the 2 puzzles
bool displayWorld() {
    printf("Encrypted: %s\n", currentPuzzle->phrase);
    printf("Decrypted: ");
    bool puzzleCompleted = true;
    for (int i = 0; i < strlen(currentPuzzle->phrase); i++) {
        if (isalpha(currentPuzzle->phrase[i])) {
            char keyChar = playerKey[toupper(currentPuzzle->phrase[i]) - 'A'];
            if (keyChar == '\0') {
                printf("_");
                puzzleCompleted = false;
            } else {
                printf("%c", keyChar);
            }
        } else {
            printf("%c", currentPuzzle->phrase[i]);
        }
    }
    printf("\n");
    return puzzleCompleted;
}
