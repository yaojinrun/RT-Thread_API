/*
 * This file is only used for doxygen document generation.
 */
 
/**
 * @defgroup Kernel 内核
 *
 * @brief RT-Thread 内核API是RT-Thread的核心API
 *
 * 它支持以下功能：
 * - 多线程管理
 * - 同步机制
 * - 线程间通信
 * - 内存管理
 * - 异步计时器
 */

/**
 * @addtogroup Kernel
 */
/*@{*/

/**
 * @defgroup BasicDef 基础定义
 *
 * @brief RT-Thread RTOS中的基本数据类型
 *
 * 这些是RT-Thread RTOS中使用的基本定义。 通常，RT-Thread内核使用自己的基本数据类型定义，
 * 例如rt_uint32_t，rt_uint8_t等，它们不依赖于编译器或体系结构。
 */
 
/**
 * @defgroup SystemInit 系统初始化
 *
 * @brief RT-Thread 的启动流程
 *
 * RT-Thread 的启动代码，大致可以分为四个部分：
 *
 * （1）初始化与系统相关的硬件；
 *
 * （2）初始化系统内核对象，例如定时器、调度器、信号；
 *
 * （3）创建main线程，在main线程中对各类模块依次进行初始化；
 *
 * （4）初始化定时器线程、空闲线程，并启动调度器。

 * RT-Thread的启动流程如下图所示：
 *
 * @image html rtthread_startup.png "系统启动流程"
 * 
 */
 
/**
 * @defgroup component 组件初始化
 *
 * @brief RT-Thread 的自动初始化机制
 *
 * 一般来说，在系统里添加新的功能模块的时候，在使用前都必须先初始化，
 * 通常的做法是在主程序运行前手动添加调用初始化函数。
 * 而 RT-Thread 提供了另一种低耦合高内聚的初始化方式，它不需要我们再
 * 手动添加调用初始化函数，它能在系统运行前自动完成各模块的初始化。
 * 
 * 组件初始化方式要求初始化函数主动通过一些宏接口进行申明，链接器会
 * 自动收集所有被申明的初始化函数，放到特定的数据段中，数据段中的
 * 所有函数在系统初始化时会被自动调用。
 *
 * 组件初始化的所有宏接口及其被初始化的顺序如表所示：

 * 初始化顺序 | 接口 
 * ---|---
 *  1  | INIT_BOARD_EXPORT(fn); 
 *  2 | INIT_PREV_EXPORT(fn); 
 *  3  | INIT_DEVICE_EXPORT(fn);
 *  4  | INIT_COMPONENT_EXPORT(fn);
 *  5  | INIT_ENV_EXPORT(fn);
 *  6  | INIT_APP_EXPORT(fn);
 */
 
 
/**
 * @defgroup KernelObject 内核对象管理
 * @brief 内核对象管理接口
 *
 * 内核对象系统可以访问和管理所有内核对象。
 *
 * 内核对象包括内核中的大多数设备：
 * - 线程
 * - 信号量和互斥量
 * - 事件/快速事件，邮箱，消息队列
 * - 内存池
 * - 定时器
 * @image html Kernel_Object.png "Figure 2: Kernel Object"
 * @image rtf  Kernel_Object.png "Figure 2: Kernel Object"
 *
 * 内核对象可以是静态对象，其内存在编译时就已分配。
 * 也可以是动态对象，其内存是在运行时从系统堆中分配。
 */

/**
 * @defgroup Thread 线程管理
 * @brief 线程管理接口
 *
 * RT-Thread操作系统是基于线程调度的多任务系统。
 * - 调度过程是一种完全抢占式的基于优先级的调度算法。
 * - 支持8/32/256优先级，其中0表示最高，7/31/255表示最低。最低优先级7/31/255优先级用于空闲线程。
 * - 支持以相同优先级运行的线程。 共享时间片循环调度用于这种情况。
 * - 调度器选择下一个已就绪的有最高优先级的线程运行。
 * - 线程管理有四种状态
 * - ＃初始状态（还未开始运行的状态）
 * - ＃运行中/就绪
 * - ＃挂起
 * - ＃已结束
 * - 系统中的线程数不受限制，仅与RAM大小有关。
 *
 */

/**
 * @defgroup Clock 时钟管理
 * @brief 时钟管理
 *
 * RT-Thread使用时钟节拍来实现共享时间片调度。
 */


/**
 * @defgroup Timer 定时器管理
 * @brief 定时器管理接口
 *
 * 线程的定时精度由定时器决定。 定时器可以设置为单次或定期超时。
 *
 */
 
