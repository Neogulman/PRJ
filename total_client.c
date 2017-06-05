/*
 * barcode reader기로 barcode값을 읽어들인 뒤,
 * socket 통신을 통해 해당 barcode를 aws ubuntu server로 전송
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/input.h>

#define IP "13.58.123.237"
#define PORT 9000
#define KEY_PRESS 1
#define KEY_KEEPING_PRESSED 2

void error_handling(char *message);

int send_barcode(char *barcode, int size_barcode)
	// socket 통신하는 함수
{
    int serv_sock;
    struct sockaddr_in serv_addr;

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(serv_sock == -1)
        error_handling("socket() error");
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(IP);
    serv_addr.sin_port = htons(PORT);
	// 소켓 생성

    if(connect(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error");
    // 서버 연결   
    write(serv_sock, barcode, size_barcode);
	// 바코드 값 전송
    close(serv_sock);
    return 0;
}

int main (int argc, char *argv[])
{
    struct input_event ev[64];
    int fd, rd,size = sizeof (struct input_event);
    char name[256] = "Unknown",i;
    char *device = NULL;
    char decode_scancode[]={0,0,1,2,3,4,5,6,7,8,9,0};
    char barcode[20];
    int index, sock_flag=0;
 
   if (argv[1] == NULL){
        printf("Please enter path to device\n");
        return 1;
    }

    if ((getuid ()) != 0)
        printf ("You must be as root for correct work!\n");

    if (argc > 1)
        device = argv[1];
 
    if ((fd = open (device, O_RDONLY)) == -1)
    {
        printf ("Open error of device %s\n", device);
        return 1;
    }
	// 바코드 리더기 파일 오픈
 
    ioctl (fd, EVIOCGNAME (sizeof (name)), name);
    //printf ("Reading From : %s (%s)\n", device, name);
 
    while (1)
    {
        if ((rd = read (fd, ev, size * 64)) < size)
        {
            perror ("Read error");
            return 1;
        }
        if(sock_flag == 1)
			// flag가 1이면 socket 통신하는 함수 실행
		{
            send_barcode(barcode, index+1);
            index = 0;
            sock_flag = 0;
		}
        for (i=0; i< ((int)(rd/size)); i++)
        {
            if (ev[i].type == EV_KEY)
            {
                if ((ev[i].value == KEY_PRESS) || (ev[i].value == KEY_KEEPING_PRESSED))
                {
                    if (ev[i].code < 12)
                    {
                        barcode[index] = decode_scancode[ev[i].code]+'0';
                        index++;
                    }
                    else if (ev[i].code == 28)
						// ASCII 28 : 줄바꿈 = 바코드의 끝
                    {
                        barcode[index] = '\0';
                        sock_flag = 1;
                    }
                }
            }
        }
    }
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}