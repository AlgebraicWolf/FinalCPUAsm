#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include "MyAsm.h"

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

const unsigned short MAX_CMD_SIZE = 33;
const unsigned short MAX_SARG_SIZE = 33;

struct label_t {
    char *label;
    int pos;
};

int parseParams(int argc, char *argv[], char **filename);

int loadFile(FILE **f, const char *loadpath, const char *mode);

label_t *findLabel(label_t *labels, char *name);

unsigned long fileSize(FILE *f);

int countLines(const char *txt, const char delimiter);

char *concatenate(const char *str1, const char *str2);

char *makeName(char *original, char *ext);

int parseRegister(const char *reg);

char *
generateMachineCode(FILE *source, int lines, int *fileSize, label_t *labels = NULL);

const char *DEFAULT_FILENAME = "output.asm";

const int DEFAULT_FILENAME_LENGTH = 9;

enum argumentTypes {
    NONE,
    NUMBER,
    REGISTER,
    RAM_IMMED,
    RAM_REG,
    RAM_REG_IMMED,
    LABEL
};

const int REGS_NUM = 4;
const int REGNAME_LENGTH = 3;
const int MAX_ARGS_NUM = 2;

int main(int argc, char *argv[]) {
    char *filename = nullptr;

    if (parseParams(argc, argv, &filename) == -1) return -1;

    printf(ANSI_COLOR_BLUE "Compiling file %s\n" ANSI_COLOR_RESET, filename);

    FILE *source = {};

    if (!loadFile(&source, filename, "r")) {
        printf(ANSI_COLOR_RED "Error while loading the file. Check the filename and permissions. Terminating...\n" ANSI_COLOR_RESET);
        return -1;
    }

    int size = fileSize(source);
    char *sourceCode = (char *) calloc(size + 1, sizeof(char));
    fread(sourceCode, sizeof(char), size, source);
    fseek(source, 0, SEEK_SET);

    int lines = countLines(sourceCode, '\n');
    int fSize = 0;
    char *bin = generateMachineCode(source, lines, &fSize);
    char *outputName = makeName(filename, (char *) ".bin");


    FILE *output = fopen(outputName, "wb"); // TODO make separate function
    fwrite(bin, sizeof(char), fSize, output);

    fclose(output);
    fclose(source);

    free(bin);
    free(sourceCode);
    free(filename);
    free(outputName);

    return 0;
}

char *makeName(char *original, char *ext) {
    assert(original);
    assert(ext);

    char *dotPosition = strrchr(original, '.');
    char *newName = nullptr;

    if (dotPosition) {
        *dotPosition = '\0';
        newName = concatenate(original, ext);
        *dotPosition = '.';
    } else {
        newName = concatenate(original, ext);
    }

    return newName;
}

int parseRegister(const char *reg) {
    char registers[REGS_NUM][REGNAME_LENGTH] = {"ax", "bx", "cx", "dx"};
    assert(reg);

    for (int i = 0; i < REGS_NUM; i++)
        if (strcmp(reg, registers[i]) == 0)
            return i;

    return -1;
}

char *concatenate(const char *str1, const char *str2) {
    assert(str1);
    assert(str2);

    const char *end1 = strchr(str1, '\0');
    const char *end2 = strchr(str2, '\0');

    if ((end1 == nullptr) || (end2 == nullptr)) return nullptr;
    int len = (end1 - str1) + (end2 - str2) + 1;
    auto concatenated = (char *) calloc(len, sizeof(char));

    strcpy(concatenated, str1);
    strcat(concatenated, str2);

    return concatenated;
}

int processNumberArgument(char **machineCode, const char *sarg) {
    assert(machineCode);
    assert(*machineCode);
    assert(sarg);

    int arg = atoi(sarg);
    *((int *) (*machineCode)) = arg;
    *machineCode = (char *) ((int *) (*machineCode) + 1);

    return 0;
}

int processRegisterArgument(char **machineCode, const char *sarg) {
    assert(machineCode);
    assert(*machineCode);
    assert(sarg);

    int arg = parseRegister(sarg);
    if (arg == -1) {
        printf(ANSI_COLOR_RED "Invalid register name provided. Terminating...\n" ANSI_COLOR_RESET);
        return -1;
    }
    *((int *) (*machineCode)) = arg;
    *machineCode = (char *) ((int *) (*machineCode) + 1);

    return 0;
}

