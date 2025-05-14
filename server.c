#include "server.h"


int main(int argc, char *argv[]){

    if(argc != 3){
        printf("wrong number of args\n");
        printf("Usage: %s <PORT> <FS_PATH> \n", argv[0]);
        return -1;
    }

    //init port and ip
    init_contact_info(atoi(argv[1]));
    //init lists and threads
    server_init(argv[2]);

    sleep(100000);

    server_destroy();

    return 0;
}