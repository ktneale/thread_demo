#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>

#define MAX_MESSAGE_LENGTH 255
#define SUCCESS 0

#define DEBUG0(C, M) debug0(C, M,__func__, __FILE__, __LINE__)
#define DEBUG(C, M, ...) debug(C, M, ##__VA_ARGS__, __func__, __FILE__, __LINE__)

const char * version =  "1.0";

typedef enum
{
    DEBUG_ALL = 0,
    DEBUG_INFO = 1,
    DEBUG_WARNING = 2,
    DEBUG_MAJOR = 3,
    DEBUG_CRITICAL = 4,
    DEBUG_NONE = 5
}debug_levels_t;

typedef enum
{
    ERROR_BAD_PARAMETER = 1,    /* Arguments supplied to the function are invalid */
    ERROR_BAD_ALLOC = 2,        /* Required memory failed to be allocated */
    ERROR_BAD_INTERNAL = 3,     /* The function has failed due to an internal error */
    ERROR_UNKNOWN = 4           /* An unknown failure */
}error_codes_t;

typedef struct message_t
{
    struct message_t * next;
    char * msg;
}message_t;

typedef struct
{
    message_t * head;
    message_t * tail;
}message_queue_t;

typedef struct
{
    int debug_level;
    message_queue_t queue;
}app_config_t;

static app_config_t g_myapp_config = {0};

int enqueue(message_queue_t * queue, const char * data);
int dequeue(message_queue_t * queue, char * const data, const int length);

pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

void dump_queue(message_queue_t * queue)
{
    if(queue && queue->head)
    {
        printf("QUEUE [START]\n");

        message_t * message = queue->head;

        while(message->next != NULL)
        {
            printf("Queue data: %s\n",message->msg);
            printf("Queue next: %p\n",&(message->next));
            message = (message_t *)message->next;
        }

        //Print final tail message
        printf("Queue data: %s\n",message->msg);
        printf("Queue next: %p\n",&(message->next));
        printf("QUEUE [END]\n");
    }
}

message_t * create_message(const char * msg)
{
    if(!msg)
    {
        DEBUG0(DEBUG_CRITICAL, "Bad parameters");
        return NULL; 
    }

    message_t *message = (message_t *)malloc(sizeof(message_t));

    if(!message)
    {
        DEBUG0(DEBUG_CRITICAL, "Bad allocation");
        return NULL; 
    }

    message->next = NULL;
    message->msg = strdup(msg);

    if (message->msg == NULL)
    {
        /* Out of memory */
        free(message);
        DEBUG0(DEBUG_CRITICAL, "Bad allocation");
        return NULL;
    }

    return message;
}

void release_message(message_t * message)
{
    if(message)
    {
        free(message->msg);
        free(message);
    }
}

int enqueue(message_queue_t * queue, const char * data)
{
    pthread_mutex_lock( &g_mutex );

    if(!queue || !data)
    {
        DEBUG0(DEBUG_WARNING, "Error! Bad parameter(s)");
        pthread_mutex_unlock( &g_mutex );
        return ERROR_BAD_PARAMETER;
    }

    message_t * message = create_message(data);

    if (message == NULL)
    {
        DEBUG0(DEBUG_CRITICAL,"Could not create a new message!");
        pthread_mutex_unlock( &g_mutex );
        return ERROR_BAD_INTERNAL; 
    }

    if (queue->head == NULL)
    {
        queue->head = queue->tail = message;
    }
    else
    {
        queue->tail->next = (message_t *)message;
        queue->tail = message;
    }
    DEBUG0(DEBUG_INFO,"New message queued!");

    pthread_mutex_unlock( &g_mutex );
    return SUCCESS;
}

