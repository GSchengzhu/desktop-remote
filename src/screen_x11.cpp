#include "screen_x11.h"
#include <png.h>
#include <string.h>
#include <fmt/printf.h>


ScreenCapture::~ScreenCapture()
{
    if(display_ != NULL)
    {
        XCloseDisplay(display_);
    }
}

std::unique_ptr<ScreenCapture> ScreenCapture::create()
{
    std::unique_ptr<ScreenCapture> instance = std::make_unique<ScreenCapture>();
    if(instance->init())
    {
        return instance;
    }
    return nullptr;
}

bool ScreenCapture::init()
{
    display_ = XOpenDisplay(nullptr);
    if(display_ == nullptr)
    {
        fmt::print("XOpenDisplay failed\n");
        return false;
    }
    root_window_ = XDefaultRootWindow(display_);
    XWindowAttributes attr;
    XGetWindowAttributes(display_,root_window_,&attr);
    
    XImage* tmp = XGetImage(display_,root_window_,0,0,
                            DisplayWidth(display_,0),
                            DisplayHeight(display_,0),
                            AllPlanes,ZPixmap);

    xshmsegmentinfo_ = new XShmSegmentInfo;
    image_ = XShmCreateImage(display_,DefaultVisual(display_,0),
                           tmp->depth,ZPixmap,NULL,xshmsegmentinfo_,
                           tmp->width,tmp->height);
    if(image_ == NULL)
    {
        fmt::print("XShmCreateImage failed\n");
        return false;
    }

    xshmsegmentinfo_->shmid = shmget(IPC_PRIVATE,tmp->height*tmp->bytes_per_line,IPC_CREAT|0777);
    if(xshmsegmentinfo_->shmid == -1)
    {
        fmt::print("shmget failed\n");
        return false;
    }

    xshmsegmentinfo_->shmaddr = image_->data = (char*)shmat(xshmsegmentinfo_->shmid,NULL,0);
    if(!XShmAttach(display_,xshmsegmentinfo_))
    {
        fmt::print("XShmAttach failed\n");
        return false;
    }
    return true;
}

bool ScreenCapture::captureFrame(MessagePicture& picture)
{
    if(XShmGetImage(display_,root_window_,image_,0,0,AllPlanes))
    {
        picture.width = image_->width;
        picture.height = image_->height;
        picture.row_stride = image_->bytes_per_line;
        int dataSize = picture.height*picture.row_stride;
        picture.data = (char*)malloc(dataSize);
        bzero(picture.data,dataSize);
        memcpy(picture.data,image_->data,dataSize);

        saveXImage();
        return true;
    }
    return false;
}

bool ScreenCapture::saveXImage()
{
    FILE *fp = fopen("test.png","w");
    png_image pi;
    pi.version = PNG_IMAGE_VERSION;
    pi.width = image_->width;
    pi.height = image_->height;
    pi.format = PNG_FORMAT_BGRA;

    unsigned int *tmp = (unsigned int*)image_->data;
    for(int x = 0; x < image_->width; x++)
    {
        for(int y = 0; y < image_->height; y++)
        {
            *tmp++ |= 0xff000000;
        }
    }
    fmt::print("width:{},height:{},row_stride:{}",image_->width,
               image_->height,image_->bytes_per_line);

    png_image_write_to_stdio(&pi,fp,0,image_->data,image_->bytes_per_line,NULL);
    fclose(fp); 

    FILE* cc = fopen("server.data","w");
    fwrite(image_->data,1,1024,cc);
    fclose(cc);
    return true;
}