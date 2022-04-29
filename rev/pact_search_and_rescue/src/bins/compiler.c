#include "src/compiler.h"
#include "src/chunk.h"
#include "src/memory.h"
#include "src/object.h"
#include "src/value.h"
#include "src/vm.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *output_file;

static char *readFile(const char *path) {
  FILE *file = fopen(path, "rb");
  if (!file) {
    fprintf(stderr, "Couldn't open file \"%s\".\n", path);
    exit(74);
  }
  fseek(file, 0L, SEEK_END);
  size_t fileSize = ftell(file);
  rewind(file);

  char *buf = (char *)malloc(fileSize + 1);
  if (!buf) {
    fprintf(stderr, "Couldn't allocate enough memory to read \"%s\".\n", path);
    exit(74);
  }
  size_t bytesRead = fread(buf, sizeof(char), fileSize, file);
  if (bytesRead != fileSize) {
    fprintf(stderr, "File changed while reading \"%s\".\n", path);
    exit(74);
  }
  buf[bytesRead] = 0;
  fclose(file);
  return buf;
}

typedef struct {
  uint8_t *buf;
  size_t size;
  size_t capacity;
} Bytecode;

Bytecode bytes;

void initBuffer() {
  bytes.capacity = 16;
  bytes.buf = malloc(sizeof(uint8_t) * 16);
  bytes.size = 0;
}

void write_buffer(const void *ptr, unsigned long size, unsigned long nelem) {
  size_t req = size * nelem;
  if (bytes.size + req >= bytes.capacity) {
    size_t new_cap = bytes.capacity * 2;
    if (new_cap < bytes.size + req) {
      new_cap = bytes.capacity + req;
    }
    bytes.buf = realloc(bytes.buf, new_cap);
    bytes.capacity = new_cap;
  }
  memcpy(bytes.buf + bytes.size, ptr, req);
  bytes.size += req;
}

void freeBuffer() { free(bytes.buf); }

// Should only need to save functions and strings, the vm can rebuild everything
// else from that

void write_string(ObjString *str) {
  int tmp = OBJ_STRING;
  write_buffer(&tmp, sizeof(int), 1);
  ObjString *rename = getRenamedSymbol(str);
  if (rename &&
      !(memcmp("init", str->chars, (str->length > 4 ? 4 : str->length)) == 0)) {
    write_buffer(&rename->length, sizeof(int), 1);
    write_buffer(rename->chars, sizeof(char), rename->length);
  } else {
    write_buffer(&str->length, sizeof(int), 1);
    write_buffer(str->chars, sizeof(char), str->length);
  }
}

void write_function(ObjFunction *f);

void write_value(Value v) {
  int tmp = v.type;
  write_buffer(&tmp, sizeof(int), 1);
  switch (v.type) {
  case VAL_BOOL: {
    bool tmp = (bool)v.as.boolean;
    write_buffer(&tmp, sizeof(bool), 1);
  }
  case VAL_CHARACTER: {
    uint8_t tmp = (uint8_t)v.as.character;
    write_buffer(&tmp, sizeof(uint8_t), 1);
    break;
  }
  case VAL_NIL: {
    uint64_t tmp = (uint64_t)v.as.integer;
    write_buffer(&tmp, sizeof(uint64_t), 1);
    break;
  }
  case VAL_INTEGER: {
    uint64_t tmp = (uint64_t)v.as.integer;
    write_buffer(&tmp, sizeof(uint64_t), 1);
    break;
  }
  case VAL_FLOAT: {
    double tmp = (uint64_t)v.as.floating;
    write_buffer(&tmp, sizeof(double), 1);
    break;
  }
  case VAL_OBJ:
    if (IS_STRING(v)) {
      write_string(AS_STRING(v));
    } else if (IS_FUNCTION(v)) {
      write_function(AS_FUNCTION(v));
    } else {
      fprintf(stderr, "UH-OH\n");
    };
    break;
  }
}

void write_function(ObjFunction *f) {
  int tmp = OBJ_FUNCTION;
  write_buffer(&tmp, sizeof(int), 1);
  write_buffer(&f->arity, sizeof(int), 1);
  write_buffer(&f->upvalueCount, sizeof(int), 1);
  write_buffer(&f->chunk.count, sizeof(int), 1);
  write_buffer(f->chunk.code, sizeof(uint8_t), f->chunk.count);
  write_buffer(f->chunk.lines, sizeof(int), f->chunk.count);
  write_buffer(&f->chunk.constants.count, sizeof(int), 1);
  for (int i = 0; i < f->chunk.constants.count; i++) {
    Value v = f->chunk.constants.values[i];
    write_value(v);
  }
  if (f->name) {
    uint8_t marker = 1;
    write_buffer(&marker, sizeof(uint8_t), 1);
    write_string(f->name);
  } else {
    uint8_t marker = 0;
    write_buffer(&marker, sizeof(uint8_t), 1);
  }
}

void write_prelude() {
  char *native_names[14] = {"clock", "append", "delete", "input", "len",
                            "range", "type",   "chr",    "ord",   "int",
                            "join",  "split",  "alloc",  "exit"};
  int tmp = 14;
  write_buffer(&tmp, sizeof(int), 1);
  for (int i = 0; i < 14; i++) {
    ObjString *symString =
        copyString(native_names[i], (int)strlen(native_names[i]));
    ObjString *renamed = getRenamedSymbol(symString);
    if (!renamed) {
      symTablePut(symString);
      write_string(getRenamedSymbol(symString));
    } else {
      write_string(renamed);
    }
  }
}

int main(int argc, char **argv) {
  if (argc < 2 || argc > 3) {
    fprintf(stderr, "Usage %s input.pact [output.pactb]\n", argv[0]);
    return 1;
  }
  char *output_name;
  if (argc == 2) {
    output_name = "out.pactb";
  } else {
    output_name = argv[2];
  }
  initVM();
  char *src = readFile(argv[1]);
  ObjFunction *func = compile(src);
  initBuffer();

  output_file = fopen(output_name, "wb");
  write_prelude();
  int tmp = VAL_OBJ;
  write_buffer(&tmp, sizeof(int), 1);
  write_function(func);

  fwrite(bytes.buf, sizeof(uint8_t), bytes.size, output_file);
  fclose(output_file);

  freeBuffer();
  free(src);
  freeVM();
}
