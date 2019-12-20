//
// Created by alexey on 30.09.2019.
//

#include <stdlib.h>
#include <stdio.h>

#ifndef STACK_STACK_H
#define STACK_STACK_H

#define CONSTRUCT_STACK(stack) stackConstruct(&stack, (char *) #stack);

#define USE_CANARIES

#define USE_HASH

typedef int elem_t;

const size_t DEFAULT_INIT_SIZE = 16;

const int SIZE_MULTIPLIER = 2;

const elem_t DEFAULT_POISON = 4417;

const bool ABORT_ON_STACK_CORRUPTION = true;

const bool STACK_CORRUPTION_SILENT = false;

const unsigned long HASH_SEED = 0x44174417;

extern const char *DUMP_PATH;

#ifdef USE_CANARIES
const unsigned int CANARY_STRUCT_SIZE = 4;

const unsigned int CANARY_STACK_SIZE = 4;

const elem_t CANARY_STRUCT_VALUE = 4417; // Arbitrary magic constant

const elem_t CANARY_STACK_VALUE = 5665; // Arbitrary magic constant
#endif


struct stack_t {
#ifdef USE_CANARIES
    int beginning_canary[CANARY_STRUCT_SIZE] = {};
#endif

    elem_t *data;
    elem_t poisonValue;
    size_t size;
    size_t maxsize;
    char *stackName;

#ifdef USE_HASH
    unsigned long int structHash;
    unsigned long int stackHash;
#endif

#ifdef USE_CANARIES
    int ending_canary[CANARY_STRUCT_SIZE] = {};
#endif
};

int stackConstruct(stack_t *stack, char *stackName, size_t size = DEFAULT_INIT_SIZE, elem_t poison = DEFAULT_POISON);

int stackPush(stack_t *stack, elem_t element);

int stackPop(stack_t *stack, elem_t *destination);

int stackExtend(stack_t *stack);

int stackOk(stack_t stack);

int
checkStackValidity(stack_t *stack, const char *dumpPath = DUMP_PATH, bool abortOnCorruption = ABORT_ON_STACK_CORRUPTION,
                   bool silent = STACK_CORRUPTION_SILENT);

int stackDestruct(stack_t *stack);

int stackDump(FILE *f, stack_t *stack, const char *prompt = (const char *) "", bool colored = true);

void updateHashes(stack_t *stk);

unsigned long getStackHash(stack_t *stk);

unsigned long getStructHash(stack_t *stk);

#endif //STACK_STACK_H

