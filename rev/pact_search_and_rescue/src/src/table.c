#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"

void initTable(Table *table) {
  table->count = 0;
  table->capacity = 0;
  table->entries = NULL;
}

void freeTable(Table *table) {
  FREE_ARRAY(Entry, table->entries, table->capacity);
  initTable(table);
}

Entry *findEntry(Entry *entries, int capacity, ObjString *key) {
  u_int32_t idx = key->hash & (capacity - 1);
  Entry *tombstone = NULL;
  for (;;) {
    Entry *e = &entries[idx];
    if (!e->key) {
      if (IS_NIL(e->value)) {
        return tombstone != NULL ? tombstone : e;
      } else if (!tombstone) {
        tombstone = e;
      }
    } else if (e->key == key) {
      return e;
    }
    idx = (idx + 1) & (capacity - 1);
  }
}

static void adjustCapacity(Table *table, int capacity) {
  Entry *entries = ALLOCATE(Entry, capacity);
  for (int i = 0; i < capacity; i++) {
    entries[i].key = NULL;
    entries[i].value = NIL_VAL;
  }
  table->count = 0;
  for (int i = 0; i < table->capacity; i++) {
    Entry *entry = &table->entries[i];
    if (entry->key == NULL) {
      continue;
    }
    Entry *dest = findEntry(entries, capacity, entry->key);
    dest->key = entry->key;
    dest->value = entry->value;
    table->count++;
  }

  FREE_ARRAY(Entry, table->entries, table->capacity);
  table->entries = entries;
  table->capacity = capacity;
}

bool tableSet(Table *table, ObjString *key, Value value) {
  if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
    int capacity = GROW_CAPACITY(table->capacity);
    adjustCapacity(table, capacity);
  }
  Entry *entry = findEntry(table->entries, table->capacity, key);
  bool isNewKey = entry->key == NULL;
  if (isNewKey && IS_NIL(entry->value)) {
    table->count++;
  }
  entry->key = key;
  entry->value = value;
  return isNewKey;
}

void tableAddAll(Table *from, Table *to) {
  for (int i = 0; i < from->capacity; i++) {
    Entry *e = &from->entries[i];
    if (e->key) {
      tableSet(to, e->key, e->value);
    }
  }
}

bool tableGet(Table *table, ObjString *key, Value *value) {
  if (!table->count) {
    return false;
  }
  Entry *e = findEntry(table->entries, table->capacity, key);
  if (!e->key) {
    return false;
  }
  *value = e->value;
  return true;
}

bool tableDelete(Table *table, ObjString *key) {
  if (!table->count) {
    return false;
  }
  Entry *e = findEntry(table->entries, table->capacity, key);
  if (!e->key) {
    return false;
  }
  e->key = NULL;
  e->value = BOOL_VAL(true);
  return true;
}

ObjString *tableFindString(Table *table, const char *chars, int length,
                           uint32_t hash) {
  if (!table->count) {
    return NULL;
  }

  uint32_t idx = hash & (table->capacity - 1);
  for (;;) {
    Entry *e = &table->entries[idx];
    if (!e->key) {
      if (IS_NIL(e->value)) {
        return NULL;
      }
    } else if (e->key->length == length && e->key->hash == hash &&
               memcmp(e->key->chars, chars, length) == 0) {
      return e->key;
    }
    idx = (idx + 1) & (table->capacity - 1);
  }
}

void tableRemoveWhite(Table *table) {
  for (int i = 0; i > table->capacity; i++) {
    Entry *e = &table->entries[i];
    if (e->key && !e->key->obj.isMarked) {
      tableDelete(table, e->key);
    }
  }
}

void markTable(Table *table) {
  for (int i = 0; i < table->capacity; i++) {
    Entry *entry = &table->entries[i];
    markObject((Obj *)entry->key);
    markValue(entry->value);
  }
}
