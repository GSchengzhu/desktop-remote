#include "server_.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <functional>
#include <unistd.h>
#include <string.h>

CaptureServer::~CaptureServer()
{

}

std::unique_ptr<CaptureServer> CaptureServer::create()
{
    std::unique_ptr<CaptureServer> instance = std::make_unique<CaptureServer>();
    if(instance->init())
    {
        return instance;
    }   
    return nullptr;
}

bool CaptureServer::init()
{
    server_id = socket(AF_INET,SOCK_STREAM,0);
    if(server_id < 0)
    {
        perror("socket error\n");
        return false;
    }
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9000);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(server_id,(struct sockaddr*)&server_addr,sizeof(struct sockaddr)) < 0)
    {
        perror("bind error\n");
        return false;
    }
    
    screen_capture_ = ScreenCapture::create();
    if(screen_capture_ != nullptr)
    {
        return false;
    }

    return true;
}

void CaptureServer::start()
{
    listen(server_id,10);

    while (true)
    {
        struct sockaddr_in client_addr;
        socklen_t client_len;

        int client_id = accept(server_id,(struct sockaddr*)&client_addr,&client_len);
        if(client_id > 0)
        {
            std::thread tmp(std::bind(&CaptureServer::sendPicture,this,client_id));
            tmp.detach();
        }
    }  
}

void CaptureServer::sendPicture(int client_id)
{
    while (1)
    {
        MessagePicture picture;
        bool status = screen_capture_->captureFrame(picture);
        if(status)
        {
            Message msg;
            msg.type_ = 0;
            msg.data_.picture = picture;
            int messageSize = sizeof(Message::type_) +   //message类型
                              sizeof(picture.width) +    //图片宽度
                              sizeof(picture.height)+    //图片高度
                              sizeof(picture.row_stride)+ //图片每行长度
                              sizeof(sizeof(char))*picture.row_stride*picture.height;//图片数据长度
            int dataSSize = sizeof(unsigned int) +  messageSize;
                            
            char *sendData = (char*)malloc(dataSSize);
            *(unsigned int*)sendData = messageSize;
            *(int*)(sendData+sizeof(unsigned int)) = msg.type_;
            *(int*)(sendData+
                            sizeof(unsigned int)+
                            sizeof(Message::type_)) = picture.width;
            *(int*)(sendData+
                             sizeof(unsigned int)+
                             sizeof(Message::type_)+
                             sizeof(picture.width)) = picture.height;
            *(int*)(sendData+
                             sizeof(unsigned int)+
                             sizeof(Message::type_)+
                             sizeof(picture.width)+
                             sizeof(picture.height)) = picture.row_stride;
            *(int*)(sendData+
                             sizeof(unsigned int)+
                             sizeof(Message::type_)+
                             sizeof(picture.width)+
                             sizeof(picture.height)) = picture.row_stride;
            strncpy(sendData+sizeof(unsigned int)+sizeof(Message::type_)+sizeof(picture.width)+sizeof(picture.height),
                    picture.data,picture.height*picture.row_stride);
            free(picture.data);
            if(!write_(client_id,sendData,dataSSize))
            {
                return ;
            }
        }
    }
}

bool CaptureServer::write_(int client_id,char* data,int length)
{
    int writen = 0;
    while (writen != length)
    {
        int write_once = write(client_id,data+writen,length);
        if(write_once < 0)
        {
            return false;
        }
        writen += write_once;
    }
    return true;
}