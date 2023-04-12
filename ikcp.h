//=====================================================================
//
// KCP - A Better ARQ Protocol Implementation
// skywind3000 (at) gmail.com, 2010-2011
//  
// Features:
// + Average RTT reduce 30% - 40% vs traditional ARQ like tcp.
// + Maximum RTT reduce three times vs tcp.
// + Lightweight, distributed as a single source file.
//
//=====================================================================
#ifndef __IKCP_H__
#define __IKCP_H__

#include <stddef.h>
#include <stdlib.h>
#include <assert.h>


//=====================================================================
// 32BIT INTEGER DEFINITION 
//=====================================================================
#ifndef __INTEGER_32_BITS__
#define __INTEGER_32_BITS__
#if defined(_WIN64) || defined(WIN64) || defined(__amd64__) || \
	defined(__x86_64) || defined(__x86_64__) || defined(_M_IA64) || \
	defined(_M_AMD64)
	typedef unsigned int ISTDUINT32;
	typedef int ISTDINT32;
#elif defined(_WIN32) || defined(WIN32) || defined(__i386__) || \
	defined(__i386) || defined(_M_X86)
	typedef unsigned long ISTDUINT32;
	typedef long ISTDINT32;
#elif defined(__MACOS__)
	typedef UInt32 ISTDUINT32;
	typedef SInt32 ISTDINT32;
#elif defined(__APPLE__) && defined(__MACH__)
	#include <sys/types.h>
	typedef u_int32_t ISTDUINT32;
	typedef int32_t ISTDINT32;
#elif defined(__BEOS__)
	#include <sys/inttypes.h>
	typedef u_int32_t ISTDUINT32;
	typedef int32_t ISTDINT32;
#elif (defined(_MSC_VER) || defined(__BORLANDC__)) && (!defined(__MSDOS__))
	typedef unsigned __int32 ISTDUINT32;
	typedef __int32 ISTDINT32;
#elif defined(__GNUC__)
	#include <stdint.h>
	typedef uint32_t ISTDUINT32;
	typedef int32_t ISTDINT32;
#else 
	typedef unsigned long ISTDUINT32; 
	typedef long ISTDINT32;
#endif
#endif


//=====================================================================
// Integer Definition
//=====================================================================
#ifndef __IINT8_DEFINED
#define __IINT8_DEFINED
typedef char IINT8;
#endif

#ifndef __IUINT8_DEFINED
#define __IUINT8_DEFINED
typedef unsigned char IUINT8;
#endif

#ifndef __IUINT16_DEFINED
#define __IUINT16_DEFINED
typedef unsigned short IUINT16;
#endif

#ifndef __IINT16_DEFINED
#define __IINT16_DEFINED
typedef short IINT16;
#endif

#ifndef __IINT32_DEFINED
#define __IINT32_DEFINED
typedef ISTDINT32 IINT32;
#endif

#ifndef __IUINT32_DEFINED
#define __IUINT32_DEFINED
typedef ISTDUINT32 IUINT32;
#endif

#ifndef __IINT64_DEFINED
#define __IINT64_DEFINED
#if defined(_MSC_VER) || defined(__BORLANDC__)
typedef __int64 IINT64;
#else
typedef long long IINT64;
#endif
#endif

#ifndef __IUINT64_DEFINED
#define __IUINT64_DEFINED
#if defined(_MSC_VER) || defined(__BORLANDC__)
typedef unsigned __int64 IUINT64;
#else
typedef unsigned long long IUINT64;
#endif
#endif

#ifndef INLINE
#if defined(__GNUC__)

#if (__GNUC__ > 3) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1))
#define INLINE         __inline__ __attribute__((always_inline))
#else
#define INLINE         __inline__
#endif

#elif (defined(_MSC_VER) || defined(__BORLANDC__) || defined(__WATCOMC__))
#define INLINE __inline
#else
#define INLINE 
#endif
#endif

#if (!defined(__cplusplus)) && (!defined(inline))
#define inline INLINE
#endif


//=====================================================================
// QUEUE DEFINITION                                                  
//=====================================================================
#ifndef __IQUEUE_DEF__
#define __IQUEUE_DEF__
/*
链表结构本身并没有定义任何value字段，而是让持有IQUEUEHEAD的结构体自己去定义。
当持有了IQUEUEHEAD，该结构体就可以通过IQUEUEHEAD将自己构建成一个环状双向链表
*/

//KCP中设计的链表是一个环状双向链表
struct IQUEUEHEAD {
	struct IQUEUEHEAD *next, *prev;
};

typedef struct IQUEUEHEAD iqueue_head;


//---------------------------------------------------------------------
// queue init                                                         
//---------------------------------------------------------------------
#define IQUEUE_HEAD_INIT(name) { &(name), &(name) }
#define IQUEUE_HEAD(name) \
	struct IQUEUEHEAD name = IQUEUE_HEAD_INIT(name)

