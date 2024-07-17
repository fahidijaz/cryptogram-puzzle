#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

// Hold quote text
struct Quote
{
    char *phrase;
    char *author;
    struct Quote *next;
};
typedef struct Quote Quote;

// Global variables
char playerKey[26] = {'\0'};
Quote *currentPuzzle = NULL;
Quote *quoteList = NULL;
int numQuotes = 0;

// Function prototypes
void shuffle(char key[]);
void loadPuzzles();
Quote *getPuzzle();
bool updateState(char input[]);
bool isGameOver();
void initialization();
void teardown();

// reset the game state
void resetGame()
{
    Quote *current = quoteList;
    while (current != NULL)
    {
        // Reset the state of each quote
        // This will depend on the structure of your Quote type
        // For example, if each quote has a 'solved' field, you might do:
        // current->solved = false;

        current = current->next;
    }
    numQuotes = 0;

    // Reset playerKey
    for (int i = 0; i < 26; i++)
    {
        playerKey[i] = '\0';
    }

    // Get a new puzzle
    currentPuzzle = getPuzzle();
}

// shuffle the decryption key
void shuffle(char key[])
{
    int i, j;
    char temp;
    for (i = 25; i > 0; i--)
    {
        j = rand() % (i + 1);
        temp = key[i];
        key[i] = key[j];
        key[j] = temp;
    }
}

// initialize the game
void initialization()
{
    // Reset playerKey
    for (int i = 0; i < 26; i++)
    {
        playerKey[i] = '\0';
    }

    // Load puzzles if not already loaded
    if (quoteList == NULL)
    {
        loadPuzzles();
    }

    // Get a new puzzle
    currentPuzzle = getPuzzle();
}

// free allocated memory
void teardown()
{
    Quote *current = quoteList;
    while (current != NULL)
    {
        Quote *next = current->next;
        free(current->phrase);
        free(current->author);
        free(current);
        current = next;
    }
    quoteList = NULL;
    currentPuzzle = NULL; // Add this line
    numQuotes = 0;
}

// load puzzle from file
void loadPuzzles()
{
    FILE *file = fopen("quotes.txt", "r");
    if (file == NULL)
    {
        printf("Error: Unable to open file.\n");
        exit(1);
    }

    char line[1000];
    Quote *lastQuote = NULL;
    bool newQuote = true;
    while (fgets(line, sizeof(line), file) != NULL)
    {
        if (strlen(line) < 3)
        {
            // If blank line then quote ends
            newQuote = true;
        }
        else if (line[0] == '-')
        {
            // Author line
            if (lastQuote != NULL)
            {
                lastQuote->author = strdup(line + 2);                         // Skip "- "
                lastQuote->author[strcspn(lastQuote->author, "\r\n")] = '\0'; // Remove newline character
            }
        }
        else
        {
            // Phrase line
            if (newQuote)
            {
                if (lastQuote != NULL)
                {
                    lastQuote->next = (Quote *)malloc(sizeof(Quote));
                    lastQuote = lastQuote->next;
                }
                else
                {
                    quoteList = (Quote *)malloc(sizeof(Quote));
                    lastQuote = quoteList;
                }
                lastQuote->next = NULL;
                numQuotes++;
                newQuote = false;
            }
            if (lastQuote != NULL)
            {
                if (lastQuote->phrase == NULL)
                {
                    lastQuote->phrase = strdup(line);
                }
                else
                {
                    char *temp = realloc(lastQuote->phrase, strlen(lastQuote->phrase) + strlen(line) + 1);
                    if (temp == NULL)
                    {
                        printf("Error: Memory allocation failed.\n");
                        exit(1);
                    }
                    //printf("Old size: %zu, New size: %zu\n", strlen(lastQuote->phrase), strlen(lastQuote->phrase) + strlen(line) + 1);
                    lastQuote->phrase = temp;
                    strcpy(lastQuote->phrase + strlen(lastQuote->phrase), line); // Append line directly
                }
            }
        }
    }

    fclose(file);
}

// get randomly seeded puzzle
Quote *getPuzzle()
{
    if (numQuotes == 0)
    {
        loadPuzzles();
    }
    int randomIndex = rand() % numQuotes;

    Quote *currentQuote = quoteList;
    for (int i = 0; i < randomIndex; i++)
    {
        if (currentQuote == NULL || currentQuote->next == NULL)
        {
            printf("Error: currentQuote is NULL or currentQuote->next is NULL\n");
            return NULL;
        }
        currentQuote = currentQuote->next;
    }
    return currentQuote;
}

// update the decryption key
bool updateState(char input[])
{
    if (strcmp(input, "quit") == 0)
    {
        printf("Quitting...\n");
        return true;
    }
    if (strlen(input) == 2 && isalpha(input[0]) && isalpha(input[1]))
    {
        int index = toupper(input[0]) - 'A';
        playerKey[index] = input[1];
    }
    else
    {
        printf("Invalid input, enter a pair of characters or 'quit' to quit.\n");
    }
    return false;
}

// check if the game is over
bool isGameOver()
{
    for (int i = 0; i < strlen(currentPuzzle->phrase); i++)
    {
        if (isalpha(currentPuzzle->phrase[i]))
        {
            char keyChar = playerKey[toupper(currentPuzzle->phrase[i]) - 'A'];
            if (keyChar == '\0')
            {
                return false;
            }
        }
    }
    return true;
}
