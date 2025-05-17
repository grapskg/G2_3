//LIST IMPLEMENTATION FILE 
#include "server.h"

// 1. Initialize the list
list_t* list_init() {
    list_t *list = malloc(sizeof(list_t));
    if (!list) {
        return NULL;
    }
    
    list->head = NULL;
    list->size = 0;
    if (pthread_mutex_init(&list->lock, NULL) != 0) {
        free(list);
        return NULL;
    }
    
    return list;
}

// 2. Add a file to the list
int add(list_t *list, char *fname, int fd, int fid) {
    if (!list) return -1;
    
    pthread_mutex_lock(&list->lock);
    
    file_t *new_file = malloc(sizeof(file_t));
    if (!new_file) {
        pthread_mutex_unlock(&list->lock);
        return -1;
    }
    
    new_file->fname = strdup(fname);
    new_file->fd = fd;
    new_file->fid = fid;
    new_file->last_activity = time(NULL);
    new_file->next = list->head;
    
    list->head = new_file;
    list->size++;
    
    pthread_mutex_unlock(&list->lock);
    return 0;
}

// 3. Delete a file from the list by filename
int delete_by_fname(list_t *list, const char *fname) {
    if (!list) return -1;
    
    pthread_mutex_lock(&list->lock);
    
    file_t *curr = list->head;
    file_t *prev = NULL;
    
    while (curr != NULL) {
        if (strcmp(curr->fname, fname) == 0) {
            if (prev == NULL) {
                list->head = curr->next;
            } else {
                prev->next = curr->next;
            }
            
            free(curr->fname);
            free(curr);
            list->size--;
            
            pthread_mutex_unlock(&list->lock);
            return 0;
        }
        
        prev = curr;
        curr = curr->next;
    }
    
    pthread_mutex_unlock(&list->lock);
    return -1; // File not found
}

// 4. Find a file by filename
file_t *find_by_fname(list_t *list, const char *fname) {
    if (!list) return NULL;
    
    pthread_mutex_lock(&list->lock);
    
    file_t *curr = list->head;
    
    while (curr != NULL) {
        if (strcmp(curr->fname, fname) == 0) {
            pthread_mutex_unlock(&list->lock);
            return curr;
        }
        curr = curr->next;
    }
    
    pthread_mutex_unlock(&list->lock);
    return NULL; // File not found
}

//5. find by fid
file_t *find_by_fid(list_t *list, int fid) {
    if (!list) return NULL;
    
    pthread_mutex_lock(&list->lock);
    
    file_t *curr = list->head;
    
    while (curr != NULL) {
        if (curr->fid == fid) {
            pthread_mutex_unlock(&list->lock);
            return curr;
        }
        curr = curr->next;
    }
    
    pthread_mutex_unlock(&list->lock);
    return NULL; // File not found
}

// 6. Destroy the list
void destroy(list_t *list) {
    if (!list) return;
    
    pthread_mutex_lock(&list->lock);
    
    file_t *curr = list->head;
    file_t *next;
    
    while (curr != NULL) {
        next = curr->next;
        free(curr->fname);
        free(curr);
        curr = next;
    }
    
    list->head = NULL;
    list->size = 0;
    
    pthread_mutex_unlock(&list->lock);
    pthread_mutex_destroy(&list->lock);
    free(list);  // Free the list structure itself
}


void print_list(const list_t *list) {
    if (!list) {
        printf("List is NULL\n");
        return;
    }

    pthread_mutex_lock(&list->lock);

    file_t *current = list->head;
    while (current) {
        printf("File name: %s, fd: %d, fid: %d\n",
               current->fname ? current->fname : "(null)",
               current->fd,
               current->fid);

        current = current->next;
    }

    pthread_mutex_unlock(&list->lock);
}