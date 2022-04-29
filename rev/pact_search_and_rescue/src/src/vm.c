#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "compiler.h"
#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"

#ifdef DEBUG_TRACE_EXECUTION
#include "debug.h"
#endif

VM vm;

static Value peek(int distance);
static bool callValue(Value callee, int argCount);
static bool invoke(ObjString *name, int argCount);
static bool invokeFromClass(ObjClass *clazz, ObjString *name, int argCount);
static bool bindMethod(ObjClass *clazz, ObjString *name);
static ObjUpvalue *captureUpvalue(Value *local);
static void defineMethod(ObjString *name);
static bool isFalsey(Value val);
static void concatenateStrings();
static void concatenateLists();
static void closeUpvalues(Value *last);

static void resetStack() {
  vm.stackTop = vm.stack;
  vm.frameCount = 0;
  vm.openUpvalues = NULL;
}

static void runtimeError(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);

  fputs("\n", stderr);
  for (int i = vm.frameCount - 1; i >= 0; i--) {
    CallFrame *frame = &vm.frames[i];
    ObjFunction *function = frame->closure->function;
    size_t instruction = frame->ip - function->chunk.code - 1;
    fprintf(stderr, "[line %d] in ", function->chunk.lines[instruction]);
    if (function->name == NULL) {
      fprintf(stderr, "script\n");
    } else {
      fprintf(stderr, "%s()\n", function->name->chars);
    }
  }

  CallFrame *frame = &vm.frames[vm.frameCount - 1];
  ObjFunction *function = frame->closure->function;
  size_t instr = frame->ip - function->chunk.code - 1;
  int line = function->chunk.lines[instr];

  fprintf(stderr, "[line %d] in script\n", line);
  resetStack();
}

static Value clockNative(int argCount, Value *args) {
  return FLOAT_VAL((double)clock() / CLOCKS_PER_SEC);
}

static Value appendNative(int argc, Value *args) {
  if (argc != 2) {
    runtimeError("Runtime Error");
    return NIL_VAL;
  }
  if (!IS_LIST(args[0])) {
    runtimeError("Runtime Error");
    return NIL_VAL;
  }
  ObjList *list = AS_LIST(args[0]);
  push(args[0]);
  appendToList(list, args[1]);
  pop();
  return NIL_VAL;
}

static Value deleteNative(int argc, Value *args) {
  if (argc != 2) {
    runtimeError("Runtime Error");
    return NIL_VAL;
  }
  if (!IS_LIST(args[0])) {
    runtimeError("Runtime Error");
    return NIL_VAL;
  }
  if (!IS_INTEGER(args[1])) {
    runtimeError("Runtime Error");
    return NIL_VAL;
  }

  ObjList *list = AS_LIST(args[0]);
  int idx = AS_INTEGER(args[1]);

  if (deleteFromList(list, idx)) {
    runtimeError("Runtime Error");
  }
  return NIL_VAL;
}

static Value inputNative(int argc, Value *args) {
  if (argc != 0 && argc != 1) {
    runtimeError("Runtime Error");
    return NIL_VAL;
  }
  if (argc == 1) {
    if (args[0].type == VAL_OBJ && IS_STRING(args[0])) {
      ObjString *str = AS_STRING(args[0]);
      printf("%s", str->chars);
      fflush(stdout);
    }
  }
  size_t buf_cap = 16;
  char *buf = malloc(sizeof(char) * buf_cap);
  char c;
  size_t cnt = 0;
  while (read(1, &c, 1) == 1) {
    if (cnt >= buf_cap) {
      buf_cap = GROW_CAPACITY(buf_cap);
      buf = realloc(buf, buf_cap);
    }
    if (c == '\n' || c == '\0') {
      buf[cnt] = 0;
      break;
    }
    buf[cnt++] = c;
  }
  ObjString *str = copyString(buf, cnt);
  free(buf);
  return OBJ_VAL(str);
}

