// Improved single threaded version of the program with hash table

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_KMER 100
#define INITIAL_HASH_SIZE 1024
#define LOAD_FACTOR_THRESHOLD 0.75
#define MAX_KMER 100
#define INITIAL_HASH_SIZE 1024
#define LOAD_FACTOR_THRESHOLD 0.75
#define POOL_CHUNK_SIZE 1024  // Number of entries per pool chunk

typedef struct KmerEntry {
    char              kmer[MAX_KMER + 1];  // Fixed-size array instead of pointer
    int               count;
    struct KmerEntry *next;
} KmerEntry;

typedef struct PoolChunk {
    KmerEntry         entries[POOL_CHUNK_SIZE];
    int               next_free;
    struct PoolChunk *next;
} PoolChunk;

typedef struct {
    KmerEntry **buckets;
    int         size;
    int         count;
    PoolChunk  *pool_head;  // Memory pool for entries
} KmerHashTable;

// Simple hash function (djb2)
unsigned int hash_function(const char *str, int table_size) {
    unsigned int hash = 5381;
    int          c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % table_size;
}

void init_hash_table(KmerHashTable *table) {
    table->size    = INITIAL_HASH_SIZE;
    table->count   = 0;
    table->buckets = calloc(table->size, sizeof(KmerEntry *));
    if (!table->buckets) {
        perror("Error allocating hash table");
        exit(1);
    }

    // Initialize memory pool
    table->pool_head = malloc(sizeof(PoolChunk));
    if (!table->pool_head) {
        perror("Error allocating memory pool");
        exit(1);
    }
    table->pool_head->next_free = 0;
    table->pool_head->next      = NULL;
}

void resize_hash_table(KmerHashTable *table) {
    int         old_size    = table->size;
    KmerEntry **old_buckets = table->buckets;

    // Double the size
    table->size *= 2;
    table->count   = 0;
    table->buckets = calloc(table->size, sizeof(KmerEntry *));
    if (!table->buckets) {
        perror("Error resizing hash table");
        exit(1);
    }

    // Rehash all entries
    for (int i = 0; i < old_size; i++) {
        KmerEntry *entry = old_buckets[i];
        while (entry) {
            KmerEntry *next = entry->next;

            // Rehash this entry
            unsigned int new_index    = hash_function(entry->kmer, table->size);
            entry->next               = table->buckets[new_index];
            table->buckets[new_index] = entry;
            table->count++;

            entry = next;
        }
    }

    free(old_buckets);
}

KmerEntry *allocate_entry(KmerHashTable *table) {
    PoolChunk *chunk = table->pool_head;

    // Find a chunk with free space
    while (chunk && chunk->next_free >= POOL_CHUNK_SIZE) {
        chunk = chunk->next;
    }

    // If no chunk has space, create a new one
    if (!chunk) {
        chunk = malloc(sizeof(PoolChunk));
        if (!chunk) {
            perror("Error allocating new pool chunk");
            exit(1);
        }
        chunk->next_free = 0;
        chunk->next      = table->pool_head;
        table->pool_head = chunk;
    }

    return &chunk->entries[chunk->next_free++];
}

void add_kmer(KmerHashTable *table, const char *kmer) {
    unsigned int index = hash_function(kmer, table->size);

    // Check if kmer already exists
    KmerEntry *entry = table->buckets[index];
    while (entry) {
        if (strcmp(entry->kmer, kmer) == 0) {
            entry->count++;
            return;
        }
        entry = entry->next;
    }

    // Create new entry using memory pool
    entry = allocate_entry(table);

    strcpy(entry->kmer, kmer);  // Direct copy, no malloc needed
    entry->count          = 1;
    entry->next           = table->buckets[index];
    table->buckets[index] = entry;
    table->count++;

    // Check if we need to resize
    if ((double)table->count / table->size > LOAD_FACTOR_THRESHOLD) {
        resize_hash_table(table);
    }
}

void free_hash_table(KmerHashTable *table) {
    // Free memory pool chunks
    PoolChunk *chunk = table->pool_head;
    while (chunk) {
        PoolChunk *next = chunk->next;
        free(chunk);
        chunk = next;
    }

    // Free buckets array
    free(table->buckets);
}

void print_results(KmerHashTable *table) {
    printf("Results:\n");
    for (int i = 0; i < table->size; i++) {
        KmerEntry *entry = table->buckets[i];
        while (entry) {
            printf("%s: %d\n", entry->kmer, entry->count);
            entry = entry->next;
        }
    }
}

void read_kmer(const char *data, long position, int k, char *kmer) {
    for (int i = 0; i < k; i++) {
        kmer[i] = data[position + i];
    }
    kmer[k] = '\0';
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file> <k>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *input_file = argv[1];
    int         k          = atoi(argv[2]);
    char        kmer[MAX_KMER + 1];

    if (k <= 0 || k > MAX_KMER) {
        fprintf(stderr, "Error: k must be between 1 and %d.\n", MAX_KMER);
        return EXIT_FAILURE;
    }

    int fd = open(input_file, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("Error getting file size");
        close(fd);
        return EXIT_FAILURE;
    }

    long file_size = st.st_size;

    if (file_size < k) {
        fprintf(stderr, "Error: File size is smaller than k.\n");
        close(fd);
        return EXIT_FAILURE;
    }

    char *data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        perror("Error mapping file");
        close(fd);
        return EXIT_FAILURE;
    }

    KmerHashTable table;
    init_hash_table(&table);

    // Process k-mers with bounds checking
    for (long i = 0; i <= file_size - k; i++) {
        read_kmer(data, i, k, kmer);
        add_kmer(&table, kmer);
    }

    print_results(&table);

    free_hash_table(&table);
    munmap(data, file_size);
    close(fd);

    return 0;
}
