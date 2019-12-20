//
// Created by alexey on 30.09.2019.
//

#include "stack.h"
#include "MurMurHash3.h"
#include <assert.h>
#include <string.h>
#include <wchar.h>

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"


/**
 * Path to default dump file
 */
const char *DUMP_PATH = "stackDumps.txt";

/**
 * Stack constructor that initializes structure
 * @param stack Pointer to stack_t structure
 * @param size Desired size of stack
 * @param poisonValue Desired poison value
 * @return 0 if allocation error happened, 0 otherwise
 */

int stackConstruct(stack_t *stack, char *stackName, size_t size, elem_t poisonValue) {
    assert(stack);
    assert(size > 0);

    stack->maxsize = size;
#ifdef USE_CANARIES
    stack->data = (elem_t *) calloc(size + 2 * CANARY_STACK_SIZE, sizeof(elem_t));
#else
    stack->data = (elem_t *) calloc(size, sizeof(elem_t));
#endif
    stack->size = 0;
    stack->poisonValue = poisonValue;
    stack->stackName = stackName;

    if (stack->data == nullptr) return 0;

#ifdef USE_CANARIES
    for (int i = 0; i < CANARY_STRUCT_SIZE; i++) {
        stack->beginning_canary[i] = CANARY_STRUCT_VALUE;
        stack->ending_canary[i] = CANARY_STRUCT_VALUE;
    }

    for (int i = 0; i < CANARY_STACK_SIZE; i++) {
        stack->data[i] = CANARY_STACK_VALUE;
        stack->data[i + CANARY_STACK_SIZE + stack->maxsize] = CANARY_STACK_VALUE;
    }
#endif

#ifdef USE_HASH
    updateHashes(stack);
#endif

    checkStackValidity(stack);

    return 1;
}

/**
 * Function that pushes element to stack
 * @param stack Pointer to stack
 * @param element Element to push
 * @return 1 in case of success, 0 if allocation error happened and stack cannot be expanded
 */

int stackPush(stack_t *stack, elem_t element) {
    assert(stack);

    checkStackValidity(stack);

    if ((stack->size) >= (stack->maxsize)) {
        if (!stackExtend(stack)) return 0;
    }

#ifdef USE_CANARIES
    stack->data[CANARY_STACK_SIZE + stack->size++] = element;
#else
    stack->data[stack->size++] = element;
#endif

#ifdef USE_HASH
    updateHashes(stack);
#endif

    checkStackValidity(stack);

    return 1;
}

/**
 * Function that pops the element from stack
 * @param stack Pointer to stack
 * @param destination Where to put stack element
 * @return 1 if successful, 0 if no elements have left
 */

int stackPop(stack_t *stack, elem_t *destination) {
    assert(stack);
    assert(destination);

    checkStackValidity(stack);

    if (stack->size == 0) return 0;

#ifdef USE_CANARIES
    *destination = stack->data[CANARY_STACK_SIZE + --stack->size];
    stack->data[CANARY_STACK_SIZE + stack->size] = stack->poisonValue;
#else
    *destination = stack->data[--stack->size];
    stack->data[stack->size] = stack->poisonValue;
#endif

#ifdef USE_HASH
    updateHashes(stack);
#endif

    checkStackValidity(stack);

    return 1;
}

/**
 * Function that extends stack
 * @param stack Pointer to stack
 * @return 0 if allocation error happened, 1 otherwise
 */

int stackExtend(stack_t *stack) {
    assert(stack);

    checkStackValidity(stack);

    size_t newSize = stack->maxsize * SIZE_MULTIPLIER;
#ifdef USE_CANARIES
    auto *newPointer = (elem_t *) realloc(stack->data, (newSize + 2 * CANARY_STACK_SIZE) * sizeof(elem_t));
#else
    auto *newPointer = (elem_t *) realloc(stack->data, newSize * sizeof(elem_t));
#endif

    if (!newPointer) return 0;
    else {
        stack->data = newPointer;
#ifdef USE_CANARIES
        wmemset((wchar_t *) (stack->data + stack->maxsize + CANARY_STACK_SIZE), (wchar_t) (DEFAULT_POISON),
                (newSize - stack->maxsize) * sizeof(elem_t) / sizeof(wchar_t));

//        wmemset((wchar_t *) (stack->data + stack->maxsize), (wchar_t) (DEFAULT_POISON),
//                (newSize - stack->maxsize) * sizeof(elem_t) / sizeof(wchar_t));
#else
        wmemset((wchar_t *)(stack->data + stack->maxsize), (wchar_t)(DEFAULT_POISON), (newSize - stack->maxsize) * sizeof(elem_t) / sizeof(wchar_t));
#endif
        stack->maxsize = newSize;

#ifdef USE_CANARIES
        for (int i = 0; i < CANARY_STACK_SIZE; i++) {
            stack->data[i + CANARY_STACK_SIZE + stack->maxsize] = CANARY_STACK_VALUE;
        }
#endif

#ifdef USE_HASH
        updateHashes(stack);
#endif

        checkStackValidity(stack);

        return 1;
    }
}

/**
 * Checks whether given stack is valid or not
 * @param stack Stack to check
 * @return 1 if stack is OK, 0 if data pointer is 0, -1 if size>maxsize, -2 if maxsize = 0
 */

