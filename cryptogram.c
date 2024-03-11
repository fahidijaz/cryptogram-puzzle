#include <stdio.h>
#include <stdbool.h>
#include <string.h>

void initialization();
void gameLoop();
void tearDown();
char* getPuzzle();
void displayWorld();
bool updateState(char* input);
char* acceptInput();

int main() {
    initialization(); 
    gameLoop(); 
    tearDown(); 
    return 0;
}

char puzzle[100]; // store the puzzle

char* getPuzzle() {
    char* puzzleString = "Cryptogram Puzzle"; // define the puzzle string
    return puzzleString;
}

void initialization() {
    char* puzzleString = getPuzzle(); 
    strcpy(puzzle, getPuzzle()); // start the puzzle
}

void displayWorld() {
    printf("Puzzle: %s\n", puzzle);
}

bool updateState(char* input) {
    if (input == NULL) {
        return true; // see if input is null and should quit
    }
 
    input[strcspn(input, "\n")] = '\0';
    // s if the input is "quit"
    if (strcmp(input, "quit") == 0) {
        printf("Quitting the game.\n");
        return true; // quit the game
    }

    // makes sure input is of exactly two characters
    if (strlen(input) != 2) {
        printf("Invalid input. Please enter a letter then enter the letter it should be changed to.\n");
        return false; // continue the game
    }

    // update the puzzle based on the input
    char oldChar = input[0], newChar = input[1];
    // Replace oldChar with newChar in the puzzle
    for (int i = 0; i < strlen(puzzle); i++) {
        if (puzzle[i] == oldChar) {
            puzzle[i] = newChar;
        }
    }

    return false; // continue the game
}

char* acceptInput() {
    static char input[100]; // allocate space
    printf("Enter a letter then enter the letter it should be changed to or type 'quit' to quit.\n");
    fgets(input, sizeof(input), stdin); // read input
    strtok(input, "\n");
    fflush(stdin);
    return input;
}

void gameLoop() {
    char* input;
    while (true) {
        input = acceptInput(); // gets the users input
        if (updateState(input)) // checks to see if the game will end
            break; // breaks out of the loop 
        displayWorld();
    }
}

void tearDown() {
    printf("All done\n"); // the end of the game
}