int processMixedRAM(char **machineCode, const char *sarg) {
    assert(machineCode);
    assert(*machineCode);
    assert(sarg);

    int arg = 0;
    char ram_sarg[MAX_SARG_SIZE] = "";

    if (strchr(sarg, '+')) {
        if (sscanf(sarg, "[%[a-z]+%d]", ram_sarg, &arg) == EOF) {
            printf(ANSI_COLOR_RED "Invalid RAM address declaration. Terminating..." ANSI_COLOR_RESET);
            return -1;
        }
    } else if (sarg, '-') {
        if (sscanf(sarg, "[%[a-z]-%d]", ram_sarg, &arg) == EOF) {
            printf(ANSI_COLOR_RED "Invalid RAM address declaration. Terminating..." ANSI_COLOR_RESET);
            return -1;
        }
        arg *= -1;
    } else {
        printf(ANSI_COLOR_RED"Invalid register + immediate RAM value. Terminating...\n" ANSI_COLOR_RESET);
        return -1;
    }

    processRegisterArgument(machineCode, ram_sarg);
    *((int *) (*machineCode)) = arg;
    *machineCode = (char *) ((int *) (*machineCode) + 1);
}

int addLabel(label_t *labels, char *cmd, char *end, int len) {
    *end = '\0';
    auto name = (char *) calloc(end - cmd, sizeof(char));
    strcpy(name, cmd);
    *end = ':';
    label_t *curLabel = findLabel(labels, name);

    if(curLabel->label) {
        printf(ANSI_COLOR_RED "Label already defined. Terminating...\n" ANSI_COLOR_RESET);
        return 0;
    }
    curLabel->pos = len;
    curLabel->label = name;
    return 1;
}

int processLabel(char **machineCode, const label_t *labels, const char *sarg) {
    assert(machineCode);
    assert(*machineCode);
    assert(labels);
    assert(sarg);

    bool found = false;
    while (labels->label) {
        if (strcmp(labels->label, sarg) == 0) {
            *((int *) (*machineCode)) = labels->pos;
            return 0;
        }
        labels++;
    }

    printf(ANSI_COLOR_RED "Label \"%s\" not found. Terminating...\n" ANSI_COLOR_RESET, sarg);
    return -1;
}

char *
defineCommandOverload(char **machine_code, int *len, int opcode, argumentTypes argtype, bool parseLabels, char *sarg,
                      label_t *labels) {
    assert(machine_code);
    assert(*machine_code);
    assert(len);

    char ram_sarg[MAX_SARG_SIZE] = "";
    int arg = 0;
    int arg2 = 0;

    **machine_code = opcode;
    (*machine_code)++;
    *len += sizeof(char);
    switch (argtype) {
        case NUMBER:
            processNumberArgument(machine_code, sarg);
            *len += sizeof(int);
            break;
        case REGISTER:
            if (processRegisterArgument(machine_code, sarg) == -1)
                return nullptr;
            *len += sizeof(int);
            break;
        case RAM_REG:
            if (sscanf(sarg, "[%[a-z]]", ram_sarg) != EOF) {
                if (processRegisterArgument(machine_code, sarg) == -1)
                    return nullptr;
                *len += sizeof(int);
            } else {
                printf(ANSI_COLOR_RED "Invalid RAM address declaration. Terminating..." ANSI_COLOR_RESET);
                return nullptr;
            }
            break;
        case RAM_IMMED:
            if (sscanf(sarg, "[%d]", &arg) != EOF) {
                processNumberArgument(machine_code, sarg);
                *len += sizeof(int);
            } else {
                printf(ANSI_COLOR_RED "Invalid RAM address declaration. Terminating..." ANSI_COLOR_RESET);
                return nullptr;
            }
            break;
        case NONE:
            break;
        case RAM_REG_IMMED:
            if (processMixedRAM(machine_code, sarg) == -1) return nullptr;
            *len += 2 * sizeof(int);
            break;
        case LABEL:
            if (!parseLabels) {
                if (processLabel(machine_code, labels, sarg) == -1)
                    return nullptr;
            } else {
                *((int *) (*machine_code)) = 0;
            }
            (*machine_code) = (char *) ((int *) (*machine_code) + 1);
            *len += sizeof(int);
    }

    return (char *) 1;
}

