#ifndef CLIENT_H__
#define CLIENT_H__

#include "proto.h"

//用户界面
void login_menu(void);
//重复用户页面
void sendmyself(int sd);
//初始化界面
void init_menu();
//重复初始界面
void init_menu__(int sd);
//注册
void regist(int sd);
//登陆
void login(int sd);
//群聊
void c_togetherchat(int sd);
//私聊
void c_alonechar(int sd);
//修改个性签名
void c_changesig(int sd);
//查询在线人数
void c_checkperson(int sd);
//子线程处理接收
void * readrecv(void *p);
//修改密码
void c_changepassword(int sd);
//注销账号
void c_destroy(int sd);
//退出登陆
void c_exit(int sd);
//发送文件
void c_sendfile(int sd);
//接收文件
void saveFileContent(struct user rc);
//查看聊天记录
void c_checkchat(int sd);
//保存聊天记录
void savechatmessgae(struct user rc,int op);

#endif
