#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp(char *args){	// 分配节点
  if(free_==NULL)assert(0);	// 没有空闲节点时报错
  if(head==NULL){	// 使用链表为空
    head=wp_pool;
    free_=free_->next;
    head->next=NULL;
    return head;
  }
  else {  // 使用链表不为空
    WP *res = head;
    while(res->next)res=res->next;
    res->next=free_;
    free_=free_->next;
    res=res->next;
    res->next=NULL;
    return res;
  }
}

void free_wp(int n){	// 回收节点
  WP *res=head;
  while(res){
    if(res->NO==n)break;
    res=res->next;
  }
  if(res==NULL)return;  // 找不到要回收的节点
  else if(res==head){  // 要回收的是头节点
    head=head->next;
    res->next=free_;
    free_=res;
  }
  else{  //回收的不是头节点
    res=head;
    while(res->next){
      if(res->next->NO==n)break;
      res=res->next;
    }
    WP *t=res->next;
    res->next=t->next;
    t->next=free_;
    free_=t;
  }
}

void show_wp(){	  // 打印监视点信息
  if(head==NULL){printf("当前无监视点\n");return;}
  printf("现存监视点：\n");
  WP *t=head;
  while(t){
    printf("%d  %s\n",t->NO,t->exprv);
    t=t->next;
  }
}

bool value_change(){ // 判断表达式是否改变
  WP *t=head;
  bool f=0;
  while(t){
    bool succ;
    int new_value = expr(t->exprv,&succ);
    if(new_value!=t->value){
      printf("触发%d号监视点\n",t->NO);
      printf("原值：%d  新值：%d\n",t->value,new_value);
      t->value=new_value;
      f=1;	// 不退出，打印所有被改变的监视点
    }
    t=t->next;
  }
  return f;
}

