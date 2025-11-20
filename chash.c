#include "chash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

// Thread function that executes commands
void *thread_function(void *arg) {
    ThreadArg *targ = (ThreadArg *)arg;
    
    // Wait for our turn based on priority
    pthread_mutex_lock(targ->priority_mutex);
    
    pthread_mutex_lock(targ->log_mutex);
    fprintf(targ->log_file, "%lld: THREAD %d WAITING FOR MY TURN\n", 
            current_timestamp(), targ->cmd.priority);
    fflush(targ->log_file);
    pthread_mutex_unlock(targ->log_mutex);
    
    while (*targ->current_priority != targ->cmd.priority) {
        pthread_cond_wait(targ->priority_cond, targ->priority_mutex);
    }
    
    pthread_mutex_lock(targ->log_mutex);
    fprintf(targ->log_file, "%lld: THREAD %d AWAKENED FOR WORK\n", 
            current_timestamp(), targ->cmd.priority);
    fflush(targ->log_file);
    pthread_mutex_unlock(targ->log_mutex);
    
    pthread_mutex_unlock(targ->priority_mutex);
    
    // Execute the command
    switch (targ->cmd.type) {
        case CMD_INSERT:
            insert_record(targ->hash_table, targ->cmd.name, targ->cmd.salary,
                         targ->rw_lock, targ->log_file, targ->log_mutex, targ->cmd.priority);
            break;
        case CMD_DELETE:
            delete_record(targ->hash_table, targ->cmd.name,
                         targ->rw_lock, targ->log_file, targ->log_mutex, targ->cmd.priority);
            break;
        case CMD_UPDATE:
            update_salary(targ->hash_table, targ->cmd.name, targ->cmd.salary,
                         targ->rw_lock, targ->log_file, targ->log_mutex, targ->cmd.priority);
            break;
        case CMD_SEARCH:
            search_record(targ->hash_table, targ->cmd.name,
                         targ->rw_lock, targ->log_file, targ->log_mutex, targ->cmd.priority);
            break;
        case CMD_PRINT:
            print_table(targ->hash_table, targ->rw_lock,
                       targ->log_file, targ->log_mutex, targ->cmd.priority);
            break;
    }
    
    // Signal next thread
    pthread_mutex_lock(targ->priority_mutex);
    (*targ->current_priority)++;
    pthread_cond_broadcast(targ->priority_cond);
    pthread_mutex_unlock(targ->priority_mutex);
    
    return NULL;
}

// Parse a command line from the file
int parse_command(const char *line, Command *cmd) {
    char cmd_type[20];
    char buffer[200];
    strncpy(buffer, line, 199);
    buffer[199] = '\0';
    
    // Remove newline
    char *newline = strchr(buffer, '\n');
    if (newline) *newline = '\0';
    
    char *token = strtok(buffer, ",");
    if (!token) return 0;
    
    strncpy(cmd_type, token, 19);
    cmd_type[19] = '\0';
    
    if (strcmp(cmd_type, "insert") == 0) {
        cmd->type = CMD_INSERT;
        token = strtok(NULL, ",");
        if (!token) return 0;
        strncpy(cmd->name, token, 49);
        cmd->name[49] = '\0';
        
        token = strtok(NULL, ",");
        if (!token) return 0;
        cmd->salary = atoi(token);
        
        token = strtok(NULL, ",");
        if (!token) return 0;
        cmd->priority = atoi(token);
        
    } else if (strcmp(cmd_type, "delete") == 0) {
        cmd->type = CMD_DELETE;
        token = strtok(NULL, ",");
        if (!token) return 0;
        strncpy(cmd->name, token, 49);
        cmd->name[49] = '\0';
        
        token = strtok(NULL, ",");
        if (!token) return 0;
        cmd->priority = atoi(token);
        
    } else if (strcmp(cmd_type, "update") == 0) {
        cmd->type = CMD_UPDATE;
        token = strtok(NULL, ",");
        if (!token) return 0;
        strncpy(cmd->name, token, 49);
        cmd->name[49] = '\0';
        
        token = strtok(NULL, ",");
        if (!token) return 0;
        cmd->salary = atoi(token);
        
        token = strtok(NULL, ",");
        if (!token) return 0;
        cmd->priority = atoi(token);
        
    } else if (strcmp(cmd_type, "search") == 0) {
        cmd->type = CMD_SEARCH;
        token = strtok(NULL, ",");
        if (!token) return 0;
        strncpy(cmd->name, token, 49);
        cmd->name[49] = '\0';
        
        token = strtok(NULL, ",");
        if (!token) return 0;
        cmd->priority = atoi(token);
        
    } else if (strcmp(cmd_type, "print") == 0) {
        cmd->type = CMD_PRINT;
        token = strtok(NULL, ",");
        if (!token) return 0;
        cmd->priority = atoi(token);
        
    } else {
        return 0;
    }
    
    return 1;
}

