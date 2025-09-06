#include "headers.h"

#define MAX_FILES 1000
#define THRESHOLD 42
#define MAX_NAME_LENGTH 128
#define MAX_THREADS 4
#define MOD 1000000007 // A large prime number for the modulus to reduce hash collisions
#define BASE 31        // A small prime base, commonly used in string hashing

void print_memory_usage() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    printf("Maximum resident set size: %ld kilobytes\n", usage.ru_maxrss);
}

typedef struct File
{
    char name[MAX_NAME_LENGTH];
    int id;
    char timestamp[20]; // ISO format YYYY-MM-DDTHH:MM:SS
    long value_used_to_sort;
} File;

typedef struct
{
    File *files;
    int start;
    int end;
    long max_id;
    int *local_count;
} ThreadData;

typedef enum {
    SORT_BY_NAME,
    SORT_BY_ID,
    SORT_BY_TIMESTAMP
} SortCriteria;

// Global variable for sorting criteria
SortCriteria global_criteria;

File files[MAX_FILES];
int total_num_files;
char sort_column[10];

long hashString(const char *str)
{
    long hash = 0;
    long power = 1;

    // Using a prime BASE and a modulus to avoid overflow and collisions
    for (long i = 0; str[i] != '\0'; i++)
    {
        hash = (hash + (str[i]) * power) % MOD; // str[i] is the ASCII value of the character
        power = (power * BASE) % MOD;
    }

    return hash;
}

time_t parse_timestamp_to_time_t(const char *timestamp)
{
    struct tm tm;
    memset(&tm, 0, sizeof(struct tm));

    // Parse the timestamp "YYYY-MM-DDTHH:MM:SS"
    if (sscanf(timestamp, "%4d-%2d-%2dT%2d:%2d:%2d",&tm.tm_year, &tm.tm_mon, &tm.tm_mday,&tm.tm_hour, &tm.tm_min, &tm.tm_sec) != 6)
    {
        fprintf(stderr, "Error: Invalid timestamp format.\n");
        return -1;
    }

    tm.tm_year -= 1900; // tm_year is year since 1900
    tm.tm_mon -= 1;     // tm_mon is 0-based (0 = January)

    return mktime(&tm);
}

void time_to_string(time_t timestamp, char *buffer)
{
    struct tm *tm_info = localtime(&timestamp);

    strftime(buffer, 20, "%Y-%m-%dT%H:%M:%S", tm_info);
}

long hashTimestamp(time_t timestamp)
{
    return (long)(timestamp % 100000003);
}

void *countSortEachThread(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    int *local_count = data->local_count;
    File *files = data->files;

    for (int i = data->start; i < data->end; i++)
    {
        local_count[files[i].value_used_to_sort]++;
    }
    return NULL;
}

void distributedCountSort(File *files, int total_num_files, char *sort_column)
{
    if (strcmp(sort_column, "ID") == 0)
    {
        for (int i = 0; i < total_num_files; i++)
        {
            files[i].value_used_to_sort = files[i].id;
        }
    }
    else if (strcmp(sort_column, "Name") == 0)
    {
        for (int i = 0; i < total_num_files; i++)
        {
            files[i].value_used_to_sort = hashString(files[i].name);
        }
    }
    else if (strcmp(sort_column, "Timestamp") == 0)
    {
        for (int i = 0; i < total_num_files; i++)
        {
            time_t parsed_timestamp = parse_timestamp_to_time_t(files[i].timestamp);
            files[i].value_used_to_sort = hashTimestamp(parsed_timestamp);
        }
    }
    long max_file_value_used_to_sort = 0;
    for (int i = 0; i < total_num_files; i++)
    {
        if (files[i].value_used_to_sort > max_file_value_used_to_sort)
        {
            max_file_value_used_to_sort = files[i].value_used_to_sort; // As the files-id is unique for all files
        }
    }
    int *global_frequency_array = (int *)malloc((max_file_value_used_to_sort + 1) * sizeof(int));
    memset(global_frequency_array, 0, (max_file_value_used_to_sort + 1) * sizeof(int));
    pthread_t threads[MAX_THREADS];
    ThreadData thread_data[MAX_THREADS];
    int chunk_size = total_num_files / MAX_THREADS;
    int *local_counts = (int *)malloc(MAX_THREADS * (max_file_value_used_to_sort + 1) * sizeof(int));
    for (int i = 0; i < MAX_THREADS; i++)
    {
        thread_data[i].files = files;
        thread_data[i].start = i * chunk_size;
        if (i == MAX_THREADS - 1)
        {
            thread_data[i].end = total_num_files;
        }
        else
        {
            thread_data[i].end = (i + 1) * chunk_size;
        }
        thread_data[i].max_id = max_file_value_used_to_sort; // Each chunk will have max_file_id length local array only
        // thread_data[i].local_count = malloc((max_file_value_used_to_sort + 1) * sizeof(int));
        thread_data[i].local_count = &local_counts[i * (max_file_value_used_to_sort + 1)];
        memset(thread_data[i].local_count, 0, (max_file_value_used_to_sort + 1) * sizeof(int));
        pthread_create(&threads[i], NULL, countSortEachThread, &thread_data[i]);
    }
    for (int i = 0; i < MAX_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
        for (int j = 0; j <= max_file_value_used_to_sort; j++)
        {
            global_frequency_array[j] += thread_data[i].local_count[j]; // As divided in chunks, whenever we encounter the id, we increment the global array[id]
        }
        // free(thread_data[i].local_count);
    }
    free(local_counts);
    for (long i = 1; i <= max_file_value_used_to_sort; i++)
    {
        global_frequency_array[i] = global_frequency_array[i] + global_frequency_array[i - 1];
    }
    File *sorted_array = malloc(total_num_files * sizeof(File));
    for (int i = total_num_files - 1; i >= 0; i--)
    {
        sorted_array[global_frequency_array[files[i].value_used_to_sort] - 1] = files[i];
        global_frequency_array[files[i].value_used_to_sort]--; // For duplicate elements if present in original array
    }
    for (long i = 0; i < total_num_files; i++)
    {
        files[i] = sorted_array[i]; // Needed as sorted_array contains max_id number of elements where rest are 0's, and we need only total_num_files elements
    }
    for (long i = 0; i < total_num_files; i++)
    {
        printf("%s %d %s\n", files[i].name, files[i].id, files[i].timestamp);
    }
    free(global_frequency_array);
    free(sorted_array);
}

