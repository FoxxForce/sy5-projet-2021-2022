#ifndef REQUEST_SATURND_H
#define REQUEST_SATURND_H
#include "read-pipe.h" 

uint16_t read_request(int fd, const char *path_reply_path);
uint64_t read_request_cr(int fd);

#endif // REQUEST_SATURND