// Comparison function for final print
static int compare_records_final(const void *a, const void *b) {
    hashRecord *rec_a = *(hashRecord **)a;
    hashRecord *rec_b = *(hashRecord **)b;
    if (rec_a->hash < rec_b->hash) return -1;
    if (rec_a->hash > rec_b->hash) return 1;
    return 0;
}

int main() {
    // Initialize hash table
    hashRecord *hash_table = NULL;
    
    // Initialize synchronization primitives
    pthread_rwlock_t rw_lock;
    pthread_rwlock_init(&rw_lock, NULL);
    
    pthread_mutex_t priority_mutex;
    pthread_mutex_init(&priority_mutex, NULL);
    
    pthread_cond_t priority_cond;
    pthread_cond_init(&priority_cond, NULL);
    
    pthread_mutex_t log_mutex;
    pthread_mutex_init(&log_mutex, NULL);
    
    int current_priority = 1;
    
    // Open log file
    FILE *log_file = fopen("hash.log", "w");
    if (!log_file) {
        fprintf(stderr, "Error: Could not open hash.log\n");
        return 1;
    }
    
    // Open commands file
    FILE *cmd_file = fopen("commands.txt", "r");
    if (!cmd_file) {
        fprintf(stderr, "Error: Could not open commands.txt\n");
        fclose(log_file);
        return 1;
    }
    
    // Read all commands
    Command commands[1000];
    int num_commands = 0;
    char line[200];
    
    while (fgets(line, sizeof(line), cmd_file)) {
        // Skip empty lines
        if (line[0] == '\n' || line[0] == '\0') continue;
        
        if (parse_command(line, &commands[num_commands])) {
            num_commands++;
        }
    }
    fclose(cmd_file);
    
    // Create threads for each command
    pthread_t threads[1000];
    ThreadArg thread_args[1000];
    
    for (int i = 0; i < num_commands; i++) {
        thread_args[i].cmd = commands[i];
        thread_args[i].hash_table = &hash_table;
        thread_args[i].rw_lock = &rw_lock;
        thread_args[i].priority_mutex = &priority_mutex;
        thread_args[i].priority_cond = &priority_cond;
        thread_args[i].current_priority = &current_priority;
        thread_args[i].log_file = log_file;
        thread_args[i].log_mutex = &log_mutex;
        
        pthread_create(&threads[i], NULL, thread_function, &thread_args[i]);
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < num_commands; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Print final table
    printf("Current Database:\n");
    
    // Count records
    int count = 0;
    hashRecord *current = hash_table;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    
    if (count > 0) {
        // Create array for sorting
        hashRecord **records = (hashRecord **)malloc(count * sizeof(hashRecord *));
        current = hash_table;
        int i = 0;
        while (current != NULL) {
            records[i++] = current;
            current = current->next;
        }
        
        // Sort by hash value
        qsort(records, count, sizeof(hashRecord *), compare_records_final);
        
        for (i = 0; i < count; i++) {
            printf("%u, %s, %u\n", records[i]->hash, records[i]->name, records[i]->salary);
        }
        
        free(records);
    }
    
    // Cleanup
    free_hash_table(&hash_table);
    fclose(log_file);
    
    pthread_rwlock_destroy(&rw_lock);
    pthread_mutex_destroy(&priority_mutex);
    pthread_cond_destroy(&priority_cond);
    pthread_mutex_destroy(&log_mutex);
    
    return 0;
}
