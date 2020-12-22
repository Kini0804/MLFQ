#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PERIOD 500
typedef struct node /*进程节点信息*/
{
  char name[20];     /*进程的名字*/
  int queue;         /*进程处于的队列*/
  int type;          /*进程的类型，0——CPU任务，1——IO任务*/
  int round;         /*分配CPU的时间片*/
  int cputime;       /*CPU执行时间*/
  int needtime;      /*进程执行所需要的时间*/
  char state;        /*进程的状态，W——就绪态，R——执行态，F——完成态*/
  int count;         /*记录执行的次数*/
  struct node *next; /*下一个进程节点*/
} PCB;
typedef struct Queue /*多级就绪队列节点信息*/
{
  PCB *LinkPCB;       /*就绪队列中的进程队列指针*/
  int prio;           /*本就绪队列的优先级*/
  int round;          /*本就绪队列所分配的时间片*/
  struct Queue *next; /*指向下一个就绪队列的链表指针*/
} ReadyQueue;
PCB *run = NULL, *finish = NULL;             /*定义三个队列，就绪队列，执行队列和完成队列*/
ReadyQueue *Head = NULL;                     /*定义第一个就绪队列*/
int num;                                     /*进程个数*/
int ReadyNum;                                /*就绪队列个数*/
int period_count = 0;                        /*当前运行的时间*/
void Output();                               /*进程信息输出函数*/
void InsertFinish(PCB *in);                  /*将进程插入到完成队列尾部*/
void InsertPrio(ReadyQueue *in);             /*创建就绪队列，规定优先数越小，优先级越低*/
void PrioCreate();                           /*创建就绪队列输入函数*/
void GetFirst(ReadyQueue *queue);            /*取得某一个就绪队列中的队头进程*/
void InsertLast(PCB *in, ReadyQueue *queue); /*将进程插入到就绪队列尾部*/
void ProcessCreate();                        /*进程创建函数*/
void RoundRun(ReadyQueue *timechip);         /*时间片轮转调度算法*/
void MultiDispatch();                        /*多级调度算法，每次执行一个时间片*/
void MoveToHead();                           /*一定周期后将所有任务移动到最顶层队列*/