/**
 * @defgroup IPC 线程间通讯
 * @brief 线程间通讯设施
 *
 * RT-Thread操作系统支持传统的信号量和互斥量。
 * - 互斥对象使用继承的优先级来防止优先级反转。
 * - 信号量释放操作对中断服务程序是安全的。
 *
 * 此外，线程获取信号量或互斥锁的阻塞队列可以按优先级或FIFO排序。有两个标志来表示这种机制。
 * - RT_IPC_FLAG_FIFO
 * 当资源可用时，最先在此资源上挂起的线程将获得资源。
 * - RT_IPC_FLAG_PRIO
 * 当资源可用时，具有最高优先级的线程将获得资源。
 *
 * RT-Thread操作系统支持事件/快速事件，邮箱和消息队列。
 * - 事件机制通过设置一个或多个相应标志位唤醒thead事件发生时的二进制数位。
 * - 快速事件支持事件线程队列。一旦发生一位事件，可以及时得找到相应阻塞的线程，然后唤醒。
 * - 在邮箱中，邮件长度固定为4字节，这比消息队列更有效。
 * - 通信设备的发送动作对于中断服务程序也是安全的。
 */
 
/**
 * @defgroup Pool 内存池管理
 * @brief 静态内存池的接口
 *
 * 内存池（Memory Pool）是一种内存分配方式，用于分配大量大小相同的小内存块。
 * 它可以极大地加快内存分配与释放的速度，且能尽量避免内存碎片化。当内存池为空时，
 * 可以阻止分配的线程（或者立即返回，或者等待一段时间返回，这由timeout参数确定）。
 * 当其他线程向此内存池释放内存块时，被阻塞的线程将被唤醒。
 *
 */

/**
 * @defgroup MM 动态内存的管理
 * @brief 动态内存堆的API接口
 *
 * 动态内存管理是一个真实的堆内存管理模块，可以在当前资源满足的情况下，根据用户
 * 的需求分配任意大小的内存块。而当用户不需要再使用这些内存块时，又可以释放回堆中
 * 供其他应用分配使用。RT-Thread 系统为了满足不同的需求，提供了两套不同的动态内存
 * 管理算法，分别是小堆内存管理算法和 SLAB 内存管理算法。
 *
 * 小堆内存管理模块主要针对系统资源比较少，一般用于小于2MB内存空间的系统；而SLAB
 * 内存管理模块则主要是在系统资源比较丰富时，提供了一种近似多内存池管理算法的快速
 * 算法。两种内存管理模块在系统运行时只能选择其中之一或者完全不使用动态堆内存管理
 * 器。这两种管理模块提供的API接口完全相同。
 *
 * 除上述之外，RT-Thread还有一种针对多内存堆的管理机制，即memheap内存管理。memheap方法
 * 适用于系统存在多个内存堆的情况，它可以将多个内存“粘贴”在一起，形成一个大的内存堆，
 * 用户使用起来会感到格外便捷。
 *
 */
 

/**
 * @defgroup bsp 中断管理
 *
 * @brief 中断管理接口
 *
 * RT-Thread的中断管理功能主要是管理中断设备、中断服务例程、中断嵌套、中断栈的维护、
 * 线程切换时的现场保护和恢复等。
 *
 * 当CPU正在处理内部数据时，外界发生了紧急情况，要求CPU暂停当前的工作转去处理这个异步事件。
 * 处理完毕后，再回到原来被中断的地址，继续原来的工作，这样的过程称为中断。实现这一功能的
 * 系统称为中断系统，申请CPU中断的请求源称为中断源，当多个中断源同时向CPU请求中断时，就存在
 * 一个中断优先权的问题。通常根据中断源的优先级别，优先处理最紧急事件的中断请求源，即最先
 * 响应级别最高的中断请求。
 *
 * 当中断产生时，CPU将按如下的顺序执行：
 * - （1）保存当前处理机状态信息；
 * - （2）载入异常或中断处理函数到PC寄存器；
 * - （3）把控制权转交给处理函数并开始执行；
 * - （4）当处理函数执行完成时，恢复处理器状态信息；
 * - （5）从异常或中断中返回到前一个程序执行点。
 *
 * 中断使得CPU可以在事件发生时才进行处理，而不必让CPU连续不断地查询是否有相应的事件发生。
 *
 */

/**
 * @defgroup Hook 系统钩子函数
 * @brief 系统钩子函数
 *
 * 为了在运行时跟踪和记录RT-Thread的活动，引入了一种钩子机制。
 *
 * 钩子函数是在一些特殊检查点调用的一类函数。
 * 钩子函数包括：
 * - 对象钩子函数，在创建，删除，获取和放置等对象时调用。
 * - 调度器钩子函数，在线程切换和空闲线程循环中调用。
 * - 内存钩子函数，在分配或释放内存块时调用。
 * - 定时器钩子函数，在定时器超时时调用。
 *
 */

 
/**
 * @defgroup KernelService 其他内核服务
 * @brief 内核中其他有用的服务
 */

/**
 * @defgroup Error 错误代码
 * @brief 错误代码
 *
 * 定义错误代码以识别发生哪种错误。 当发生错误时，将设置当前线程的错误号。
 * see @ref _rt_errno
 */

/*@}*/



 
/**
 * @addtogroup IPC
 */
