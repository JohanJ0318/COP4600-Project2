#include "chash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Insert a record into the hash table
void insert_record(hashRecord **hash_table, const char *name, uint32_t salary,
                   pthread_rwlock_t *rw_lock, FILE *log_file, pthread_mutex_t *log_mutex, int priority) {
    uint32_t hash = jenkins_one_at_a_time_hash(name);
    
    // Log the command
    pthread_mutex_lock(log_mutex);
    fprintf(log_file, "%lld: THREAD %d INSERT,%u,%s,%u\n", 
            current_timestamp(), priority, hash, name, salary);
    fflush(log_file);
    pthread_mutex_unlock(log_mutex);
    
    // Acquire write lock
    pthread_mutex_lock(log_mutex);
    fprintf(log_file, "%lld: THREAD %d WRITE LOCK ACQUIRED\n", 
            current_timestamp(), priority);
    fflush(log_file);
    pthread_mutex_unlock(log_mutex);
    
    pthread_rwlock_wrlock(rw_lock);
    
    // Check for duplicates
    hashRecord *current = *hash_table;
    while (current != NULL) {
        if (current->hash == hash && strcmp(current->name, name) == 0) {
            pthread_rwlock_unlock(rw_lock);
            
            pthread_mutex_lock(log_mutex);
            fprintf(log_file, "%lld: THREAD %d WRITE LOCK RELEASED\n", 
                    current_timestamp(), priority);
            fflush(log_file);
            pthread_mutex_unlock(log_mutex);
            
            printf("Insert failed. Entry %u is a duplicate.\n", hash);
            return;
        }
        current = current->next;
    }
    
    // Create new node
    hashRecord *new_node = (hashRecord *)malloc(sizeof(hashRecord));
    new_node->hash = hash;
    strncpy(new_node->name, name, 49);
    new_node->name[49] = '\0';
    new_node->salary = salary;
    new_node->next = *hash_table;
    *hash_table = new_node;
    
    pthread_rwlock_unlock(rw_lock);
    
    // Log lock release
    pthread_mutex_lock(log_mutex);
    fprintf(log_file, "%lld: THREAD %d WRITE LOCK RELEASED\n", 
            current_timestamp(), priority);
    fflush(log_file);
    pthread_mutex_unlock(log_mutex);
    
    printf("Inserted: %u, %s, %u\n", hash, name, salary);
}

// Delete a record from the hash table
void delete_record(hashRecord **hash_table, const char *name,
                   pthread_rwlock_t *rw_lock, FILE *log_file, pthread_mutex_t *log_mutex, int priority) {
    uint32_t hash = jenkins_one_at_a_time_hash(name);
    
    // Log the command
    pthread_mutex_lock(log_mutex);
    fprintf(log_file, "%lld: THREAD %d DELETE,%u,%s\n", 
            current_timestamp(), priority, hash, name);
    fflush(log_file);
    pthread_mutex_unlock(log_mutex);
    
    // Acquire write lock
    pthread_mutex_lock(log_mutex);
    fprintf(log_file, "%lld: THREAD %d WRITE LOCK ACQUIRED\n", 
            current_timestamp(), priority);
    fflush(log_file);
    pthread_mutex_unlock(log_mutex);
    
    pthread_rwlock_wrlock(rw_lock);
    
    hashRecord *current = *hash_table;
    hashRecord *prev = NULL;
    
    while (current != NULL) {
        if (current->hash == hash && strcmp(current->name, name) == 0) {
            // Found the record to delete
            if (prev == NULL) {
                *hash_table = current->next;
            } else {
                prev->next = current->next;
            }
            
            printf("Deleted record for %u, %s, %u\n", current->hash, current->name, current->salary);
            free(current);
            
            pthread_rwlock_unlock(rw_lock);
            
            pthread_mutex_lock(log_mutex);
            fprintf(log_file, "%lld: THREAD %d WRITE LOCK RELEASED\n", 
                    current_timestamp(), priority);
            fflush(log_file);
            pthread_mutex_unlock(log_mutex);
            
            return;
        }
        prev = current;
        current = current->next;
    }
    
    pthread_rwlock_unlock(rw_lock);
    
    pthread_mutex_lock(log_mutex);
    fprintf(log_file, "%lld: THREAD %d WRITE LOCK RELEASED\n", 
            current_timestamp(), priority);
    fflush(log_file);
    pthread_mutex_unlock(log_mutex);
    
    printf("Entry %u not deleted. Not in database.\n", hash);
}

// Update salary for a record
void update_salary(hashRecord **hash_table, const char *name, uint32_t new_salary,
                   pthread_rwlock_t *rw_lock, FILE *log_file, pthread_mutex_t *log_mutex, int priority) {
    uint32_t hash = jenkins_one_at_a_time_hash(name);
    
    // Log the command
    pthread_mutex_lock(log_mutex);
    fprintf(log_file, "%lld: THREAD %d UPDATE,%u,%s,%u\n", 
            current_timestamp(), priority, hash, name, new_salary);
    fflush(log_file);
    pthread_mutex_unlock(log_mutex);
    
    // Acquire write lock
    pthread_mutex_lock(log_mutex);
    fprintf(log_file, "%lld: THREAD %d WRITE LOCK ACQUIRED\n", 
            current_timestamp(), priority);
    fflush(log_file);
    pthread_mutex_unlock(log_mutex);
    
    pthread_rwlock_wrlock(rw_lock);
    
    hashRecord *current = *hash_table;
    
    while (current != NULL) {
        if (current->hash == hash && strcmp(current->name, name) == 0) {
            uint32_t old_salary = current->salary;
            current->salary = new_salary;
            
            pthread_rwlock_unlock(rw_lock);
            
            pthread_mutex_lock(log_mutex);
            fprintf(log_file, "%lld: THREAD %d WRITE LOCK RELEASED\n", 
                    current_timestamp(), priority);
            fflush(log_file);
            pthread_mutex_unlock(log_mutex);
            
            printf("Updated record %u from %u, %s, %u to %u, %s, %u\n",
                   hash, hash, name, old_salary, hash, name, new_salary);
            return;
        }
        current = current->next;
    }
    
    pthread_rwlock_unlock(rw_lock);
    
    pthread_mutex_lock(log_mutex);
    fprintf(log_file, "%lld: THREAD %d WRITE LOCK RELEASED\n", 
            current_timestamp(), priority);
    fflush(log_file);
    pthread_mutex_unlock(log_mutex);
    
    printf("Update failed. Entry %u not found.\n", hash);
}

