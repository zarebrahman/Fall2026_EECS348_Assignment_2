/*
 * Program: EECS 348 Assignment 2 - CEO Email Priority Queue
 * Description: Reads email commands and stores unread emails in a MaxHeap.
 *              Sender category is the main priority, and newer dates break ties.
 * Inputs: An optional command-line test-file name. If no file is given, input
 *         is read from standard input. Commands are EMAIL, NEXT, READ, and COUNT.
 * Outputs: The unread-email count and the next email selected for the CEO.
 * Collaborators: None.
 * Sources: OpenAI ChatGPT was used to create the starting design and comments.
 *          Google Gemini was used only to make the required comparison program;
 *          its code was not copied into this final program. The final organization
 *          follows the provided Assignment 2 instructions.
 * Author: Zareb Rahman
 * Creation date: September 16, 2026
 * Revision date: September 16, 2026
 * Revisions: Added dynamic heap growth, stable tie handling, input checks,
 *            empty-queue handling, and support for a file or standard input.
 */

#include <stdio.h>   // Provides file input, terminal output, and sscanf.
#include <stdlib.h>  // Provides malloc, realloc, and free.
#include <string.h>  // Provides string comparison, searching, and copying.

#define START_CAPACITY 10  // Sets the starting number of heap positions.
#define SENDER_SIZE 32     // Sets the maximum stored sender-category length.
#define SUBJECT_SIZE 256   // Sets the maximum stored subject-line length.
#define DATE_SIZE 11       // Stores MM-DD-YYYY plus the null character.
#define LINE_SIZE 512      // Sets the maximum command-line length.

/* Source for this block: ChatGPT starting code, revised for the assignment. */
typedef struct {                         // Starts the structure for one email.
    char sender[SENDER_SIZE];            // Stores the sender category.
    char subject[SUBJECT_SIZE];          // Stores the subject line.
    char date[DATE_SIZE];                // Stores the displayed date.
    int senderPriority;                  // Stores the numeric sender priority.
    int dateValue;                       // Stores the date as YYYYMMDD for comparison.
    long arrivalOrder;                   // Stores insertion order for exact ties.
} Email;                                 // Ends the Email structure.

/* Source for this block: ChatGPT starting code, revised for dynamic storage. */
typedef struct {                         // Starts the structure for the MaxHeap.
    Email *items;                        // Points to the list that stores emails.
    int size;                            // Stores the current number of emails.
    int capacity;                        // Stores the current list capacity.
    long nextArrivalOrder;               // Stores the next insertion-order value.
} MaxHeap;                               // Ends the MaxHeap structure.

/* Source for this function: ChatGPT, revised to safely remove CR and LF. */
void trimNewline(char *text) {                           // Removes the line ending from text.
    text[strcspn(text, "\r\n")] = '\0';                 // Replaces the first CR or LF with null.
}                                                       // Ends trimNewline.

/* Source for this function: ChatGPT, revised to guarantee null termination. */
void copyText(char *destination, const char *source, size_t size) { // Copies text safely.
    strncpy(destination, source, size - 1);              // Copies at most size minus one characters.
    destination[size - 1] = '\0';                        // Always adds the final null character.
}                                                       // Ends copyText.

/* Source for this function: ChatGPT based on the assignment's priority order. */
int getSenderPriority(const char *sender) {              // Converts a sender category to a number.
    if (strcmp(sender, "Boss") == 0) {                   // Checks for the highest category.
        return 5;                                       // Gives Boss the highest priority.
    }                                                   // Ends the Boss check.
    if (strcmp(sender, "Subordinate") == 0) {            // Checks for the second category.
        return 4;                                       // Gives Subordinate the next priority.
    }                                                   // Ends the Subordinate check.
    if (strcmp(sender, "Peer") == 0) {                   // Checks for the third category.
        return 3;                                       // Gives Peer the middle priority.
    }                                                   // Ends the Peer check.
    if (strcmp(sender, "ImportantPerson") == 0) {        // Checks for the fourth category.
        return 2;                                       // Gives ImportantPerson the next priority.
    }                                                   // Ends the ImportantPerson check.
    return 1;                                           // Gives OtherPerson the lowest priority.
}                                                       // Ends getSenderPriority.

