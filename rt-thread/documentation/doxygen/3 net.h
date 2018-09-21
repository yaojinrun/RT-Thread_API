/*
 * This file is only used for doxygen document generation.
 */

/**
 * @defgroup NET 网络框架
 *
 */

/**
 * @addtogroup NET
 */
/*@{*/

/**
 * @defgroup SAL 网络框架
 *
 */

/**
 * @defgroup SOC 套接字使用
 *
 */


/*@}*/

/**
 * @ingroup SAL
 *
 * @brief SAL组件初始化
 *
 * 该函数完成对组件中使用的互斥锁等资源的初始化。
 *
 * @return 0 SAL组件初始化成功。
 */
int sal_init(void)
{
}


/**
 * @ingroup SAL
 *
 * @brief 网络协议簇注册
 *
 * 该函数用于往SAL组件中注册网络协议簇。
 *
 * @param pf 协议簇结构体指针
 *
 * @return 0 注册成功；失败返回 -1。
 */
int sal_proto_family_register(const struct proto_family *pf)
{
}


/**
 * @ingroup SOC
 *
 * @brief 创建套接字（socket）
 *
 * 该函数用于根据指定的地址族、数据类型和协议来分配一个套接字描述符及其所用的资源。
 *
 * @param domain 协议族
 * @param type 协议类型
 * @param protocol 实际使用的运输层
 *
 * @return 成功，返回一个代表套接字描述符的整数；失败返回 -1。
 */
int socket(int domain, int type, int protocol)
{
}


/**
 * @ingroup SOC
 *
 * @brief 创建套接字（socket）
 *
 * 该函数用于根据指定的地址族、数据类型和协议来分配一个套接字描述符及其所用的资源。
 *
 * @param domain 协议族
 * @param type 协议类型
 * @param protocol 实际使用的运输层
 *
 * @return 成功，返回一个代表套接字描述符的整数；失败返回 -1。
 */
int socket(int domain, int type, int protocol)
{
}

/**
 * @ingroup SOC
 *
 * @brief 绑定套接字
 *
 * 该函数用于根据指定的地址族、数据类型和协议来分配一个套接字描述符及其所用的资源。
 *
 * @param s 套接字描述符
 * @param name 指向sockaddr结构体的指针，代表要绑定的地址
 * @param namelen sockaddr结构体的长度
 *
 * @return 0 成功；-1 失败。
 */
int bind(int s, const struct sockaddr *name, socklen_t namelen)
{
}

/**
 * @ingroup SOC
 *
 * @brief 监听套接字
 *
 * 该函数用于TCP服务器监听指定套接字连接。
 *
 * @param s 套接字描述符
 * @param backlog 表示一次能够等待的最大连接数目
 *
 * @return 0 成功；-1 失败。
 */
int listen(int s, int backlog)
{
}

/**
 * @ingroup SOC
 *
 * @brief 接收连接
 *
 * 当应用程序监听来自其他主机的连接时，使用该函数初始化连接，accept()为每个
 * 连接创立新的套接字并从监听队列中移除这个连接。
 *
 * @param s 套接字描述符
 * @param addr 客户端设备地址信息
 * @param addrlen 客户端设备地址结构体的长度
 *
 * @return 成功，返回新创建的套接字描述符；失败返回 -1。
 */
int accept(int s, struct sockaddr *addr, socklen_t *addrlen)
{
}


/**
 * @ingroup SOC
 *
 * @brief 建立连接
 *
 * 该函数用于建立与指定socket的连接。
 *
 * @param s 套接字描述符
 * @param name 服务器地址信息
 * @param namelen 服务器地址结构体的长度
 *
 * @return 0 成功；-1 失败。
 */
int connect(int s, const struct sockaddr *name, socklen_t namelen)
{
}


/**
 * @ingroup SOC
 *
 * @brief TCP 数据发送
 *
 * @param s 套接字描述符
 * @param dataptr 要发送的数据指针
 * @param size 发送的数据长度
 * @param flags 标志，一般为0
 *
 * @return 成功，返回发送的数据的长度；<=0 失败。
 */
int send(int s, const void *dataptr, size_t size, int flags)
{
}


/**
 * @ingroup SOC
 *
 * @brief TCP 数据接收
 *
 * @param s 套接字描述符
 * @param mem 接收的数据指针
 * @param len 接收的数据长度
 * @param flags 标志，一般为0
 *
 * @return 成功，返回接收的数据的长度；0 表示目标地址已传输完并关闭连接。
 */
int recv(int s, void *mem, size_t len, int flags)
{
}


/**
 * @ingroup SOC
 *
 * @brief UDP 数据发送
 *
 * @param s 套接字描述符
 * @param dataptr 发送的数据指针
 * @param size 发送的数据长度
 * @param flags 标志，一般为0
 * @param to 目标地址结构体指针
 * @param tolen 目标地址结构体长度
 *
 * @return 成功，返回发送的数据的长度；小于等于0 失败。
 */
int sendto(int s, const void *dataptr, size_t size, int flags, const struct sockaddr *to, socklen_t tolen)
{
}


/**
 * @ingroup SOC
 *
 * @brief UDP 数据接收
 *
 * @param s 套接字描述符
 * @param mem 接收的数据指针
 * @param len 接收的数据长度
 * @param flags 标志，一般为0
 * @param flags 标志，一般为0
 * @param from 接收地址结构体指针
 * @param fromlen 接收地址结构体长度
 *
 * @return 成功，返回接收的数据的长度；0 接收地址已传输完并关闭连接；小于0 失败。
 */
int recvfrom(int s, void *mem, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen)
{
}


/**
 * @ingroup SOC
 *
 * @brief 关闭套接字
 *
 * @param s 套接字描述符
 *
 * @return 0 成功；-1 失败。
 */
int closesocket(int s)
{
}


/**
 * @ingroup SOC
 *
 * @brief 按设置关闭套接字
 *
 * @param s 套接字描述符
 * @param how 套接字控制的方式
 *
 * @return 0 成功；-1 失败。
 */
int shutdown(int s, int how)
{
}


/**
 * @ingroup SOC
 *
 * @brief 获取套接字选项
 *
 * @param s 套接字描述符
 * @param level 协议栈配置选项
 * @param optname 需要设置的选项名
 * @param optval 获取选项值的缓冲区地址
 * @param optlen 获取选项值的缓冲区长度地址
 *
 * @return 0 成功；小于0 失败。
 */
int getsockopt(int s, int level, int optname, void *optval, socklen_t *optlen)
{
}


/**
 * @ingroup SOC
 *
 * @brief 获取远端地址信息
 *
 * @param s 套接字描述符
 * @param name 接收信息的地址结构体指针
 * @param namelen 接收信息的地址结构体长度
 *
 * @return 0 成功；小于0 失败。
 */
int getpeername(int s, struct sockaddr *name, socklen_t *namelen)
{
}


/**
 * @ingroup SOC
 *
 * @brief 获取本地地址信息
 *
 * @param s 套接字描述符
 * @param name 接收信息的地址结构体指针
 * @param namelen 接收信息的地址结构体长度
 *
 * @return 0 成功；小于0 失败。
 */
int getsockname(int s, struct sockaddr *name, socklen_t *namelen)
{
}


/**
 * @ingroup SOC
 *
 * @brief 配置套接字参数
 *
 * @param s 套接字描述符
 * @param cmd 套接字操作命令
 * @param arg 操作命令所带参数
 *
 * @return 0 成功；小于0 失败。
 */
int ioctlsocket(int s, long cmd, void *arg)
{
}