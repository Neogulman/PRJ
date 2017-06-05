/*
 * socket을 이용한 client(arduino) - server(ubuntu aws server) 통신
 * mysql(mariaDB) 연결 및 barcode 정보 쿼리 전달
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <mysql.h>

void error_handling(char *message);

#define PORT 9000
#define DB_HOST "13.58.123.237"
#define DB_USER "root"
#define DB_PASS "111111"
#define DB_NAME "neogulman"


int query(char * barcode)
/* arduino 에서 보내오는 barcode 값을 아래의 main 함수에서 받아서 DB로 보내는 함수*/
{
    MYSQL *connection = NULL, conn;
    MYSQL_RES *sql_result;
    MYSQL_ROW sql_row;
    int query_stat;
    char query[255];

    mysql_init(&conn);
    connection = mysql_real_connect(&conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 3306, (char *)NULL, 0);
	// 위에서 정의한 포트, IP주소, 유저, 패스워드, DB이름을 통해 mysql 연결

    if(connection == NULL)
    {
        fprintf(stderr, "Mysql connection error : %s", mysql_error(&conn));
        return 1;
    }
    
    sprintf(query, "insert into barcode_test values ('%s')", barcode);
    query_stat = mysql_query(connection, query);
	// insert 문에 전달받은 barcode를 포함하여 쿼리문 전송

    if(query_stat != 0)
    {
        fprintf(stderr, "Mysql query error : %s", mysql_error(&conn));
        return 1;
    }
    mysql_close(connection);
	// mysql 연결 종료
}

int main()
	// socket 통신 main 함수
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    int clnt_addr_size, str_len;
    char message[20], end;

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(serv_sock == -1)
    {
        error_handling("socket() error");
    }
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	// 소켓 생성

    if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");
	// 소켓에 주소 할당

    if(listen(serv_sock, 5) == -1)
        error_handling("listen() error");
	// client 대기

	while(end!=EOF)
	{
        memset(message, 0, sizeof(message));
        clnt_addr_size = sizeof(clnt_addr);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
		// client 연결
        if(clnt_sock == -1)
            error_handling("accept() error");
	
        str_len = read(clnt_sock, message, sizeof(message) -1);
        if(str_len == -1)
            error_handling("read() error");
		// message (barcode 정보) 받음

        if(query(message) == 1)
            error_handling("query() error");
		// message를 포함하여 query() 함수 실행

//        printf("read data : %s\n",message);
        close(clnt_sock);
	}
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}