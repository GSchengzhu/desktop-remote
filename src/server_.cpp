#include "server_.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <functional>
#include <unistd.h>
#include <string.h>
#include <fmt/printf.h>
#include <png.h>
#include <signal.h>

void sig_pipe_handle(int)
{

}

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
    if(screen_capture_ == nullptr)
    {
        fmt::print("ScreenCapture::create failed\n");
        return false;
    }

// 
    // sigaction(SIGPIPE,&(struct sigaction){SIG_IGN},NULL);
    signal(SIGPIPE,sig_pipe_handle);

    return true;
}

void CaptureServer::start()
{
    listen(server_id,10);

    while (true)
    {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_id = accept(server_id,(struct sockaddr*)&client_addr,&client_len);
        fmt::print("accept client id :{}\n",client_id);
        if(client_id > 0)
        {
            std::thread tmp(std::bind(&CaptureServer::sendPicture,this,client_id));
            tmp.detach();
        }else
        {
            perror("accept error\n");
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
            fmt::print("screen capture success ,start send\n");
            Message msg;
            msg.type_ = 0;
            msg.data_.picture = picture;
            int messageSize = sizeof(Message::type_) +   //message类型
                              sizeof(picture.width) +    //图片宽度
                              sizeof(picture.height)+    //图片高度
                              sizeof(picture.row_stride)+ //图片每行长度
                              picture.row_stride*picture.height;//图片数据长度
            int dataSSize = sizeof(unsigned int) +  messageSize;
            fmt::print("message size {}\n",messageSize);
            fmt::print("width:{},height:{},row_stride:{}\n",
                       picture.width,picture.height,picture.row_stride);
            char *sendData = (char*)malloc(dataSSize);
            bzero(sendData,dataSSize);
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
            memcpy(sendData+sizeof(unsigned int)+sizeof(int)*4,
                    picture.data,picture.height*picture.row_stride);

            free(picture.data);
            if(!write_(client_id,sendData,dataSSize))
            {
                fmt::print("write to client failed\n");
                return ;
            }

            fmt::print("write to client success\n");
        }
        else
        {
            fmt::print("screen capture failed\n");
        }
    }
}

bool CaptureServer::write_(int client_id,char* data,int length)
{
    int writen = 0;
    while (writen != length)
    {
        // sedn
        int write_once = write(client_id,data+writen,length);
        if(write_once < 0)
        {
            perror("write error:");
            return false;
        }
        writen += write_once;
    }
    return true;
}