#include <stdio.h>
#include <string.h>

#define MAX_EMAILS 100
#define MAX_LINE 512

typedef struct {
    char sender[30];
    char subject[256];
    char date[11];
    int senderPriority;
    int datePriority;
} Email;

typedef struct {
    Email list[MAX_EMAILS];
    int size;
} MaxHeap;

int senderPriority(char *sender) {
    if (strcmp(sender, "Boss") == 0) return 5;
    if (strcmp(sender, "Subordinate") == 0) return 4;
    if (strcmp(sender, "Peer") == 0) return 3;
    if (strcmp(sender, "ImportantPerson") == 0) return 2;
    return 1;
}

int datePriority(char *date) {
    int month, day, year;
    sscanf(date, "%d-%d-%d", &month, &day, &year);
    return year * 10000 + month * 100 + day;
}

int higherPriority(Email a, Email b) {
    if (a.senderPriority != b.senderPriority) {
        return a.senderPriority > b.senderPriority;
    }
    return a.datePriority > b.datePriority;
}

void swap(Email *a, Email *b) {
    Email temp = *a;
    *a = *b;
    *b = temp;
}

void insert(MaxHeap *heap, Email email) {
    if (heap->size >= MAX_EMAILS) return;

    int index = heap->size;
    heap->list[index] = email;
    heap->size++;

    while (index > 0) {
        int parent = (index - 1) / 2;
        if (!higherPriority(heap->list[index], heap->list[parent])) break;
        swap(&heap->list[index], &heap->list[parent]);
        index = parent;
    }
}

void removeMax(MaxHeap *heap) {
    if (heap->size == 0) return;

    heap->size--;
    heap->list[0] = heap->list[heap->size];
    int index = 0;

    while (1) {
        int left = index * 2 + 1;
        int right = index * 2 + 2;
        int largest = index;

        if (left < heap->size && higherPriority(heap->list[left], heap->list[largest])) {
            largest = left;
        }
        if (right < heap->size && higherPriority(heap->list[right], heap->list[largest])) {
            largest = right;
        }
        if (largest == index) break;

        swap(&heap->list[index], &heap->list[largest]);
        index = largest;
    }
}

void addEmail(MaxHeap *heap, char *line) {
    Email email;
    char *sender = line + 6;
    char *comma1 = strchr(sender, ',');
    *comma1 = '\0';
    char *subject = comma1 + 1;
    char *comma2 = strchr(subject, ',');
    *comma2 = '\0';
    char *date = comma2 + 1;
    date[strcspn(date, "\r\n")] = '\0';

    strcpy(email.sender, sender);
    strcpy(email.subject, subject);
    strcpy(email.date, date);
    email.senderPriority = senderPriority(email.sender);
    email.datePriority = datePriority(email.date);
    insert(heap, email);
}

int main(int argc, char *argv[]) {
    FILE *input = stdin;
    if (argc == 2) {
        input = fopen(argv[1], "r");
        if (input == NULL) {
            printf("Could not open file.\n");
            return 1;
        }
    }

    MaxHeap heap;
    heap.size = 0;
    char line[MAX_LINE];

    while (fgets(line, sizeof(line), input) != NULL) {
        if (strncmp(line, "EMAIL ", 6) == 0) {
            addEmail(&heap, line);
        } else {
            line[strcspn(line, "\r\n")] = '\0';

            if (strcmp(line, "NEXT") == 0) {
                if (heap.size == 0) {
                    printf("No emails to read.\n\n");
                } else {
                    printf("Next email:\n");
                    printf("      Sender: %s\n", heap.list[0].sender);
                    printf("      Subject: %s\n", heap.list[0].subject);
                    printf("      Date: %s\n\n", heap.list[0].date);
                }
            } else if (strcmp(line, "READ") == 0) {
                removeMax(&heap);
            } else if (strcmp(line, "COUNT") == 0) {
                printf("There are %d emails to read.\n\n", heap.size);
            }
        }
    }

    if (input != stdin) fclose(input);
    return 0;
}