static Value lenNative(int argc, Value *args) {
  if (argc != 1) {
    runtimeError("Runtime Error");
    return NIL_VAL;
  }
  if (IS_STRING(args[0])) {
    ObjString *str = AS_STRING(args[0]);
    return INTEGER_VAL(str->length);
  } else if (IS_LIST(args[0])) {
    ObjList *list = AS_LIST(args[0]);
    return INTEGER_VAL(list->count);
  } else {
    runtimeError("Runtime Error");
    return NIL_VAL;
  }
}

static Value rangeNative(int argc, Value *args) {
  if (argc == 0 || argc > 3) {
    runtimeError("Runtime Error");
    return NIL_VAL;
  }
  for (int i = 0; i < argc; i++) {
    if (!IS_INTEGER(args[i])) {
      runtimeError("Runtime Error");
      return NIL_VAL;
    }
  }
  int start, stop, step;
  switch (argc) {
  case 1:
    start = 0;
    stop = AS_INTEGER(args[0]);
    step = 1;
    break;
  case 2:
    start = AS_INTEGER(args[0]);
    stop = AS_INTEGER(args[1]);
    step = 1;
    break;
  case 3:
    start = AS_INTEGER(args[0]);
    stop = AS_INTEGER(args[1]);
    step = AS_INTEGER(args[2]);
    break;
  default: // Not possible, just so compiler doesn't complain
    start = 0;
    stop = 0;
    step = 1;
    break;
  }

  if (step == 0) {
    runtimeError("Runtime Error");
    return NIL_VAL;
  }
  if ((start > stop && step > 0) || (start < stop && step < 0)) {
    runtimeError("Runtime Error");
    return NIL_VAL;
  }

  ObjList *list = ALLOCATE_OBJ(ObjList, OBJ_LIST);
  push(OBJ_VAL(list));
  long size = (stop - start) / step;
  list->items = reallocate(list->items, sizeof(Value) * list->capcity,
                           sizeof(Value) * size);
  list->capcity = size;
  for (int i = 0; i < size; i++) {
    list->items[i] = INTEGER_VAL(start);
    start += step;
  }
  list->count = size;
  return pop();
}

static Value allocNative(int argc, Value *args) {
  if (argc != 1 || !IS_INTEGER(args[0])) {
    runtimeError("Runtime Error");
    return NIL_VAL;
  }
  long size = AS_INTEGER(args[0]);
  ObjList *list = ALLOCATE_OBJ(ObjList, OBJ_LIST);
  push(OBJ_VAL(list));
  list->items = reallocate(list->items, sizeof(Value) * list->capcity,
                           sizeof(Value) * size);
  list->capcity = size;
  for (int i = 0; i < size; i++) {
    list->items[i] = INTEGER_VAL(0);
  }
  list->count = size;
  return pop();
}

static Value typeNative(int argc, Value *args) {
  char *type_str;
  int type_len;
  switch (args[0].type) {
  case VAL_BOOL:
    type_str = "bool";
    type_len = 4;
    break;
  case VAL_NIL:
    type_str = "nil";
    type_len = 3;
    break;
  case VAL_CHARACTER:
    type_str = "char";
    type_len = 4;
    break;
  case VAL_INTEGER:
    type_str = "int";
    type_len = 3;
    break;
  case VAL_FLOAT:
    type_str = "float";
    type_len = 5;
    break;
  case VAL_OBJ:
    type_str = "obj";
    type_len = 3;
    break;
  default:
    type_str = NULL;
    type_len = 0;
  }
  if (type_str) {
    return OBJ_VAL(copyString(type_str, type_len));
  } else {
    return NIL_VAL;
  }
}

static Value chrNative(int argc, Value *args) {
  if (argc != 1) {
    runtimeError("Runtime Error");
    return NIL_VAL;
  }
  if (!IS_INTEGER(args[0])) {
    runtimeError("Runtime Error");
    return NIL_VAL;
  }
  long val = AS_INTEGER(args[0]);
  if (val < 0 || val > 255) {
    runtimeError("Runtime Error");
    return NIL_VAL;
  }
  return CHAR_VAL(val);
}

