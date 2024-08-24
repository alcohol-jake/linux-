#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <mysql/mysql.h>   // 包含MySQL数据库相关头文件
#include <sys/stat.h>
#include <time.h>

#include "server.h"
#include "proto.h"

UserNode *userList = NULL; // 用户链表头指针
struct user getuser;
void s_user_create(struct user getuser,int i)//判断注册的用户名是否已经存在
{
    MYSQL mysql_conn;                   
    MYSQL *mysql = mysql_init(&mysql_conn);
    if (mysql == NULL){
        printf("mysql init err");        
        exit(1);                         
    }
    mysql = mysql_real_connect(mysql, "localhost", "root", NULL, "chatuser", 3306, NULL, 0);  
    if (mysql == NULL){
        printf("connect err\n");        
        exit(1);                      
    }
    char *sql = "SELECT * FROM users"; 
    if (mysql_query(mysql, sql)!= 0){
        printf("query sql err: %s\n", mysql_error(mysql));  
    }
    MYSQL_RES *res = mysql_store_result(mysql); 
    if (res == NULL){
        printf("res err: %s\n", mysql_error(mysql));  
        exit(1);                              
    }
    int flag=0;
    int num = mysql_num_rows(res);       
    int count = mysql_field_count(mysql);  
    for (int i = 0; i < num; i++){
        MYSQL_ROW row = mysql_fetch_row(res);  
        if(strcmp(row[0],getuser.username)==0){
            flag=1;
        }
    } 
    if(flag){//说明当前用户名存在创建失败
        strncpy(getuser.message,"0",strlen("0"));
    }
    else{//说明当前用户名不存在创建成功
        strncpy(getuser.message,"111",strlen("111"));
        char sql[255]; // 分配足够的空间，这里假设 SQL 语句不超过 255 个字符
        sprintf(sql, "INSERT INTO users (name, password,signature) VALUES ('%s', '%s', '')", getuser.username, getuser.password);
        mysql_query(mysql, sql);
    }
    mysql_free_result(res);  
    mysql_close(mysql);   
    send(i,&getuser,sizeof(getuser),0);

}
void s_user_check(struct user getuser,int i)//mysql用户匹配
{
    MYSQL mysql_conn;                     // 定义MYSQL结构体变量
    MYSQL *mysql = mysql_init(&mysql_conn);  // 初始化MYSQL指针
    if (mysql == NULL){
        printf("mysql init err");         // 输出错误信息
        exit(1);                          // 退出程序
    }
    mysql = mysql_real_connect(mysql, "localhost", "root", NULL, "chatuser", 3306, NULL, 0);  // 连接到MySQL数据库
    if (mysql == NULL){
        printf("connect err\n");          // 输出连接错误信息
        exit(1);                          // 退出程序
    }
    char *sql = "SELECT * FROM users"; 
    if (mysql_query(mysql, sql)!= 0){
        printf("query sql err: %s\n", mysql_error(mysql));  // 执行SQL查询错误提示
    }
    MYSQL_RES *res = mysql_store_result(mysql);  // 获取查询结果集
    if (res == NULL){
        printf("res err: %s\n", mysql_error(mysql));  // 获取结果集出错提示
        exit(1);                                      // 退出程序
    }
    int flag=0;
    int num = mysql_num_rows(res);        // 获取结果集行数
    int count = mysql_field_count(mysql);  // 获取结果集列数
    for (int i = 0; i < num; i++){
        MYSQL_ROW row = mysql_fetch_row(res);  // 获取结果集的一行数据
        if(strcmp(row[0],getuser.username)==0&&strcmp(row[1],getuser.password)==0){
            strncpy(getuser.signature,row[2],strlen(row[2]));
            flag=1;
            break;
        }
    } 
    mysql_free_result(res);  // 释放结果集内存
    mysql_close(mysql);      // 关闭数据库连接
    if(flag){
        strncpy(getuser.message,"111",strlen("111"));
    }
    else{
        strncpy(getuser.message,"0",strlen("0"));
    }
    send(i,&getuser,sizeof(getuser),0);
}

