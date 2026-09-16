/*
 * EECS 348 Assignment 2: Email Priority Queue
 * Language: C (std=c11)
 *
 * Description:
 * Implements a MaxHeap from scratch to manage and prioritize emails for a CEO.
 * Emails are prioritized first by sender category (Boss > Subordinate > Peer > 
 * ImportantPerson > OtherPerson) and second by date (newer dates take precedence).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 512
#define INITIAL_CAPACITY 16

/* Enumeration for sender categories with assigned priority values */
typedef enum {
    OTHER_PERSON = 0,
    IMPORTANT_PERSON = 1,
    PEER = 2,
    SUBORDINATE = 3,
    BOSS = 4,
    UNKNOWN = -1
} CategoryPriority;

/* Structure representing an Email record */
typedef struct {
    char sender[64];
    char subject[256];
    char date_str[11]; /* MM-DD-YYYY format */
    CategoryPriority priority;
    int year;
    int month;
    int day;
} Email;

/* Structure representing the Dynamic MaxHeap */
typedef struct {
    Email *data;
    int size;
    int capacity;
} MaxHeap;

/* Function Prototypes */
CategoryPriority parse_category(const char *cat_str);
void parse_date(const char *date_str, int *month, int *day, int *year);
int compare_emails(const Email *a, const Email *b);

MaxHeap* create_heap(void);
void free_heap(MaxHeap *heap);
void heap_push(MaxHeap *heap, Email email);
Email heap_peek(const MaxHeap *heap);
Email heap_pop(MaxHeap *heap);
void heapify_up(MaxHeap *heap, int index);
void heapify_down(MaxHeap *heap, int index);
void swap_emails(Email *a, Email *b);

void process_stream(FILE *fp);

/* 
 * Maps string category names to CategoryPriority enum values.
 */
CategoryPriority parse_category(const char *cat_str) {
    if (strcmp(cat_str, "Boss") == 0) return BOSS;
    if (strcmp(cat_str, "Subordinate") == 0) return SUBORDINATE;
    if (strcmp(cat_str, "Peer") == 0) return PEER;
    if (strcmp(cat_str, "ImportantPerson") == 0) return IMPORTANT_PERSON;
    if (strcmp(cat_str, "OtherPerson") == 0) return OTHER_PERSON;
    return UNKNOWN;
}

/* 
 * Parses a date string formatted as "MM-DD-YYYY" into numeric components.
 */
void parse_date(const char *date_str, int *month, int *day, int *year) {
    sscanf(date_str, "%d-%d-%d", month, day, year);
}

/* 
 * Compares two emails to determine relative priority.
 * Returns > 0 if email 'a' has higher priority than email 'b'.
 * Returns < 0 if email 'b' has higher priority than email 'a'.
 * Returns 0 if both are identical in priority.
 */
int compare_emails(const Email *a, const Email *b) {
    if (a->priority != b->priority) {
        return (int)a->priority - (int)b->priority;
    }
    
    /* Secondary priority: Newer dates are higher priority */
    if (a->year != b->year) {
        return a->year - b->year;
    }
    if (a->month != b->month) {
        return a->month - b->month;
    }
    return a->day - b->day;
}

/* 
 * Initializes a new MaxHeap instance.
 */
MaxHeap* create_heap(void) {
    MaxHeap *heap = (MaxHeap*)malloc(sizeof(MaxHeap));
    if (!heap) {
        perror("Failed to allocate heap memory");
        exit(EXIT_FAILURE);
    }
    heap->capacity = INITIAL_CAPACITY;
    heap->size = 0;
    heap->data = (Email*)malloc(heap->capacity * sizeof(Email));
    if (!heap->data) {
        perror("Failed to allocate heap data memory");
        free(heap);
        exit(EXIT_FAILURE);
    }
    return heap;
}

/* 
 * Frees allocated memory for the heap.
 */
void free_heap(MaxHeap *heap) {
    if (heap) {
        free(heap->data);
        free(heap);
    }
}

/* 
 * Swaps two Email structures.
 */
void swap_emails(Email *a, Email *b) {
    Email temp = *a;
    *a = *b;
    *b = temp;
}

/* 
 * Restores max-heap property upwards from a target index.
 */
void heapify_up(MaxHeap *heap, int index) {
    while (index > 0) {
        int parent = (index - 1) / 2;
        if (compare_emails(&heap->data[index], &heap->data[parent]) > 0) {
            swap_emails(&heap->data[index], &heap->data[parent]);
            index = parent;
        } else {
            break;
        }
    }
}

/* 
 * Restores max-heap property downwards from a target index.
 */