label_t *findLabel(label_t *labels, char *name) {
    assert(labels);
    assert(name);

    while (labels->label != nullptr) {
        if (strcmp(labels->label, name) == 0) {
            return labels;
        }
        labels++;
    }

    return labels;
}

char *generateMachineCode(FILE *sourceFile, int lines, int *fileSize, label_t *labels) {
    assert(sourceFile);
    assert(fileSize);

    auto machine_code = (char *) calloc(sizeof(char) + MAX_ARGS_NUM * sizeof(int),
                                        lines);

    bool parseLabels = false;
    if (!labels) {
        parseLabels = true;
        labels = (label_t *) calloc(lines + 1, sizeof(label_t));
    }

    char *machine_code_start = machine_code;

    char cmd[MAX_CMD_SIZE] = "";
    char sarg[MAX_SARG_SIZE] = "";

    int len = 0;

    for (int i = 0; i < lines; i++) {
        if (fscanf(sourceFile, "%s", cmd) == 0) break;

#define CMD_OVRLD(opcode, cond, argtype, execcode) \
        if (cond) defineCommandOverload(&machine_code, &len, opcode, argtype, parseLabels, sarg, labels); \
        else

#define DEF_CMD(name, args, overloaders) \
    if(strcmp(cmd, #name) == 0) {\
        if(args) \
            if(!fscanf(sourceFile, "%s", sarg)) {\
            printf(ANSI_COLOR_RED "Invalid number of arguments!\n" ANSI_COLOR_RESET);\
            return nullptr;\
         } \
        overloaders \
        {\
            printf(ANSI_COLOR_RED "Invalid argument parameter %s for command %s in line %d. Terminating...\n" ANSI_COLOR_RESET, sarg, cmd, i + 1); \
            return nullptr; \
        } \
    }\
    else

#include "../commands.h"
        {
            char *end = strchr(cmd, ':');
            if (end && parseLabels)
                if (!addLabel(labels, cmd, end, len))
                    return nullptr;
        }

#undef DEF_CMD
#undef CMD_OVRLD

        fscanf(sourceFile, "\n");
        memset(sarg, 0, MAX_SARG_SIZE);
        memset(cmd, 0, MAX_CMD_SIZE);
    }

    *fileSize = sizeof(char) * len;
    machine_code = (char *) realloc(machine_code_start, *fileSize);
    rewind(sourceFile);

    if (!parseLabels) {
        free(labels);
        return machine_code;
    } else
        return generateMachineCode(sourceFile, lines, fileSize, labels);
}

int parseParams(int argc, char *argv[], char **filename) {
    assert(filename);
    if (argc == 1) {
        printf(ANSI_COLOR_YELLOW "Neither file nor compiler type specified. Using default parameters\n" ANSI_COLOR_RESET);
        *filename = (char *) calloc(DEFAULT_FILENAME_LENGTH + 1, sizeof(char));
        strcpy(*filename, DEFAULT_FILENAME);
    } else if (argc == 2) {
        *filename = (char *) calloc(strlen(argv[1]) + 1, sizeof(char));
        strcpy(*filename, argv[1]);
    } else {
        printf(ANSI_COLOR_RED "Invalid number of arguments. Terminating...\n" ANSI_COLOR_RESET);
        return -1;
    }

    return 0;
}

int loadFile(FILE **f, const char *loadpath, const char *mode) {
    assert(f);
    assert(loadpath);
    assert(*loadpath);
    assert(mode);
    assert(*mode);

    *f = fopen(loadpath, mode);
    return *f != nullptr;
}

unsigned long fileSize(FILE *f) {
    assert(f);

    fseek(f, 0, SEEK_END);
    unsigned long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    return size;
}

int countLines(const char *txt, const char delimiter) {
    assert(txt);

    const char *pointer = NULL;
    int lines = 0;
    while ((pointer = strchr(txt, delimiter)) != nullptr) {
        ++lines;
        txt = pointer + 1;
    }
    ++lines;

    return lines;
}