void s_alonechat(struct user getuser,int i)//私聊
{
    int tosd=findUserSocket(getuser.toname);
    if(send(tosd, &getuser,sizeof(getuser),0)==-1){
        perror("send count");
    }
}

void s_togetherchat(struct user getuser,int i)//群聊
{
    UserNode *current = userList;
    while (current != NULL) {
        int tosd=current->socketsd;
        if(tosd!=i){
            if(send(tosd, &getuser,sizeof(getuser),0)==-1){
                perror("send count");
            }
        }
        current = current->next;
    }
}

void s_changeUsers(struct user getuser,int i)//修改个性签名
{
    //修改服务器本地数据库中的个性签名然后发回客户端
    MYSQL mysql_conn;                   
    MYSQL *mysql = mysql_init(&mysql_conn);
    if (mysql == NULL){
        printf("mysql init err");        
        exit(1);                         
    }
    mysql = mysql_real_connect(mysql, "localhost", "root", NULL, "chatuser", 3306, NULL, 0);  
    if (mysql == NULL){
        printf("connect err\n");        
        exit(1);                      
    }
    char sql[255]; // 分配足够的空间，这里假设 SQL 语句不超过 255 个字符
    sprintf(sql, "UPDATE users SET signature = '%s' WHERE name = '%s' AND password = '%s'", getuser.signature,getuser.username, getuser.password);
    mysql_query(mysql, sql);
    mysql_close(mysql);   
    send(i,&getuser,sizeof(getuser),0);
}

void s_changepassword(struct user getuser,int i)//修改密码
{
    MYSQL mysql_conn;                   
    MYSQL *mysql = mysql_init(&mysql_conn);
    if (mysql == NULL){
        printf("mysql init err");        
        exit(1);                         
    }
    mysql = mysql_real_connect(mysql, "localhost", "root", NULL, "chatuser", 3306, NULL, 0);  
    if (mysql == NULL){
        printf("connect err\n");        
        exit(1);                      
    }
    char sql[255]; // 分配足够的空间，这里假设 SQL 语句不超过 255 个字符
    sprintf(sql, "UPDATE users SET password = '%s' WHERE name = '%s' AND signature = '%s'",  getuser.password,getuser.username,getuser.signature);
    mysql_query(mysql, sql);
    mysql_close(mysql);   
    send(i,&getuser,sizeof(getuser),0);

}

void s_user_destroy(struct user getuser,int i)//用户要求注销账号
{
    MYSQL mysql_conn;                   
    MYSQL *mysql = mysql_init(&mysql_conn);
    if (mysql == NULL){
        printf("mysql init err");        
        exit(1);                         
    }
    mysql = mysql_real_connect(mysql, "localhost", "root", NULL, "chatuser", 3306, NULL, 0);  
    if (mysql == NULL){
        printf("connect err\n");        
        exit(1);                      
    }
    char sql[255]; // 分配足够的空间，这里假设 SQL 语句不超过 255 个字符
    sprintf(sql, "DELETE FROM users WHERE name = '%s' AND password = '%s'", getuser.username,getuser.password);
    mysql_query(mysql, sql);
    mysql_close(mysql);   
    send(i,&getuser,sizeof(getuser),0);
}

void s_user_exit(struct user getuser,int i)//用户要求退出登陆
{
    deleteUser(getuser.username);//把该用户从在线链表中删除
    send(i,&getuser,sizeof(getuser),0);
}

void s_sendfile(struct user getuser,int i)//发送文件
{
    int tosd=findUserSocket(getuser.toname);//先找到他要发送的用户的套接字
    if(send(tosd, &getuser,sizeof(getuser),0)==-1){//然后直接发送
        perror("send count");
    }
}

