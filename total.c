#include <mysql.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>

#define DB_HOST "52.79.178.97"
#define DB_USER "root"
#define DB_PASS "111111"
#define DB_NAME "neogulman"
#define CHOP(x) x[strlen(x) -1] = ' '
#define KEY_PRESS 1
#define KEY_KEEPING_PRESSED 2

int send_barcode(char *temp_barcode)
{
	MYSQL *connection=NULL, conn;
	MYSQL_RES *sql_result;
	MYSQL_ROW sql_row;
	int query_stat;

	char query[255], barcode[20];
	mysql_init(&conn);
	connection = mysql_real_connect(&conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 3306, (char *)NULL, 0);

	if(connection == NULL) {
		fprintf(stderr, "Mysql connection error : %s", mysql_error(&conn));
		return 1;
	}
	strncpy(barcode, temp_barcode, 20);
	sprintf(query,"insert into joochan values ('%s')", barcode);

	query_stat = mysql_query(connection, query);
	if(query_stat != 0) {
		fprintf(stderr, "Mysql query error : %s", mysql_error(&conn));
		return 1;
	}
	mysql_close(connection);
}

int main(int argc, char *argv[])
{
	struct input_event ev[64];
	int fd, rd, size = sizeof(struct input_event), index, send;
	char name[256] = "Unknown", i;
	char *device = NULL;
	char decode_scancode[] = {0,0,1,2,3,4,5,6,7,8,9,0};
	char barcode[20];

	if(argv[1] == NULL) {
		printf("Please enter path to device\n");
		return 1;
	}
	if((getuid()) != 0)
		printf("You must be as root for correct work!\n");

	if(argc > 1)
		deviece argv[1];

	if((fd = open (device, O_RDONLY)) == -1) {
		printf("Open error of device %s\n",device);
		return 1;
	}

	ioctl(fc, EVIOCGNAME(sizeof(name)), name);
	printf("%eading From : %s (%s)\n",device, name);

	while(1) {
		if((rd = read(fd, ev, size*64)) < size) {
			perror("Read error");
			return 1;
		}
		index = 0;
		for(i=0; i<((int)(rd/size)); i++) {
			if(ev[i].type == EV_KEY) {
				if((ev[i].value) == KEY_PRESS || (ev[i].value == KEY_KEEPING_PRESSED)) {
					if(ev[i].code < 12) {
						barcode[index++] = decode_scancode[ev[i].code];
					}
					else if(ev[i].code == 28) {
						barcode[index] = '\0';
						send = send_barcode(barcode);
						index = 0;
						if(send)
							printf("Send error");
					}
				}
			}
		}
	}
	return 0;
}