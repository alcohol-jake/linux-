#ifndef SERVER_H__
#define SERVER_H__

#include "proto.h"
// 结构体定义：用户节点
typedef struct UserNode {
    char username[50];
    int socketsd;
    struct UserNode *next;
} UserNode;

// 添加用户到链表
void s_addUserToList(char *username, int socketsd);
// 查找用户的socket描述符
int findUserSocket(char *username);
// 删除用户节点
void deleteUser(char *username);
// 释放链表内存
void freeUserList();
//查询在线人数
void s_countUsers(struct user getuser,int i);
//私聊
void s_alonechat(struct user getuser,int i);
//群聊
void s_togetherchat(struct user getuser,int i);
//修改个性签名
void s_changeUsers(struct user getuser,int i);
//mysql用户匹配
void s_user_check(struct user getuser,int i);
//判断注册的用户名是否已经存在
void s_user_create(struct user getuser,int i);
//修改密码
void s_changepassword(struct user getuser,int i);
//退出登陆
void s_user_exit(struct user getuser,int i);
//有用户给其他用户发送文件
void s_sendfile(struct user getuser,int i);
//用户要求注销账号
void s_user_destroy(struct user getuser,int i);

#endif
