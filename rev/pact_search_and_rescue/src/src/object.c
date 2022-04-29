#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "chunk.h"
#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"

Obj *allocateObject(size_t size, ObjType type) {
  Obj *obj = (Obj *)reallocate(NULL, 0, size);
  obj->type = type;

  obj->next = vm.objects;

#ifdef DEBUG_LOG_GC
  printf("%p allocate %zu for ", (void *)obj, size);
  switch (type){
    case OBJ_FUNCTION:
      printf("OBJ_FUNCTION\n");
      break;
    case OBJ_CLOSURE:
      printf("OBJ_CLOSURE\n");
      break;
    case OBJ_NATIVE:
      printf("OBJ_NATIVE\n");
      break;
    case OBJ_STRING:
      printf("OBJ_STRING\n");
      break;
    case OBJ_UPVALUE:
      printf("OBJ_UPVALUE\n");
      break;
    case OBJ_CLASS:
      printf("OBJ_CLASS\n");
      break;
    case OBJ_INSTANCE:
      printf("OBJ_INSTANCE\n");
      break;
    case OBJ_LIST:
      printf("OBJ_LIST\n");
      break;
    case OBJ_BOUND_METHOD:
      printf("OBJ_BOUND_METHOD\n");
      break;
  }
#endif

  vm.objects = obj;
  return obj;
}

ObjFunction *newFunction() {
  ObjFunction *function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
  function->arity = 0;
  function->upvalueCount = 0;
  function->name = NULL;
  initChunk(&function->chunk);
  return function;
}

ObjClosure *newClosure(ObjFunction *function) {
  ObjUpvalue **upvalues = ALLOCATE(ObjUpvalue *, function->upvalueCount);
  for (int i = 0; i < function->upvalueCount; i++) {
    upvalues[i] = NULL;
  }
  ObjClosure *closure = ALLOCATE_OBJ(ObjClosure, OBJ_CLOSURE);
  closure->function = function;
  closure->upvalues = upvalues;
  closure->upvalueCount = function->upvalueCount;
  return closure;
}

ObjNative *newNative(NativeFn func) {
  ObjNative *native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
  native->function = func;
  return native;
}

ObjList *newList() {
  ObjList *list = ALLOCATE_OBJ(ObjList, OBJ_LIST);
  list->items = NULL;
  list->count = 0;
  list->capcity = 0;
  return list;
}

void appendToList(ObjList *list, Value value) {
  if (list->capcity < list->count + 1) {
    int oldCap = list->capcity;
    list->capcity = GROW_CAPACITY(oldCap);
    list->items = GROW_ARRAY(Value, list->items, oldCap, list->capcity);
  }
  list->items[list->count++] = value;
}

int storeToList(ObjList *list, int index, Value value) {
  if (index < 0) {
    index = list->count - index;
  }
  if (index < list->count && index >= 0) {
    list->items[index] = value;
    return 0;
  } else {
    return 1;
  }
}

int indexFromList(ObjList *list, int index, Value *val_str) {
  if (index < 0) {
    index = list->count + index;
  }
  if (index < list->count && index >= 0) {
    *val_str = list->items[index];
    return 0;
  } else {
    *val_str = NIL_VAL;
    return 1;
  }
}

int deleteFromList(ObjList *list, int idx) {
  if (idx < 0) {
    idx = list->count - idx;
  }
  if (idx < list->count && idx >= 0) {
    for (int i = idx; i < list->count - 1; i++) {
      list->items[i] = list->items[i + 1];
    }
    list->items[--list->count] = NIL_VAL;
    return 0;
  } else {
    return 1;
  }
}

static ObjString *allocateString(char *chars, int length, uint32_t hash) {
  ObjString *string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
  string->length = length;
  string->chars = chars;
  string->hash = hash;
  push(OBJ_VAL(string)); // Make sure there is some ref to the string on the
                         // stack so gc doesnt eat it
  tableSet(&vm.strings, string, NIL_VAL);
  pop();
  return string;
}

uint32_t hashString(const char *key, int length) {
  uint32_t hash = 2166136261u;
  for (int i = 0; i < length; i++) {
    hash ^= (uint8_t)key[i];
    hash *= 16777619;
  }
  return hash;
}

ObjString *copyString(const char *chars, int length) {
  uint32_t hash = hashString(chars, length);
  ObjString *interned = tableFindString(&vm.strings, chars, length, hash);
  if (interned) {
    return interned;
  }
  char *heapChars = ALLOCATE(char, length + 1);
  memcpy(heapChars, chars, length);
  heapChars[length] = '\0';
  return allocateString(heapChars, length, hash);
}

ObjUpvalue *newUpvalue(Value *slot) {
  ObjUpvalue *upvalue = ALLOCATE_OBJ(ObjUpvalue, OBJ_UPVALUE);
  upvalue->closed = NIL_VAL;
  upvalue->location = slot;
  upvalue->next = NULL;
  return upvalue;
}

ObjClass *newClass(ObjString *name) {
  ObjClass *class = ALLOCATE_OBJ(ObjClass, OBJ_CLASS);
  class->name = name;
  initTable(&class->methods);
  return class;
}

ObjInstance *newInstance(ObjClass *klass) {
  ObjInstance *instance = ALLOCATE_OBJ(ObjInstance, OBJ_INSTANCE);
  instance->klass = klass;
  initTable(&instance->fields);
  return instance;
}

ObjBoundMethod *newBoundMethod(Value receiver, ObjClosure *method) {
  ObjBoundMethod *bound = ALLOCATE_OBJ(ObjBoundMethod, OBJ_BOUND_METHOD);
  bound->receiver = receiver;
  bound->method = method;
  return bound;
}

ObjString *takeString(char *chars, int length) {
  uint32_t hash = hashString(chars, length);
  ObjString *interned = tableFindString(&vm.strings, chars, length, hash);
  if (interned) {
    FREE_ARRAY(char, chars, length + 1);
    return interned;
  }
  return allocateString(chars, length, hash);
}

static void printFunction(ObjFunction *func) {
  if (func->name == NULL) {
    printf("<script>");
    return;
  }
  printf("<fn %s>", func->name->chars);
}

static void printList(ObjList *list) {
  printf("[");
  for (int i = 0; i < list->count - 1; i++) {
    printValue(list->items[i]);
    printf(", ");
  }
  if (list->count) {
    printValue(list->items[list->count - 1]);
  }
  printf("]\n");
}

void printObject(Value value) {
  switch (OBJ_TYPE(value)) {
  case OBJ_CLASS:
    printf("%s", AS_CLASS(value)->name->chars);
    break;
  case OBJ_CLOSURE:
    printFunction(AS_CLOSURE(value)->function);
    break;
  case OBJ_FUNCTION:
    printFunction(AS_FUNCTION(value));
    break;
  case OBJ_NATIVE:
    printf("<native fn>");
    break;
  case OBJ_STRING:
    printf("%s", AS_CSTRING(value));
    break;
  case OBJ_INSTANCE:
    printf("%s instance", AS_INSTANCE(value)->klass->name->chars);
    break;
  case OBJ_UPVALUE:
    printf("upvalue");
    break;
  case OBJ_LIST:
    printList(AS_LIST(value));
    break;
  case OBJ_BOUND_METHOD:
    printFunction(AS_BOUND_METHOD(value)->method->function);
    break;
  }
}