static Value ordNative(int argc, Value *args) {
  if (argc != 1) {
    runtimeError("Runtime Error");
    return NIL_VAL;
  }
  if (IS_CHARACTER(args[0])) {
    return INTEGER_VAL(AS_CHARACTER(args[0]) & 255);
  } else if (IS_STRING(args[0]) && AS_STRING(args[0])->length == 1) {
    return INTEGER_VAL(AS_STRING(args[0])->chars[0] & 255);
  } else {
    runtimeError("Runtime Error");
    return NIL_VAL;
  }
}

static Value intNative(int argc, Value *args) {
  if (argc != 1) {
    runtimeError("Runtime Error");
    return NIL_VAL;
  }
  switch (args[0].type) {
  case VAL_FLOAT:
    return INTEGER_VAL((long)AS_FLOATING(args[0]));
  case VAL_CHARACTER:
    return INTEGER_VAL((long)AS_CHARACTER(args[0]));
  case VAL_INTEGER:
    return args[0];
  default:
    runtimeError("Runtime Error");
    return NIL_VAL;
  }
}

static Value joinNative(int argc, Value *args) {
  if (argc != 1) {
    runtimeError("Runtime Error");
  }
  if (!IS_LIST(args[0])) {
    runtimeError("Runtime Error");
  }
  push(args[0]);
  ObjList *list = AS_LIST(args[0]);
  char *str = (char *)malloc(sizeof(char) * list->count);
  for (int i = 0; i < list->count; i++) {
    if (!IS_CHARACTER(list->items[i])) {
      free(str);
      runtimeError("Runtime Error");
      return NIL_VAL;
    }
    str[i] = list->items[i].as.character;
  }
  pop();
  ObjString *str_obj = copyString(str, list->count);
  free(str);
  return OBJ_VAL(str_obj);
}

static Value splitNative(int argc, Value *args) {
  if (argc != 1) {
    runtimeError("Runtime Error");
  }
  if (!IS_STRING(args[0])) {
    runtimeError("Runtime Error");
  }
  ObjString *str = AS_STRING(args[0]);
  push(args[0]);
  ObjList *list = ALLOCATE_OBJ(ObjList, OBJ_LIST);
  push(OBJ_VAL(list));
  list->items = reallocate(list->items, sizeof(Value) * list->capcity,
                           sizeof(Value) * str->length);
  list->capcity = str->length;
  list->count = str->length;
  for (int i = 0; i < str->length; i++) {
    list->items[i].type = VAL_CHARACTER;
    list->items[i].as.character = str->chars[i];
  }
  Value rv = pop();
  pop();
  return rv;
}

static Value exitNative(int argc, Value *args) {
  if (argc != 1) {
    runtimeError("Runtime Error");
  }
  if (!IS_INTEGER(args[0])) {
    runtimeError("Runtime Error");
  }
  freeVM();
  exit(AS_INTEGER(args[0]));
}

static void defineNative(ObjString *name, NativeFn function) {
  push(OBJ_VAL(copyString(name->chars, name->length)));
  push(OBJ_VAL(newNative(function)));
  tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
  pop();
  pop();
}

void initVM(ObjString **names) {
  resetStack();
  vm.objects = NULL;
  vm.bytesAllocated = 0;
  vm.nextGC = 1024 * 1024;
  vm.grayCount = 0;
  vm.grayCapacity = 0;
  vm.grayStack = NULL;
  initTable(&vm.strings);
  initTable(&vm.globals);
  vm.initString = NULL;
  vm.initString = copyString("init", 4);

}

void nameNatives(ObjString **names) {
  NativeFn funcs[14] = {
    clockNative, appendNative, deleteNative, inputNative, lenNative, rangeNative, typeNative, chrNative, ordNative, intNative, joinNative, splitNative, allocNative, exitNative
  };
  for (int i = 0; i < 14; i++) {
    defineNative(names[i], funcs[i]);
  }
}

void freeVM() {
  freeTable(&vm.strings);
  freeTable(&vm.globals);
  vm.initString = NULL;
  freeObjects();
}

