#include "client.h"
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <unistd.h>
#include <png.h>
#include <fmt/printf.h>

std::unique_ptr<CaptureClient> CaptureClient::create()
{
    std::unique_ptr<CaptureClient> instance = std::make_unique<CaptureClient>();
    if(instance->init())
    {
        return instance;
    }
    return nullptr;
}

void CaptureClient::receivePicture()
{
    char *len_data = (char*)malloc(sizeof(unsigned int));
    int ret = readDataByLength(client_id,sizeof(unsigned int),len_data);
    fmt::print("read message length : {}\n",ret);
    if(ret > 0)
    {
        unsigned int dataSize = *((unsigned int*)len_data);
        fmt::print("actual read message length : {}\n",dataSize);
        free(len_data);
        char *data = (char*)malloc(dataSize);
        bzero(data,dataSize);
        ret = readDataByLength(client_id,dataSize,data);
        fmt::print("actual read data length --  : {}\n",ret);
        if(ret < 0)
        {
            return;
        }
        
        int messageType = *((int*)data);
        int width = *(int*)(data+sizeof(int));
        int height = *(int*)(data+sizeof(int)*2);
        int row_stride = *(int*)(data+sizeof(int)*3);
        fmt::print("messagetype:{},width:{},height:{},row_stride:{}\n",
                    messageType,width,height,row_stride);
        savePicture(width,height,row_stride,data+sizeof(int)*4);
        free(data);
    }

    return;
}

bool CaptureClient::init()
{
    client_id = socket(AF_INET,SOCK_STREAM,0);
    if(client_id < 0)
    {
        perror("socket error\n");
        return false;
    }

    sockaddr_in client_addr;
    bzero(&client_addr,sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(9000);
    if(inet_pton(AF_INET,"192.168.121.210",&client_addr.sin_addr.s_addr) < 0)
    {
        perror("inet_pton error\n");
        return false;
    }

    if(connect(client_id,(struct sockaddr*)&client_addr,sizeof(client_addr)) < 0)
    {
        perror("connect error\n");
        return false;
    }

    return true;
}

int CaptureClient::readDataByLength(int client_id,int length,char* retData)
{
    int readed = 0;
    while (readed != length)
    {
        int once_read = read(client_id,retData+readed,length - readed);
        if(once_read < 0)
        {
            perror("read error:");
            return -1;
        }
        
        readed += once_read;
        // fmt::print("readed:{} , length:{}\n",readed,length);
    }

    return readed;
}

bool CaptureClient::savePicture(int width,int height,int row_stride,char* data)
{
    if(index_ > 10)
    {
        return false;
    }
    char filename[1024] = {0};
    sprintf(filename,"test_frame_%d.png",index_);
    FILE *file = fopen(filename,"w");
    png_image pi;
    pi.width = width;
    pi.height = height;
    pi.version = PNG_IMAGE_VERSION;
    pi.format = PNG_FORMAT_BGRA;
    fmt::print("save picture\n");
    unsigned int *tmp = (unsigned int*)data;
    for(int x = 0; x < width; x++)
    {
        for(int y = 0; y < height; y++)
        {
            *tmp++ |= 0xff000000;
        }
    }
    png_image_write_to_stdio(&pi,file,0,data,row_stride,NULL);
    fclose(file);

    index_++;
    return true;   

}

CaptureClient::~CaptureClient()
{
    close(client_id);
}