int compareFiles(const File *a, const File *b) {
    switch (global_criteria) {
        case SORT_BY_NAME:
            return strcmp(a->name, b->name);
        case SORT_BY_ID:
            return (a->id > b->id) - (a->id < b->id);
        case SORT_BY_TIMESTAMP:
            time_t time1 = parse_timestamp_to_time_t(a->timestamp);
            time_t time2 = parse_timestamp_to_time_t(b->timestamp);
            return (time1 > time2) - (time1 < time2);
        default:
            return 0;
    }
}

void merge(File *files, int left, int mid, int right) {
    int i, j, k;
    int n1 = mid - left + 1;
    int n2 = right - mid;

    // Create temporary arrays
    File *L = (File *)malloc(n1 * sizeof(File));
    File *R = (File *)malloc(n2 * sizeof(File));

    // Copy data to temporary arrays
    for (i = 0; i < n1; i++)
        L[i] = files[left + i];
    for (j = 0; j < n2; j++)
        R[j] = files[mid + 1 + j];

    // Merge the temporary arrays back into files[left..right]
    i = 0;
    j = 0;
    k = left;
    while (i < n1 && j < n2) {
        if (compareFiles(&L[i], &R[j]) <= 0) {
            files[k] = L[i];
            i++;
        } else {
            files[k] = R[j];
            j++;
        }
        k++;
    }

    // Copy remaining elements of L[] if any
    while (i < n1) {
        files[k] = L[i];
        i++;
        k++;
    }

    // Copy remaining elements of R[] if any
    while (j < n2) {
        files[k] = R[j];
        j++;
        k++;
    }

    free(L);
    free(R);
}

void* mergeSortThread(void* arg)
{
    ThreadData *data = (ThreadData *)arg;
    int left = data->start;
    int right = data->end -1;

    if (left < right)
    {
        int mid = left + (right-left)/2;
        ThreadData leftData = {data->files, left, mid + 1, 0, NULL};
        ThreadData rightData = {data->files, mid + 1, right + 1, 0, NULL};
        
        mergeSortThread(&leftData);
        mergeSortThread(&rightData);
        
        merge(data->files, left, mid, right);
    }
}

void distributedMergeSort(File *files, int total_num_files, SortCriteria criteria)
{
    global_criteria = criteria;
    pthread_t threads[MAX_THREADS];
    ThreadData thread_data[MAX_THREADS];

    int chunk_size = total_num_files / MAX_THREADS;
    int remainder = total_num_files % MAX_THREADS;

    int current_start = 0;
    for (int i = 0; i < MAX_THREADS; i++)
    {
        thread_data[i].files = files;
        thread_data[i].start = current_start;
        int current_chunk = chunk_size + (i < remainder ? 1 : 0);
        thread_data[i].end = current_start + current_chunk;
        current_start = thread_data[i].end;
        pthread_create(&threads[i], NULL, mergeSortThread, &thread_data[i]);
    }

    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    for (int size = 1; size < MAX_THREADS; size = size * 2) {
        for (int i = 0; i < MAX_THREADS - size; i += size * 2) {
            int left = thread_data[i].start;
            int mid = thread_data[i + size - 1].end - 1;
            int right = (i + size * 2 <= MAX_THREADS) ? 
                       thread_data[i + size * 2 - 1].end - 1 : 
                       thread_data[MAX_THREADS - 1].end - 1;
            
            merge(files, left, mid, right);
        }
    }
    for (int i = 0; i < total_num_files; i++) {
        printf("%s %d %s\n", files[i].name, files[i].id, files[i].timestamp);
    }
}

int main()
{
    scanf("%d", &total_num_files);
    for (int i = 0; i < total_num_files; i++)
    {
        scanf("%s %d %s ", files[i].name, &files[i].id, files[i].timestamp);
    }
    scanf("%s", sort_column);

    printf("%s\n", sort_column);
    if (total_num_files < THRESHOLD)
    {
        distributedCountSort(files, total_num_files, sort_column);
    }
    else
    {
        SortCriteria criteria;
        if (strcmp(sort_column, "Name") == 0) 
            criteria = SORT_BY_NAME;
        else if (strcmp(sort_column, "ID") == 0) 
            criteria = SORT_BY_ID;
        else 
            criteria = SORT_BY_TIMESTAMP;
        distributedMergeSort(files, total_num_files, criteria);
    }
    print_memory_usage();
    return 0;
}
