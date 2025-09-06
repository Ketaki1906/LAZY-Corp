#include "headers.h"

#define MAX_USERS 1000
#define MAX_FILES 1000

int read_time, write_time, delete_time;
int total_num_files, allowed_conc_users, max_wait_time;
int request_count = 0;

typedef struct
{
    int user_id;
    int file_id;
    char operation[10];
    int time_request_sent;
    int status; // 0 - pending, 1 - processing, -1 - canceled
} Request;

typedef struct
{
    int file_id;
    int num_curr_total_users;
    int num_curr_write_users;
} FileInfo;

Request requests[MAX_USERS];
FileInfo files_info[MAX_FILES];
pthread_mutex_t file_mutex[MAX_FILES];
pthread_cond_t file_cond[MAX_FILES];
int file_status[MAX_FILES];
int start_time;

int get_time_seconds()
{
    return (int)time(NULL);
}

void *process_request(void *arg)
{
    Request *curr_req_ptr = (Request *)arg;
    sleep(curr_req_ptr->time_request_sent); // Wait until the when the time that request was initiated comes

    printf("\033[33mUser %d has made request for performing %s on file %d at %d seconds\033[0m\n",curr_req_ptr->user_id, curr_req_ptr->operation, curr_req_ptr->file_id, curr_req_ptr->time_request_sent);

    sleep(1); // Waiting for 1 second before LAZY starts processing

    if (files_info[curr_req_ptr->file_id].num_curr_total_users > allowed_conc_users)
    {
        pthread_mutex_lock(&file_mutex[curr_req_ptr->file_id]);
    }

    if (file_status[curr_req_ptr->file_id] == -1)
    {
        printf("\033[37mLAZY has declined the request of User %d at %d seconds because a deleted file was requested.\033[0m\n",curr_req_ptr->user_id, get_time_seconds() - start_time);
        pthread_mutex_unlock(&file_mutex[curr_req_ptr->file_id]);
        return NULL;
    }

    if (strcmp(curr_req_ptr->operation, "READ") == 0)
    {
        while (1)
        {
            if ((get_time_seconds() - start_time) - curr_req_ptr->time_request_sent >= max_wait_time && curr_req_ptr->status == 0)
            {
                printf("\033[31mUser %d canceled the request due to no response at %d seconds\033[0m\n",curr_req_ptr->user_id, get_time_seconds() - start_time);
                curr_req_ptr->status = -1;
                break;
            }
            else if (curr_req_ptr->status == 0)
            {
                printf("\033[35mLAZY has taken up the request of User %d at %d seconds\033[0m\n",curr_req_ptr->user_id, get_time_seconds() - start_time);
                files_info[curr_req_ptr->file_id].num_curr_total_users++;
                sleep(read_time);
                printf("\033[32mThe request for User %d was completed at %d seconds\033[0m\n",curr_req_ptr->user_id, get_time_seconds() - start_time);
                if (files_info[curr_req_ptr->file_id].num_curr_total_users > allowed_conc_users)
                {
                    pthread_mutex_unlock(&file_mutex[curr_req_ptr->file_id]);
                }
                files_info[curr_req_ptr->file_id].num_curr_total_users--;
                break;
            }
        }
    }

    else if (strcmp(curr_req_ptr->operation, "WRITE") == 0)
    {
        while (1)
        {
            if ((get_time_seconds() - start_time) - curr_req_ptr->time_request_sent >= max_wait_time && curr_req_ptr->status == 0)
            {
                printf("\033[31mUser %d canceled the request due to no response at %d seconds\033[0m\n",curr_req_ptr->user_id, get_time_seconds() - start_time);
                curr_req_ptr->status = -1;
                break;
            }
            else if (files_info[curr_req_ptr->file_id].num_curr_write_users == 0 && curr_req_ptr->status == 0)
            {
                printf("\033[35mLAZY has taken up the request of User %d at %d seconds\033[0m\n",curr_req_ptr->user_id, get_time_seconds() - start_time);
                files_info[curr_req_ptr->file_id].num_curr_total_users++;
                files_info[curr_req_ptr->file_id].num_curr_write_users++;
                sleep(write_time);
                printf("\033[32mThe request for User %d was completed at %d seconds\033[0m\n",curr_req_ptr->user_id, get_time_seconds() - start_time);
                if (files_info[curr_req_ptr->file_id].num_curr_total_users > allowed_conc_users)
                {
                    pthread_mutex_unlock(&file_mutex[curr_req_ptr->file_id]);
                }
                files_info[curr_req_ptr->file_id].num_curr_total_users--;
                files_info[curr_req_ptr->file_id].num_curr_write_users--;
                break;
            }
        }
    }

    if (strcmp(curr_req_ptr->operation, "DELETE") == 0)
    {
        while (1)
        {
            if ((get_time_seconds() - start_time) - curr_req_ptr->time_request_sent >= max_wait_time && curr_req_ptr->status == 0)
            {
                printf("\033[31mUser %d canceled the request due to no response at %d seconds\033[0m\n",curr_req_ptr->user_id, get_time_seconds() - start_time);
                curr_req_ptr->status = -1;
                break;
            }

            else if (files_info[curr_req_ptr->file_id].num_curr_total_users == 0 && curr_req_ptr->status == 0)
            {
                printf("\033[35mLAZY has taken up the request of User %d at %d seconds\033[0m\n",curr_req_ptr->user_id, get_time_seconds() - start_time);
                file_status[curr_req_ptr->file_id] = -1;
                sleep(delete_time);
                printf("\033[32mThe request for User %d was completed at %d seconds\033[0m\n",curr_req_ptr->user_id, get_time_seconds() - start_time);
                break;
            }
        }
    }

    return NULL;
}

int main()
{
    scanf("%d %d %d", &read_time, &write_time, &delete_time);
    scanf("%d %d %d", &total_num_files, &allowed_conc_users, &max_wait_time);

    for (int i = 0; i < total_num_files; i++)
    {
        pthread_mutex_init(&file_mutex[i], NULL);
        pthread_cond_init(&file_cond[i], NULL);
        file_status[i] = 0; // All files initially exist
        files_info[i] = (FileInfo){i, 0, 0};
    }

    int user_id, file_id, required_time;
    char operation[10];
    char stop_exp[5];
    while (scanf("%d %d %s %d", &user_id, &file_id, operation, &required_time) == 4 || (scanf("%s", stop_exp) && strcmp(stop_exp, "STOP") != 0))
    {
        if (strcmp(operation, "READ") == 0)
        {
            requests[request_count] = (Request){user_id, file_id, "READ", required_time, 0};
            request_count++;
        }
        if (strcmp(operation, "WRITE") == 0)
        {
            requests[request_count] = (Request){user_id, file_id, "WRITE", required_time, 0};
            request_count++;
        }
        if (strcmp(operation, "DELETE") == 0)
        {
            requests[request_count] = (Request){user_id, file_id, "DELETE", required_time, 0};
            request_count++;
        }
    }

    start_time = get_time_seconds();
    printf("\033[33mLAZY woken up!\033[0m\n");

    pthread_t threads[request_count];
    for (int i = 0; i < request_count; i++)
    {
        pthread_create(&threads[i], NULL, process_request, &requests[i]);
    }

    for (int i = 0; i < request_count; i++)
    {
        pthread_join(threads[i], NULL);
    }

    for (int i = 0; i < total_num_files; i++)
    {
        pthread_mutex_destroy(&file_mutex[i]);
        pthread_cond_destroy(&file_cond[i]);
    }

    printf("\033[33mLAZY has no more pending requests and is going back to sleep!\033[0m\n");
    return 0;
}
