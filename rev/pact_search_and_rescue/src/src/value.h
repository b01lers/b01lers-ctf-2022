#ifndef clox_value_h
#define clox_value_h

#include "common.h"

typedef struct Obj Obj;
typedef struct ObjString ObjString;

typedef enum {
  VAL_BOOL,
  VAL_NIL,
  VAL_CHARACTER,
  VAL_INTEGER,
  VAL_FLOAT,
  VAL_OBJ,
} ValueType;

typedef struct {
  ValueType type;
  union {
    bool boolean;
    char character;
    long integer;
    double floating;
    Obj *obj;
  } as;
} Value;

#define BOOL_VAL(value) ((Value){VAL_BOOL, {.boolean = value}})
#define NIL_VAL ((Value){VAL_NIL, {.integer = 0}})
#define CHAR_VAL(value) ((Value){VAL_CHARACTER, {.character = value}})
#define INTEGER_VAL(value) ((Value){VAL_INTEGER, {.integer = value}})
#define FLOAT_VAL(value) ((Value){VAL_FLOAT, {.floating = value}})
#define OBJ_VAL(object) ((Value){VAL_OBJ, {.obj = (Obj *)object}})

#define AS_BOOL(value) ((value).as.boolean)
#define AS_INTEGER(value) ((value).as.integer)
#define AS_FLOATING(value) ((value).as.floating)
#define AS_CHARACTER(value) ((value).as.character)
#define AS_OBJ(value) ((value).as.obj)

#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NIL(value) ((value).type == VAL_NIL)
#define IS_CHARACTER(value) ((value).type == VAL_CHARACTER)
#define IS_NUMBER(value) ((value).type == VAL_INTEGER || (value).type == VAL_FLOAT)
#define IS_INTEGER(value) ((value).type == VAL_INTEGER)
#define IS_FLOATING(value) ((value).type == VAL_FLOAT)
#define IS_OBJ(value) ((value).type == VAL_OBJ)

typedef struct {
  int capacity;
  int count;
  Value *values;
} ValueArray;

bool valuesEqual(Value a, Value b);
void initValueArray(ValueArray *array);
void writeValueArray(ValueArray *array, Value value);
void freeValueArray(ValueArray *array);
void printValue(Value value);

#endif
