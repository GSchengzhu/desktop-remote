#include "client.h"
#include <unistd.h>

int main()
{
    {
        std::unique_ptr<CaptureClient> instance = CaptureClient::create();
        for(int i = 0; i < 10; i++)
        {
            instance->receivePicture();
        }
    }

    return 0;
}