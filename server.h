#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <ifaddrs.h>
#include <fcntl.h>

#define SIZE 1024
#define IP "127.0.0.1"


typedef enum {
    OPEN,
    READ,
    WRITE,
    TRUNC,
    OPEN_REP,
    READ_DONE,
    WRITE_DONE,
    TRUNC_DONE,

} msg_type;

typedef struct file_t file_t;

struct file_t{
    char *fname;
    int fd;
    int fid;
    time_t last_activity;
    file_t *next;
};

typedef struct{
    file_t *head;
    int size;
    pthread_mutex_t lock;
}list_t;

extern list_t *files;

/// LIST FUNCTIONS ///
list_t *list_init();
int add(list_t *list, char *fname, int fd, int fid);
int delete_by_fname(list_t *list, const char *fname);
file_t *find_by_fname(list_t *list, const char *fname);
void destroy(list_t *list);
/////////////////////

void server_init();
void server_destroy();
void init_contact_info(int port_num);
int init_udp_socket(struct sockaddr_in *server_addr);


int serialize(void *buffer,msg_type *msg , char *fname, int *fid, int *pos, int *size, char *payload);
int deserialize(void *buffer,msg_type *msg , char *fname, int *fid, int *pos, int *size, char *payload);

#endif