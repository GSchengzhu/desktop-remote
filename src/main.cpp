#include "server_.h"

int main()
{
    std::unique_ptr<CaptureServer> server_ = CaptureServer::create();
    if(server_ != nullptr)
    {
        server_->start();
    }

    return 0;
}