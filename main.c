#include "include.h"
#include "double_list.h"
#include "json.h"
#include "sqlite.h"

static sqlite3 *db;

void pthread_envdev( void *msg )
{
    printf("first in son pthread\n");
	int tmpfd1 = *(int *)msg;
	printf("tmpfd1=%d\n",tmpfd1);
	char tmpbuf[1024];
	int tmpret;
	pthread_detach(pthread_self());

	while(1){
		printf("after while(1),before read\n");

		memset(tmpbuf, 0, sizeof(tmpbuf));
		tmpret = read(tmpfd1, tmpbuf, sizeof(tmpbuf));
		printf("after read\n");
		if(tmpret < 0){
			perror("read");
			pthread_exit(NULL);
		}
		if(tmpret == 0){
			printf("设备断开连接\n");
		}
		struct json_object *fat, *json_result, *json_type, *json_owndev, *json_envdev, *json_tmp, *json_hum;
		fat = json_checker(tmpbuf);
		if(fat == NULL){
			printf("不是json数据格式，重新发送\n");
			char *json_wrong = "{\"result\" : 0}";
			write(tmpfd1, json_wrong, strlen(json_wrong));
			continue;
		}

		int result,type;
		char *owndev,*envdev,*tmper,*hum;

		json_envdev_todata(fat, &type, &owndev, &envdev, &result, &tmper, &hum);

		printf("i am son pthread\n");
		printf("type:%d,owndev:%s,envdev:%s\n", type,owndev,envdev);
		if(result == 0){
			printf("上次发送的数据有异常\n");
		}
	//插入数据库,在子线程插入数据库env_dev
	sqlite_insert_envdev(db,type,owndev,envdev,result,tmper,hum);
    }
    pthread_exit(NULL);
}

