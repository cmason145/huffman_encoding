#include "pq.h"

#include "node.h"

#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

void fix_heap(PriorityQueue *, uint32_t, uint32_t);
void swap(Node **, Node **);
uint32_t min_child(Node **, uint32_t, uint32_t);
void build_heap(PriorityQueue *, uint32_t, uint32_t);

struct PriorityQueue {
    uint32_t capacity;
    uint32_t nextFree;
    uint32_t front;
    Node **items;
};

PriorityQueue *pq_create(uint32_t capacity) {
    PriorityQueue *q = (PriorityQueue *) malloc(sizeof(PriorityQueue));
    if (q) {
        q->capacity = capacity;
        q->nextFree = q->front = 0;
        q->items = (Node **) calloc(capacity, sizeof(Node *));
        if (!q->items) {
            free(q);
            q = NULL;
        }
    }
    return q;
}

void pq_delete(PriorityQueue **q) {
    if (*q && (*q)->items) {
        free((*q)->items);
        free(*q);
        *q = NULL;
    }
    return;
}

bool pq_empty(PriorityQueue *q) {
    if (q->nextFree == 0) {
        return true;
    }
    return false;
}

bool pq_full(PriorityQueue *q) {
    if (q->nextFree == q->capacity) {
        return true;
    }
    return false;
}

uint32_t pq_size(PriorityQueue *q) {
    return q->nextFree - q->front;
}

bool enqueue(PriorityQueue *q, Node *n) {
    if (pq_full(q)) {
        // As a rule of thumb, we should double the size of the array here.
        // Use realloc to double the size
        return false;
    }
    if (pq_empty(q)) {
        // Queue is empty
        q->items[0] = n;
        q->nextFree += 1;
    } else {
        // A node is already in the items array
        q->items[q->nextFree] = n;
        q->nextFree += 1;
    }
    build_heap(q, q->front + 1, q->nextFree);
    return true;
}

bool dequeue(PriorityQueue *q, Node **n) {
    if (pq_empty(q)) {
        // We can't dequeue from an empty queue
        return false;
    }
    *n = q->items[0];
    // Swap root and last item at last index
    swap(&(q->items[0]), &(q->items[(q->nextFree) - 1]));
    q->nextFree -= 1;
    fix_heap(q, q->front + 1, q->nextFree);
    return true;
}

void pq_print(PriorityQueue *q) {
    printf("Sequence of frequences is: ");
    printf("[ ");
    for (uint32_t k = q->front; k < q->nextFree; k += 1) {
        printf("%" PRIu64, q->items[k]->frequency);
        printf(", ");
    }
    printf("]\n");
}

void swap(Node **x, Node **y) {
    Node *temp = *x;
    *x = *y;
    *y = temp;
    return;
}

uint32_t min_child(Node **n, uint32_t first, uint32_t last) {
    uint32_t left = 2 * first;
    uint32_t right = left + 1;
    if ((right <= last) && ((n[right - 1]->frequency) < (n[left - 1]->frequency))) {
        return right;
    }
    return left;
}

void fix_heap(PriorityQueue *q, uint32_t first, uint32_t last) {
    bool found = false;
    uint32_t mother = first;
    uint32_t great = min_child(q->items, mother, last);
    while ((mother <= (uint32_t)(floor(last / 2))) && (found != true)) {
        if ((q->items[mother - 1]->frequency) > (q->items[great - 1]->frequency)) {
            swap(&(q->items[mother - 1]), &(q->items[great - 1]));
            mother = great;
            great = min_child(q->items, mother, last);
        } else {
            found = true;
        }
    }
    return;
}

void build_heap(PriorityQueue *q, uint32_t first, uint32_t last) {
    for (uint32_t father = floor(last / 2); father > first - 1; father -= 1) {
        fix_heap(q, father, last);
    }
}
