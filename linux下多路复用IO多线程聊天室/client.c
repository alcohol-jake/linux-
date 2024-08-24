#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <time.h>

#include "client.h"
#include "proto.h"

#define IP "127.0.0.1"

struct user myself;

void savechatmessgae(struct user rc,int op)//保存聊天记录
{
    FILE *file;
    
    time_t now;
    struct tm *local_time;
    char timestamp[20];
    time(&now);
    local_time = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y%m%d%H%M%S", local_time);//将当前时间转换为字符串存在timestamp中
    
    file = fopen(myself.username, "a");
    if (file == NULL) {
        perror("File opening failed");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "%s\n", timestamp);//先把当前时间存下
    char message[2*MAXSIZE];
    if(op==1){
        sprintf(message,"%s(私聊):%s", rc.username,rc.message);
    }
    else{
        sprintf(message,"%s(群聊):%s", rc.username,rc.message);
    }
    fprintf(file, "%s\n", message);//对方发送的消息存下来

    fclose(file);
}

void saveFileContent(struct user rc)//保存文件
{
    char folderName[] = "file";
    char fileName[256];
    FILE *file;
    
    time_t now;
    struct tm *local_time;
    char timestamp[20];
    time(&now);
    local_time = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y%m%d%H%M%S", local_time);
    
    struct stat st = {0};
    if (stat(folderName, &st) == -1) {
        mkdir(folderName, 0700);
    }
    sprintf(fileName, "%s/%s_%s_received_file.txt", folderName, rc.username, timestamp);
    file = fopen(fileName, "w");
    if (file == NULL) {
        perror("File opening failed");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "%s", rc.message);

    fclose(file);

    printf("文件已保存至 %s\n", fileName);
}

void * readrecv(void *p)//子线程处理接收
{
    struct user rc;
    int sd=*((int *)p);
    while(1){
        if(recv(sd, &rc, sizeof(rc), 0) == -1) {
            perror("recv count");
        }
        switch(rc.type)
        {
            case 1://私聊
                printf("%s(私聊):%s\n",rc.username,rc.message);
                savechatmessgae(rc,1);
                break;
            case 2://群聊
                printf("%s(群聊):%s\n",rc.username,rc.message);
                savechatmessgae(rc,2);
                break;
            case 3://查看在线人数
                printf("当前在线人数为:%s\n",rc.message);
                break;
            case 4://修改个性签名
                printf("您新的个性签名为:%s\n",rc.signature);
                break;
            case 5://修改密码
                printf("修改密码成功！\n");
                break;
            case 6://发送文件
                printf("您受到来自%s发送的文件\n",rc.username);
                saveFileContent(rc);
                break;
            case 7://退出登陆
                pthread_exit(NULL);
                break;
            case 8://注销账号
                printf("注销账号成功!\n");
                pthread_exit(NULL);
                break;   
            default:
                break;
        }
    }
}
void c_togetherchat(int sd)//群聊
{
    myself.type=2;
    printf("请输入您要发送的内容:\n");
    scanf("%s",myself.message);
    while (getchar() != '\n');
    send(sd,&myself,sizeof(myself),0);
}

void c_alonechar(int sd)//私聊
{
    myself.type=1;
    printf("请输入对方的名称:\n");
    scanf("%s",myself.toname);
    while (getchar() != '\n');
    printf("请输入您要发送的内容:\n");
    scanf("%s",myself.message);
    while (getchar() != '\n');
    send(sd,&myself,sizeof(myself),0);
}

void c_changesig(int sd)//修改个性签名
{
    myself.type=4;
    printf("请输入新的个性签名:\n");
    scanf("%s",myself.signature);
    while (getchar() != '\n');
    send(sd,&myself,sizeof(myself),0);
}

void c_changepassword(int sd)//修改密码
{
    myself.type=5;
    printf("请输入新的密码:\n");
    scanf("%s",myself.password);
    while (getchar() != '\n');
    send(sd,&myself,sizeof(myself),0);
}

void c_checkperson(int sd)//查询在线人数
{
    myself.type=3;
    send(sd,&myself,sizeof(myself),0);
}

void c_destroy(int sd)//注销账号
{
    myself.type=8;
    char x;
    printf("您确定要注销账号吗?:Y/N\n");
    scanf("%c",&x);
    while (getchar() != '\n');
    if(x=='Y'){
        send(sd,&myself,sizeof(myself),0);
    }
    else{
        printf("明智的选择！\n");
        sleep(2);
    }
}

void c_exit(int sd)//退出登陆
{
    myself.type=7;
    send(sd,&myself,sizeof(myself),0);
}

void c_sendfile(int sd)//发送文件
{
    char filepath[50];
    myself.type=6;
    printf("请输入接收方的名称:\n");
    scanf("%s",myself.toname);
    while (getchar() != '\n');
    printf("请输入您要发送的文件路径:\n");
    scanf("%s",filepath);
    while (getchar() != '\n');
    FILE *fp;
    fp = fopen(filepath, "r");
    if (fp == NULL) {
        perror("File opening failed");
        exit(EXIT_FAILURE);
    }
    fgets(myself.message,MAXSIZE,fp);
    fclose(fp);
    send(sd,&myself,sizeof(myself),0);
}

void c_checkchat(int sd)//查看聊天记录
{
    FILE *file;
    char buf[MAXSIZE];
    file = fopen(myself.username, "r");
    if (file == NULL) {
        perror("File opening failed");
        exit(EXIT_FAILURE);
    }
    
    while(fgets(buf,MAXSIZE,file)!=NULL){
        printf("%s", buf);
    }

    fclose(file);
    // 添加阻塞等待用户按下回车键
    printf("按下回车键继续...\n");
    while (getchar() != '\n');
    
}

