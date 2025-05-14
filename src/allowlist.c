#include "allowlist.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

// === Reader lock ===

void allowlist_search_lock(allowlist_t *self) {
    pthread_mutex_lock(&self->lock);
    while (self->writer_waiting) {
        pthread_cond_wait(&self->cond, &self->lock);
    }
    self->reader_count++;
    pthread_mutex_unlock(&self->lock);
}

void allowlist_search_unlock(allowlist_t *self) {
    pthread_mutex_lock(&self->lock);
    self->reader_count--;
    if (self->reader_count == 0)
        pthread_cond_signal(&self->cond);
    pthread_mutex_unlock(&self->lock);
}

// === Writer lock ===

void allowlist_writer_lock(allowlist_t *self) {
    pthread_mutex_lock(&self->lock);
    self->writer_waiting = 1;
    while (self->reader_count > 0) {
        pthread_cond_wait(&self->cond, &self->lock);
    }
    // now writer has exclusive access
}

void allowlist_writer_unlock(allowlist_t *self) {
    self->writer_waiting = 0;
    pthread_cond_broadcast(&self->cond);
    pthread_mutex_unlock(&self->lock);
}

// === Hash methods ===

void allowlist_add(allowlist_t *self, const char *name) {
    allowlist_writer_lock(self);

    allowlist_t *entry = malloc(sizeof(allowlist_t));
    if (!entry) {
        fprintf( stderr, "allowlist_add(): malloc failed");
        exit(1);
    }

    strncpy(entry->name, name, sizeof(entry->name) - 1);
    entry->name[sizeof(entry->name) - 1] = '\0';
    entry->id = self->next_id++;
    HASH_ADD_STR(self->entries, name, entry);

    allowlist_writer_unlock(self);
}

allowlist_t *allowlist_find(allowlist_t *self, const char *name) {
    allowlist_search_lock(self);

    allowlist_t *entry = NULL;
    HASH_FIND_STR(self->entries, name, entry);

    allowlist_search_unlock(self);
    return entry;
}

void allowlist_delete(allowlist_t *self, const char *name) {
    allowlist_writer_lock(self);

    allowlist_t *entry = NULL;
    HASH_FIND_STR(self->entries, name, entry);
    if (entry) {
        HASH_DEL(self->entries, entry);
        free(entry);
    }

    allowlist_writer_unlock(self);
}

void allowlist_print(allowlist_t *self) {
    allowlist_search_lock(self);

    allowlist_t *entry, *tmp;
    HASH_ITER(hh, self->entries, entry, tmp) {
        printf("Name: %s, ID: %d\n", entry->name, entry->id);
    }

    allowlist_search_unlock(self);
}

void allowlist_cleanup(allowlist_t *self) {
    allowlist_writer_lock(self);

    allowlist_t *entry, *tmp;
    HASH_ITER(hh, self->entries, entry, tmp) {
        HASH_DEL(self->entries, entry);
        free(entry);
    }

    allowlist_writer_unlock(self);

    pthread_mutex_destroy(&self->lock);
    pthread_cond_destroy(&self->cond);
    free(self);
}

// === Constructor ===

allowlist_t *allowlist_init(void) {
    allowlist_t *map = malloc(sizeof(allowlist_t));
    if (!map) {
        fprintf( stderr, "allowlist_init(): malloc failed");
        exit(1);
    }

    map->next_id = 1;
    map->entries = NULL;

    map->add = allowlist_add;
    map->find = allowlist_find;
    map->del = allowlist_delete;
    map->print = allowlist_print;
    map->cleanup = allowlist_cleanup;

    pthread_mutex_init(&map->lock, NULL);
    pthread_cond_init(&map->cond, NULL);
    map->reader_count = 0;
    map->writer_waiting = 0;

    return map;
}


int allowlist_run_tests() {
    allowlist_t *root = allowlist_init();

    root->add(root, "Alice");
    root->add(root, "Bob");
    root->add(root, "Charlie");

    printf("All names:\n");
    root->print(root);

    printf("\nSearching for Bob:\n");
    allowlist_t *e = root->find(root, "Bob");
    if (e) {
        printf("Found: %s, ID: %d\n", e->name, e->id);
    }

    printf("\nDeleting Alice...\n");
    root->del(root, "Alice");

    printf("All names after deletion:\n");
    root->print(root);

    root->cleanup(root);
    return 0;
}