// 添加用户到链表
void s_addUserToList(char *username, int socketsd) {
    UserNode *newUser = (UserNode *)malloc(sizeof(UserNode));
    if (newUser == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    strcpy(newUser->username, username);
    newUser->socketsd = socketsd;
    newUser->next = userList;
    userList = newUser;
}

// 查找用户的socket描述符
int findUserSocket(char *username) {
    UserNode *current = userList;
    while (current != NULL) {
        if (strcmp(current->username, username) == 0) {
            return current->socketsd;
        }
        current = current->next;
    }
    return -1; // 用户不存在
}

// 删除用户节点
void deleteUser(char *username) {
    UserNode *current = userList;
    UserNode *prev = NULL;

    while (current != NULL) {
        if (strcmp(current->username, username) == 0) {
            if (prev == NULL) {
                userList = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

// 释放链表内存
void freeUserList() {
    UserNode *current = userList;
    UserNode *next;
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
}

//查询链表长度，即查询在线人数
void s_countUsers(struct user getuser,int i) {
    int count = 0;
    UserNode *current = userList;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    sprintf(getuser.message,"%d",count);
    if(send(i, &getuser,sizeof(getuser),0)==-1){
        perror("send count");
    }
}

int main() {
    struct sockaddr_in laddr;
    pthread_t tid;

    int sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0) {
        perror("socket()");
        exit(1);
    }

    laddr.sin_family = AF_INET;
    laddr.sin_port = htons(atoi(SERVERPORT));
    inet_pton(AF_INET, "0.0.0.0", &laddr.sin_addr);

    if (bind(sd, (void *)&laddr, sizeof(laddr)) < 0) {
        perror("bind()");
        exit(1);
    }

    if (listen(sd, 200) < 0) {
        perror("listen()");
        exit(1);
    }
    
    fd_set rd_fds, rd_fds_bk;
    FD_ZERO(&rd_fds);
    FD_SET(sd, &rd_fds);//将服务器的fd设置进去
    int maxfd = sd + 1;//设置maxfd
    rd_fds_bk = rd_fds;
    
    while(1)
    {
        int ret = select(maxfd, &rd_fds, NULL, NULL, 0);
        if (ret < 0)
        {
            perror("Error:select");
            break;
        }
        else{
            int i;
            for (i = 0; i < maxfd; i++){
                if (FD_ISSET(i, &rd_fds)){
                    if (i == sd){//有客户端请求连接
                        //accept
                        struct sockaddr_in raddr;
                        socklen_t raddr_len = sizeof(raddr);
                        int new_fd = accept(sd, (struct sockaddr *)&raddr, &raddr_len);
                        FD_SET(new_fd, &rd_fds_bk);
                        maxfd = maxfd > new_fd + 1 ? maxfd : new_fd + 1;
                    }
                    else{//有客户端发送消息
                        if(recv(i,&getuser,sizeof(getuser),0) <= 0){//接收失败
                            perror("recv struct");
                            close(sd);
                            continue;
                        }
                        else{//接收成功，此时以及接收到了客户端发送的结构体，通过type判断客户端需要哪种服务
                            switch(getuser.type)
                            {
                                case 0://有用户登陆，向在线用户链表中插入一个用户
                                    s_addUserToList(getuser.username,i);
                                    break;
                                case 1://有用户发送私聊请求
                                    s_alonechat(getuser,i);
                                    break;
                                case 2://有用户发送群聊请求
                                    s_togetherchat(getuser,i);
                                    break;
                                case 3://有用户查看在线人数
                                    s_countUsers(getuser,i);
                                    break;
                                case 4://用户要求修改个性签名
                                    s_changeUsers(getuser,i);
                                    break;
                                case 5://用户要求修改密码
                                    s_changepassword(getuser,i);
                                    break;
                                case 6://用户要给某用户发送文件
                                    s_sendfile(getuser,i);
                                    break;
                                case 7://用户要求退出登陆
                                    s_user_exit(getuser,i);
                                    break;
                                case 8://用户要求注销账号
                                    s_user_destroy(getuser,i);
                                    break;   
                                case 9://判断请求登陆的这个用户名密码是否正确
                                    s_user_check(getuser,i);
                                case 10://判断用户注册用户名是否已被使用
                                    s_user_create(getuser,i);
                                default:
                                    break;
                            }
                       }
                   }
               }
            }
        }
        rd_fds = rd_fds_bk;//保证下次调用select的时候，rd_fds里面有所有的fd
    }
    
    close(sd);

    exit(0);
}






