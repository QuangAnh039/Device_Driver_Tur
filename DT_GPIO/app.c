#include<stdio.h> 
#include<string.h> 
#include<unistd.h> 
#include<fcntl.h> 

#define DEVICE_FILE "/dev/led"

int main()
{
    int fd = 0;
    char buff[4];
    char var;
    ssize_t size = 0;
    while(1)
    {
        printf("\nYou want ON or OFF led?: \n");
        scanf("%s",&buff[0]);
        fd = open(DEVICE_FILE , O_RDWR);
        if (-1 == fd) 
        { 
	       printf("open() file failed\n");
            return -1;
        }
        if(strcmp(&buff[0], "ON") == 0)
        {
            var = '1';
            size = write(fd, var, strlen(var));
            if(size < 0)
            {
                printf("Write failed\n");
                return -1;
            }
        }
        if(strcmp(&buff[0], "OFF") == 0)
        {
            var = '0';
            size = write(fd, var, strlen(var));
            if(size < 0)
            {
                printf("Write failed\n");
                return -1;
            }
        } 
    }
    close(fd); 
    return 0;
}