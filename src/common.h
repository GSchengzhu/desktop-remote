#ifndef COMMON_H_H
#define COMMON_H_H

#include <vector>
#include <stdint.h>
using ByteArray = std::vector<uint8_t>;

// enum  MessageType{ PICTURE,COMMAND };
typedef struct MessagePicture_
{
    int width;
    int height;
    int row_stride;
    char *data;
} MessagePicture;

typedef struct MessageCommand_
{

} MessageCommand;

typedef struct Message_
{
    int type_;
    union MessageData
    {
        MessagePicture picture;
        MessageCommand command;
    } data_;
    
} Message;

#endif