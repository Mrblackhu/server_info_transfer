#include "include.h"

void init_doublelist(double_pnode *H)
{
    *H = (double_pnode)malloc(sizeof(double_node));
    if(NULL == *H){
		perror("malloc");
		exit(1);
    }
    (*H)->prev = (*H)->next = *H;
}

void insert_doublelist_next(double_pnode p, double_pnode new)
{
    new->next = p->next;
    p->next->prev = new->prev;
    new->prev = p;
    p->next = new;
}

void del_doublelist_cur(double_pnode p)
{
    p->prev->next = p->next;
    p->next->prev = p->prev;
    free(p);
}

//传进去client的ip，根据这个结点的toip，去找到对应结构体的struct sockaddr_in
double_pnode find_doublelist_myip(double_pnode h, int fd)
{
	double_pnode p;
	for(p=h->next; p!=h; p=p->next){
		if(fd == p->node.fd){
			return p;
		}	
	}
}

double_pnode find_doublelist_toip(double_pnode h, char *name)
{
	double_pnode p;
	for(p=h->next; p!=h; p=p->next){
	    //字符串需要这样对比是否等于
			if( !strcmp(name,p->node.chrname) ){
				return p;
			}
		}
}

void doublelist_display_fd(double_pnode h)
{
	double_pnode p;
	for(p=h->next; p!=h; p=p->next){
		printf("%d\t", (p->node).fd);
	}
	printf("\n");
}

int doublelist_display_returnfd(double_pnode h)
{
	double_pnode p;
	for(p=h->next; p!=h; p=p->next){
		return (p->node).fd;
	}
}
