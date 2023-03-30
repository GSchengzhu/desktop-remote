#include <thread>
#include <stdio.h>
#include <unistd.h>
#include <functional>

void print(int c)
{
    printf("print\n");
}

int main()
{
    std::thread a(std::bind(print,1));
    a.detach();

    while (1)
    {
        pause();
    }
    

    return 0;
}