// Search for a record
void search_record(hashRecord **hash_table, const char *name,
                   pthread_rwlock_t *rw_lock, FILE *log_file, pthread_mutex_t *log_mutex, int priority) {
    uint32_t hash = jenkins_one_at_a_time_hash(name);
    
    // Log the command
    pthread_mutex_lock(log_mutex);
    fprintf(log_file, "%lld: THREAD %d SEARCH,%u,%s\n", 
            current_timestamp(), priority, hash, name);
    fflush(log_file);
    pthread_mutex_unlock(log_mutex);
    
    // Acquire read lock
    pthread_mutex_lock(log_mutex);
    fprintf(log_file, "%lld: THREAD %d READ LOCK ACQUIRED\n", 
            current_timestamp(), priority);
    fflush(log_file);
    pthread_mutex_unlock(log_mutex);
    
    pthread_rwlock_rdlock(rw_lock);
    
    hashRecord *current = *hash_table;
    
    while (current != NULL) {
        if (current->hash == hash && strcmp(current->name, name) == 0) {
            printf("Found: %u, %s, %u\n", current->hash, current->name, current->salary);
            
            pthread_rwlock_unlock(rw_lock);
            
            pthread_mutex_lock(log_mutex);
            fprintf(log_file, "%lld: THREAD %d READ LOCK RELEASED\n", 
                    current_timestamp(), priority);
            fflush(log_file);
            pthread_mutex_unlock(log_mutex);
            
            return;
        }
        current = current->next;
    }
    
    pthread_rwlock_unlock(rw_lock);
    
    pthread_mutex_lock(log_mutex);
    fprintf(log_file, "%lld: THREAD %d READ LOCK RELEASED\n", 
            current_timestamp(), priority);
    fflush(log_file);
    pthread_mutex_unlock(log_mutex);
    
    printf("Not Found: %s not found.\n", name);
}

// Comparison function for qsort
static int compare_records(const void *a, const void *b) {
    hashRecord *rec_a = *(hashRecord **)a;
    hashRecord *rec_b = *(hashRecord **)b;
    
    if (rec_a->hash < rec_b->hash) return -1;
    if (rec_a->hash > rec_b->hash) return 1;
    return 0;
}

// Print the entire hash table sorted by hash value
void print_table(hashRecord **hash_table, pthread_rwlock_t *rw_lock,
                 FILE *log_file, pthread_mutex_t *log_mutex, int priority) {
    // Log the command
    pthread_mutex_lock(log_mutex);
    fprintf(log_file, "%lld: THREAD %d PRINT\n", 
            current_timestamp(), priority);
    fflush(log_file);
    pthread_mutex_unlock(log_mutex);
    
    // Acquire read lock
    pthread_mutex_lock(log_mutex);
    fprintf(log_file, "%lld: THREAD %d READ LOCK ACQUIRED\n", 
            current_timestamp(), priority);
    fflush(log_file);
    pthread_mutex_unlock(log_mutex);
    
    pthread_rwlock_rdlock(rw_lock);
    
    // Count records
    int count = 0;
    hashRecord *current = *hash_table;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    
    if (count == 0) {
        printf("Current Database:\n(empty)\n");
        pthread_rwlock_unlock(rw_lock);
        
        pthread_mutex_lock(log_mutex);
        fprintf(log_file, "%lld: THREAD %d READ LOCK RELEASED\n", 
                current_timestamp(), priority);
        fflush(log_file);
        pthread_mutex_unlock(log_mutex);
        
        return;
    }
    
    // Create array for sorting
    hashRecord **records = (hashRecord **)malloc(count * sizeof(hashRecord *));
    current = *hash_table;
    int i = 0;
    while (current != NULL) {
        records[i++] = current;
        current = current->next;
    }
    
    // Sort by hash value
    qsort(records, count, sizeof(hashRecord *), compare_records);
    
    printf("Current Database:\n");
    for (i = 0; i < count; i++) {
        printf("%u, %s, %u\n", records[i]->hash, records[i]->name, records[i]->salary);
    }
    
    free(records);
    
    pthread_rwlock_unlock(rw_lock);
    
    pthread_mutex_lock(log_mutex);
    fprintf(log_file, "%lld: THREAD %d READ LOCK RELEASED\n", 
            current_timestamp(), priority);
    fflush(log_file);
    pthread_mutex_unlock(log_mutex);
}

// Free all memory in the hash table
void free_hash_table(hashRecord **hash_table) {
    hashRecord *current = *hash_table;
    while (current != NULL) {
        hashRecord *next = current->next;
        free(current);
        current = next;
    }
    *hash_table = NULL;
}