/* Source for this function: ChatGPT, revised to validate the parsed date parts. */
int getDateValue(const char *date) {                     // Converts MM-DD-YYYY into YYYYMMDD.
    int month = 0;                                      // Stores the month from the date.
    int day = 0;                                        // Stores the day from the date.
    int year = 0;                                       // Stores the year from the date.

    if (sscanf(date, "%d-%d-%d", &month, &day, &year) != 3) { // Tries to read all three date parts.
        return 0;                                       // Returns zero if the date cannot be parsed.
    }                                                   // Ends the parsing check.

    if (month < 1 || month > 12 || day < 1 || day > 31) { // Checks basic date ranges.
        return 0;                                       // Returns zero for an invalid range.
    }                                                   // Ends the range check.

    return year * 10000 + month * 100 + day;            // Returns a number that sorts by date.
}                                                       // Ends getDateValue.

/* Source for this function: ChatGPT, revised to keep exact ties stable. */
int hasHigherPriority(const Email *first, const Email *second) { // Compares two emails.
    if (first->senderPriority != second->senderPriority) { // Checks sender priority first.
        return first->senderPriority > second->senderPriority; // Picks the higher sender value.
    }                                                   // Ends the sender-priority check.

    if (first->dateValue != second->dateValue) {         // Checks dates when senders tie.
        return first->dateValue > second->dateValue;     // Picks the newer date.
    }                                                   // Ends the date check.

    return first->arrivalOrder < second->arrivalOrder;   // Keeps the earlier exact tie first.
}                                                       // Ends hasHigherPriority.

/* Source for this function: ChatGPT. */
void swapEmails(Email *first, Email *second) {           // Swaps two heap entries.
    Email temporary = *first;                            // Saves the first email temporarily.
    *first = *second;                                    // Moves the second email into the first spot.
    *second = temporary;                                 // Moves the saved email into the second spot.
}                                                       // Ends swapEmails.

/* Source for this function: ChatGPT, revised to report allocation failure. */
int initializeHeap(MaxHeap *heap) {                      // Creates an empty MaxHeap.
    heap->items = malloc(START_CAPACITY * sizeof(Email)); // Allocates the starting list.
    if (heap->items == NULL) {                           // Checks whether allocation failed.
        return 0;                                       // Reports failure to the caller.
    }                                                   // Ends the allocation check.
    heap->size = 0;                                     // Starts the heap with no emails.
    heap->capacity = START_CAPACITY;                     // Saves the starting capacity.
    heap->nextArrivalOrder = 0;                          // Starts insertion numbering at zero.
    return 1;                                           // Reports successful initialization.
}                                                       // Ends initializeHeap.

/* Source for this function: ChatGPT. */
void destroyHeap(MaxHeap *heap) {                        // Releases the heap's memory.
    free(heap->items);                                   // Frees the dynamic email list.
    heap->items = NULL;                                  // Removes the old pointer value.
    heap->size = 0;                                      // Resets the stored size.
    heap->capacity = 0;                                  // Resets the stored capacity.
}                                                       // Ends destroyHeap.

/* Source for this function: ChatGPT, revised to double capacity as needed. */
int growHeap(MaxHeap *heap) {                            // Makes the heap list larger.
    int newCapacity = heap->capacity * 2;                // Doubles the current capacity.
    Email *newItems = realloc(heap->items, newCapacity * sizeof(Email)); // Resizes the list.
    if (newItems == NULL) {                              // Checks whether resizing failed.
        return 0;                                       // Reports failure to the caller.
    }                                                   // Ends the resizing check.
    heap->items = newItems;                              // Saves the resized list pointer.
    heap->capacity = newCapacity;                        // Saves the larger capacity.
    return 1;                                           // Reports successful growth.
}                                                       // Ends growHeap.

/* Source for this function: ChatGPT using the standard array MaxHeap algorithm. */
int insertEmail(MaxHeap *heap, Email email) {            // Inserts an email into the MaxHeap.
    if (heap->size == heap->capacity && !growHeap(heap)) { // Grows the list when it is full.
        return 0;                                       // Reports failure if growth failed.
    }                                                   // Ends the capacity check.

    email.arrivalOrder = heap->nextArrivalOrder;         // Gives the email its tie-breaking order.
    heap->nextArrivalOrder++;                            // Prepares the next order number.

    int index = heap->size;                              // Starts at the next open position.
    heap->items[index] = email;                          // Places the email at the end of the list.
    heap->size++;                                        // Increases the unread-email count.

    while (index > 0) {                                  // Moves the email upward when needed.
        int parent = (index - 1) / 2;                    // Finds the parent position.
        if (!hasHigherPriority(&heap->items[index], &heap->items[parent])) { // Checks heap order.
            break;                                      // Stops when the parent already belongs first.
        }                                               // Ends the heap-order check.
        swapEmails(&heap->items[index], &heap->items[parent]); // Swaps child and parent.
        index = parent;                                  // Continues from the parent's old position.
    }                                                   // Ends the upward movement loop.

    return 1;                                           // Reports a successful insertion.
}                                                       // Ends insertEmail.

