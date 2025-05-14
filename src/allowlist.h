#ifndef __ALLOWLIST_H
#define __ALLOWLIST_H

#include <uthash.h>

typedef struct allowlist_t allowlist_t;

struct allowlist_t {
    char name[10];
    int id;

    // Root-only fields
    int next_id;
    allowlist_t *entries;

    // Method pointers
    void          (*add)(allowlist_t *self, const char *name);
    allowlist_t*  (*find)(allowlist_t *self, const char *name);
    void          (*del)(allowlist_t *self, const char *name);
    void          (*print)(allowlist_t *self);
    void          (*cleanup)(allowlist_t *self);

    // Reader-writer lock
    pthread_mutex_t lock;
    pthread_cond_t cond;
    int reader_count;
    int writer_waiting;

    UT_hash_handle hh;
};

#endif	/* __ALLOWLIST_H */
