#ifndef ARRAYS
#define ARRAYS

// * Inclusions
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// * Constant values
#define TABLE_SIZE 100

// * Structures
typedef struct Pair
{
    char *key;
    int value;
    struct Pair *next;
} Pair;

typedef struct
{
    Pair **entries;
} dict;

// * Functions
// Upgrading scanf
int input_int(const char *msg, const char *error_msg);
double input_double(const char *msg, const char *error_msg);
char input_char(const char *msg, const char *error_msg);

// Upgrading printf
int qprint(int value);

// Creating the Dict type using hashtables
unsigned int hash(const char *key);
dict *new_dict();
void dict_set(dict *table, const char *key, int value);
int dict_get(dict *table, const char *key, int *found);
void dict_rem(dict *table, const char *key);
void free_dict(dict *table);

#endif // ARRAYS