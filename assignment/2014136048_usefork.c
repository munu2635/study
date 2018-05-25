#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define MAXLINE 511
#define MAX_SOCK 1024
#define MAX_MESSAGE 5 //총 5개의 메세지를 저장한다.

char *EXIT_STRING = "exit";
char *START_STRING = " Connected to chat_server \n";

int maxfdp1;
int num_chat = 0;
int clisock_list[MAX_SOCK]; 		// 소캣 리스트 배열
time_t clisock_time[MAX_SOCK];		// 소캣 생성 시간 배열
char *clisock_ip[MAX_SOCK];		// 소캣 ip값 저장 배열

char* message_buf[MAX_MESSAGE];		// 사용자가 보낸 메세지 값을 저장할 배열
time_t message_time[MAX_MESSAGE];	// 사용자가 보낸 메세지의 시간값을 저장할 배열

int listen_sock;
int message_num = 0;
int last_save_point = 0;


int p_server(int argc, char *argv[]);	// 기본서버 함수

void addClient(int s, struct sockaddr_in *newcliaddr, time_t now);
int getmax();

void set_send_message(char* buf, time_t now);
void save_send_message(); 		// 메세지 내용  저장
void print_control_data(); 		// 접속자 내용 출력
void removeClient(int s);
int tcp_listen(int host, int port, int backlog);
void errquit(char *mesg);


int main(int argc, char *argv[]) {
	message_buf[MAX_MESSAGE] = NULL;
	if(argc != 2) {
		printf( "Usage : %s port \n", argv[0]);
		exit(0);
	}
 	p_server(argc, argv);


}
int p_server(int argc, char *argv[]) {
	struct sockaddr_in cliaddr;
	char buf[MAXLINE + 1];
	int i, j, nbyte, accp_sock, addrlen = sizeof(struct sockaddr_in);
	fd_set read_fds;
	time_t now;

	listen_sock = tcp_listen(INADDR_ANY, atoi(argv[1]), 5);

	while(1){
		FD_ZERO(&read_fds);
		FD_SET(listen_sock, &read_fds);
		for(i=0; i<num_chat; i++)
			FD_SET(clisock_list[i], &read_fds);
		maxfdp1 = getmax() + 1;
		puts("wait for client");
		if(select(maxfdp1, &read_fds, NULL, NULL, NULL) < 0)
			errquit("select fail");

		if(FD_ISSET(listen_sock, &read_fds)){
			accp_sock = accept(listen_sock, (struct sockaddr *)&cliaddr, &addrlen);
			if(accp_sock == -1)
				errquit("accept fail");
			addClient(accp_sock, &cliaddr, now);
			send(accp_sock, START_STRING, strlen(START_STRING), 0);
			printf("add user number - %d \n", num_chat);
		}

		for(i = 0; i < num_chat; i++){
			if(FD_ISSET(clisock_list[i], &read_fds)){

				nbyte = recv(clisock_list[i], buf, MAXLINE, 0);
				if(nbyte <= 0) {
					removeClient(i);
					continue;
				}

				buf[nbyte] = 0;
				if(strstr(buf, EXIT_STRING) != NULL) {
					removeClient(i);
					continue;
				}

				for ( j = 0; j < num_chat; j++)
					send(clisock_list[j], buf, nbyte, 0);
				set_send_message(buf, now);
				printf("%s\n", buf);
			}
		}
	}
	return 0;
}

void addClient(int s, struct sockaddr_in *newcliaddr, time_t now){
	char buf[20];
	inet_ntop(AF_INET, &newcliaddr -> sin_addr, buf, sizeof(buf));
	printf("new client : %s\n", buf);
	clisock_list[ num_chat ] = s;
	time(&now); clisock_time[ num_chat ] = now;
	clisock_ip[ num_chat ] = inet_ntoa(newcliaddr->sin_addr);
	num_chat++;
	print_control_data();
}

void removeClient(int s){
	close(clisock_list[s]);
	if( s != num_chat -1 ){
		clisock_list[s] = clisock_list[num_chat-1];
		clisock_time[s] = clisock_time[num_chat-1];
		clisock_ip[s] = clisock_ip[num_chat-1];
	}
	num_chat--;
	print_control_data(); 
}

int getmax(){
	int max = listen_sock;
	int i;
	for( i = 0; i < num_chat; i++)
		if(clisock_list[i] > max )
			max = clisock_list[i];
	return max;
}
// 컨트롤 데이터 관련 정보는 클라이언트를 삭제하거나 추가할때 배열을 통해 정보를 저장한다.
void print_control_data() {
	int i;
	printf("---------------------------------------------\n");
	printf(" 현재 서버 총 접속자수 : %d\n", num_chat);
	printf("---------------------------------------------\n");
	printf(" 현재 접속 중인 클라이언트 IP - 접속시간\n");
	if(num_chat == 0 ) printf(" 현재 접속자 수가 없습니다\n");
	for(i = 0; i < num_chat; i++){
		printf("%s - %s", clisock_ip[i], ctime(&clisock_time[i]) );
	}
	printf("---------------------------------------------\n");
}

// 메세지 관련 정보는 따로 저장하는 함수를 만들어서 저장한다.
void set_send_message(char* buf, time_t now){
	time(&now);
	if(message_num >= MAX_MESSAGE){
		save_send_message();
		message_num = 0;
	}

	message_buf[ message_num ] = buf;
	message_time[ message_num ] = now;
	message_num++;

}
// 메세지는 5개 이상의 메세지가 보내졌을때 txt값으로 데이터에 저장된다.
void save_send_message(){
	FILE *f;
	int i;
	f = fopen("save_message.txt", "a");
	for(i = 0; i < message_num; i++)
		fprintf(f, "(%lu)%s - %s", strlen(message_buf[i])-6, message_buf[i], ctime(&message_time[i]));
	fclose(f);
}
 

int tcp_listen(int host, int port, int backlog){
	int sd;
	struct sockaddr_in servaddr;

	sd = socket(AF_INET, SOCK_STREAM, 0);
	if(sd == -1)
		errquit("socket fail");

	bzero((char *)&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(host);
	servaddr.sin_port = htons(port);

	if(bind(sd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
		errquit("bind fail");

	listen(sd, backlog);
	return sd;
}

void errquit(char *mesg){
	perror(mesg);
	exit(0);
}
