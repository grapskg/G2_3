#include "server.h"

pthread_t receiver;
list_t *files;
struct sockaddr_in server;
socklen_t server_len;
int udp_sock, fid_cnt=0;
//need to destroy at the end
char *bdir;
int rn = 0;
char rn_file[32] = "rn.txt";

void *receive_udp(void *arg){
    char buff[SIZE];
    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);
    msg_type type;
    char * fname=NULL, *payload=NULL;
    char filepath[SIZE];
    int fid, pos, size, ret,seqno;
    file_t *temp=NULL;
    
    while(1){

        memset(buff,0, SIZE);
        memset(filepath,0, SIZE);


        //need to make this blocking/nonblocking
        ret = recvfrom(udp_sock, buff, SIZE, 0, (struct sockaddr *)&client,&client_len);

        if(ret > 0){
            fname = NULL;
            fid = -1;
            pos = -1;
            seqno=-1;
            size = -1;
            payload = NULL;
            temp = NULL;
            
            deserialize(buff,&type,&fname, &fid, &pos,&size ,&seqno,&rn, &payload);

            if(type == OPEN){
                //check if fname is valid
                if(fname != NULL){

                    printf("got open for %s\n", fname);
                    //try to find if file is already open
                    temp = find_by_fid(files,fid);

                    //CASE THAT FILE IS OPEN
                    if(temp != NULL){
                        //FILE EXIST SERIALIZE AND SEND ANSWER
                        type = OPEN_REP;
                        serialize(buff,&type, fname, &temp->fid, &pos, &size, &seqno,&rn, payload);
                        int ret = sendto(udp_sock,buff, SIZE, 0,  (struct sockaddr*)&client, client_len);
                        //printf("ret from sendto is %d\n", ret);
                        //printf("Client addr: %s:%d\n", 
                        //    inet_ntoa(client.sin_addr),
                        //    ntohs(client.sin_port));
                        if (ret == -1) {
                            perror("sendto");
                            printf("Client addr: %s:%d\n", 
                                inet_ntoa(client.sin_addr),
                                ntohs(client.sin_port));
                        }

                    }//CASE IT IS NOT OPEN
                    else{
                        printf("file info not exists try to create\n");
                        //CREATE PATH NAME
                        snprintf(filepath, sizeof(filepath), "%s/%s", bdir, fname);
                        //printf("filepath is: %s\n", filepath);
                        int fd = open(filepath, O_RDWR | O_CREAT, 0644);
                        
                        //TODO: need to create my own fd fid mapping
                        if(fd != -1){
                            //add file to the list along with its fd , fid and fname
                            add(files, fname, fd, fid_cnt);
                            //send answer
                            type = OPEN_REP;
                            serialize(buff,&type, fname,&fid_cnt, &pos, &size, &seqno,&rn ,payload);
                            int ret = sendto(udp_sock,buff, SIZE, 0,  (struct sockaddr*)&client, client_len);
                            //printf("ret from sendto is %d\n", ret);
                            //printf("Client addr: %s:%d\n", 
                            //    inet_ntoa(client.sin_addr),
                            //    ntohs(client.sin_port));
                            if (ret == -1) {
                                perror("sendto");
                                printf("Client addr: %s:%d\n", 
                                    inet_ntoa(client.sin_addr),
                                    ntohs(client.sin_port));
                            }
                            fid_cnt++;
                                
                        }
                        else{
                            perror("open"); // Prints a descriptive error message to stderr
                            printf("errno: %d\n", errno); // Prints the errno value
                        }


                    }
                }else{
                    printf("fname is NULL\n");
                }
            }
            else if(type == READ){
                //TODO: REOPEN MECHANISM
                if(fname != NULL){
                    printf("got read for %s\n", fname);
                    temp = find_by_fid(files, fid);
                    //file is open read
                    if(temp != NULL){
                        //malloc size of the requested read data
                        //if(size > 0){}
                        char *rbuff;
                        rbuff = (char *)malloc(size*sizeof(char));
                        memset(rbuff, 0, size);
                        int count = 0, ret = 0;
                        //SEEK TO POS (ASSUME POS IS FROM THE START FO THE FILE)
                        lseek(temp->fd, pos, SEEK_SET);

                        while (count < size) {
                            ret = read(temp->fd, rbuff + count, size - count);
                            if (ret == 0) {
                                // EOF reached
                                break;
                            } else if (ret == -1) {
                                perror("read");
                                break;
                            }
                            count += ret;
                        }

                        type = READ_DONE;
                        //CHECK IF COUNT EQUAL SIZE OR LESS
                        if(rbuff != NULL){
                            printf("read: %s\n", rbuff);
                        }
                        serialize(buff, &type, fname, &temp->fid,&pos , &count, &seqno,&rn, rbuff);
                        ret = sendto(udp_sock,buff, SIZE, 0,  (struct sockaddr*)&client, client_len);
                        if(ret == -1){
                            perror("sendto");
                        }
                        free(rbuff);
                        rbuff = NULL;
                        
                    }//reopen file and read
                    else{ 
                        //REOPEN AND SAVE WITH PREVIOUS FID
                        snprintf(filepath, sizeof(filepath), "%s/%s", bdir, fname);
                        //printf("filepath is: %s\n", filepath);
                        int fd = open(filepath, O_RDWR | O_CREAT, 0644);
                        
                        
                        if(fd != -1){
                            //add file to the list along with its fd, fname and FID FROM REQ
                            add(files, fname, fd, fid);
                            //send answer
                            type = OPEN_REP;
                            serialize(buff,&type, fname,&fid_cnt, &pos, &size, &seqno,&rn, payload);
                            sendto(udp_sock,buff, SIZE, 0,  (struct sockaddr*)&client, client_len);
                            //update fid
                            fid_cnt++;
                                
                            
                        }
                        //GET THE NEWLY OPENED FILE
                        temp = find_by_fid(files, fid);

                        //DO READ OPERATIONS
                        char *rbuff;
                        rbuff = (char *)malloc(size*sizeof(char));
                        memset(rbuff, 0, size);
                        int count = 0, ret = 0;
                        //SEEK TO POS (ASSUME POS IS FROM THE START FO THE FILE)
                        lseek(temp->fd, pos, SEEK_SET);

                        while (count < size) {
                            ret = read(temp->fd, rbuff + count, size - count);
                            if (ret == 0) {
                                // EOF reached
                                break;
                            } else if (ret == -1) {
                                perror("read");
                                break;
                            }
                            count += ret;
                        }
                        type = READ_DONE;
                        //CHECK IF COUNT EQUAL SIZE OR LESS
                        if(rbuff != NULL){
                            printf("read: %s\n", rbuff);
                        }
                        serialize(buff, &type, fname, &temp->fid,&pos , &count, &seqno,&rn ,rbuff);
                        sendto(udp_sock,buff, SIZE, 0,  (struct sockaddr*)&client, client_len);
                        if(ret == -1){
                            perror("sendto");
                        }
                        free(rbuff);
                        rbuff = NULL;


                    }
                }
                else{
                    printf("READ:\n");
                    printf("file name is NULL\n");
                }

            }
            else if(type == WRITE){
                
                if(fname != NULL){
                    printf("got write\n");
                    temp = find_by_fid(files, fid);
                    //FILE IS OPEN
                    if(temp != NULL){
                        char *wbuff = payload;

                        int count = 0, ret = 0;
                        //SEEK TO POS (ASSUME POS IS FROM THE START FO THE FILE)
                        lseek(temp->fd, pos, SEEK_SET);

                        while (count < size) {
                            ret = write(temp->fd, wbuff + count, size - count);
                            if (ret == 0) {
                                // EOF reached
                                break;
                            } else if (ret == -1) {
                                perror("read");
                                break;
                            }
                            count += ret;
                        }

                        type = WRITE_DONE;
                        //CHECK IF COUNT EQUAL SIZE OR LESS
                        if(wbuff != NULL){
                            printf("wrote: %s\n", wbuff);
                        }
                        serialize(buff, &type, fname, &temp->fid,&pos , &count, &seqno,&rn, wbuff);
                        sendto(udp_sock,buff, SIZE, 0,  (struct sockaddr*)&client, client_len);
                        wbuff = NULL;


                    }//FILE HAS CLOSED NEED TO REOPEN
                    else{
                        printf("got write\n");

                        //REOPEN AND SAVE WITH PREVIOUS FID
                        snprintf(filepath, sizeof(filepath), "%s/%s", bdir, fname);
                        //printf("filepath is: %s\n", filepath);
                        int fd = open(filepath, O_RDWR | O_CREAT, 0644);
                        
                        
                        if(fd != -1){
                            //add file to the list along with its fd, fname and FID FROM REQ
                            add(files, fname, fd, fid);
                            //send answer
                            type = OPEN_REP;
                            serialize(buff,&type, fname,&fid_cnt, &pos, &size, &seqno,&rn, payload);
                            sendto(udp_sock,buff, SIZE, 0,  (struct sockaddr*)&client, client_len);
                            //update fid
                            fid_cnt++;
                                
                            
                        }
                        //GET THE NEWLY OPENED FILE
                        temp = find_by_fid(files, fid);

                        //WRITE CODE
                        char *wbuff = payload;
                        int count = 0, ret = 0;
                        //SEEK TO POS (ASSUME POS IS FROM THE START FO THE FILE)
                        lseek(temp->fd, pos, SEEK_SET);

                        while (count < size) {
                            ret = write(temp->fd, wbuff + count, size - count);
                            if (ret == 0) {
                                // EOF reached
                                break;
                            } else if (ret == -1) {
                                perror("read");
                                break;
                            }
                            count += ret;
                        }

                        type = WRITE_DONE;
                        //CHECK IF COUNT EQUAL SIZE OR LESS
                        if(wbuff != NULL){
                            printf("wrote: %s\n", wbuff);
                        }
                        serialize(buff, &type, fname, &temp->fid,&pos , &count, &seqno,&rn, wbuff);
                        sendto(udp_sock,buff, SIZE, 0,  (struct sockaddr*)&client, client_len);
                        wbuff = NULL;

                    }
                }
                else{
                    printf("WRITE:\n");
                    printf("fname is NULL\n");
                }
            }
            else if(type == TRUNC){
                if(fname != NULL && fid > 0){
                    printf("got truncate\n");
                    temp = find_by_fid(files, fid);

                    //file is already open
                    if(temp != NULL){

                        ret = ftruncate(temp->fd, size);
                        printf("truncate ret is: %d\n", ret);
                        
                        char empty[2] = {"\0"};
                        type = TRUNC_DONE;
                        serialize(buff, &type, fname, &temp->fid, &pos, &size, &seqno, &rn, empty);
                        sendto(udp_sock, buff, SIZE,0, (struct sockaddr*)&client, client_len);

                    }//file needs to be reopened
                    else{
                        //TODO: OPEN AND TRUNCATE
                        printf("got truncate\n");
                        //REOPEN AND SAVE WITH PREVIOUS FID
                        snprintf(filepath, sizeof(filepath), "%s/%s", bdir, fname);
                        //printf("filepath is: %s\n", filepath);
                        int fd = open(filepath, O_RDWR | O_CREAT, 0644);
                        
                        
                        if(fd != -1){
                            //add file to the list along with its fd, fname and FID FROM REQ
                            add(files, fname, fd, fid);
                            //send answer
                            type = OPEN_REP;
                            serialize(buff,&type, fname,&fid_cnt, &pos, &size, &seqno,&rn, payload);
                            sendto(udp_sock,buff, SIZE, 0,  (struct sockaddr*)&client, client_len);
                            //update fid
                            fid_cnt++;
                                
                            
                        }
                        //GET THE NEWLY OPENED FILE
                        temp = find_by_fid(files, fid);

                        ret = ftruncate(temp->fd, size);
                        printf("truncate ret is: %d\n", ret);
                        
                        char empty[2] = {"\0"};
                        type = TRUNC_DONE;
                        serialize(buff, &type, fname, &temp->fid, &pos, &size, &seqno, &rn, empty);
                        sendto(udp_sock, buff, SIZE,0, (struct sockaddr*)&client, client_len);

                    }

                }else{
                    printf("TRUNCATE:\n");
                    printf("fname is invalid or fid < 0\n");                    
                }

            }    

            if(fname != NULL){
                free(fname);
                fname = NULL;
            }
            if(payload != NULL){
                free(payload);
                payload = NULL;
            }
        }


    }

    return NULL;

}


