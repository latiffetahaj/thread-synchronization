#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define FILENAME "file.txt"

// A struct that represents a reader-writer lock
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cv;
    int readers;
    int writer;
} rwlock_t;

// File descriptor for the shared file
FILE *file;

// A function that initializes a reader-writer lock
void rwlock_init(rwlock_t *lock) {
    pthread_mutex_init(&lock->mutex, NULL);
    pthread_cond_init(&lock->cv, NULL);
    lock->readers = 0;
    lock->writer = 0;
}

// A function that acquires a read lock
void readLock(rwlock_t *lock) {
    while (1) {
        pthread_mutex_lock(&lock->mutex);
        while (lock->writer > 0) {
            pthread_cond_wait(&lock->cv, &lock->mutex);
        }
        lock->readers++;
        pthread_mutex_unlock(&lock->mutex);
        break;
    }
}

// A function that releases a read lock
void readUnlock(rwlock_t *lock) {
    pthread_mutex_lock(&lock->mutex);
    lock->readers--;
    if (lock->readers == 0) {
        pthread_cond_signal(&lock->cv);
    }
    pthread_mutex_unlock(&lock->mutex);
}

// A function that acquires a write lock
void writeLock(rwlock_t *lock) {
    pthread_mutex_lock(&lock->mutex);
    while (lock->readers > 0 || lock->writer > 0) {
        pthread_cond_wait(&lock->cv, &lock->mutex);
    }
    lock->writer = 1;
    pthread_mutex_unlock(&lock->mutex);
}

// A function that releases a write lock
void writeUnlock(rwlock_t *lock) {
    pthread_mutex_lock(&lock->mutex);
    lock->writer = 0;
    pthread_cond_broadcast(&lock->cv);
    pthread_mutex_unlock(&lock->mutex);
}

// Global variables that represent the shared data
int data = 0;

// A global variable that represents the reader-writer lock
rwlock_t lock;

// A function that simulates a reader thread
void *reader(void *arg) {
    int id = *(int *)arg;
    readLock(&lock);
    //printf("Reader %d: data = %d\n", id, data);

    // Read the content of the file
    char buffer[100];
    fseek(file, 0, SEEK_SET);
    fread(buffer, sizeof(char), sizeof(buffer), file);
    buffer[sizeof(buffer) - 1] = '\0';

    printf("Reader thread id: %d -> with data: %d %s\n", id, data, buffer);

    readUnlock(&lock);
    return NULL;
}

// A function that simulates a writer thread
void *writer(void *arg) {
    int id = *(int *)arg;
    writeLock(&lock);
    data++;
    //printf("Writer %d: data = %d\n", id, data);

    // Write to the file
    fseek(file, 0, SEEK_END);
    fprintf(file, "(Writer thread : %d with data: %d)\n", id, data);

    writeUnlock(&lock);
    return NULL;
}

#define NUM_READERS 5
#define NUM_WRITERS 3

int main(int argc, char *argv[]) {
    // Open the file in read-write mode, creating it if it doesn't exist
    file = fopen(FILENAME, "a+");
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    rwlock_init(&lock);

    int readerIds[NUM_READERS];
    int writerIds[NUM_WRITERS];

    pthread_t readerThreads[NUM_READERS];
    pthread_t writerThreads[NUM_WRITERS];

    for (int i = 0; i < NUM_WRITERS; i++) {
        writerIds[i] = i + 1;
        pthread_create(&writerThreads[i], NULL, writer, &writerIds[i]);
    }

    for (int i = 0; i < NUM_READERS; i++) {
        readerIds[i] = i + 1;
        pthread_create(&readerThreads[i], NULL, reader, &readerIds[i]);
    }

    for (int i = 0; i < NUM_WRITERS; i++) {
        pthread_join(writerThreads[i], NULL);
    }

    for (int i = 0; i < NUM_READERS; i++) {
        pthread_join(readerThreads[i], NULL);
    }

    fclose(file);

    return 0;
}
