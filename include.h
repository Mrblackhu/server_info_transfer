#ifndef __INCLUDE_H__
#define __INCLUDE_H__

#define SRV_PORT 8888
#define TABLE_ENV_DEV 0
#define TABLE_PHONE_DEV 1
#define TABLE_USR_PSD 2

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sqlite3.h>
#include <json/json.h>
#include <pthread.h>

typedef struct client{
	int fd;
	char *chrname;
	struct sockaddr_in myaddr;
}pcli;

typedef struct list_head{
	pcli node;
	struct list_head *next, *prev;
}double_node, *double_pnode;


extern void init_doublelist(double_pnode *H);
extern void insert_doublelist_next(double_pnode p, double_pnode new);
extern void del_doublelist_cur(double_pnode p);
extern double_pnode find_doublelist_myip(double_pnode h, int fd);
extern double_pnode find_doublelist_toip(double_pnode h, char *name);
extern void doublelist_display_fd(double_pnode h);
extern int doublelist_display_returnfd(double_pnode h);


extern struct json_object *json_checker(const char *str);
extern void json_table_commondata(json_object *fat, int *type, char **owndev,int *result);
extern void json_usrpsd_todata(json_object *fat, int *type, char **owndev, char **usr, char **psd, int *status, int *result, int *login);
extern void json_envdev_todata(json_object *fat, int *type, char **owndev, char **envdev, int *result, char **tmp, char **hum);
extern void json_phonedev_todata(json_object *fat, int *type, char **owndev, char **desctldev, char **desdev, int *status, int *result, int *tmp, int *hum);


extern void sqlite_insert_usrpsd(sqlite3 *db,int type,char *owndev,char *usr,char *psd,int status,int result,int login);
extern void sqlite_insert_phonedev(sqlite3 *db,int type,char *owndev,char *desctldev,char *desdev,int status,int result,int tmp,int hum);
extern void sqlite_insert_envdev(sqlite3 *db,int type,char *owndev,char *envdev,int result,char *tmp,char *hum);

#endif