#define IQUEUE_INIT(ptr) ( \
	(ptr)->next = (ptr), (ptr)->prev = (ptr))

#define IOFFSETOF(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define ICONTAINEROF(ptr, type, member) ( \
		(type*)( ((char*)((type*)ptr)) - IOFFSETOF(type, member)) )

#define IQUEUE_ENTRY(ptr, type, member) ICONTAINEROF(ptr, type, member)


//---------------------------------------------------------------------
// queue operation                     
//---------------------------------------------------------------------
//node插入到head后面,head->node
#define IQUEUE_ADD(node, head) ( \
	(node)->prev = (head), (node)->next = (head)->next, \
	(head)->next->prev = (node), (head)->next = (node))
//node插入到head前面,node->head
#define IQUEUE_ADD_TAIL(node, head) ( \
	(node)->prev = (head)->prev, (node)->next = (head), \
	(head)->prev->next = (node), (head)->prev = (node))
//删除p,n之间的节点
#define IQUEUE_DEL_BETWEEN(p, n) ((n)->prev = (p), (p)->next = (n))

//删除entry,prev->entry->next =====>> prev->next
#define IQUEUE_DEL(entry) (\
	(entry)->next->prev = (entry)->prev, \
	(entry)->prev->next = (entry)->next, \
	(entry)->next = 0, (entry)->prev = 0)

#define IQUEUE_DEL_INIT(entry) do { \
	IQUEUE_DEL(entry); IQUEUE_INIT(entry); } while (0)

#define IQUEUE_IS_EMPTY(entry) ((entry) == (entry)->next)

#define iqueue_init		IQUEUE_INIT
#define iqueue_entry	IQUEUE_ENTRY
#define iqueue_add		IQUEUE_ADD
#define iqueue_add_tail	IQUEUE_ADD_TAIL
#define iqueue_del		IQUEUE_DEL
#define iqueue_del_init	IQUEUE_DEL_INIT
#define iqueue_is_empty IQUEUE_IS_EMPTY

#define IQUEUE_FOREACH(iterator, head, TYPE, MEMBER) \
	for ((iterator) = iqueue_entry((head)->next, TYPE, MEMBER); \
		&((iterator)->MEMBER) != (head); \
		(iterator) = iqueue_entry((iterator)->MEMBER.next, TYPE, MEMBER))

#define iqueue_foreach(iterator, head, TYPE, MEMBER) \
	IQUEUE_FOREACH(iterator, head, TYPE, MEMBER)

#define iqueue_foreach_entry(pos, head) \
	for( (pos) = (head)->next; (pos) != (head) ; (pos) = (pos)->next )
	

#define __iqueue_splice(list, head) do {	\
		iqueue_head *first = (list)->next, *last = (list)->prev; \
		iqueue_head *at = (head)->next; \
		(first)->prev = (head), (head)->next = (first);		\
		(last)->next = (at), (at)->prev = (last); }	while (0)

#define iqueue_splice(list, head) do { \
	if (!iqueue_is_empty(list)) __iqueue_splice(list, head); } while (0)

#define iqueue_splice_init(list, head) do {	\
	iqueue_splice(list, head);	iqueue_init(list); } while (0)


#ifdef _MSC_VER
#pragma warning(disable:4311)
#pragma warning(disable:4312)
#pragma warning(disable:4996)
#endif

#endif


//---------------------------------------------------------------------
// BYTE ORDER & ALIGNMENT
//---------------------------------------------------------------------
#ifndef IWORDS_BIG_ENDIAN
    #ifdef _BIG_ENDIAN_
        #if _BIG_ENDIAN_
            #define IWORDS_BIG_ENDIAN 1
        #endif
    #endif
    #ifndef IWORDS_BIG_ENDIAN
        #if defined(__hppa__) || \
            defined(__m68k__) || defined(mc68000) || defined(_M_M68K) || \
            (defined(__MIPS__) && defined(__MIPSEB__)) || \
            defined(__ppc__) || defined(__POWERPC__) || defined(_M_PPC) || \
            defined(__sparc__) || defined(__powerpc__) || \
            defined(__mc68000__) || defined(__s390x__) || defined(__s390__)
            #define IWORDS_BIG_ENDIAN 1
        #endif
    #endif
    #ifndef IWORDS_BIG_ENDIAN
        #define IWORDS_BIG_ENDIAN  0
    #endif
#endif

#ifndef IWORDS_MUST_ALIGN
	#if defined(__i386__) || defined(__i386) || defined(_i386_)
		#define IWORDS_MUST_ALIGN 0
	#elif defined(_M_IX86) || defined(_X86_) || defined(__x86_64__)
		#define IWORDS_MUST_ALIGN 0
	#elif defined(__amd64) || defined(__amd64__)
		#define IWORDS_MUST_ALIGN 0
	#else
		#define IWORDS_MUST_ALIGN 1
	#endif