void init_contact_info(int port_num){
    struct ifaddrs *ifaddr, *ifa;
    int found = 0;
    
    // Initialize server structure
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port_num);  // Example port, change as needed    
    inet_pton(AF_INET, IP, &(server.sin_addr));
 
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return;
    }

    // Walk through linked list of interfaces
    for (ifa = ifaddr; ifa != NULL && !found; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        // Check for IPv4 addresses only
        if (ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in *addr = (struct sockaddr_in *)ifa->ifa_addr;
            
            // Skip localhost (127.0.0.1)
            if (addr->sin_addr.s_addr != htonl(INADDR_LOOPBACK)) {
                // Copy the IP address to our server structure
                server.sin_addr = addr->sin_addr;
                server_len = sizeof(server);
                found = 1;
                
                // Print the IP for verification
                char ip_str[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(server.sin_addr), ip_str, INET_ADDRSTRLEN);
                printf("IP address: %s, PORT: %d\n", ip_str, port_num);
            }
        }
    }

    freeifaddrs(ifaddr);
    
    if (!found) {
        printf("No suitable network interface found\n");
        return;
    }
    
}

void server_init(char *base_dir) {
    //init list
    files = list_init();

    udp_sock = init_udp_socket(&server);

    if (mkdir(base_dir, 0777) == -1) {
        if (errno != EEXIST) {
            perror("mkdir");
            exit(EXIT_FAILURE);
        }
    }

    //TODO: FREE BDIR WHEN SERVER IS DESTROYED
    bdir = strdup(base_dir);
//////////////////////////////////////////////
    int rn_fd = open(rn_file, O_RDWR | O_CREAT | O_EXCL, 0644);
    if (rn_fd == -1) {
    
        if (errno == EEXIST) {

            // Open the file without O_EXCL if you want to work with it
            rn_fd = open(rn_file, O_RDWR, 0644);
            
            int last_rn;
            read(rn_fd, &last_rn, sizeof(int));
            rn =  last_rn + 1;
            lseek(rn_fd, 0, SEEK_SET);
            int ret = ftruncate(rn_fd, sizeof(int));
            //printf("truncate ret is: %d\n", ret);
            write(rn_fd, &rn, sizeof(int));
            printf("rn:%d\n", rn);

            if (rn_fd == -1) {
                perror("open (second attempt)");
                exit(EXIT_FAILURE);
            }
        } else {
            perror("open");
            exit(EXIT_FAILURE);
        }
    } else {
        //file created set rn to 0
        rn = 0;
        write(rn_fd, &rn, sizeof(rn));
        printf("rn:%d\n", rn);

    }


/////////////////////////////////////////////
    //start receiving thread    
    int result = pthread_create(&receiver, NULL, receive_udp, NULL);

    if (result != 0) {
        perror("pthread_create failed");
        exit(EXIT_FAILURE);  
    }
}

void server_destroy() {
    
    destroy(files);
    free(bdir);

    int result = pthread_join(receiver, NULL);
    if (result != 0) {
        perror("pthread_join failed");
        exit(EXIT_FAILURE);  
    }
}

int init_udp_socket(struct sockaddr_in *server_addr) {
    int sock_fd;
    int opt = 1;
    
    // Create UDP socket
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        perror("socket creation failed");
        return -1;
    }
    
    // Set socket options
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        close(sock_fd);
        return -1;
    }
    
    // Bind the socket to the server address
    if (bind(sock_fd, (struct sockaddr *)server_addr, sizeof(*server_addr)) < 0) {
        perror("bind failed");
        close(sock_fd);
        return -1;
    }
    
    return sock_fd;
}