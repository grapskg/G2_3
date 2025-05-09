//SERIALIZATION/DESERIALIZATION FILE
#include "server.h"

int serialize_int(void *buffer,  int *x, int *offset){
    int y;

    y = htonl(*x);//network byte order
    memcpy(buffer+(*offset), &y, sizeof(int));
    *offset += sizeof(int);

    return 1;
}

int serialize_string(void *buffer, const char *str, int *offset) {
    // Handle NULL strings
    if (str == NULL) {
        // Option 1: Serialize as empty string
        int net_len = htonl(0);
        memcpy((char*)buffer + *offset, &net_len, sizeof(int));
        *offset += sizeof(int);
        return 1;
        
        // Option 2: Return error
        // return -1;
    }
    
    int len = strlen(str);           // Get string length (no null terminator)
    int net_len = htonl(len);        // Convert length to network byte order
    
    // First, write the length of the string
    memcpy((char*)buffer + *offset, &net_len, sizeof(int));
    *offset += sizeof(int);

    // Then, write the actual string bytes
    memcpy((char*)buffer + *offset, str, len);
    *offset += len;

    return 1;
}

int serialize_enum(void *buffer,msg_type *msg, int *offset){
    msg_type temp;

    temp = htonl(*msg);//network byte order
    memcpy(buffer+(*offset), &temp, sizeof( msg_type));
    *offset += sizeof(msg_type);

    return 1;
}

//ASSUME  BUFFER HAS ENOUGH SPACE
//need to declare
int serialize(void *buffer, msg_type *msg , char *fname, int *fid, int *pos, int *size, char *payload){
    int offset=0;


    return offset;
}


int deserialize_int(void *buffer,  int *x, int *offset){
    int y;

    memcpy( &y, buffer+(*offset), sizeof(int));
    *x = ntohl(y);//host byte order
    *offset += sizeof(int);

    return 1;
}

int deserialize_enum(void *buffer, msg_type *msg, int *offset){
     msg_type temp;

    memcpy( &temp, buffer+(*offset), sizeof(msg_type));
    *msg = ntohl(temp);//host byte order
    *offset += sizeof( msg_type);

    return 1;
}

char *deserialize_string(void *buffer, int *offset) {
    int net_len;
    memcpy(&net_len, (char *)buffer + *offset, sizeof(int));
    *offset += sizeof(int);

    int len = ntohl(net_len);  // Convert from network byte order

    // Allocate memory for string + null terminator
    char *str = malloc(len + 1);
    if (!str) return NULL; // Allocation failed

    memcpy(str, (char *)buffer + *offset, len);
    str[len] = '\0';  // Null-terminate
    *offset += len;

    return str;
}


//need to declare
int deserialize(void *buffer, msg_type *msg , char *fname, int *fid, int *pos, int *size, char *payload){
    int offset = 0;


    
    return offset;
}

