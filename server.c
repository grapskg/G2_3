#include "server.h"


int main(int argc, char *argv[]){

    if(argc != 2){
        printf("Usage: %s <PORT>\n", argv[0]);
        return -1;
    }

    //init port and ip
    init_contact_info(atoi(argv[1]));
    //init lists and threads
    server_init();

    sleep(1000);

    server_destroy();

    return 0;
}