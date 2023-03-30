#include "screen_x11.h"

int main()
{
    std::unique_ptr<ScreenCapture> screenCapture = ScreenCapture::create();
    if(screenCapture != nullptr)
    {
        // screenCapture->captureFrame();
    }

    return 0;

}