#endif


//=====================================================================
// SEGMENT
//=====================================================================
struct IKCPSEG
{
	struct IQUEUEHEAD node;//node节点用来串接多个KCP segment，也就是前向后向指针
	IUINT32 conv;//会话编号，通信双方必须一致才能使用KCP协议交换数据
	IUINT32 cmd;//表明当前报文的类型，KCP共有四种类型
	IUINT32 frg;//分片的编号，当输出数据大于MSS时，需要将数据进行分片
	IUINT32 wnd;//填写己方的可用窗口大小
	IUINT32 ts;//记录发送时的时间戳，用来估计RTT
	IUINT32 sn;//为data报文的编号或者ack报文的确认编号
	IUINT32 una;//应该是UNA协议下表示收到的包（之前的包都收到）
	IUINT32 len;//
	IUINT32 resendts;//为下一次重发该报文的时间
	IUINT32 rto;//重传超时时间
	IUINT32 fastack;//记录该报文在收到ACK时被跳过了几次，用于快重传
	IUINT32 xmit;//记录该报文被传输了几次
	char data[1];//为实际传输的数据，即payload
};


//---------------------------------------------------------------------
// IKCPCB
//---------------------------------------------------------------------
//每一个KCP用户都需要调用 ikcp_create 创建一个kcp控制块ikcpcb.这个块用来实现整个KCP协议
struct IKCPCB
{
    IUINT32 conv;   // 标识会话
    IUINT32 mtu;    // 最大传输单元,默认数据为1400,最小为50
    IUINT32 mss;    // 最大分片大小,不大于mtu
    IUINT32 state;  // 连接状态(0xffffffff表示断开连接)
    
    IUINT32 snd_una;    // 第一个未确认的包
    IUINT32 snd_nxt;    // 下一个待分配包的序号
    IUINT32 rcv_nxt;    // 待接收消息序号.为了保证包的顺序,接收方会维护一个接收窗口,接收窗口有一个起始序号rcv_nxt 以及尾序号rcv_nxt + rcv_wnd(接收窗口大小)

    IUINT32 ts_recent;  
    IUINT32 ts_lastack;
    IUINT32 ssthresh;       // 拥塞窗口的阈值
    
    IINT32  rx_rttval;      // RTT的变化量,代表连接的抖动情况
    IINT32  rx_srtt;        // smoothed round trip time，平滑后的RTT；
    IINT32  rx_rto;         // 收ACK接收延迟计算出来的重传超时时间
    IINT32  rx_minrto;      // 最小重传超时时间

    IUINT32 snd_wnd;        // 发送窗口大小
    IUINT32 rcv_wnd;        // 接收窗口大小，本质上而言如果接收端一直不去读取数据则rcv_queue就会满(达到rcv_wnd)
    IUINT32 rmt_wnd;        // 远端接收窗口大小
    IUINT32 cwnd;           // 拥塞窗口大小, 动态变化
    IUINT32 probe;          // 探查变量, IKCP_ASK_TELL表示告知远端窗口大小。IKCP_ASK_SEND表示请求远端告知窗口大小；

    IUINT32 current;
    IUINT32 interval;       // 内部flush刷新间隔，对系统循环效率有非常重要影响, 间隔小了cpu占用率高, 间隔大了响应慢
    IUINT32 ts_flush;       // 下次flush刷新的时间戳
    IUINT32 xmit;           // 发送segment的次数, 当segment的xmit增加时，xmit增加(重传除外)

    IUINT32 nrcv_buf;       // 接收缓存中的消息数量
    IUINT32 nsnd_buf;       // 发送缓存中的消息数量

    IUINT32 nrcv_que;       // 接收队列中消息数量
    IUINT32 nsnd_que;       // 发送队列中消息数量

    IUINT32 nodelay;        // 是否启动无延迟模式。无延迟模式rtomin将设置为0，拥塞控制不启动；
    IUINT32 updated;         //是否调用过update函数的标识；

    IUINT32 ts_probe;       // 下次探查窗口的时间戳；
    IUINT32 probe_wait;     // 探查窗口需要等待的时间；

    IUINT32 dead_link;      // 最大重传次数，被认为连接中断；
    IUINT32 incr;           // 可发送的最大数据量；

    struct IQUEUEHEAD snd_queue;    //发送消息的队列 
    struct IQUEUEHEAD rcv_queue;    //接收消息的队列, 确认过用户可读取
    struct IQUEUEHEAD snd_buf;      //发送消息的缓存
    struct IQUEUEHEAD rcv_buf;      //接收消息的缓存

