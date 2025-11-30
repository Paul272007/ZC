#include "utils.h"

/**
 * @brief Upgraded input functions with error handling, returning the correct type.
 * @param msg The message to display to the user
 * @param error_msg The error message to display in case of invalid input
 * @return The input value of the correct type
 */
int input_int(const char *msg, const char *error_msg)
{
    int nb;

    printf(msg);

    while (scanf("%d", &nb) != 1)
    {
        while (getchar() != '\n')
        {
            /* code */
        }
        printf("%s", error_msg);
    }

    while (getchar() != '\n')
    {
        /* code */
    }

    return nb;
}

/**
 * @brief Upgraded input functions with error handling, returning the correct type.
 * @param msg The message to display to the user
 * @param error_msg The error message to display in case of invalid input
 * @return The input value of the correct type
 */
double input_double(const char *msg, const char *error_msg)
{
    double nb;

    printf(msg);

    while (scanf("%lf", &nb) != 1)
    {
        while (getchar() != '\n')
        {
            /* code */
        }
        printf("%s", error_msg);
    }

    while (getchar() != '\n')
    {
        /* code */
    }

    return nb;
}

/**
 * @brief Upgraded input functions with error handling, returning the correct type.
 * @param msg The message to display to the user
 * @param error_msg The error message to display in case of invalid input
 * @return The input value of the correct type
 */
char input_char(const char *msg, const char *error_msg)
{
    char res;

    printf(msg);

    while (scanf("%c", &res) != 1)
    {
        while (getchar() != '\n')
        {
            /* code */
        }
        printf("%s", error_msg);
    }

    while (getchar() != '\n')
    {
        /* code */
    }

    return res;
}

/**
 * Shortcut for simply displaying an integer value
 * @param value The value to be printed
 */
int qprint(int value)
{
    return printf("%d\n", value);
}

unsigned int hash(const char *key)
{
    unsigned long int hash_value = 0;
    for (int i = 0; key[i] != '\0'; i++)
    {
        hash_value = hash_value * 31 + key[i];
    }
    return hash_value % TABLE_SIZE;
}

dict *new_dict()
{
    dict *table = malloc(sizeof(dict));
    table->entries = calloc(TABLE_SIZE, sizeof(Pair *));
    return table;
}

void dict_set(dict *table, const char *key, int value)
{
    unsigned int index = hash(key);
    Pair *entry = table->entries[index];

    // Check if the key already exists
    while (entry != NULL)
    {
        if (strcmp(entry->key, key) == 0)
        {
            entry->value = value;
            return;
        }
        entry = entry->next;
    }

    // If the key doesn't exist, we create it
    Pair *new_entry = malloc(sizeof(Pair));
    new_entry->key = strdup(key);
    new_entry->value = value;
    new_entry->next = table->entries[index];
    table->entries[index] = new_entry;
}

int dict_get(dict *table, const char *key, int *found)
{
    unsigned int index = hash(key);
    Pair *entry = table->entries[index];

    while (entry != NULL)
    {
        if (strcmp(entry->key, key) == 0)
        {
            *found = 1;
            return entry->value;
        }
        entry = entry->next;
    }

    *found = 0;
    return 0;
}

void dict_rem(dict *table, const char *key)
{
    unsigned int index = hash(key);
    Pair *entry = table->entries[index];
    Pair *prev = NULL;

    while (entry != NULL)
    {
        if (strcmp(entry->key, key) == 0)
        {
            if (prev == NULL)
            {
                table->entries[index] = entry->next;
            }
            else
            {
                prev->next = entry->next;
            }
            free(entry->key);
            free(entry);
            return;
        }
        prev = entry;
        entry = entry->next;
    }
}

void free_dict(dict *table)
{
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        Pair *entry = table->entries[i];
        while (entry != NULL)
        {
            Pair *next = entry->next;
            free(entry->next);
            free(entry);
            entry = next;
        }
    }
    free(table->entries);
    free(table);
}