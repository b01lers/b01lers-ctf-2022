#include <stddef.h>
#include <stdlib.h>

#include "compiler.h"
#include "memory.h"
#include "src/object.h"
#include "table.h"
#include "value.h"
#include "vm.h"

#ifdef DEBUG_LOG_GC
#include "debug.h"
#include <stdio.h>
#endif

void *reallocate(void *pointer, size_t oldSize, size_t newSize) {
  vm.bytesAllocated += newSize - oldSize;
  if (newSize > oldSize) {
#ifdef DEBUG_STRESS_GC
    collectGarbage();
#endif
    if (vm.bytesAllocated > vm.nextGC) {
      collectGarbage();
    }
  }
  if (newSize == 0) {
    free(pointer);
    return NULL;
  }
  void *result = realloc(pointer, newSize);
  if (result == NULL) {
    exit(1);
  }
  return result;
}

void markValue(Value v) {
  if (IS_OBJ(v)) {
    markObject(AS_OBJ(v));
  }
}

void markObject(Obj *obj) {
  if (!obj || obj->isMarked) {
    return;
  }
#ifdef DEBUG_LOG_GC
  printf("%p mark ", (void *)obj);
  printValue(OBJ_VAL(obj));
  printf("\n");
#endif
  obj->isMarked = true;
  if (vm.grayCapacity < vm.grayCount + 1) {
    vm.grayCapacity = GROW_CAPACITY(vm.grayCapacity);
    vm.grayStack =
        (Obj **)realloc(vm.grayStack, sizeof(Obj *) * vm.grayCapacity);
    if (!vm.grayStack) {
      exit(1);
    }
  }
  vm.grayStack[vm.grayCount++] = obj;
}

static void freeObject(Obj *obj) {
#ifdef DEBUG_LOG_GC
  printf("%p free type ", (void *)obj);
  switch (obj->type) {
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
  switch (obj->type) {
  case OBJ_CLASS: {
    ObjClass *clazz = (ObjClass *)obj;
    freeTable(&clazz->methods);
    FREE(ObjClass, obj);
    break;
  }
  case OBJ_FUNCTION: {
    ObjFunction *func = (ObjFunction *)obj;
    freeChunk(&func->chunk);
    FREE(ObjFunction, obj);
    break;
  }
  case OBJ_INSTANCE: {
    ObjInstance *instance = (ObjInstance *)obj;
    freeTable(&instance->fields);
    FREE(ObjInstance, obj);
    break;
  }
  case OBJ_NATIVE: {
    FREE(ObjNative, obj);
    break;
  }
  case OBJ_CLOSURE: {
    ObjClosure *closure = (ObjClosure *)obj;
    FREE_ARRAY(ObjUpvalue *, closure->upvalues, closure->upvalueCount);
    FREE(ObjClosure, obj);
    break;
  }
  case OBJ_STRING: {
    ObjString *str = (ObjString *)obj;
    FREE_ARRAY(char, str->chars, str->length + 1);
    FREE(ObjString, obj);
    break;
  }
  case OBJ_LIST: {
    ObjList *list = (ObjList *)obj;
    FREE_ARRAY(Value *, list->items, list->count);
    FREE(ObjList, obj);
    break;
  }
  case OBJ_UPVALUE:
    FREE(ObjUpvalue, obj);
    break;
  case OBJ_BOUND_METHOD:
    FREE(ObjBoundMethod, obj);
    break;
  }
}

void freeObjects() {
  Obj *obj = vm.objects;
  while (obj) {
    Obj *nxt = obj->next;
    freeObject(obj);
    obj = nxt;
  }

  free(vm.grayStack);
}

static void markRoots() {
  for (Value *slot = vm.stack; slot < vm.stackTop; slot++) {
    markValue(*slot);
  }
  for (ObjUpvalue *upvalue = vm.openUpvalues; upvalue != NULL;
       upvalue = upvalue->next) {
    markObject((Obj *)upvalue);
  }
  markTable(&vm.globals);
#ifndef VM_ONLY
  markCompilerRoots();
#endif
  markObject((Obj *)vm.initString);
}

static void markArray(ValueArray *array) {
  for (int i = 0; i < array->count; i++) {
    markValue(array->values[i]);
  }
}

static void blackenObject(Obj *obj) {
#ifdef DEBUG_LOG_GC
  printf("%p blacken ", (void *)obj);
  printValue(OBJ_VAL(obj));
  printf("\n");
#endif
  switch (obj->type) {
  case OBJ_CLASS: {
    ObjClass *clazz = (ObjClass *)obj;
    markObject((Obj *)clazz->name);
    markTable(&clazz->methods);
    break;
  }
  case OBJ_CLOSURE: {
    ObjClosure *closure = (ObjClosure *)obj;
    markObject((Obj *)closure->function);
    for (int i = 0; i < closure->upvalueCount; i++) {
      markObject((Obj *)closure->upvalues[i]);
    }
    break;
  }
  case OBJ_FUNCTION: {
    ObjFunction *func = (ObjFunction *)obj;
    markObject((Obj *)func->name);
    markArray(&func->chunk.constants);
    break;
  }
  case OBJ_INSTANCE: {
    ObjInstance *inst = (ObjInstance *)obj;
    markObject((Obj *)inst->klass);
    markTable(&inst->fields);
    break;
  }
  case OBJ_UPVALUE:
    markValue(((ObjUpvalue *)obj)->closed);
    break;
  case OBJ_BOUND_METHOD: {
    ObjBoundMethod *bound = (ObjBoundMethod *)obj;
    markValue(bound->receiver);
    markObject((Obj *)bound->method);
    break;
  }
  case OBJ_LIST: {
    ObjList *list = (ObjList *)obj;
    for (int i = 0; i < list->count; i++) {
      markValue(list->items[i]);
    }
    break;
  }
  case OBJ_NATIVE:
  case OBJ_STRING:
    break;
  }
}

static void traceReferences() {
  while (vm.grayCount > 0) {
    Obj *obj = vm.grayStack[--vm.grayCount];
    blackenObject(obj);
  }
}

static void sweep() {
  Obj *prev = NULL;
  Obj *cur = vm.objects;
  while (cur) {
    if (cur->isMarked) {
      cur->isMarked = false;
      prev = cur;
      cur = cur->next;
    } else {
      Obj *unreached = cur;
      cur = cur->next;
      if (prev) {
        prev->next = cur;
      } else {
        vm.objects = cur;
      }
      free(unreached);
    }
  }
}

void collectGarbage() {
#ifdef DEBUG_LOG_GC
  printf("-- gc begin\n");
  size_t before = vm.bytesAllocated;
#endif
  markRoots();
  traceReferences();
  tableRemoveWhite(&vm.strings);
  sweep();
  vm.nextGC = vm.bytesAllocated * GC_HEAP_GROW_FACTOR;
#ifdef DEBUG_LOG_GC
  printf("-- gc end\n");
  printf("   collected %zu bytes (from %zu to %zu) next at %zu\n",
         before - vm.bytesAllocated, before, vm.bytesAllocated, vm.nextGC);
#endif
}