InterpretResult run() {
  CallFrame *frame = &vm.frames[vm.frameCount - 1];

#define READ_BYTE() (*frame->ip++)
#define READ_SHORT()                                                           \
  (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))
#define READ_CONSTANT()                                                        \
  (frame->closure->function->chunk.constants.values[READ_BYTE()])
#define READ_STRING() AS_STRING(READ_CONSTANT())
#define BOOLEAN_OP(op)                                                         \
  do {                                                                         \
    if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {                          \
      runtimeError("Runtime Error");                               \
      return INTERPRET_RUNTIME_ERROR;                                          \
    }                                                                          \
    if (IS_INTEGER(peek(0)) && IS_INTEGER(peek(1))) {                          \
      long b = AS_INTEGER(pop());                                              \
      long a = AS_INTEGER(pop());                                              \
      push(BOOL_VAL(a op b));                                                  \
    } else if (IS_INTEGER(peek(0))) {                                          \
      double b = (double)AS_INTEGER(pop());                                    \
      double a = AS_FLOATING(pop());                                           \
      push(BOOL_VAL(a op b));                                                  \
    } else if (IS_INTEGER(peek(1))) {                                          \
      double b = AS_FLOATING(pop());                                           \
      double a = (double)AS_INTEGER(pop());                                    \
      push(BOOL_VAL(a op b));                                                  \
    } else {                                                                   \
      double b = AS_FLOATING(pop());                                           \
      double a = AS_FLOATING(pop());                                           \
      push(BOOL_VAL(a op b));                                                  \
    }                                                                          \
  } while (false)

#define BITWISE_OP(op)                                                         \
  do {                                                                         \
    if (!IS_INTEGER(peek(0)) || !IS_INTEGER(peek(1))) {                        \
      runtimeError("Runtime Error");                              \
      return INTERPRET_RUNTIME_ERROR;                                          \
    }                                                                          \
    long b = AS_INTEGER(pop());                                                \
    long a = AS_INTEGER(pop());                                                \
    push(INTEGER_VAL(a op b));                                                 \
  } while (false)

#define BINARY_OP(op)                                                          \
  do {                                                                         \
    if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {                          \
      runtimeError("Runtime Error");                               \
      return INTERPRET_RUNTIME_ERROR;                                          \
    }                                                                          \
    if (IS_INTEGER(peek(0)) && IS_INTEGER(peek(1))) {                          \
      long b = AS_INTEGER(pop());                                              \
      long a = AS_INTEGER(pop());                                              \
      push(INTEGER_VAL(a op b));                                               \
    } else if (IS_INTEGER(peek(0))) {                                          \
      double b = (double)AS_INTEGER(pop());                                    \
      double a = AS_FLOATING(pop());                                           \
      push(FLOAT_VAL(a op b));                                                 \
    } else if (IS_INTEGER(peek(1))) {                                          \
      double b = AS_FLOATING(pop());                                           \
      double a = (double)AS_INTEGER(pop());                                    \
      push(FLOAT_VAL(a op b));                                                 \
    } else {                                                                   \
      double b = AS_FLOATING(pop());                                           \
      double a = AS_FLOATING(pop());                                           \
      push(FLOAT_VAL(a op b));                                                 \
    }                                                                          \
  } while (false)
  for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
    printf("          ");
    for (Value *slot = vm.stack; slot < vm.stackTop; slot++) {
      printf("[ ");
      printValue(*slot);
      printf(" ]");
    }
    printf("\n");
    disassembleInstruction(
        &frame->closure->function->chunk,
        (int)(frame->ip - frame->closure->function->chunk.code));
