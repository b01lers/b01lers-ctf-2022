#include "src/vm.h"
#include "src/memory.h"
#include "src/object.h"
#include "src/value.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

ObjString *read_string(FILE *file) {
  int len;
  fread(&len, sizeof(int), 1, file);
  char *chars = ALLOCATE(char, len);
  int n_read = fread(chars, sizeof(char), len, file);
  if (n_read != len) {
    fprintf(stderr, "Number of chars read does not equal number expected\n");
    free(chars);
    return NULL;
  }
  ObjString *rv = copyString(chars, len);
  rv->obj.type = OBJ_STRING;
  FREE(char, chars);
  return rv;
}

ObjFunction *read_function(FILE *file);

Value read_value(FILE *file, bool *err) {
  *err = false;
  int value_type;
  fread(&value_type, sizeof(int), 1, file);
  switch (value_type) {
  case VAL_BOOL: {
    bool value;
    fread(&value, sizeof(bool), 1, file);
    Value rv = {.type = VAL_BOOL, .as.boolean = value};
    return rv;
  }
  case VAL_CHARACTER: {
    uint8_t value;
    fread(&value, sizeof(uint8_t), 1, file);
    Value rv = {.type = VAL_CHARACTER, .as.character = value};
    return rv;
  }
  case VAL_NIL: {
    uint64_t value;
    fread(&value, sizeof(uint64_t), 1, file);
    Value rv = {.type = VAL_NIL, .as.integer = value};
    return rv;
  }
  case VAL_INTEGER: {
    uint64_t value;
    fread(&value, sizeof(uint64_t), 1, file);
    Value rv = {.type = VAL_INTEGER, .as.integer = value};
    return rv;
  }
  case VAL_FLOAT: {
    double value;
    fread(&value, sizeof(double), 1, file);
    Value rv = {.type = VAL_FLOAT, .as.floating = value};
    return rv;
  }
  case VAL_OBJ: {
    int obj_type;
    fread(&obj_type, sizeof(int), 1, file);
    if (obj_type == OBJ_STRING) {
      ObjString *str = read_string(file);
      if (!str) {
        *err = true;
        return NIL_VAL;
      }
      Value rv = OBJ_VAL(str);
      return rv;
    } else if (obj_type == OBJ_FUNCTION) {
      ObjFunction *func = read_function(file);
      if (!func) {
        *err = true;
        return NIL_VAL;
      }
      Value rv = OBJ_VAL(func);
      return rv;
    } else {
      *err = true;
      return NIL_VAL;
    }
  }
  default:
    *err = true;
    return NIL_VAL;
  }
}

ObjFunction *read_function(FILE *file) {
  ObjFunction *f = newFunction();
  push(OBJ_VAL(f));
  f->obj.type = OBJ_FUNCTION;
  fread(&f->arity, sizeof(int), 1, file);
  fread(&f->upvalueCount, sizeof(int), 1, file);
  fread(&f->chunk.count, sizeof(int), 1, file);
  f->chunk.capacity = f->chunk.count;

  // TODO replace malloc with vm controlled allocation
  f->chunk.code = (uint8_t *)malloc(sizeof(uint8_t) * f->chunk.count);
  f->chunk.lines = (int *)malloc(sizeof(int) * f->chunk.count);
  fread(f->chunk.code, sizeof(uint8_t), f->chunk.count, file);
  fread(f->chunk.lines, sizeof(int), f->chunk.count, file);

  fread(&f->chunk.constants.count, sizeof(int), 1, file);
  f->chunk.constants.values =
      (Value *)malloc(sizeof(Value) * f->chunk.constants.count);
  f->chunk.constants.capacity = f->chunk.constants.count;

  for (int i = 0; i < f->chunk.constants.count; i++) {
    bool err;
    f->chunk.constants.values[i] = read_value(file, &err);
    if (err) {
      fprintf(stderr, "Error reading value!\n");
      return NULL;
    }
  }

  uint8_t name_marker;
  fread(&name_marker, sizeof(uint8_t), 1, file);
  if (name_marker == 1) {
    int tmp;
    // Read the OBJ_STRING marker
    fread(&tmp, sizeof(int), 1, file);
    f->name = read_string(file);
  } else {
    f->name = NULL;
  }
  pop();
  return f;
}

ObjFunction *load_program(const char *filename) {
  FILE *file = fopen(filename, "rb");
  // Make sure that the top level functionis valid
  int tmp;
  fread(&tmp,  sizeof(int), 1, file);
  ObjString **names = malloc(sizeof(ObjString*) * tmp);
  for(int i = 0; i < tmp; i++) {
    int marker;
    fread(&marker, sizeof(int), 1, file);
    names[i] = read_string(file);
  }
  nameNatives(names);
  fread(&tmp, sizeof(int), 1, file);
  ObjFunction *rv;
  if (tmp != VAL_OBJ) {
    rv = NULL;
  }
  fread(&tmp, sizeof(int), 1, file);
  if (tmp != OBJ_FUNCTION) {
    fclose(file);
    return NULL;
  }
  // Read the function
  rv = read_function(file);
  fclose(file);
  return rv;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage %s input.pactb", argv[0]);
    return 1;
  }
  initVM();
  ObjFunction *function = load_program(argv[1]);
  if (!function) {
    fprintf(stderr, "Invalid file\n");
    return -1;
  }
  ObjClosure *closure = newClosure(function);
  pop();
  push(OBJ_VAL(closure));
  callClosure(closure, 0);
  run();
  freeVM();
}
