/*
 * Copyright (c) 2004-2006 Hyperic, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "sigar.h"
#include "sigar_private.h"
#include "sigar_util.h"
#include <stdio.h>
/*
 * hash table to cache values where key is a unique number
 * such as:
 *  pid -> some process data
 *  uid -> user name
 *  gid -> group name
 */

#pragma warning(disable:4996)
#pragma warning(disable:4819)
#pragma warning(disable:4049)
#pragma warning(disable:4217)
#pragma warning(disable:4013)
#pragma warning(disable:4018)

#define ENTRIES_SIZE(n) \
    (sizeof(sigar_cache_entry_t *) * (n))

/* wrap free() for use w/ dmalloc */
static void free_value(void *ptr)
{
    free(ptr);
}

sigar_cache_t *sigar_cache_new(int size)
{
    sigar_cache_t *table = malloc(sizeof(*table));
    table->count = 0;
    table->size = size;
    table->entries = malloc(ENTRIES_SIZE(size));
    memset(table->entries, '\0', ENTRIES_SIZE(size));
    table->free_value = free_value;
    return table;
}

#ifdef DEBUG_CACHE
/* see how well entries are distributed */
static void sigar_cache_dump(sigar_cache_t *table)
{
    int i;
    sigar_cache_entry_t **entries = table->entries;

    for (i=0; i<table->size; i++) {
        sigar_cache_entry_t *entry = *entries++;

        printf("|");
        while (entry) {
            printf("%lld", entry->id);
            if (entry->next) {
                printf(",");
            }
            entry = entry->next;
        }
    }
    printf("\n");
    fflush(stdout);
}
#endif

static void sigar_cache_rehash(sigar_cache_t *table)
{
    int i;
    unsigned int new_size = table->size * 2 + 1;
    sigar_cache_entry_t **entries = table->entries;
    sigar_cache_entry_t **new_entries =
        malloc(ENTRIES_SIZE(new_size));

    memset(new_entries, '\0', ENTRIES_SIZE(new_size));

    for (i=0; i<table->size; i++) {
        sigar_cache_entry_t *entry = *entries++;

        while (entry) {
            sigar_cache_entry_t *next = entry->next;
            sigar_uint64_t hash = entry->id % new_size;

            entry->next = new_entries[hash];
            new_entries[hash] = entry;
            entry = next;
        }
    }

    free(table->entries);
    table->entries = new_entries;
    table->size = new_size;
}

#define SIGAR_CACHE_IX(t, k) \
    t->entries + (k % t->size)

sigar_cache_entry_t *sigar_cache_find(sigar_cache_t *table,
                                      sigar_uint64_t key)
{
    sigar_cache_entry_t *entry, **ptr;

    for (ptr = SIGAR_CACHE_IX(table, key), entry = *ptr;
         entry;
         ptr = &entry->next, entry = *ptr)
    {
        if (entry->id == key) {
            return entry;
        }
    }

    return NULL;
}

/* create entry if it does not exist */
sigar_cache_entry_t *sigar_cache_get(sigar_cache_t *table,
                                     sigar_uint64_t key)
{
    sigar_cache_entry_t *entry, **ptr;

    for (ptr = SIGAR_CACHE_IX(table, key), entry = *ptr;
         entry;
         ptr = &entry->next, entry = *ptr)
    {
        if (entry->id == key) {
            return entry;
        }
    }

    if (table->count++ > table->size) {
        sigar_cache_rehash(table);

        for (ptr = SIGAR_CACHE_IX(table, key), entry = *ptr;
             entry;
             ptr = &entry->next, entry = *ptr)
        {
        }
    }

    *ptr = entry = malloc(sizeof(*entry));
    entry->id = key;
    entry->value = NULL;
    entry->next = NULL;

    return entry;
}

void sigar_cache_destroy(sigar_cache_t *table)
{
    int i;
    sigar_cache_entry_t **entries = table->entries;

#ifdef DEBUG_CACHE
    sigar_cache_dump(table);
#endif

    for (i=0; i<table->size; i++) {
        sigar_cache_entry_t *entry, *ptr;
        entry = *entries++;

        while (entry) {
            if (entry->value) {
                table->free_value(entry->value);
            }
            ptr = entry->next;
            free(entry);
            entry = ptr;
        }
    }

    free(table->entries);
    free(table);
}
