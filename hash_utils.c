#include "chash.h"
#include <stdint.h>
#include <string.h>
#include <sys/time.h>

// Jenkins one-at-a-time hash function
uint32_t jenkins_one_at_a_time_hash(const char *key) {
    uint32_t hash = 0;
    size_t length = strlen(key);
    
    for (size_t i = 0; i < length; i++) {
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    
    return hash;
}

// Get current timestamp in microseconds
long long current_timestamp() {
    struct timeval te;
    gettimeofday(&te, NULL);
    long long microseconds = (te.tv_sec * 1000000LL) + te.tv_usec;
    return microseconds;
}