/* Source for this function: ChatGPT. */
const Email *peekEmail(const MaxHeap *heap) {             // Finds the next email without removing it.
    if (heap->size == 0) {                               // Checks whether the heap is empty.
        return NULL;                                    // Reports that no email exists.
    }                                                   // Ends the empty check.
    return &heap->items[0];                              // Returns the highest-priority email.
}                                                       // Ends peekEmail.

/* Source for this function: ChatGPT using the standard array MaxHeap algorithm. */
int removeEmail(MaxHeap *heap) {                         // Removes the highest-priority email.
    if (heap->size == 0) {                               // Checks whether the heap is empty.
        return 0;                                       // Reports that nothing was removed.
    }                                                   // Ends the empty check.

    heap->size--;                                        // Reduces the unread-email count.
    if (heap->size == 0) {                               // Checks whether that was the final email.
        return 1;                                       // Finishes without moving another item.
    }                                                   // Ends the final-email check.

    heap->items[0] = heap->items[heap->size];            // Moves the last email to the root.
    int index = 0;                                      // Starts downward movement at the root.

    while (1) {                                         // Repeats until heap order is restored.
        int left = index * 2 + 1;                        // Finds the left-child position.
        int right = index * 2 + 2;                       // Finds the right-child position.
        int largest = index;                             // Assumes the current email belongs first.

        if (left < heap->size &&                         // Confirms that the left child exists.
            hasHigherPriority(&heap->items[left], &heap->items[largest])) { // Compares it to the current best.
            largest = left;                              // Selects the left child as the better email.
        }                                               // Ends the left-child check.

        if (right < heap->size &&                        // Confirms that the right child exists.
            hasHigherPriority(&heap->items[right], &heap->items[largest])) { // Compares it to the current best.
            largest = right;                             // Selects the right child as the better email.
        }                                               // Ends the right-child check.

        if (largest == index) {                          // Checks whether heap order is correct.
            break;                                      // Stops when no child should move upward.
        }                                               // Ends the finished check.

        swapEmails(&heap->items[index], &heap->items[largest]); // Moves the better child upward.
        index = largest;                                 // Continues from the child's old position.
    }                                                   // Ends the downward movement loop.

    return 1;                                           // Reports a successful removal.
}                                                       // Ends removeEmail.

/* Source for this function: ChatGPT, revised to validate delimiters and dates. */
int parseEmailCommand(char *line, Email *email) {        // Converts an EMAIL command into a structure.
    char *sender = line + 6;                             // Skips the word EMAIL and its space.
    char *firstComma = strchr(sender, ',');              // Finds the end of the sender category.
    if (firstComma == NULL) {                            // Checks for a missing first comma.
        return 0;                                       // Reports an invalid EMAIL command.
    }                                                   // Ends the first-comma check.
    *firstComma = '\0';                                  // Ends the sender string at the comma.

    char *subject = firstComma + 1;                      // Starts the subject after the first comma.
    char *secondComma = strchr(subject, ',');            // Finds the end of the subject line.
    if (secondComma == NULL) {                           // Checks for a missing second comma.
        return 0;                                       // Reports an invalid EMAIL command.
    }                                                   // Ends the second-comma check.
    *secondComma = '\0';                                 // Ends the subject string at the comma.

    char *date = secondComma + 1;                        // Starts the date after the second comma.
    trimNewline(date);                                   // Removes the date's line ending.

    copyText(email->sender, sender, SENDER_SIZE);         // Copies the sender into the structure.
    copyText(email->subject, subject, SUBJECT_SIZE);      // Copies the subject into the structure.
    copyText(email->date, date, DATE_SIZE);               // Copies the date into the structure.
    email->senderPriority = getSenderPriority(email->sender); // Calculates sender priority.
    email->dateValue = getDateValue(email->date);         // Calculates the sortable date value.
    email->arrivalOrder = 0;                             // Creates a temporary order before insertion.

    return email->dateValue != 0;                        // Accepts the email only if its date parsed.
}                                                       // Ends parseEmailCommand.