void heapify_down(MaxHeap *heap, int index) {
    int largest = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    if (left < heap->size && compare_emails(&heap->data[left], &heap->data[largest]) > 0) {
        largest = left;
    }
    if (right < heap->size && compare_emails(&heap->data[right], &heap->data[largest]) > 0) {
        largest = right;
    }
    if (largest != index) {
        swap_emails(&heap->data[index], &heap->data[largest]);
        heapify_down(heap, largest);
    }
}

/* 
 * Inserts a new email into the MaxHeap, growing dynamic array if required.
 */
void heap_push(MaxHeap *heap, Email email) {
    if (heap->size >= heap->capacity) {
        heap->capacity *= 2;
        heap->data = (Email*)realloc(heap->data, heap->capacity * sizeof(Email));
        if (!heap->data) {
            perror("Failed to reallocate heap memory");
            exit(EXIT_FAILURE);
        }
    }
    heap->data[heap->size] = email;
    heap->size++;
    heapify_up(heap, heap->size - 1);
}

/* 
 * Returns the highest priority email without removing it.
 */
Email heap_peek(const MaxHeap *heap) {
    return heap->data[0];
}

/* 
 * Removes and returns the highest priority email from the MaxHeap.
 */
Email heap_pop(MaxHeap *heap) {
    Email top = heap->data[0];
    heap->data[0] = heap->data[heap->size - 1];
    heap->size--;
    if (heap->size > 0) {
        heapify_down(heap, 0);
    }
    return top;
}

/* 
 * Parses standard input or file commands and manages heap operations.
 */
void process_stream(FILE *fp) {
    MaxHeap *heap = create_heap();
    char line[MAX_LINE_LENGTH];

    while (fgets(line, sizeof(line), fp)) {
        /* Remove trailing newline characters */
        line[strcspn(line, "\r\n")] = '\0';

        if (strncmp(line, "EMAIL ", 6) == 0) {
            char *args = line + 6;
            char cat_buf[64] = {0};
            char subj_buf[256] = {0};
            char date_buf[16] = {0};

            /* Extract comma-delimited fields */
            char *first_comma = strchr(args, ',');
            char *second_comma = NULL;
            if (first_comma) {
                second_comma = strchr(first_comma + 1, ',');
            }

            if (first_comma && second_comma) {
                size_t cat_len = first_comma - args;
                size_t subj_len = second_comma - (first_comma + 1);

                if (cat_len < sizeof(cat_buf) && subj_len < sizeof(subj_buf)) {
                    strncpy(cat_buf, args, cat_len);
                    cat_buf[cat_len] = '\0';

                    strncpy(subj_buf, first_comma + 1, subj_len);
                    subj_buf[subj_len] = '\0';

                    strncpy(date_buf, second_comma + 1, sizeof(date_buf) - 1);
                    date_buf[sizeof(date_buf) - 1] = '\0';

                    Email new_email;
                    strncpy(new_email.sender, cat_buf, sizeof(new_email.sender) - 1);
                    new_email.sender[sizeof(new_email.sender) - 1] = '\0';

                    strncpy(new_email.subject, subj_buf, sizeof(new_email.subject) - 1);
                    new_email.subject[sizeof(new_email.subject) - 1] = '\0';

                    strncpy(new_email.date_str, date_buf, sizeof(new_email.date_str) - 1);
                    new_email.date_str[sizeof(new_email.date_str) - 1] = '\0';

                    new_email.priority = parse_category(cat_buf);
                    parse_date(date_buf, &new_email.month, &new_email.day, &new_email.year);

                    heap_push(heap, new_email);
                }
            }
        } else if (strcmp(line, "NEXT") == 0) {
            if (heap->size > 0) {
                Email next_email = heap_peek(heap);
                printf("Next email:\n");
                printf("      Sender: %s\n", next_email.sender);
                printf("      Subject: %s\n", next_email.subject);
                printf("      Date: %s\n", next_email.date_str);
            }
        } else if (strcmp(line, "READ") == 0) {
            if (heap->size > 0) {
                heap_pop(heap);
            }
        } else if (strcmp(line, "COUNT") == 0) {
            printf("There are %d emails to read.\n", heap->size);
        }
    }

    free_heap(heap);
}

int main(int argc, char *argv[]) {
    FILE *input_fp = stdin;

    if (argc > 1) {
        input_fp = fopen(argv[1], "r");
        if (!input_fp) {
            perror("Error opening input file");
            return EXIT_FAILURE;
        }
    }

    process_stream(input_fp);

    if (input_fp != stdin) {
        fclose(input_fp);
    }

    return EXIT_SUCCESS;
}
