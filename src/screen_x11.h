#ifndef SCREEN_X11_H_
#define SCREEN_X11_H_

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
// #include <X11/extensions/shmproto.h>
// #include <X11/extensions/shmstr.h>
#include <sys/shm.h>

#include <memory>
#include "common.h"


class ScreenCapture
{
public:
    ScreenCapture() = default;
    ~ScreenCapture();
    static std::unique_ptr<ScreenCapture> create();
    bool captureFrame(MessagePicture& pciture);

private:
    bool init();
    bool saveXImage();

private:
    Display* display_;
    Window root_window_;
    XImage* image_;
    XShmSegmentInfo* xshmsegmentinfo_;
};


#endif