#endif
    uint8_t instruction;
    switch (instruction = READ_BYTE()) {
    case OP_CONSTANT: {
      Value constant = READ_CONSTANT();
      push(constant);
      break;
    }
    case OP_NIL:
      push(NIL_VAL);
      break;
    case OP_TRUE:
      push(BOOL_VAL(true));
      break;
    case OP_FALSE:
      push(BOOL_VAL(false));
      break;
    case OP_POP:
      pop();
      break;
    case OP_GET_LOCAL: {
      uint8_t slot = READ_BYTE();
      push(frame->slots[slot]);
      break;
    }
    case OP_SET_LOCAL: {
      uint8_t slot = READ_BYTE();
      frame->slots[slot] = peek(0);
      break;
    }
    case OP_GET_GLOBAL: {
      ObjString *name = READ_STRING();
      Value value;
      if (!tableGet(&vm.globals, name, &value)) {
        runtimeError("Runtime Error");
        return INTERPRET_RUNTIME_ERROR;
      }
      push(value);
      break;
    }
    case OP_DEFINE_GLOBAL: {
      ObjString *name = READ_STRING();
      tableSet(&vm.globals, name, peek(0));
      pop();
      break;
    }
    case OP_SET_GLOBAL: {
      ObjString *name = READ_STRING();
      if (tableSet(&vm.globals, name, peek(0))) {
        tableDelete(&vm.globals, name);
        runtimeError("Runtime Error");
        return INTERPRET_RUNTIME_ERROR;
      }
      break;
    }
    case OP_GET_UPVALUE: {
      uint8_t slot = READ_BYTE();
      push(*frame->closure->upvalues[slot]->location);
      break;
    }
    case OP_SET_UPVALUE: {
      uint8_t slot = READ_BYTE();
      *frame->closure->upvalues[slot]->location = peek(0);
      break;
    }
    case OP_GET_PROPERTY: {
      if (!IS_INSTANCE(peek(0))) {
        runtimeError("Runtime Error");
        return INTERPRET_RUNTIME_ERROR;
      }
      ObjInstance *inst = AS_INSTANCE(peek(0));
      ObjString *name = READ_STRING();
      Value val;
      if (tableGet(&inst->fields, name, &val)) {
        pop(); // clear the instance
        push(val);
        break;
      }
      if (!bindMethod(inst->klass, name)) {
        return INTERPRET_RUNTIME_ERROR;
      }
      break;
    }
    case OP_SET_PROPERTY: {
      if (!IS_INSTANCE(peek(1))) {
        runtimeError("Runtime Error");
        return INTERPRET_RUNTIME_ERROR;
      }
      ObjInstance *inst = AS_INSTANCE(peek(1));
      tableSet(&inst->fields, READ_STRING(), peek(0));
      Value value = pop();
      pop();
      push(value);
      break;
    }
    case OP_GET_SUPER: {
      ObjString *name = READ_STRING();
      ObjClass *superclass = AS_CLASS(pop());
      if (!bindMethod(superclass, name)) {
        return INTERPRET_RUNTIME_ERROR;
      }
      break;
    }
    case OP_EQUAL: {
      Value b = pop();
      Value a = pop();
      push(BOOL_VAL(valuesEqual(a, b)));
      break;
    }
    case OP_GREATER:
      BOOLEAN_OP(>);
      break;
    case OP_LESS:
      BOOLEAN_OP(<);
      break;
    case OP_ADD: {
      if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
        concatenateStrings();
      } else if (IS_LIST(peek(0)) && IS_LIST(peek(1))) {
        concatenateLists();
      } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
        BINARY_OP(+);
      } else {
        runtimeError("Runtime Error");
        return INTERPRET_RUNTIME_ERROR;
      }
      break;
    }
    case OP_SUBTRACT:
      BINARY_OP(-);
      break;
    case OP_MULTIPLY:
      BINARY_OP(*);
      break;
    case OP_DIVIDE:
      BINARY_OP(/);
      break;
    case OP_BIT_AND:
      BITWISE_OP(&);
      break;
    case OP_BIT_OR:
      BITWISE_OP(|);
      break;
    case OP_BIT_XOR:
      BITWISE_OP(^);
      break;
    case OP_LSL:
      BITWISE_OP(<<);
      break;
    case OP_LSR:
      BITWISE_OP(>>);
      break;
    case OP_NOT:
      push(BOOL_VAL(isFalsey(pop())));
      break;
    case OP_NEGATE: {
      Value v = peek(0);
      if (!IS_NUMBER(v)) {
        runtimeError("Runtime Error");
        return INTERPRET_RUNTIME_ERROR;
      }
      pop();
      if (IS_INTEGER(v)) {
        push(INTEGER_VAL(-AS_INTEGER(v)));
      } else {
        push(FLOAT_VAL(-AS_FLOATING(v)));
      }
      break;
    }
    case OP_PRINT:
      printValue(pop());
      printf("\n");
      break;
    case OP_JUMP_IF_FALSE: {
      uint16_t offset = READ_SHORT();
      frame->ip += offset * (uint16_t)isFalsey(peek(0));
      break;
    }
    case OP_JUMP: {
      uint16_t offset = READ_SHORT();
      frame->ip += offset;
      break;
    }
    case OP_LOOP: {
      uint16_t offset = READ_SHORT();
      frame->ip -= offset;
      break;
    }
    case OP_CALL: {
      int count = READ_BYTE();
      if (!callValue(peek(count), count)) {
        return INTERPRET_RUNTIME_ERROR;
      }
      frame = &vm.frames[vm.frameCount - 1];
      break;
    }
    case OP_SUPER_INVOKE: {
      ObjString *method = READ_STRING();
      int argCount = READ_BYTE();
      ObjClass *superclass = AS_CLASS(pop());
      if (!invokeFromClass(superclass, method, argCount)) {
        return INTERPRET_RUNTIME_ERROR;
      }
      frame = &vm.frames[vm.frameCount - 1];
      break;
    }
    case OP_CLOSURE: {
      ObjFunction *function = AS_FUNCTION(READ_CONSTANT());
      ObjClosure *closure = newClosure(function);
      push(OBJ_VAL(closure));
      for (int i = 0; i < closure->upvalueCount; i++) {
        uint8_t isLocal = READ_BYTE();
        uint8_t index = READ_BYTE();
        if (isLocal) {
          closure->upvalues[i] = captureUpvalue(frame->slots + index);
        } else {
          closure->upvalues[i] = frame->closure->upvalues[index];
        }
      }
      break;
    }
    case OP_INVOKE: {
      ObjString *method = READ_STRING();
      int argc = READ_BYTE();
      if (!invoke(method, argc)) {
        return INTERPRET_RUNTIME_ERROR;
      }
      frame = &vm.frames[vm.frameCount - 1];
      break;
    }
    case OP_CLOSE_UPVALUE:
      closeUpvalues(vm.stackTop - 1);
      pop();
      break;
    case OP_RETURN: {
      Value result = pop();
      closeUpvalues(frame->slots);
      vm.frameCount--;
      if (vm.frameCount == 0) {
        pop();
        return INTERPRET_OK;
      }
      vm.stackTop = frame->slots;
      push(result);
      frame = &vm.frames[vm.frameCount - 1];
      break;
    }
    case OP_CLASS:
      push(OBJ_VAL(newClass(READ_STRING())));
      break;
    case OP_INHERIT: {
      Value superclass = peek(1);
      if (!IS_CLASS(superclass)) {
        runtimeError("Runtime Error");
        return INTERPRET_RUNTIME_ERROR;
      }
      ObjClass *subclass = AS_CLASS(peek(0));
      tableAddAll(&AS_CLASS(superclass)->methods, &subclass->methods);
      pop();
      break;
    }
    case OP_METHOD:
      defineMethod(READ_STRING());
      break;
    case OP_BUILD_LIST: {
      ObjList *list = newList();
      uint8_t itemCount = READ_BYTE();
      push(OBJ_VAL(list));
      for (int i = itemCount; i > 0; i--) {
        appendToList(list, peek(i));
      }
      pop();
      while (itemCount-- > 0) {
        pop();
      }
      push(OBJ_VAL(list));
      break;
    }
    case OP_INDEX_SUBSCR: {
      // Don't need to check b/c we're gonna trust the compiler
      Value idx_val = pop();
      Value list_val = pop();
      if (IS_LIST(list_val)) {
        Value rv;
        ObjList *list = AS_LIST(list_val);
        if (!IS_NUMBER(idx_val)) {
          runtimeError("Runtime Error");
          return INTERPRET_RUNTIME_ERROR;
        }
        int idx;
        if (idx_val.type == VAL_FLOAT) {
          idx = (int)idx_val.as.floating;
        } else {
          idx = (int)idx_val.as.integer;
        }

        if (indexFromList(list, idx, &rv)) {
          runtimeError("Runtime Error");
          return INTERPRET_RUNTIME_ERROR;
        }
        push(rv);
      } else if (IS_STRING(list_val)) {
        ObjString *str = AS_STRING(list_val);
        int idx;
        if (idx_val.type == VAL_FLOAT) {
          idx = (int)idx_val.as.floating;
        } else {
          idx = (int)idx_val.as.integer;
        }
        if (idx < 0) {
          idx = str->length + idx;
        }
        if (str->length <= idx || idx < 0) {
          runtimeError("Runtime Error");
          return INTERPRET_RUNTIME_ERROR;
        }
        push(CHAR_VAL(str->chars[idx]));
      } else {
        runtimeError("Runtime Error");
        return INTERPRET_RUNTIME_ERROR;
      }
      break;
    }
    case OP_STORE_SUBSCR: {
      Value item = pop();
      Value idx_val = pop();
      Value list_val = pop();
      if (!IS_LIST(list_val)) {
        runtimeError("Runtime Error");
      }
      if (!IS_NUMBER(idx_val)) {
        runtimeError("Runtime Error");
        return INTERPRET_RUNTIME_ERROR;
      }
      ObjList *list = AS_LIST(list_val);
      int idx;
      if (idx_val.type == VAL_FLOAT) {
        idx = (int)idx_val.as.floating;
      } else {
        idx = (int)idx_val.as.integer;
      }
      if (storeToList(list, idx, item)) {
        runtimeError("Runtime Error");
        return INTERPRET_RUNTIME_ERROR;
      }
      push(item);
      break;
    }
    }
  }
