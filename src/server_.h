#ifndef CAPTURE_SERVER_H_
#define CAPTURE_SERVER_H_

#include <memory>
#include "screen_x11.h"

class CaptureServer
{
public:
    CaptureServer() = default;
    ~CaptureServer();
    static std::unique_ptr<CaptureServer> create();
    void start();

private:
    bool init();
    void sendPicture(int client_id);
    bool write_(int client_id,char* data,int length);

private:
    int server_id;
    std::unique_ptr<ScreenCapture> screen_capture_;
};


#endif