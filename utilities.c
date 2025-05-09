#include "server.h"

pthread_t receiver;
list_t *files;
struct sockaddr_in server;
socklen_t server_len;
int udp_sock;

void *receive_udp(void *arg){
    char buff[SIZE];
    struct sockaddr_in client;
    socklen_t client_len;
    msg_type type;
    char * fname=NULL, *payload=NULL;
    int fid, pos, size, ret;
    while(1){
        ret = recvfrom(udp_sock, buff, SIZE, 0, (struct sockaddr *)&client,&client_len);

        if(ret > 0){
            deserialize(buff,&type,fname, &fid, &pos,&size,payload);

            if(type == OPEN){

            }
            else if(type == READ){

            }
            else if(type == WRITE){

            }
            else if(type == TRUNC){

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

void server_init() {
    //init list
    files = list_init();

    udp_sock = init_udp_socket(&server);
    //start receiving thread    
    int result = pthread_create(&receiver, NULL, receive_udp, NULL);

    if (result != 0) {
        perror("pthread_create failed");
        exit(EXIT_FAILURE);  
    }
}

void server_destroy() {
    
    destroy(files);

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