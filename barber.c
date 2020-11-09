#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<unistd.h>
#include<string.h>
#include<pthread.h>
#include<semaphore.h>
#include<sys/time.h>

int CUT_TIME;          //控制理发速度
int VISIT_TIME;        //控制到店速度
int LEAVE_TIME;        //控制离店速度

int SNUM;        //椅子数目 Seat
int BNUM;        //理发师数目 Barber
int CNUM;        //顾客数目 Customer

sem_t cus;                 //顾客状态，用于唤醒理发师
sem_t bar;                 //理发师
sem_t mutex;               //顾客和理发师之间互斥
sem_t mutex2;              //理发师和理发师之间互斥
sem_t worker;              //自旋锁

int i,j,k;
int working = 0;          //统计理发师状态
int waiting = 0;          //等待中的顾客数
int leave = 0;            //因没有座位而直接离开的顾客数
int count = 0;           //统计理发师服务的顾客总数
int served[20] = {0};     //统计各个理发师的业绩情况

void *barber(void *arg) //理发师线程
{
    while(1){
    	struct timeval tv;
    	gettimeofday(&tv, NULL);
    	srand(tv.tv_sec + tv.tv_usec + getpid());  //毫秒级种子
    	CUT_TIME = rand()%10001;

    	sem_wait(&worker);
    	sem_wait(&mutex);
    	if(waiting == 0) 
    	{
    	    //没有顾客，理发师睡觉，等待cus信号	
    		printf("********没有顾客,第 %ld 号理发师正在睡觉!*********\n",(unsigned long )arg);		
    		working--; 
    		sem_post(&mutex);
    		sem_post(&worker);
    		sem_wait(&cus); //等待顾客进店
    	}
    	else 
    	{ 
    	    //唤醒一位顾客开始理发
    		sem_post(&bar);
    		waiting--;
    		//统计人数
    		printf("第 %ld 号理发师开始理发,已服务人数:%d\n",(unsigned long )arg,served[(unsigned long )arg]);
    		//printf("本次理发时间：%d\n",CUT_TIME); 
    		printf("一位顾客正在理发,等待理发的顾客数: %d\n",waiting);
    		sem_post(&mutex); //保证理发师之间互斥
    		//理发
    		usleep(CUT_TIME); //非必要语句,控制理发速度,模拟理发师的效率,程序执行过程与该值密切相关.
    		printf("一位顾客理发结束!\n");
    		count++;
    		served[(unsigned long )arg]++;
    		sem_post(&worker); //保证理发师只能同时为一位顾客理发
     }     
  }
}

void *customer(void *arg) //顾客线程
{
	struct timeval tv;
    gettimeofday(&tv, NULL);
    srand(tv.tv_sec + tv.tv_usec + getpid()); //毫秒级种子
	LEAVE_TIME = rand()%11;
 	sem_wait(&mutex2); //互斥锁
    printf("第 %ld 号顾客进店...\n",(unsigned long )arg);

    if(waiting == SNUM) //没有空位，顾客离开.
    {
        //统计离开人数
    	leave++;
    	sem_post(&mutex2);
    	printf("没有座位,第 %ld 号顾客离开!离开人数:%d\n",(unsigned long )arg,leave);    
    }
    else
    {
        //统计等待人数
    	waiting++;
    	printf("第 %ld 号顾客坐下等待理发,等待理发的顾客数:%d\n",(unsigned long )arg,waiting);
    	if(waiting == 1 && working < 0)   //如果是第一位顾客，唤醒理发师，唤醒之后工作到没有顾客为止
    	{  
    	    //唤醒理发师
        	printf("一位理发师被唤醒,正在准备理发!\n");
            //统计工作理发师人数
        	working++;
            //printf("目前工作的理发师为：%d\n",BNUM+working);
        	sem_post(&cus);
        }
        sem_post(&mutex2);
        sem_wait(&bar);
        //等待理发师
    }

    usleep(LEAVE_TIME);  //非必要语句,控制客人离开速度  
}

void main()
{   
	int temp;

	printf("请输入椅子数目:");
    scanf("%d",&SNUM);
	printf("请输入理发师数目:");
    scanf("%d",&BNUM);
    printf("请输入顾客数目:");
    scanf("%d",&CNUM);
    printf("********设置成功！*********\n");

	int res;
    pthread_t barber_thread[BNUM],customer_thread[CNUM];

    //初始化信号量
	sem_init(&bar,0,0);
	sem_init(&cus,0,0);
	sem_init(&worker,0,BNUM);
	sem_init(&mutex,0,1);
	sem_init(&mutex2,0,1);

	struct timeval tv;
    gettimeofday(&tv, NULL);
    srand(tv.tv_sec + tv.tv_usec + getpid()); 
	VISIT_TIME = rand()%11;

    //创建理发师进程
    printf("********理发店开业啦!*********\n");
    for(i=1;i<=BNUM;i++)
    {   
       res=pthread_create(&barber_thread[i],NULL,barber,(void*)(unsigned long)(i));
       sleep(1);
       if (res!=0)
          perror("Thread creation failure!\n");
    }

    //创建顾客进程
    for(i=1;i<=CNUM;i++)
    {
    	struct timeval tv;
    	gettimeofday(&tv, NULL);
    	srand(tv.tv_sec + tv.tv_usec + getpid()); 
    	VISIT_TIME = rand()%11;
       //非必要语句,控制消费者进店速度,模拟实际场景中的顾客流量.
       usleep(VISIT_TIME);
       res=pthread_create(&customer_thread[i],NULL,customer,(void*)(unsigned long)(i));
       if (res!=0)
           perror("Thread creation failure!\n");
    }   
    for(i=1;i<=CNUM;i++)
    {   
        //进程等待所有消费者线程结束
        pthread_join(customer_thread[i],NULL);
	}
    sleep(1);
    printf("********理发店打烊啦!*********\n");
    printf("********今日业绩排行*********\n");
    for(j=1;j<=BNUM;j++){ //冒泡法排序
    	for(k=1;k<=BNUM-j;k++){
    		if(served[k]<served[k+1]){
    			temp = served[k];
    			served[k]=served[k+1];
    			served[k+1]=temp;
    		}
    	}
    }
    for(j=1;j<=BNUM;j++){
    	printf("第 %d 号理发师服务人数:%d\n",j,served[j]);
    }
    printf("理发师服务顾客总数:%d\n",count);
    printf("直接离开的顾客总数:%d\n",leave);
    printf("\n");
}