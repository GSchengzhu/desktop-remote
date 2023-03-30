#ifndef CLIENT_H_
#define CLIENT_H_

#include <memory>

class CaptureClient
{
public:
    CaptureClient() = default;
    ~CaptureClient();

    static std::unique_ptr<CaptureClient> create();
    void receivePicture();

private:
    bool init();
    int readDataByLength(int client_id,int length,char *retData);
    bool savePicture(int width,int height,int row_stride,char* data);

private:
    int client_id = 0;
    int index_ = 0;
};

#endif