int dequeue(message_queue_t * queue, char * const data, const int length)
{
    pthread_mutex_lock( &g_mutex );

    if(!queue || !data || length <= 0)
    {
        DEBUG0(DEBUG_WARNING, "Error! Bad parameter(s)");
        pthread_mutex_unlock( &g_mutex );
        return ERROR_BAD_PARAMETER;
    }

    message_t * message = queue->head;

    if (message == NULL)
    {
        pthread_mutex_unlock( &g_mutex );
        return SUCCESS;  /* OK */
    }

    strncpy(data,message->msg, (size_t)length);

    queue->head = (message_t *)message->next;
    release_message(message);
    DEBUG0(DEBUG_INFO,"Message dequeued!");

    pthread_mutex_unlock( &g_mutex );
    return SUCCESS;
}

void* thread_main(void *arg)
{
    DEBUG0(DEBUG_INFO,"Worker thread! main function");
    //Periodically purge the message queue;

    char buffer[MAX_MESSAGE_LENGTH] = {0};
    int length = MAX_MESSAGE_LENGTH;

    int rv = 0;

    while(1)
    {
        //Clear the buffer;
        memset(buffer,'\0',MAX_MESSAGE_LENGTH);

        rv = dequeue(&(g_myapp_config.queue), buffer, length);

        if(rv == 0 && strlen(buffer) > 0)
        {
            DEBUG(DEBUG_INFO,"received message: %s",buffer);
        }

        sleep(1);
    }
}

void print_app_info()
{
    fprintf(stdout, "Thread Test Application.  Written by Kevin Neale (c) 2015\n");
}

void usage()
{
    fprintf(stderr, "Usage:-\n");
    fprintf(stderr, "   threadtest [-d <debug_level] [-v]\n\n");
    fprintf(stderr, "     -d <debug_level>   - Level of debug to display (all, info, warning, major, critical, none).\n");
    fprintf(stderr, "     -v                 - Application version info.\n");
}

int main(int argc, char **argv)
{
    char opt = 0;

    char message6[6] = {'s','t','i','l','l','\0'};
    char message7[6] = {'a','l','i','v','e','\0'};
    char buffer[256] = {0};

    int index = 0;

    pthread_t tid[1];

    g_myapp_config.debug_level = DEBUG_NONE;

    print_app_info();

    while ( '\xff' != (opt=getopt(argc, argv, "d:v")) )
    {
        switch ( opt )
        {
            case 'd':
            {
                if(!optarg)
                {
                    usage();
                    exit(SUCCESS);
                }
                
                if(strncmp("all", optarg,3) == 0)
                {
                    g_myapp_config.debug_level = DEBUG_ALL;
                }
                else if(strncmp("info", optarg,4) == 0)
                {                
                    g_myapp_config.debug_level = DEBUG_INFO;
                }
                else if(strncmp("warning", optarg,7) == 0)
                {
                    g_myapp_config.debug_level = DEBUG_WARNING;
                }
                else if(strncmp("major", optarg,5) == 0)
                {                
                    g_myapp_config.debug_level = DEBUG_MAJOR;
                }
                else if(strncmp("critical", optarg,8) == 0)
                {
                    g_myapp_config.debug_level = DEBUG_CRITICAL;
                }
                else
                {
                    g_myapp_config.debug_level = DEBUG_NONE;
                }
                break;
            }

            case 'v':
                fprintf(stdout, "%s\n", version);
                exit(SUCCESS);
            default:
                usage();
                exit(SUCCESS);
        }
    }

    DEBUG0(DEBUG_CRITICAL,"Running...");

    DEBUG(DEBUG_INFO,"test: %s, %d","help",2);
    DEBUG(DEBUG_INFO,"test2: %s, %d, %s, %d, %f","help",2,"me",-5,2.00);

    for(index=0; index<=10; index++)
    {
        memset(buffer,'\0',255);
        snprintf (buffer, 255, "TestMessage: %d",index);
        enqueue(&(g_myapp_config.queue),buffer);
    }

    //dump_queue();

    pthread_create(&(tid[0]), NULL, &thread_main, NULL);
    pthread_detach(tid[0]);

    DEBUG0(DEBUG_INFO,"Main thread!");

    while(1)
    {
        sleep(10);

        enqueue(&(g_myapp_config.queue),message6);
        enqueue(&(g_myapp_config.queue),message7);
    }

    //sleep(15);

    return 0;
}