void sendmyself(int sd)//重复用户页面
{
    pthread_t read_id;
    
    pthread_create(&read_id, NULL, readrecv, (void *)&sd);
    
    pthread_detach(read_id);
    while(1){
        int flag=0;
        login_menu();
        printf("请输入您要进行的操作:\n");
        int ch;
        /*scanf("%d",&ch);
        while (getchar() != '\n');*/
        char input[100]; // Assuming a maximum input of 100 characters
        fgets(input, sizeof(input), stdin); // Non-blocking input

        ch = atoi(input); // Convert input to integer
        switch(ch)
        {
            case 1://私聊
                c_alonechar(sd);
                break;
            case 2://群聊
                c_togetherchat(sd);
                break;
            case 3://查看在线人数
                c_checkperson(sd);
                break;
            case 4://修改个性签名
                c_changesig(sd);
                break;
            case 5://修改密码
                c_changepassword(sd);
                break;
            case 6://发送文件
                c_sendfile(sd);
                break;
            case 7://退出登陆
                c_exit(sd);
                flag=1;
                memset(&myself,0,sizeof(myself));
                break;
            case 8://注销账号
                c_destroy(sd);
                c_exit(sd);
                flag=1;
                memset(&myself,0,sizeof(myself));
                break;   
            case 9://查看聊天记录
                c_checkchat(sd);
                break;
            default:
                printf("操作无效!\n");
                sleep(2);
                break;
        }
        if(flag) break;
    }
    return;
}

void login_menu(void)//用户界面
{
    printf("\033[2J\033[H");
    printf("***用户:%s      ******************************************\n",myself.username);
    printf("***个性签名:%s    *******************************************\n",myself.signature);
    printf("***********************************************************\n");
    printf("**************************1.私聊***************************\n");
    printf("**************************2.群聊***************************\n");
    printf("**************************3.查看在线人数*******************\n");
    printf("**************************4.修改个性签名*******************\n");
    printf("**************************5.修改密码***********************\n");
    printf("**************************6.发送文件***********************\n");
    printf("**************************7.退出登陆***********************\n");
    printf("**************************8.注销账号***********************\n");
    printf("**************************9.查看聊天记录*******************\n");
    printf("***********************************************************\n");
    printf("***********************************************************\n");
}

void init_menu__(int sd)//重复初始界面
{
    while(1){
        init_menu();
        printf("请输入您要进行的操作:\n");  
        int ch;
        //scanf("%d",&ch);
        //选择登陆或注册
        char input[100]; // Assuming a maximum input of 100 characters
        fgets(input, sizeof(input), stdin); // Non-blocking input
        ch = atoi(input); // Convert input to integer
        switch(ch)
        {
            case 1:
                regist(sd);
                break;
            case 2:
                login(sd);
                break;
            default:
                break;
        }
    }
}

void init_menu(void)//初始化界面
{
    printf("\033[2J\033[H");
    printf("**********************************************************\n");
    printf("**********************************************************\n");
    printf("**********************************************************\n");
    printf("*********************swj的聊天室v0.1**********************\n");
    printf("**********************************************************\n");
    printf("*********1.注册***************************2.登陆**********\n");
    printf("**********************************************************\n");
    printf("**********************************************************\n");
}

void regist(int sd)//注册
{
    myself.type=10;
    printf("请输入您的用户名:\n");
    scanf("%s", myself.username);
    while (getchar() != '\n');
    printf("请输入您的密码:\n");
    scanf("%s", myself.password);
    while (getchar() != '\n');
    send(sd,&myself,sizeof(myself),0);//先发送要注册的用户名和密码
    
    recv(sd,&myself,sizeof(myself),0);
    
    if(strcmp(myself.message,"111")==0){
        printf("用户注册成功\n");
        sleep(2);
    }
    else{
        printf("用户名已存在\n");
        sleep(2);
    }
}

void login(int sd)//登陆
{
    myself.type=9;
    printf("请输入您的用户名:\n");
    scanf("%s", myself.username);
    while (getchar() != '\n');
    printf("请输入您的密码:\n");
    scanf("%s", myself.password);
    while (getchar() != '\n');
    //在数据库中匹配是否存在该用户，密码是否正确
    send(sd,&myself,sizeof(myself),0);
    recv(sd,&myself,sizeof(myself),0);
    
    if(strcmp(myself.message,"111")==0){//正确则登陆
        //登陆时发送自己的socket描述符和用户名给服务器
        myself.usersd=sd;
        myself.type=0;
        send(sd,&myself,sizeof(myself),0);
        sendmyself(sd);
    }
    else{//失败
        printf("用户名或密码错误\n");
        sleep(2);
    }
}

int main(int argc, char *argv[]) {
    struct sockaddr_in raddr;
    int sd = socket(AF_INET, SOCK_STREAM, 0);

    if (sd < 0) {
        perror("socket()");
        exit(1);
    }

    raddr.sin_family = AF_INET;
    raddr.sin_port = htons(atoi(SERVERPORT));
    inet_pton(AF_INET, IP, &raddr.sin_addr);

    if (connect(sd, (void *)&raddr, sizeof(raddr)) < 0) {
        perror("connect()");
        exit(1);
    }
    
    init_menu__(sd);
    
    close(sd);

    exit(0);
}