int main(int argc, char **argv)
{
	int socketfd; 
	int clientfd;
	static pthread_t pthread;
	int ret;
	char buf[1024];
	//保存fd的链表
	double_pnode p;
	double_pnode all;
	init_doublelist(&p);
	init_doublelist(&all);

	char *errmsg;
	sqlite3_open("env.db", &db);
	/*--------一共有3张表，第一张表，账户表,Usr_psd目前有3个字段，type,类型：0表示主控板;1表示手机端-----
	----------usr 类型用户名，text字段，password 类型密码，text字段 status 字段表示状态，如果是0表示注册，1表示查询--------------------*/
	ret = sqlite3_exec(db, "create table if not exists usr_psd(id integer primary key autoincrement, type int,owndev text,status int, usr text, password text, result int, login int,time not null default current_timestamp)", NULL, NULL, &errmsg);
	if(ret != SQLITE_OK){
		printf("create usr_psd table fail:%s\n", errmsg);
		return -1;
	}
/*------------第二张表，手机端设备表，phone_dev 目前有四个字段。type,类型：0表示主控板，1表示手机端，device，设备号，-----
-------------tmp 字段为int,0表示什么都不操作，1表示请求拿到主控板的tmp数据，hum字段为int,0表示什么都不做，1表示请求拿到主控板的hum数据----*/
	ret = sqlite3_exec(db, "create table if not exists phone_dev(id integer primary key autoincrement,type int,owndev text,desctldev text,desdev text,status int,result int,tmp int,hum int,time not null default current_timestamp)", NULL, NULL, &errmsg);
	if(ret != SQLITE_OK){
		printf("create table fail:%s\n", errmsg);
		return -1;
	}
/*------------第三张表，M0环境数据表，m0_env 目前有四个字段。type,类型：0表示主控板，1表示手机端，device，设备号，-----
-------------tmp 字段为text,hum字段为text----*/
	ret = sqlite3_exec(db, "create table if not exists env_dev(id integer primary key autoincrement, type int, owndev text, envdev text, result int, tmp text, hum text,time not null default current_timestamp)", NULL, NULL, &errmsg);
	if(ret != SQLITE_OK){
		printf("create table fail:%s\n", errmsg);
		return -1;
	}

//网络socket
	socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if(socketfd < 0){
		perror("socket");
		exit(1);
	}
//设置可以reuseaddr
	int on = 1;
	ret = setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	if(ret < 0){
		perror("setsockopt");
		exit(1);
	}
//绑定本地ip和端口
	struct sockaddr_in myaddr;
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(SRV_PORT);
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	ret = bind(socketfd, (struct sockaddr *)&myaddr, sizeof(myaddr));
	if(ret < 0){
		perror("bind");
		exit(1);
	}
    //设置监听
	ret = listen(socketfd, 8);
	if(ret < 0){
		perror("listen");
		exit(1);
	}
    //用来保存clientfd的ip和端口号
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);
	
	//采用多路复用IO来实现异步处理，对多个fd文件进行监听，实现收发数据
	fd_set wr_set;
	FD_ZERO(&wr_set);

	while(1)
	{
		FD_SET(socketfd, &wr_set);
		printf("before select\n");
		double_pnode tmp1;
		//把链表里面的fd都设置为监听
		for(tmp1=p->next; tmp1!=p; tmp1=tmp1->next){
			FD_SET(tmp1->node.fd, &wr_set);
		}
		//在接收到数据之前，会在这里阻塞
		ret = select(8, &wr_set, NULL, NULL, NULL);
		printf("=====after select==========\n");
		if(ret <= 0){
			break;
		}
		if(ret > 0){
			//最大监听数为8个，所以在这个循环内去判断
			if(FD_ISSET( socketfd, &wr_set)){

				bzero(&client_addr, 0);
				printf("===start accept=====\n");
				//接收客户端的连接
				clientfd = accept(socketfd, (struct sockaddr *)&client_addr, &len);
				printf("get new clientfd: %d\n", clientfd);
				if(clientfd < 0){
					perror("accept");
					break;
				}

				printf("ip:%s,port:%d: connected\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

				//把接收到的每一个fd放入到链表中，进入监听对象，这个链表代表监听的对象
				double_pnode new;
				new = (double_pnode)malloc(sizeof(struct list_head));
				if(NULL == new){
					perror("malloc");
					break;
				}
				new->node.myaddr = client_addr;
				new->node.fd = clientfd;
				//初始化设备号都为A
				new->node.chrname = "A";
				insert_doublelist_next(p, new);

				//两个链表，这个链表保存所有的数据，不会删除
				double_pnode new_all;
				new_all = (double_pnode)malloc(sizeof(struct list_head));
				if(NULL == new_all){
					perror("malloc");
					break;
				}
				new_all->node.myaddr = client_addr;
				new_all->node.fd = clientfd;
				//初始化设备号都为A
				new->node.chrname = "A";
				insert_doublelist_next(all, new_all);

			}

			double_pnode tmp;
			for(tmp=p->next; tmp!=p; tmp=tmp->next){
				//把链表里面的fd赋给clientfd
				clientfd = tmp->node.fd;
				if(FD_ISSET( clientfd, &wr_set)){
					//每次在read之前必须把buf的内存清空，否则读出的数据有误
					memset(buf, 0, sizeof(buf));
					printf("before read\n");
					//把fd里面的内容读到buf里面
					ret = read(clientfd, buf, 1024);
					printf("after read\n");
					if(ret < 0){
						perror("read");
						break;
					}else if(ret == 0){
						//断开连接后，把fd从链表中删除，并且把监听给剔除，在break当前循环，这样会进入下个循环，服务器就不会段
						printf("disconnected\n");
						del_doublelist_cur(tmp);
						FD_CLR( clientfd, &wr_set);
						break;
					}
					printf("receive msg:%s\n", buf);

					struct json_object *fat;
					//采用json-0.9提供的接口，重写了json_tokener_parse函数，能够返回fat，同时也能够判断是否为json格式字符串
					fat = json_checker(buf);
					/*
					if(fat == NULL){
						//如果不是json格式的，则往客户端发送一条json数据，要它再发送一次
						char *tmpbuf = "不是json格式字符串,请重新发送";
						printf("%s\n", tmpbuf);
						char *json_wrong;
						//如果是错误，则请求发送一次，只需要发送含有result属性的json就可以了，对方可以解析出来
						json_wrong = "{\"result\":0}";
						write(clientfd, json_wrong, strlen(json_wrong));
						break;
					}*/
					//统一用函数解析出共有的三个字段，owndev,type,result
					char *owndev;
					int type,result;
					json_table_commondata(fat, &type, &owndev, &result);
					//先判断上次发送是否是成功的
					if(result == 0){
						//表示上次发送数据失败，重新发送一次.
					}
					//提取json字符串的owndev字段，先把设备号和链表里的fd绑定
					find_doublelist_myip(p, clientfd)->node.chrname = owndev;
					//绑定另外一个链表
					find_doublelist_myip(all, clientfd)->node.chrname = owndev;

					//根据type来决定对哪一张表进行操作
					printf("type = %d\n", type);
					switch(type){
						case TABLE_USR_PSD:{
											   char *usr,*psd;
											   int status,login;
											   //json解析数据这里要注意字符串，传什么参数进去
											   json_usrpsd_todata(fat, &type, &owndev,&usr,&psd,&status,&result,&login); 
											   if(status == 0){   //表示需要查询账号和密码是否匹配成功
												   char **dbResult;
												   int nRow, nColumn, i, index;
												   ret = sqlite3_get_table(db, "select usr,password from usr_psd", &dbResult, &nRow, &nColumn, &errmsg);
												   if(ret == SQLITE_OK){
													   int flag = 0;
													   index = nColumn;
													   printf("共有多少条数据:%d\n", nRow);
													   for(i=0; i<nRow; i++){
														   int t1 = strcmp(dbResult[index], usr);
														   int t2 = strcmp(dbResult[index+1], psd);
														   if( (t1==0) && (t2==0)){
															   printf("名称为%s的设备登录成功\n", owndev);
															   //这里要发送两个属性的字符串，因为要先判断上一次是否发送成功，再这个基础上再判断是否登录成功
															   //做成json结构体，然后转换为字符串，再发送过去
															   struct json_object *obj;
															   obj = json_object_new_object();
															   json_object_object_add(obj, "type", json_object_new_int(2));
															   json_object_object_add(obj, "result", json_object_new_int(1));
															   json_object_object_add(obj, "login", json_object_new_int(1));
															   const char *json_string_send = json_object_to_json_string(obj);
															   printf("json_to_string:%s\n", json_string_send);
															   //printf("json_to_string size:%d\n", strlen(json_string_send));

															   //帐号密码匹配成功，发送返回的字符串给手机端，让它解析到底帐号密码正确，主要看login这个属性，前面的维持不变
															   write(clientfd, json_string_send, strlen(json_string_send));
															   flag = 1;
															   break;
														   }
														   index = index + 2;
													   }
													   if( flag == 0 ){
														   printf("名称为%s的设备登录失败\n", owndev);
														   //帐号密码匹配不成功，则返回login 为0 让对方去解析
								   struct json_object *obj;
								   obj = json_object_new_object();
							       json_object_object_add(obj, "type", json_object_new_int(2));
							       json_object_object_add(obj, "result", json_object_new_int(1));
							       json_object_object_add(obj, "login", json_object_new_int(0));
							       const char *json_string_send = json_object_to_json_string(obj);

							       write(clientfd, json_string_send, strlen(json_string_send));
								   break;
							   }
						       }
						   }else if(status == 1){   //表示注册账号密码信息，插入表usr_psd
						       sqlite_insert_usrpsd(db,type,owndev,usr,psd,status,result,login);
						       break;
						   }
						   break;
					       }
			    case TABLE_PHONE_DEV:{
						     char *desctldev,*desdev;
						     int status,tmper,hum;
						     //解析json 数据包，需要注意传参
						     json_phonedev_todata(fat,&type,&owndev,&desctldev,&desdev,&status,&result,&tmper,&hum);

						     //插入数据库，对phone_dev表进行操作
						     sqlite_insert_phonedev(db,type,owndev,desctldev,desdev,status,result,tmper,hum);
						     printf("after phone_msg insert into database\n");
						     //根据status 的状态来判断是请求数据还是转发数据
						     printf("desctldev:%s, desdev:%s\n", desctldev, desdev);
							 printf("status:%d, tmp:%d, hum:%d\n", status,tmper,hum);
							 if(status == 1){
								 //遍历链表，通过desctldev找到链表的fd，然后把数据完整转发过去
								 int tmpfd;
								 printf("i am status == 1 function\n");
								 tmpfd = find_doublelist_toip(all, desctldev)->node.fd;

								 doublelist_display_fd (all);

								 printf("desctldev:%s\n",desctldev);
								 printf("tmpfd:%d\n",find_doublelist_toip(all,desctldev)->node.fd);
								 write(tmpfd, buf, strlen(buf));
								 memset(buf, 0, sizeof(buf));
								 break;
							 }else if(status == 0){
								 //表示希望得到tmp数据，不希望得到hum数据
								 printf("in the status==0 function\n");
								 if((tmper == 1) && (hum == 0)){
									 char **dbResult;
									 int nRow, nColumn, index;
									 //对env_dev表进行操作
									 ret = sqlite3_get_table(db, "select tmp from env_dev order by id desc", &dbResult, &nRow, &nColumn, &errmsg);
									 if(ret == SQLITE_OK){
										 index = nColumn;
										 printf("共有多少条数据:%d\n", nRow);
										 //取前面3条数据发送
										 if( nRow > 3 ){
											 /***************************************************************************************/
											 //这里不能直接用变量，因为本来已经是字符串了，需要把变量转换为json格式字符串，再发送出去
											 struct json_object *obj1, *obj2, *obj3;
											 obj1 = json_object_new_object();
											 obj2 = json_object_new_object();
											 obj3 = json_object_new_object();
											 //这里要注意，这里请求数据，我发送的并不是phone_dev那张表，为了给手机端做判断，我这里发的json格式是env_dev表
											 json_object_object_add(obj1, "type", json_object_new_int(0));
											 json_object_object_add(obj1, "result", json_object_new_int(1));
											 json_object_object_add(obj1, "tmp", json_object_new_string(dbResult[index]));
											 const char *json_string_send1 = json_object_to_json_string(obj1);
											 write(clientfd, json_string_send1, strlen(json_string_send1));

											 //发第2条数据
											 json_object_object_add(obj2, "type", json_object_new_int(0));
											 json_object_object_add(obj2, "result", json_object_new_int(1));
											 json_object_object_add(obj2, "tmp", json_object_new_string(dbResult[index+1]));
											 const char *json_string_send2 = json_object_to_json_string(obj2);
											 write(clientfd, json_string_send2, strlen(json_string_send2));

											 //发第三条数据
											 json_object_object_add(obj3, "type", json_object_new_int(0));
											 json_object_object_add(obj3, "result", json_object_new_int(1));
											 json_object_object_add(obj3, "tmp", json_object_new_string(dbResult[index+2]));
											 const char *json_string_send3 = json_object_to_json_string(obj3);
											 write(clientfd, json_string_send3, strlen(json_string_send3));
											 break;
										 }else{
											 //少于3条则取一条数据
											 struct json_object *obj;
											 obj = json_object_new_object();
											 json_object_object_add(obj, "type", json_object_new_int(0));
											 json_object_object_add(obj, "result", json_object_new_int(1));
											 json_object_object_add(obj, "tmp", json_object_new_string(dbResult[index]));
											 const char *json_string_send = json_object_to_json_string(obj);

											 write(clientfd, json_string_send, strlen(json_string_send));
											 break;
										 }
										 sqlite3_free_table(dbResult);
									 }
								 }else if( (tmper == 1) && (hum ==1)){//表示希望得到hum和tmp数据，如果数据太多，情况太多，需要重新设计如何请求数据
									 printf("in the tmp and hum\n");
									 char **dbResult;
									 int nRow, nColumn, index;
									 //对env_dev表进行操作
									 ret = sqlite3_get_table(db, "select tmp,hum from env_dev order by id desc", &dbResult, &nRow, &nColumn, &errmsg);
									 if(ret == SQLITE_OK){
										 index = nColumn;
										 printf("共有多少条数据:%d\n", nRow);
										 //取前面3条数据发送
										 if( nRow > 3 ){
											 struct json_object *obj1,*obj2,*obj3;
											 obj1 = json_object_new_object();
											 obj2 = json_object_new_object();
											 obj3 = json_object_new_object();

											 json_object_object_add(obj1, "type", json_object_new_int(0));
											 json_object_object_add(obj1, "result", json_object_new_int(1));
											 json_object_object_add(obj1, "tmp", json_object_new_string(dbResult[index]));
											 json_object_object_add(obj1, "hum", json_object_new_string(dbResult[index+1]));
											 const char *json_string_send1 = json_object_to_json_string(obj1);
								 	 		 printf("json_string_send1:%s\n", json_string_send1);
											 write(clientfd, json_string_send1, strlen(json_string_send1));
											 sleep(2);
											
											 json_object_object_add(obj2, "type", json_object_new_int(0));
											 json_object_object_add(obj2, "result", json_object_new_int(1));
											 json_object_object_add(obj2, "tmp", json_object_new_string(dbResult[index+2]));
											 json_object_object_add(obj2, "hum", json_object_new_string(dbResult[index+3]));
											 const char *json_string_send2 = json_object_to_json_string(obj2);
											 write(clientfd, json_string_send2, strlen(json_string_send2));
											 sleep(2);

											 json_object_object_add(obj3, "type", json_object_new_int(0));
											 json_object_object_add(obj3, "result", json_object_new_int(1));
											 json_object_object_add(obj3, "tmp", json_object_new_string(dbResult[index+4]));
											 json_object_object_add(obj3, "hum", json_object_new_string(dbResult[index+5]));
											 const char *json_string_send3 = json_object_to_json_string(obj3);
											 write(clientfd, json_string_send3, strlen(json_string_send3));
											 break;
										 }else{
											 //少于3条则取一条数据
											 struct json_object *obj;
											 obj = json_object_new_object();
											 json_object_object_add(obj, "type", json_object_new_int(0));
											 json_object_object_add(obj, "result", json_object_new_int(1));
											 json_object_object_add(obj, "tmp", json_object_new_string(dbResult[index]));
											 json_object_object_add(obj, "hum", json_object_new_string(dbResult[index+1]));
											 const char *json_string_send = json_object_to_json_string(obj);
											 write(clientfd, json_string_send, strlen(json_string_send));
											 break;
										 }
									 }
									 sqlite3_free_table(dbResult);
								 }
							 }
							 break;
									 }
				case TABLE_ENV_DEV:{
									   //要先把这个带有clientfd属性的节点删除出链表，然后把clientfd解除监听绑定
									   del_doublelist_cur(tmp);
									   FD_CLR( clientfd, &wr_set);

						   //把json数据和clientfd打包好，放入新建的结构体中，再通过pthread_creat的传参，把数据传到线程中去
						   char *envdev,*tmper,*hum;
						   json_envdev_todata(fat, &type, &owndev, &envdev, &result, &tmper, &hum);
						   printf("i am father pthread,the first send\n");
						   printf("type:%d,owndev:%s,envdev:%s\n", type,owndev,envdev);

						   //插入数据库,第一次链接就在主线程插入数据库，对env_dev表进行操作
						   sqlite_insert_envdev(db,type,owndev,envdev,result,tmper,hum);
						   //创建子线程去单独接收它的数据
						   static int tmpfd;
						   tmpfd = clientfd;   //防止在插入过程中，有其它fd进行链接，把当前fd更改，所以在这里保存到中间值，更少可能出现bug
						   printf("before pthread_create\n");
						   ret = pthread_create(&pthread, NULL, (void *)pthread_envdev, (void *)&tmpfd);
						   printf("after pthread_create\n");
						   if(ret < 0){
						       printf("创建线程失败，名称为%s的设备无法采集数据\n", owndev);
						       break;
						   }
						   printf("before break\n");
						   break;
					       }
			    default:
					       break;
			}
		    }
		}
	    }		 
	}

	close(clientfd);
	close(socketfd);
	sqlite3_close(db);
	return 0;
}