/* Source for this function: ChatGPT, formatted to match the sample output. */
void printNextEmail(const MaxHeap *heap) {                // Displays the highest-priority email.
    const Email *email = peekEmail(heap);                 // Looks at the root without removing it.
    if (email == NULL) {                                  // Checks whether the queue is empty.
        printf("No emails to read.\n\n");                 // Displays a safe empty-queue message.
        return;                                          // Finishes the function early.
    }                                                    // Ends the empty-queue check.

    printf("Next email:\n");                             // Prints the section heading.
    printf("      Sender: %s\n", email->sender);          // Prints the sender category.
    printf("      Subject: %s\n", email->subject);        // Prints the subject line.
    printf("      Date: %s\n\n", email->date);            // Prints the date and a blank line.
}                                                        // Ends printNextEmail.

/* Source for this function: ChatGPT, revised for all required command cases. */
int processCommands(FILE *input, MaxHeap *heap) {         // Processes every line in the test file.
    char line[LINE_SIZE];                                 // Stores one command at a time.

    while (fgets(line, sizeof(line), input) != NULL) {    // Reads commands until the file ends.
        if (strncmp(line, "EMAIL ", 6) == 0) {            // Checks for an EMAIL command.
            Email email;                                 // Creates storage for the new email.
            if (!parseEmailCommand(line, &email)) {       // Tries to parse the command.
                fprintf(stderr, "Invalid EMAIL command.\n"); // Reports an unexpected format.
                continue;                                // Skips the invalid command safely.
            }                                            // Ends the parsing check.
            if (!insertEmail(heap, email)) {              // Tries to add the email to the MaxHeap.
                fprintf(stderr, "Unable to store another email.\n"); // Reports memory failure.
                return 0;                                // Stops processing after memory failure.
            }                                            // Ends the insertion check.
        } else {                                         // Handles commands without email fields.
            trimNewline(line);                            // Removes CR or LF from the command.

            if (strcmp(line, "NEXT") == 0) {              // Checks for a NEXT command.
                printNextEmail(heap);                     // Displays but does not remove the root.
            } else if (strcmp(line, "READ") == 0) {       // Checks for a READ command.
                removeEmail(heap);                        // Removes the root without displaying it.
            } else if (strcmp(line, "COUNT") == 0) {      // Checks for a COUNT command.
                printf("There are %d emails to read.\n\n", heap->size); // Prints the current count.
            }                                            // Ends the command checks.
        }                                                // Ends the EMAIL or other-command branch.
    }                                                    // Ends the file-reading loop.

    return 1;                                            // Reports successful command processing.
}                                                        // Ends processCommands.

/* Source for this function: ChatGPT, revised for file or redirected input. */
int main(int argc, char *argv[]) {                        // Starts the program.
    if (argc > 2) {                                      // Checks for too many command-line values.
        fprintf(stderr, "Usage: %s [test_file]\n", argv[0]); // Explains the correct command.
        return 1;                                        // Ends with an error status.
    }                                                    // Ends the argument-count check.

    FILE *input = stdin;                                 // Uses standard input by default.
    if (argc == 2) {                                     // Checks whether a file name was supplied.
        input = fopen(argv[1], "r");                     // Opens the requested test file.
        if (input == NULL) {                              // Checks whether the file opened.
            fprintf(stderr, "Could not open %s.\n", argv[1]); // Reports the file error.
            return 1;                                    // Ends with an error status.
        }                                                // Ends the file-open check.
    }                                                    // Ends the optional-file block.

    MaxHeap heap;                                        // Creates the program's MaxHeap variable.
    if (!initializeHeap(&heap)) {                        // Allocates the initial heap list.
        fprintf(stderr, "Could not create the email queue.\n"); // Reports allocation failure.
        if (input != stdin) {                            // Checks whether a file was opened.
            fclose(input);                               // Closes the opened file.
        }                                                // Ends the file check.
        return 1;                                        // Ends with an error status.
    }                                                    // Ends the heap setup check.

    int success = processCommands(input, &heap);          // Runs all commands from the input.

    destroyHeap(&heap);                                  // Releases all heap memory.
    if (input != stdin) {                                // Checks whether an input file was opened.
        fclose(input);                                   // Closes the test file.
    }                                                    // Ends the file check.

    return success ? 0 : 1;                              // Returns success or failure to the system.
}                                                        // Ends main.
