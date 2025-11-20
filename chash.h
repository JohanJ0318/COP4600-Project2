#ifndef CHASH_H
#define CHASH_H

#include <stdint.h>
#include <pthread.h>
#include <stdio.h>

// Hash table node structure
typedef struct hash_struct {
    uint32_t hash;
    char name[50];
    uint32_t salary;
    struct hash_struct *next;
} hashRecord;

// Command types
typedef enum {
    CMD_INSERT,
    CMD_DELETE,
    CMD_UPDATE,
    CMD_SEARCH,
    CMD_PRINT
} CommandType;

// Command structure
typedef struct {
    CommandType type;
    char name[50];
    uint32_t salary;
    int priority;
} Command;

// Thread argument structure
typedef struct {
    Command cmd;
    hashRecord **hash_table;
    pthread_rwlock_t *rw_lock;
    pthread_mutex_t *priority_mutex;
    pthread_cond_t *priority_cond;
    int *current_priority;
    FILE *log_file;
    pthread_mutex_t *log_mutex;
} ThreadArg;

// Function declarations
uint32_t jenkins_one_at_a_time_hash(const char *key);
long long current_timestamp();
void insert_record(hashRecord **hash_table, const char *name, uint32_t salary, 
                   pthread_rwlock_t *rw_lock, FILE *log_file, pthread_mutex_t *log_mutex, int priority);
void delete_record(hashRecord **hash_table, const char *name, 
                   pthread_rwlock_t *rw_lock, FILE *log_file, pthread_mutex_t *log_mutex, int priority);
void update_salary(hashRecord **hash_table, const char *name, uint32_t new_salary,
                   pthread_rwlock_t *rw_lock, FILE *log_file, pthread_mutex_t *log_mutex, int priority);
void search_record(hashRecord **hash_table, const char *name,
                   pthread_rwlock_t *rw_lock, FILE *log_file, pthread_mutex_t *log_mutex, int priority);
void print_table(hashRecord **hash_table, pthread_rwlock_t *rw_lock, 
                 FILE *log_file, pthread_mutex_t *log_mutex, int priority);
void *thread_function(void *arg);
void free_hash_table(hashRecord **hash_table);

#endif
