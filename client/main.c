#include "ds18b20.h"
#include <getopt.h>  //getopt_long函数需要
#include <arpa/inet.h> //网络
#include <time.h>
#include "gettime.h"
#include "mysqlite.h"
#include "transferdata.h"
#include "client.h"

void print_usage(char *progname)
{
    printf("%s usage:\n",progname);
    printf("-i(--ipaddr):specify server IP address\n");
    printf("-p(--port):specify server port.\n");
    printf("-h(--help):print this help information.\n");
	printf("-t(--time):set sending to server time interval.\n");

    return;
}

int main(int argc,char *argv[])
{
    int ds18b20_flag = 0;//定时发送数据给数据库标志
    char sbuf[100];
    package pack = {0};
    socket_t		   sock = {0};

    sqlite3			   *db;
	char			   *db_file = "../etc/client.db";
	// sqlite3_stmt       *stmt;

	int sockfd = -1;
    int rv = -1;

	// struct sockaddr_in servaddr ;

	char *servip;
    int port;
	int interval_time = 3;
    time_t			   last_sample_time = 0;//初始化 上一次采样的时间为0

	struct option opts[]=
    {
       { "ipaddr",required_argument,NULL,'i'},
       {"port",required_argument,NULL,'p'},
       {"help",no_argument,NULL,'h'},           //不用参数，no_argument
	   {"time",required_argument,NULL,'t'},
       {NULL,0,NULL,0}    //结束标志
    };
    ///////参数选择代码///////////
    int ch;
    int idx;
    while((ch=getopt_long(argc,argv,"i:p:t:h",opts,&idx))!= -1)///h不需要参数，所以没有冒号
    {
        switch(ch)
        {
            case 'i':
                servip =optarg;
                break;
            case 'p':
                port=atoi(optarg);  //字符串转换为整数
                break;
			case 't':
				interval_time = atoi(optarg);
				break;
            case 'h':
                print_usage(argv[0]);  //argv[0]程序名
                return 0;
        }
    }
	if(!servip||!port)
    {
        print_usage(argv[0]);
        return 0;
    }
	 sockfd = socket(AF_INET ,SOCK_STREAM,0);//////////创建socket
    if(sockfd<0)
    {
        printf("Create socket failure:%s\n",strerror(errno));
        return -2;
    }
    printf("create to server[%s:%d] successfully!\n",servip, port);



    db_open(&db, db_file);
	printf(" db_open over.\n");
    db_create(db,TA_CREATE);
    printf(" db_create over.\n");
    socket_init(&sock, servip, port);
   
	sock.fd=-1;
	sock.connected=0;
 
    while(1){
        if (interval_timer(&last_sample_time, interval_time))
        {
            if (makepackage(&pack))
			{
                printf("package error!!");
				continue;
			}
            memset(sbuf,0,sizeof(sbuf));
			sprintf(sbuf,"%s|%s|%.2f\n", pack.id, pack.time, pack.temperature);
            ds18b20_flag = 1;
            // write_to_db(db,&pack);

		}

        if (socket_check_connect(&sock)<=0)  //当前状态未连接
        {
			
			if (socket_connect(&sock) >= 0){  //连接状态由未连接转为连接
                printf("Connect to server[%s:%d] ok\n", sock.host, sock.port);
            }
            else {
                if(ds18b20_flag){                            //依旧未连接
                write_to_db(db, &pack);
			    printf("write to db\n");
                printf("connect() error: %s\n", strerror(errno));
            }
        }
            
		}
        else{        //连接状态
            while(db_count(db)>0){
                read_from_db(db, &pack); //读第一条，然后从数据库删除
                printf("reupload!!!\n");
			    sprintf(sbuf,"%s|%s|%.2f\n", pack.id, pack.time, pack.temperature);

			    if ( (rv=write(sock.fd, sbuf, strlen(sbuf))) < 0 )
			    {
				    // log_warn("Client write data to Server failure and save it in database\n");
				    // write_to_db(db, &pack);
                    printf("reupload数据失败: %s\n", strerror(errno));
				    socket_close(&sock);
			    }
                else{
                    printf("ReUpload SUCCESSFULLY!!\n");
                }
        }

        }
        if(ds18b20_flag)
        {
        if (write(sock.fd, sbuf, strlen(sbuf)) < 0) {
            printf("发送数据失败: %s\n", strerror(errno));
            socket_close(&sock); 
            ds18b20_flag = 0;
        }
        else{
            printf("已发送数据: %s\n", sbuf);
            ds18b20_flag = 0;
        }
        }


    }




    
}