int main(void)
{
  PrioCreate();    /*创建就绪队列*/
  ProcessCreate(); /*创建就绪进程队列*/
  MultiDispatch(); /*算法开始*/
  Output();        /*输出最终的调度序列*/
  return 0;
}
void Output() /*进程信息输出函数*/
{
  ReadyQueue *print = Head;
  PCB *p;
  printf("进程名\t队列\tcpu时间\t需要时间\t进程状态\n");
  while (print)
  {
    if (print->LinkPCB != NULL)
    {
      p = print->LinkPCB;
      while (p)
      {
        printf("%s\t%d\t%d\t%d\t\t%c\n", p->name, p->queue, p->cputime, p->needtime, p->state);
        p = p->next;
      }
    }
    print = print->next;
  }
  p = finish;
  while (p != NULL)
  {
    printf("%s\t%d\t%d\t%d\t\t%c\n", p->name, p->queue, p->cputime, p->needtime, p->state);
    p = p->next;
  }
  p = run;
  while (p != NULL)
  {
    printf("%s\t%d\t%d\t%d\t\t%c\n", p->name, p->queue, p->cputime, p->needtime, p->state, p->count);
    p = p->next;
  }
}
void InsertFinish(PCB *in) /*将进程插入到完成队列尾部*/
{
  PCB *fst;
  fst = finish;

  if (finish == NULL)
  {
    in->next = finish;
    finish = in;
  }
  else
  {
    while (fst->next != NULL)
    {
      fst = fst->next;
    }
    in->next = fst->next;
    fst->next = in;
  }
}
void InsertPrio(ReadyQueue *in) /*创建就绪队列，规定优先数越小，优先级越低*/
{
  ReadyQueue *fst, *nxt;
  fst = nxt = Head;

  if (Head == NULL) /*如果没有队列，则为第一个元素*/
  {
    in->next = Head;
    Head = in;
  }
  else /*查到合适的位置进行插入*/
  {
    if (in->prio >= fst->prio) /*比第一个还要大，则插入到队头*/
    {
      in->next = Head;
      Head = in;
    }
    else
    {
      while (fst->next != NULL) /*移动指针查找第一个别它小的元素的位置进行插入*/
      {
        nxt = fst;
        fst = fst->next;
      }

      if (fst->next == NULL) /*已经搜索到队尾，则其优先级数最小，将其插入到队尾即可*/
      {
        in->next = fst->next;
        fst->next = in;
      }
      else /*插入到队列中*/
      {
        nxt = in;
        in->next = fst;
      }
    }
  }
}
void PrioCreate() /*创建就绪队列输入函数*/
{
  ReadyQueue *tmp;
  int i;
  printf("**********多级反馈队列调度算法**********\n");
  printf("请输入就绪队列的个数：\n");
  scanf("%d", &ReadyNum);
  printf("请分别输入每个就绪队列的CPU时间片：\n");
  for (i = 0; i < ReadyNum; i++)
  {
    if ((tmp = (ReadyQueue *)malloc(sizeof(ReadyQueue))) == NULL)
    {
      perror("malloc");
      exit(1);
    }
    scanf("%d", &(tmp->round));  /*输入此就绪队列中给每个进程所分配的CPU时间片*/
    tmp->prio = 500 - tmp->round; /*设置其优先级，时间片越高，其优先级越低*/
    tmp->LinkPCB = NULL;         /*初始化其连接的进程队列为空*/
    tmp->next = NULL;
    InsertPrio(tmp); /*按照优先级从高到低，建立多个就绪队列*/
  }
}
void GetFirst(ReadyQueue *queue) /*取得某一个就绪队列中的队头进程*/
{
  run = queue->LinkPCB;

  if (queue->LinkPCB != NULL)
  {
    run->state = 'R';
    queue->LinkPCB = queue->LinkPCB->next;
    run->next = NULL;
  }
}
void InsertLast(PCB *in, ReadyQueue *queue) /*将进程插入到就绪队列尾部*/
{
  PCB *fst;
  fst = queue->LinkPCB;

  if (queue->LinkPCB == NULL)
  {
    in->next = queue->LinkPCB;
    queue->LinkPCB = in;
  }
  else
  {
    while (fst->next != NULL)
    {
      fst = fst->next;
    }
    in->next = fst->next;
    fst->next = in;
  }
}
void ProcessCreate() /*进程创建函数*/
{
  PCB *tmp;
  int i;

  printf("请输入进程的个数：\n");
  scanf("%d", &num);
  printf("请输入进程名字和进程所需时间：\nname\ttime\ttype\n");
  for (i = 0; i < num; i++)
  {
    if ((tmp = (PCB *)malloc(sizeof(PCB))) == NULL)
    {
      perror("malloc");
      exit(1);
    }
    scanf("%s", tmp->name);
    getchar(); /*吸收回车符号*/
    scanf("%d", &(tmp->needtime));
    getchar(); /*吸收回车符号*/
    scanf("%d", &(tmp->type));
    tmp->cputime = 0;
    tmp->state = 'W';
    // tmp->prio = 50 - tmp->needtime; /*设置其优先级，需要的时间越多，优先级越低*/
    tmp->queue = 0;
    tmp->round = Head->round;
    tmp->count = 0;
    InsertLast(tmp, Head); /*按照优先级从高到低，插入到就绪队列*/
  }
}
void RoundRun(ReadyQueue *timechip) /*时间片轮转调度算法*/
{

  int flag = 1;

  GetFirst(timechip);
  while (run != NULL)
  {
    while (flag && period_count < PERIOD)
    {
      period_count++;
      run->count++;
      run->cputime++;
      run->needtime--;
      if (run->needtime == 0) /*进程执行完毕*/
      {
        run->state = 'F';
        InsertFinish(run);
        flag = 0;
      }
      else if (run->count == timechip->round) /*时间片用完*/
      {
        run->state = 'W';
        run->count = 0; /*计数器清零，为下次做准备*/
        InsertLast(run, timechip);
        flag = 0;
      }
    }
    flag = 1;
    if (period_count >= PERIOD)
    {
      MoveToHead();
      MultiDispatch();
      return;
    }
    GetFirst(timechip);
  }
}
void MultiDispatch() /*多级调度算法，每次执行一个时间片*/
{
  int flag = 1;
  int k = 0;
  ReadyQueue *point;
  point = Head;
  GetFirst(point); /*获得当前执行队列的进程队列的首个进程*/
  while (run != NULL)
  {
    Output();
    while (flag && period_count < PERIOD)
    {
      period_count++;
      run->count++;
      run->cputime++;
      run->needtime--;
      if (run->needtime <= 0) /*进程执行完毕*/
      {
        run->state = 'F';
        InsertFinish(run);
        flag = 0;
      } 
      else if (run->count == run->round) /*时间片用完*/
      {
        run->state = 'W';
        run->count = 0; /*计数器清零，为下次做准备*/
        if (point->next != NULL)
        {
          run->round = point->next->round; /*设置其时间片是下一个就绪队列的时间片*/
          run->queue++;                    /*设置队列为下一个就绪队列*/
          InsertLast(run, point->next);    /*将进程插入到下一个就绪队列中*/
          flag = 0;
        }
        else
        {
          RoundRun(point); /*如果为最后一个就绪队列就调用时间片轮转算法*/
          break;
        }
      }
      else if (run->type == 1) /*IO任务退出CPU*/
      {
        run->state = 'W';
        InsertLast(run, point);
        flag = 0;
      }
    }
   
    flag = 1;

    if (period_count >= PERIOD) /*一定周期后将所有任务移到最顶端*/
    {
      MoveToHead();
      MultiDispatch();
      return;
    }
    else
    {
      if (Head->LinkPCB != NULL)
        point = Head;
      if (point->LinkPCB == NULL) /*当前队列没有就绪进程，就绪队列指针下移*/
        point = point->next;
      if (point->next == NULL)
      {
        RoundRun(point);
        break;
      } 
    }
    GetFirst(point);
  }
}

void MoveToHead() /*一个周期结束后将所有任务移动到HEAD队列*/
{
  printf("-----------move to head before-----------\n");
  Output();
  printf("-----------move to head-----------\n");
  if (run->state == 'R')
  {
    run->state = 'W';
    run->queue = 0;
    InsertLast(run, Head);
  }
  else
  {
    run = NULL;
  }
  
  ReadyQueue *other_queue = Head->next;
  PCB *p;
  while (other_queue)
  {
    if (other_queue->LinkPCB != NULL)
    {
      while (other_queue->LinkPCB)
      {
        p = other_queue->LinkPCB;
        other_queue->LinkPCB = other_queue->LinkPCB->next;
        p->next = NULL;
        p->queue = 0;
        InsertLast(p, Head);
      }
    }
    other_queue = other_queue->next;
  }
  period_count = 0;
  Output();
  printf("-----------move to head-----------\n");
  sleep(10);
}