#undef READ_CONSTANT
#undef READ_BYTE
#undef READ_SHORT
#undef BINARY_OP
#undef READ_STRING
}

#ifndef VM_ONLY
InterpretResult interpret(const char *src) {
  ObjFunction *function = compile(src);
  if (function == NULL) {
    return INTERPRET_COMPILE_ERROR;
  }
  push(OBJ_VAL(function));
  ObjClosure *closure = newClosure(function);
  pop();
  push(OBJ_VAL(closure));
  callClosure(closure, 0);

  return run();
}
#endif

void push(Value value) {
  *vm.stackTop = value;
  vm.stackTop++;
}

Value pop() {
  vm.stackTop--;
  return *vm.stackTop;
}

static Value peek(int distance) { return vm.stackTop[-1 - distance]; }

static bool isFalsey(Value value) {
  return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void concatenateStrings() {
  ObjString *b = AS_STRING(peek(0));
  ObjString *a = AS_STRING(peek(1));

  int len = a->length + b->length;
  char *chars = ALLOCATE(char, len + 1);
  memcpy(chars, a->chars, a->length);
  memcpy(chars + a->length, b->chars, b->length);
  chars[len] = 0;

  ObjString *result = takeString(chars, len);
  pop();
  pop();
  push(OBJ_VAL(result));
}

static void concatenateLists() {
  ObjList *b = AS_LIST(peek(0));
  ObjList *a = AS_LIST(peek(1));

  int len = a->count + b->count;
  Value *vals = ALLOCATE(Value, len);
  memcpy(vals, a->items, a->count);
  memcpy(vals + a->count, b->items, b->count);

  ObjList *result = ALLOCATE_OBJ(ObjList, OBJ_LIST);
  result->items = vals;
  result->count = len;
  result->capcity = len;
  pop();
  pop();
  push(OBJ_VAL(result));
}

bool callClosure(ObjClosure *closure, int argCount) {
  if (argCount != closure->function->arity) {
    runtimeError("Runtime Error");
    return false;
  }
  if (vm.frameCount == FRAMES_MAX) {
    runtimeError("Runtime Error");
    return false;
  }
  CallFrame *frame = &vm.frames[vm.frameCount++];
  frame->closure = closure;
  frame->ip = closure->function->chunk.code;
  frame->slots = vm.stackTop - argCount - 1;
  return true;
}

static bool callValue(Value callee, int argCount) {
  if (IS_OBJ(callee)) {
    switch (OBJ_TYPE(callee)) {
    case OBJ_NATIVE: {
      NativeFn native = AS_NATIVE(callee);
      Value result = native(argCount, vm.stackTop - argCount);
      vm.stackTop -= argCount + 1;
      push(result);
      return true;
    }
    case OBJ_CLOSURE:
      return callClosure(AS_CLOSURE(callee), argCount);
    case OBJ_CLASS: {
      ObjClass *klass = AS_CLASS(callee);
      vm.stackTop[-argCount - 1] = OBJ_VAL(newInstance(klass));
      Value initializer;
      if (tableGet(&klass->methods, vm.initString, &initializer)) {
        return callClosure(AS_CLOSURE(initializer), argCount);
      } else if (argCount != 0) {
        runtimeError("Runtime Error");
        return false;
      }
      return true;
    }
    case OBJ_BOUND_METHOD: {
      ObjBoundMethod *bound = AS_BOUND_METHOD(callee);
      vm.stackTop[-argCount - 1] = bound->receiver;
      return callClosure(bound->method, argCount);
    }
    default:
      break;
    }
  }
  runtimeError("Runtime Error");
  return false;
}

static bool invokeFromClass(ObjClass *clazz, ObjString *name, int argCount) {
  Value method;
  if (!tableGet(&clazz->methods, name, &method)) {
    runtimeError("Runtime Error");
    return false;
  }
  return callClosure(AS_CLOSURE(method), argCount);
}

static bool invoke(ObjString *name, int argCount) {
  Value receiver = peek(argCount);
  if (!IS_INSTANCE(receiver)) {
    runtimeError("Runtime Error");
    return false;
  }
  ObjInstance *inst = AS_INSTANCE(receiver);
  Value value;
  // Handle the case where this could be a closure
  if (tableGet(&inst->fields, name, &value)) {
    vm.stackTop[-argCount - 1] = value;
    return callValue(value, argCount);
  }
  return invokeFromClass(inst->klass, name, argCount);
}

static bool bindMethod(ObjClass *clazz, ObjString *name) {
  Value method;
  if (!tableGet(&clazz->methods, name, &method)) {
    runtimeError("Runtime Error");
    return false;
  }
  ObjBoundMethod *bound = newBoundMethod(peek(0), AS_CLOSURE(method));

  pop();
  push(OBJ_VAL(bound));
  return true;
}

static ObjUpvalue *captureUpvalue(Value *local) {
  ObjUpvalue *prev = NULL;
  ObjUpvalue *cur = vm.openUpvalues;
  while (cur != NULL && cur->location > local) {
    prev = cur;
    cur = cur->next;
  }
  if (cur && cur->location == local) {
    return cur;
  }
  ObjUpvalue *createdUpvalue = newUpvalue(local);
  createdUpvalue->next = cur;
  if (!prev) {
    vm.openUpvalues = createdUpvalue;
  } else {
    prev->next = createdUpvalue;
  }
  return createdUpvalue;
}

static void closeUpvalues(Value *last) {
  while (vm.openUpvalues && vm.openUpvalues->location >= last) {
    ObjUpvalue *upvalue = vm.openUpvalues;
    upvalue->closed = *upvalue->location;
    upvalue->location = &upvalue->closed;
    vm.openUpvalues = upvalue->next;
  }
}

static void defineMethod(ObjString *name) {
  Value method = peek(0);
  ObjClass *clazz = AS_CLASS(peek(1));
  tableSet(&clazz->methods, name, method);
  pop();
}
