#include "include.h"

//插入数据到usr_psd表
void sqlite_insert_usrpsd(sqlite3 *db,int type,char *owndev,char *usr,char *psd,int status,int result,int login)
{
	struct sqlite3_stmt *stat;
	
	int ret;
	ret = sqlite3_prepare(db, "insert into usr_psd(type,owndev,usr,password,status,result,login)values(?,?,?,?,?,?,?);", -1, &stat, NULL);
	if(ret == SQLITE_OK){
		sqlite3_bind_int(stat, 1, type);
		sqlite3_bind_text(stat, 2, owndev, 128, NULL);
		sqlite3_bind_text(stat, 3, usr, 128, NULL);
		sqlite3_bind_text(stat, 4, psd, 128, NULL);
		sqlite3_bind_int(stat, 5, status);
		sqlite3_bind_int(stat, 6, result);
		sqlite3_bind_int(stat, 7, login);
		sqlite3_step(stat);
	}
	sqlite3_finalize(stat);	
}

//插入数据到表phonedev
void sqlite_insert_phonedev(sqlite3 *db,int type,char *owndev,char *desctldev,char *desdev,int status,int result,int tmp,int hum)
{
    struct sqlite3_stmt *stat;
    int ret;
    ret = sqlite3_prepare(db, "insert into phone_dev(type,owndev,desctldev,desdev,status,result,tmp,hum)values(?,?,?,?,?,?,?,?);", -1, &stat, NULL);
    if(ret == SQLITE_OK){
	sqlite3_bind_int(stat, 1, type);
	sqlite3_bind_text(stat, 2, owndev, 128, NULL);
	sqlite3_bind_text(stat, 3, desctldev, 128, NULL);
	sqlite3_bind_text(stat, 4, desdev, 128, NULL);
	sqlite3_bind_int(stat, 5, status);
	sqlite3_bind_int(stat, 6, result);
	sqlite3_bind_int(stat, 7, tmp);
	sqlite3_bind_int(stat, 8, hum);
	sqlite3_step(stat);
    }
    sqlite3_finalize(stat);
}
//插入数据到表envdev
void sqlite_insert_envdev(sqlite3 *db,int type,char *owndev,char *envdev,int result,char *tmp,char *hum)
{
    struct sqlite3_stmt *stat;
    int ret;
    ret = sqlite3_prepare(db, "insert into env_dev(type,owndev,envdev,result,tmp,hum)values(?,?,?,?,?,?);", -1, &stat, NULL);

    if(ret == SQLITE_OK){
	sqlite3_bind_int(stat, 1, type);
	//这里要注意的是，传递过来的参数是指针，指针的大小都是4个字符，所以如果字符串大于4个字符，则无法顺利保存到数据库中，所以这里取一个较大值，也可以定义一个数组，然后用strcpy把数组大小扩大
	sqlite3_bind_text(stat, 2, owndev, 128, NULL);
	sqlite3_bind_text(stat, 3, envdev, 128, NULL);
	sqlite3_bind_int(stat, 4, result);
	sqlite3_bind_text(stat, 5, tmp, 128, NULL);
	sqlite3_bind_text(stat, 6, hum, 128, NULL);
	sqlite3_step(stat);
    }
    sqlite3_finalize(stat);
}