int stackOk(stack_t stack) {
    if (stack.data == nullptr) return 0;
    if (stack.size > stack.maxsize) return -1;
    if (stack.maxsize == 0) return -2;

#ifdef USE_CANARIES
    for (int i = 0; i < CANARY_STRUCT_SIZE; i++) {
        if (stack.beginning_canary[i] != CANARY_STRUCT_VALUE) return -3;
        if (stack.ending_canary[i] != CANARY_STRUCT_VALUE) return -4;
    }

    for (int i = 0; i < CANARY_STACK_SIZE; i++) {
        if (stack.data[i] != CANARY_STACK_VALUE) return -5;
        if (stack.data[i + CANARY_STACK_SIZE + stack.maxsize] != CANARY_STACK_VALUE) return -6;
    }
#endif

#ifdef USE_HASH
    unsigned long oldStackHash = stack.stackHash;
    unsigned long oldStructHash = stack.structHash;

    stack.stackHash = 0;
    stack.structHash = 0;
    if (getStackHash(&stack) != oldStackHash) return -7;
    if (getStructHash(&stack) != oldStructHash) return -8;
#endif

    return 1;
}

/**
 * Function that checks stack validity
 * @param stack Pointer to stack
 * @param dumpPath Path to dump file in case stack is corrupted
 * @param abortOnCorruption Wheter function should abort on error
 * @param silent Whether function should be silent of print info in an array
 * @return 1 if stack is valid, 0 if corrupted
 */

int checkStackValidity(stack_t *stack, const char *dumpPath, bool abortOnCorruption, bool silent) {
    int errno = 0;
    if ((errno = stackOk(*stack)) != 1) {
        if (!silent)
            printf(ANSI_COLOR_RED "Stack have been corrupted: error code: %d - see the %s file for stack dump\n" ANSI_COLOR_RESET,
                   errno, dumpPath);
        FILE *dumpFile = fopen(dumpPath, "at");
        stackDump(dumpFile, stack, (char *) "STACK CHECK FAILED");
        fclose(dumpFile);
        if (abortOnCorruption) {
            abort();
        }
        return 0;
    }
    return 1;
}

/**
 * Stack destructor
 * @param stack Pointer to stack
 * @return 1 if Ok, 0 otherwise
 */

int stackDestruct(stack_t *stack) {
    assert(stack);
    assert(stack->data);

    checkStackValidity(stack);

    stack->size = 0;
    stack->maxsize = 0;
    stack->poisonValue = 0;

#ifdef USE_CANARIES
    for (int i = 0; i < CANARY_STRUCT_SIZE; i++) {
        stack->beginning_canary[i] = 0;
        stack->ending_canary[i] = 0;
        memset(stack->data + 2 * CANARY_STACK_SIZE, 0, stack->size);
    }
#else
    memset(stack->data, 0, stack->size);
#endif

    free(stack->data);
    stack->data = nullptr;
    return 1;
}


/**
 * Saves stack dump to provided file
 * @param f Pointer to file
 * @param stack Pointer to stack
 * @param stack_name Name of stack for printing
 * @param prompt
 * @param colored
 * @return
 */
int stackDump(FILE *f, stack_t *stack, const char *prompt, bool colored) {
    assert(f);
    assert(stack);
    assert(prompt);

    fprintf(f, "%s\n", prompt);
    fprintf(f, "stack_t %s [%p] {\n", stack->stackName, stack);
    fprintf(f, "    size = %zu;\n    poison = %d;\n", stack->size, stack->poisonValue);

#ifdef USE_HASH
    fprintf(f, "    structHash = %lu;\n    stackHash = %lu;\n", stack->structHash, stack->stackHash);
#endif

    fprintf(f, "    data[%zu] = [%p]; {\n", stack->maxsize, stack->data);

    size_t offset = 0;

#ifdef USE_CANARIES
    offset = CANARY_STACK_SIZE;
#endif

    char customCh = '*';
    for (size_t i = offset; i < stack->maxsize + offset; i++) {
        if (i - offset < stack->size) {
            if (colored)
                if (stack->data[i] == stack->poisonValue) fprintf(f, ANSI_COLOR_RED);
                else fprintf(f, ANSI_COLOR_GREEN);
            customCh = '*';
        } else {
            if (colored) fprintf(f, ANSI_COLOR_YELLOW);
            customCh = ' ';
        }
        fprintf(f, "      %c [%zu] = %d", customCh, i - offset, stack->data[i]);
        if (stack->data[i] == stack->poisonValue)
            fprintf(f, " [POISON]\n" ANSI_COLOR_RESET);
        else
            fprintf(f, "\n" ANSI_COLOR_RESET);
    }
    fprintf(f, "    }\n}\n");
    return 1;
}


#ifdef USE_HASH

void updateHashes(stack_t *stk) {
    assert(stk);
    assert(stk->data);

    stk->structHash = 0;
    stk->stackHash = 0;

    unsigned long newStackHash = getStackHash(stk);
    unsigned long newStructHash = getStructHash(stk);

    stk->stackHash = newStackHash;
    stk->structHash = newStructHash;
}

unsigned long getStackHash(stack_t *stk) {
#ifdef USE_CANARIES
    return MurMurHash3_32(stk->data, stk->maxsize + 2 * CANARY_STACK_SIZE, HASH_SEED);
#else
    return MurMurHash3_32(stk->data, stk->maxsize, HASH_SEED);
#endif
}

unsigned long getStructHash(stack_t *stk) {
    return MurMurHash3_32(stk, sizeof(stack_t), HASH_SEED);
}

#endif