/*@{*/

 
 /**
 * @defgroup semaphore 信号量
 * @brief 信号量
 *
 * 信号量是一种轻型的用于解决线程间同步问题的内核对象，线程可以获取或释放它，
 * 从而达到同步或互斥的目的。信号量就像一把钥匙，把一段临界区给锁住，只允许
 * 有钥匙的线程进行访问：线程拿到了钥匙，才允许它进入临界区；而离开后把钥匙
 * 传递给排队在后面的等待线程，让后续线程依次进入临界区。
 *
 * 信号量是一种非常灵活的同步方式，可以运用在多种场合中。形成锁，同步，资源计数
 * 等关系，也能方便的用于线程与线程，中断与线程的同步中。
 *
 */

 /**
 * @defgroup mutex 互斥量
 * @brief 互斥量
 *
 * 互斥量又叫相互排斥的信号量，是一种特殊的二值性信号量。它和信号量不同的是，
 * 它支持互斥量所有权、递归访问以及防止优先级翻转的特性。
 *
 * 互斥量的使用比较单一，因为它是信号量的一种，并且它是以锁的形式存在。
 * 在初始化的时候，互斥量永远都处于开锁的状态，而被线程持有的时候则立刻转为
 * 闭锁的状态。互斥量更适合于：
 *
 * - 线程多次持有互斥量的情况下。这样可以避免同一线程多次递归持有而造成死锁的问题；
 * - 可能会由于多线程同步而造成优先级翻转的情况；
 *
 */

 /**
 * @defgroup event 事件
 * @brief 事件
 *
 * 事件主要用于线程间的同步，与信号量不同，它的特点是可以实现一对多，多对多的同步。
 * 事件另外一个特性是，接收线程可等待多种事件，即多个事件对应一个线程或多个线程。
 * 同时按照线程等待的参数，可选择是 “逻辑或” 触发还是 “逻辑与” 触发。
 * 
 * RT-Thread 定义的事件有以下特点：
 * 
 * - 事件只与线程相关，事件间相互独立：每个线程拥有 32 个事件标志，采用一个 32 bit 
 * 无符号整型数进行记录，每一个 bit 代表一个事件。若干个事件构成一个事件集；
 * - 事件仅用于同步，不提供数据传输功能；
 * - 事件无排队性，即多次向线程发送同一事件 (如果线程还未来得及读走)，其效果等同于只
 * 发送一次。
 *
 */

 /**
 * @defgroup mailbox 邮箱
 * @brief 邮箱
 *
 *邮箱是一种简单的线程间消息传递方式，在 RT-Thread 操作系统的实现中能够一次传递 4 字节邮件，
 * 并且邮箱具备一定的存储功能，能够缓存一定数量的邮件数 (邮件数由创建、初始化邮箱时指定的
 * 容量决定)。邮箱中一封邮件的最大长度是 4 字节，所以邮箱能够用于不超过 4 字节的消息传递，
 * 当传送的消息长度大于这个数目时就不能再采用邮箱的方式。
 *
 */
 
 /**
 * @defgroup messagequeue 消息队列
 * @brief 消息队列
 *
 * 消息队列是另一种常用的线程间通讯方式，它能够接收来自线程或中断服务例程中不固定长度的消息，
 * 并把消息缓存在自己的内存空间中。其他线程也能够从消息队列中读取相应的消息，而当消息队列是
 * 空的时候，可以挂起读取线程。当有新的消息到达时，挂起的线程将被唤醒以接收并处理消息。消息
 * 队列是一种异步的通信方式。
 * 
 * 消息队列可以应用于发送不定长消息的场合，包括线程与线程间的消息交换，以及中断服务例程中
 * 发送给线程的消息（中断服务例程不可能接收消息）。
 *
 */
 
 
 /**
 * @defgroup Signal 信号
 * @brief 信号
 *
 * 软中断信号（signal，又简称为信号）用来通知线程发生了异步事件。在软件层次上是对中断机制的一种模拟，
 * 在原理上，一个线程收到一个信号与处理器收到一个中断请求可以说是一样的。信号是线程间通信机制中唯一的
 * 异步通信机制，一个线程不必通过任何操作来等待信号的到达，事实上，线程也不知道信号到底什么时候到达，
 * 线程之间可以互相通过调用 kill 发送软中断信号。
 *
 * 收到信号的线程对各种信号有不同的处理方法。处理方法可以分为三类：
 *
 * - 第一种是类似中断的处理程序，对于需要处理的信号，线程可以指定处理函数，由该函数来处理。
 * - 第二种方法是，忽略某个信号，对该信号不做任何处理，就象未发生过一样。
 * - 第三种方法是，对该信号的处理保留系统的默认值。
 *
 */
 
/*@}*/



 
/**
 * @addtogroup KernelService
 */
/*@{*/


 /**
 * @defgroup str 字符串操作
 * @brief 字符串操作
 *
 *
 */

 /**
 * @defgroup mem 内存操作
 * @brief 内存操作
 *
 *
 */
 
 /**
 * @defgroup list 双向链表
 * @brief 双向链表
 *
 *
 */
 
 /**
 * @defgroup slist 单向链表
 * @brief 单向链表
 *
 *
 */
 

/*@}*/