    IUINT32 *acklist;   //待发送的ack的列表 当收到一个数据报文时，将其对应的 ACK 报文的 sn 号以及时间戳 ts 
                        //同时加入到acklist 中，即形成如 [sn1, ts1, sn2, ts2 …] 的列表
    IUINT32 ackcount;   // 记录 acklist 中存放的 ACK 报文的数量 
    IUINT32 ackblock;   // acklist 数组的可用长度，当 acklist 的容量不足时，需要进行扩容

    void *user;     // 指针，可以任意放置代表用户的数据，也可以设置程序中需要传递的变量；
    char *buffer;   // 存储字节流信息 

    int fastresend; // 触发快速重传的重复ACK个数；
    int fastlimit;

    int nocwnd;     // 取消拥塞控制
    int stream;     // 是否采用流传输模式

    int logmask;    // 日志的类型，如IKCP_LOG_IN_DATA，方便调试

    int (*output)(const char *buf, int len, struct IKCPCB *kcp, void *user);//发送消息的回调函数
    void (*writelog)(const char *log, struct IKCPCB *kcp, void *user);  // 写日志的回调函数
};



typedef struct IKCPCB ikcpcb;

#define IKCP_LOG_OUTPUT			1
#define IKCP_LOG_INPUT			2
#define IKCP_LOG_SEND			4
#define IKCP_LOG_RECV			8
#define IKCP_LOG_IN_DATA		16
#define IKCP_LOG_IN_ACK			32
#define IKCP_LOG_IN_PROBE		64
#define IKCP_LOG_IN_WINS		128
#define IKCP_LOG_OUT_DATA		256
#define IKCP_LOG_OUT_ACK		512
#define IKCP_LOG_OUT_PROBE		1024
#define IKCP_LOG_OUT_WINS		2048

#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------
// interface
//---------------------------------------------------------------------

// create a new kcp control object, 'conv' must equal in two endpoint
// from the same connection. 'user' will be passed to the output callback
// output callback can be setup like this: 'kcp->output = my_udp_output'
ikcpcb* ikcp_create(IUINT32 conv, void *user);

// release kcp control object
void ikcp_release(ikcpcb *kcp);

// set output callback, which will be invoked by kcp
void ikcp_setoutput(ikcpcb *kcp, int (*output)(const char *buf, int len, 
	ikcpcb *kcp, void *user));

// user/upper level recv: returns size, returns below zero for EAGAIN
int ikcp_recv(ikcpcb *kcp, char *buffer, int len);

// user/upper level send, returns below zero for error
int ikcp_send(ikcpcb *kcp, const char *buffer, int len);

// update state (call it repeatedly, every 10ms-100ms), or you can ask 
// ikcp_check when to call it again (without ikcp_input/_send calling).
// 'current' - current timestamp in millisec. 
void ikcp_update(ikcpcb *kcp, IUINT32 current);

// Determine when should you invoke ikcp_update:
// returns when you should invoke ikcp_update in millisec, if there 
// is no ikcp_input/_send calling. you can call ikcp_update in that
// time, instead of call update repeatly.
// Important to reduce unnacessary ikcp_update invoking. use it to 
// schedule ikcp_update (eg. implementing an epoll-like mechanism, 
// or optimize ikcp_update when handling massive kcp connections)
IUINT32 ikcp_check(const ikcpcb *kcp, IUINT32 current);

// when you received a low level packet (eg. UDP packet), call it
int ikcp_input(ikcpcb *kcp, const char *data, long size);

// flush pending data
void ikcp_flush(ikcpcb *kcp);

// check the size of next message in the recv queue
int ikcp_peeksize(const ikcpcb *kcp);

// change MTU size, default is 1400
int ikcp_setmtu(ikcpcb *kcp, int mtu);

// set maximum window size: sndwnd=32, rcvwnd=32 by default
int ikcp_wndsize(ikcpcb *kcp, int sndwnd, int rcvwnd);

// get how many packet is waiting to be sent
int ikcp_waitsnd(const ikcpcb *kcp);

// fastest: ikcp_nodelay(kcp, 1, 20, 2, 1)
// nodelay: 0:disable(default), 1:enable
// interval: internal update timer interval in millisec, default is 100ms 
// resend: 0:disable fast resend(default), 1:enable fast resend
// nc: 0:normal congestion control(default), 1:disable congestion control
int ikcp_nodelay(ikcpcb *kcp, int nodelay, int interval, int resend, int nc);


void ikcp_log(ikcpcb *kcp, int mask, const char *fmt, ...);

// setup allocator
void ikcp_allocator(void* (*new_malloc)(size_t), void (*new_free)(void*));

// read conv
IUINT32 ikcp_getconv(const void *ptr);


#ifdef __cplusplus
}
#endif

#endif


