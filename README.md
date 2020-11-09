# 问题解构
#### 具体描述
主程序中可以输入椅子的数量、理发师的数量（可大于1）以及顾客流量（10~20），多个顾客线程和理发师线程应该能够正确的并发执行。程序应输出并发执行的过程，能够正确统计并显示每个理发师服务的顾客数，以及因无座位直接离开的顾客数。
#### 要求剖析
输入变量 + 理发师问题 + 并发执行 = 多理发师问题
#### 基本思路
通过信号量的设置来解决阻塞；多理发师共享顾客队列。
#### 算法原理
 - 所有理发师使用相同的程序段
 - 所有顾客使用相同的程序段
 - 使用自旋锁保证理发师和理发师互斥
 - 使用互斥锁保证理发师和顾客互斥
#### 相关调用
 - Linux信号量工具：
#include<semaphore.h>中定义了很多信息量操作中常用的数据结构和系统函数，下面罗列本次实验将用到的：
**sem_t**：具体信号量的数据结构
**sem_init** ：用于创建信号量，并能初始化它的值
**sem_wait**：相当于wait操作
**sem_post**：相当于signal操作
 - POSIX线程相关：
#include<pthread.h>中用到的数据类型和函数如：
**pthread_t**：用于声明线程ID 
**pthread_create** ：创建一个线程
**pthread_join**：阻塞当前线程，直到另外一个线程运行结束
 - 常用标准库等

# 项目方案
#### 操作环境
- VMware Workstation Pro
- Ubuntu 18.04.2 LTS

#### 流程图
- 多线程流程
![多线程](https://img-blog.csdnimg.cn/20200205163115749.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2NsZWFubGlp,size_16,color_FFFFFF,t_70)
- 算法流程
![算法设计](https://img-blog.csdnimg.cn/20200205163135350.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2NsZWFubGlp,size_16,color_FFFFFF,t_70)
#### 数据结构

```cpp
int waiting=0;  //等待中的顾客数
int working=0;  //统计理发师情况
int leave = 0;  //因没有座位而直接离开的顾客数
int served[] = {0};  //统计各个理发师服务的顾客数
Int count = 0; //统计理发师服务的总顾客数
semaphore customer=0;  //是否有顾客，用于理发师和顾客之间的同步信号量
semaphore barber=0;  //理发师状态，用于顾客之间互斥使用理发师资源
semaphore worker = BNUM;  //自旋锁
semaphore mutex=1;  //理发师和理发师之间的互斥锁
semaphore mutex2=1;  //顾客与理发师之间的互斥锁

```

#### 伪代码
- 顾客进程
```cpp
    //顾客进程没有while
    wait(mutex2);
    if(waiting==N) 
    {//离开;signal(mutex2);}
    else
     { //进店坐下;
       waiting++;
       if(waiting==1&&working<0)
       //第一位顾客
       working++;
       signal(customer);
       signal(mutex2);
       wait(barber); //测试理发师
       //正在理发
       //离开
      }

```
 - 理发师进程
```cpp
   //理发师
   while(true)
   {
     wait(worker);
     wait(mutex);
     if(waiting==0) {
       working--;
       signal(worker);
       signal(mutex);
       wait(customer)}
     else{
       signal(barber);
       //为一位顾客理发;
       waiting--;
       signal(mutex);
       signal(worker);
      }
       //顾客理发结束; 
    }

```
#### 测试数据
- 固定测试数据（调试用）
① 10张椅子；2名理发师；20位顾客
② 5张椅子；5名理发师；10位顾客
③ 10张椅子；10名理发师；8位顾客
- 任意/随机测试数据（测试用）

#### 预期输出结果

```cpp
依次输入设置变量；
开店；
没有顾客，N名理发师正在睡觉；
第0号顾客进店；
第0号顾客坐下等待理发；
等待人数+1；
第0号理发师被唤醒；
第0号理发师开始理发；
工作人数+1；
第0号顾客正在被理发；
等待人数-1；
第1号顾客进店；
。。。。。。
第X号顾客进店；
第X号顾客坐下等待理发；
等待人数+1；
第X+1号顾客进店；
第X+1号顾客坐下等待理发；
等待人数+1；
。。。。。。
第Y号顾客进店；
无空位，顾客离开；
离开人数+1；
。。。。。。
第M号顾客理发结束；
服务人数+1；
。。。。。。
没有顾客，N名理发师正在睡觉；
关店；
顾客总数，离开总数；
```
#### 部分源码

```cpp
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
```
#### 运行结果
![截图1](https://img-blog.csdnimg.cn/20200205164513360.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2NsZWFubGlp,size_16,color_FFFFFF,t_70)
![截图2](https://img-blog.csdnimg.cn/20200205164535615.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2NsZWFubGlp,size_16,color_FFFFFF,t_70)
![截图3](https://img-blog.csdnimg.cn/20200205164551433.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2NsZWFubGlp,size_16,color_FFFFFF,t_70)
#### 结果分析

主要分析两个地方，一是**有没有出现理发师和理发师抢顾客的情况**，二是**各自的服务人数和总服务人数的统计有没有出错**；经过检查验证与预期结果基本一致。

# 项目总结
- 本项目的核心点在于**信号量及其PV操作**，其实所有的进程同步经典例子都是一样的；可以把信号量理解为**一把钥匙**，而PV操作则决定了你能不能拿到这把钥匙，拿到了，你才能继续执行，拿不到，你就得一直等待；
- 上述程序仅实现了最基本的要求，实际上后续可以摸索更多的功能，比如**VIP客户机制（引入优先级）**，突发情况（插队、引入抢占）等等，还是蛮有意思的；
- 理发师问题是操作系统的经典问题，也是非常灵活的项目之一，好好理解其中的原理，把一个问题把玩透彻，定会受益匪浅；
- 前路漫漫，任重道远，愿大家能在OS的探索路上越走越远！
