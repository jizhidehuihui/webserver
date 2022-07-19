#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
int main()
{
    char *p;
    int ret;
    ret=truncate("hello",100);
    if(ret<0)
    {
        perror("truncate error\n");
        exit(1);
    }
    p=mmap(NULL,100,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
    if(p==MAP_FAILED)
    {
        perror("mmap error\n");
    }
    pid_t pid=fork();
    if(pid==0)
    {
        sprintf(p,"hello this is anonymous mmap test\n");
        
    }
    else{
        sleep(1);
        printf("%s",p);
        pid_t pid=wait(NULL);
        printf("pid %d is exit succeed\n",pid);
    }
    ret=munmap(p,100);
    if(ret<0)
    {
        perror("munmap error");

    }

}