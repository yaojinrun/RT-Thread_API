/*
 * This file is only used for doxygen document generation.
 */

/**
 * @defgroup NET ����
 *
 * @brief �������API˵��
 */

/**
 * @addtogroup NET
 */
/*@{*/

/**
typedef struct fd_set
{
  unsigned char fd_bits [(FD_SETSIZE+7)/8];
} fd_set;
 * struct sockaddr
 */
struct sockaddr {
  u8_t        sa_len;		/**< @brief ��ַ���� */
  sa_family_t sa_family;	/**< @brief ��ַ�� */
  char        sa_data[14];	/**< @brief 14�ֽڣ������׽����е�Ŀ���ַ�Ͷ˿���Ϣ */
};


/**
 * struct in_addr
 */
struct in_addr {
  in_addr_t s_addr;			/**< @brief ����������ʮ�����Ʊ�ʾ��IP��ַ */
};

/**
 * struct sockaddr_in
 */
struct sockaddr_in {
  u8_t            sin_len;		/**< @brief ��ַ���� */
  sa_family_t     sin_family;	/**< @brief ��ַ�� */
  in_port_t       sin_port;		/**< @brief 16λTCP/UDP�˿ں� */
  struct in_addr  sin_addr;		/**< @brief 32λIP��ַ */
#define SIN_ZERO_LEN 8
  char            sin_zero[SIN_ZERO_LEN];
};

/**
 * struct hostent
 */
struct hostent {
    char  *h_name;      /**< @brief �����������ٷ����� */
    char **h_aliases;   /**< @brief �������б������ɵ��ַ������飬ͬһIP�ɰ󶨶������ */
    int    h_addrtype;  /**< @brief ����IP��ַ�����ͣ�����IPV4��AF_INET������IPV6 */
    int    h_length;    /**< @brief ����IP��ַ���ȣ�IPV4��ַΪ4��IPV6��ַ��Ϊ16 */
    char **h_addr_list; /**< @brief ������ip��ַ���������ֽ���洢����Ҫ��ӡ�����IP����Ҫ����inet_ntoa()�� */
#define h_addr h_addr_list[0] /**< @brief Ϊ�������� */
};

/**
 * struct addrinfo
 */
struct addrinfo {
    int               ai_flags;      /**< @brief �����־ */
    int               ai_family;     /**< @brief ��ַ���׽��� */
    int               ai_socktype;   /**< @brief �׽������� */
    int               ai_protocol;   /**< @brief �׽���Э�� */
    socklen_t         ai_addrlen;    /**< @brief �׽��ֵ�ַ�ĳ��� */
    struct sockaddr  *ai_addr;       /**< @brief �׽��ֵ��׽��ֵ�ַ */
    char             *ai_canonname;  /**< @brief ����ص�Ĺ淶���� */
    struct addrinfo  *ai_next;       /**< @brief ָ����һ���б��ָ�� */
};

/**
 * @defgroup SAL �׽��ֳ����
 * 
 * @brief SAL�����ʼ��API˵��
 */

/**
 * @defgroup SOC �׽���ʹ��
 *
 * @brief �׽���ʹ�÷�ʽ���API˵��
 */

/**
 * @defgroup DNS ��������IP��ַ�Ľ���
 *
 * @brief ��������IP��ַ�Ľ������API
 */



/*@}*/

/**
 * @ingroup SAL
 *
 * @brief SAL�����ʼ��
 *
 * �ú�����ɶ������ʹ�õĻ���������Դ�ĳ�ʼ����
 *
 * @return 0 SAL�����ʼ���ɹ���
 */
int sal_init(void);


/**
 * @ingroup SAL
 *
 * @brief ����Э���ע��
 *
 * �ú���������SAL�����ע������Э��ء�
 *
 * @param pf Э��ؽṹ��ָ��
 *
 * @return 0 ע��ɹ���ʧ�ܷ��� -1��
 */
int sal_proto_family_register(const struct proto_family *pf);


/**
 * @ingroup SOC
 *
 * @brief fd_set
 */
typedef struct fd_set
{
  unsigned char fd_bits [(FD_SETSIZE+7)/8];			/**< @brief ���飬ÿһ������Ԫ�ض�����һ�򿪵��ļ����(socket���ļ����ܵ����豸��)������ϵ */
} fd_set;


/**
 * @ingroup SOC
 *
 * @brief �����׽��֣�socket��
 *
 * �ú������ڸ���ָ���ĵ�ַ�塢�������ͺ�Э��������һ���׽����������������õ���Դ��
 *
 * @param domain Э����
 * @param type Э������
 * @param protocol ʵ��ʹ�õ������
 *
 * @return �ɹ�������һ�������׽�����������������ʧ�ܷ��� -1��
 */
int socket(int domain, int type, int protocol);


/**
 * @ingroup SOC
 *
 * @brief �����׽��֣�socket��
 *
 * �ú������ڸ���ָ���ĵ�ַ�塢�������ͺ�Э��������һ���׽����������������õ���Դ��
 *
 * @param domain Э����
 * @param type Э������
 * @param protocol ʵ��ʹ�õ������
 *
 * @return �ɹ�������һ�������׽�����������������ʧ�ܷ��� -1��
 */
int socket(int domain, int type, int protocol);

/**
 * @ingroup SOC
 *
 * @brief ���׽���
 *
 * �ú������ڸ���ָ���ĵ�ַ�塢�������ͺ�Э��������һ���׽����������������õ���Դ��
 *
 * @param s �׽���������
 * @param name ָ��sockaddr�ṹ���ָ�룬����Ҫ�󶨵ĵ�ַ
 * @param namelen sockaddr�ṹ��ĳ���
 *
 * @return 0 �ɹ���-1 ʧ�ܡ�
 */
int bind(int s, const struct sockaddr *name, socklen_t namelen);

/**
 * @ingroup SOC
 *
 * @brief �����׽���
 *
 * �ú�������TCP����������ָ���׽������ӡ�
 *
 * @param s �׽���������
 * @param backlog ��ʾһ���ܹ��ȴ������������Ŀ
 *
 * @return 0 �ɹ���-1 ʧ�ܡ�
 */
int listen(int s, int backlog);

/**
 * @ingroup SOC
 *
 * @brief ��������
 *
 * ��Ӧ�ó������������������������ʱ��ʹ�øú�����ʼ�����ӣ�accept()Ϊÿ��
 * ���Ӵ����µ��׽��ֲ��Ӽ����������Ƴ�������ӡ�
 *
 * @param s �׽���������
 * @param addr �ͻ����豸��ַ��Ϣ
 * @param addrlen �ͻ����豸��ַ�ṹ��ĳ���
 *
 * @return �ɹ��������´������׽�����������ʧ�ܷ��� -1��
 */
int accept(int s, struct sockaddr *addr, socklen_t *addrlen);


/**
 * @ingroup SOC
 *
 * @brief ��������
 *
 * �ú������ڽ�����ָ��socket�����ӡ�
 *
 * @param s �׽���������
 * @param name ��������ַ��Ϣ
 * @param namelen ��������ַ�ṹ��ĳ���
 *
 * @return 0 �ɹ���-1 ʧ�ܡ�
 */
int connect(int s, const struct sockaddr *name, socklen_t namelen);


/**
 * @ingroup SOC
 *
 * @brief TCP ���ݷ���
 *
 * @param s �׽���������
 * @param dataptr Ҫ���͵�����ָ��
 * @param size ���͵����ݳ���
 * @param flags ��־��һ��Ϊ0
 *
 * @return �ɹ������ط��͵����ݵĳ��ȣ�<=0 ʧ�ܡ�
 */
int send(int s, const void *dataptr, size_t size, int flags);


/**
 * @ingroup SOC
 *
 * @brief TCP ���ݽ���
 *
 * @param s �׽���������
 * @param mem ���յ�����ָ��
 * @param len ���յ����ݳ���
 * @param flags ��־��һ��Ϊ0
 *
 * @return �ɹ������ؽ��յ����ݵĳ��ȣ�0 ��ʾĿ���ַ�Ѵ����겢�ر����ӡ�
 */
int recv(int s, void *mem, size_t len, int flags);


/**
 * @ingroup SOC
 *
 * @brief UDP ���ݷ���
 *
 * @param s �׽���������
 * @param dataptr ���͵�����ָ��
 * @param size ���͵����ݳ���
 * @param flags ��־��һ��Ϊ0
 * @param to Ŀ���ַ�ṹ��ָ��
 * @param tolen Ŀ���ַ�ṹ�峤��
 *
 * @return �ɹ������ط��͵����ݵĳ��ȣ�С�ڵ���0 ʧ�ܡ�
 */
int sendto(int s, const void *dataptr, size_t size, int flags, const struct sockaddr *to, socklen_t tolen);


/**
 * @ingroup SOC
 *
 * @brief UDP ���ݽ���
 *
 * @param s �׽���������
 * @param mem ���յ�����ָ��
 * @param len ���յ����ݳ���
 * @param flags ��־��һ��Ϊ0
 * @param flags ��־��һ��Ϊ0
 * @param from ���յ�ַ�ṹ��ָ��
 * @param fromlen ���յ�ַ�ṹ�峤��
 *
 * @return �ɹ������ؽ��յ����ݵĳ��ȣ�0 ���յ�ַ�Ѵ����겢�ر����ӣ�С��0 ʧ�ܡ�
 */
int recvfrom(int s, void *mem, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen);


/**
 * @ingroup SOC
 *
 * @brief �ر��׽���
 *
 * @param s �׽���������
 *
 * @return 0 �ɹ���-1 ʧ�ܡ�
 */
int closesocket(int s);


/**
 * @ingroup SOC
 *
 * @brief �����ùر��׽���
 *
 * @param s �׽���������
 * @param how �׽��ֿ��Ƶķ�ʽ
 *
 * @return 0 �ɹ���-1 ʧ�ܡ�
 */
int shutdown(int s, int how);


/**
 * @ingroup SOC
 *
 * @brief ��ȡ�׽���ѡ��
 *
 * @param s �׽���������
 * @param level Э��ջ����ѡ��
 * @param optname ��Ҫ���õ�ѡ����
 * @param optval ��ȡѡ��ֵ�Ļ�������ַ
 * @param optlen ��ȡѡ��ֵ�Ļ��������ȵ�ַ
 *
 * @return 0 �ɹ���С��0 ʧ�ܡ�
 */
int getsockopt(int s, int level, int optname, void *optval, socklen_t *optlen);


/**
 * @ingroup SOC
 *
 * @brief ��ȡԶ�˵�ַ��Ϣ
 *
 * @param s �׽���������
 * @param name ������Ϣ�ĵ�ַ�ṹ��ָ��
 * @param namelen ������Ϣ�ĵ�ַ�ṹ�峤��
 *
 * @return 0 �ɹ���С��0 ʧ�ܡ�
 */
int getpeername(int s, struct sockaddr *name, socklen_t *namelen);


/**
 * @ingroup SOC
 *
 * @brief ��ȡ���ص�ַ��Ϣ
 *
 * @param s �׽���������
 * @param name ������Ϣ�ĵ�ַ�ṹ��ָ��
 * @param namelen ������Ϣ�ĵ�ַ�ṹ�峤��
 *
 * @return 0 �ɹ���С��0 ʧ�ܡ�
 */
int getsockname(int s, struct sockaddr *name, socklen_t *namelen);


/**
 * @ingroup SOC
 *
 * @brief �����׽��ֲ���
 *
 * @param s �׽���������
 * @param cmd �׽��ֲ�������
 * @param arg ����������������
 *
 * @return 0 �ɹ���С��0 ʧ�ܡ�
 */
int ioctlsocket(int s, long cmd, void *arg);


/**
 * @ingroup DNS
 *
 * @brief ��ȡ������ַ
 *
 * @param hostname һ���ַ���ָ�룬����������������
 *
 * �ú�������ʹ������������������ȡ��ַ��
 *
 * @return ���ض�Ӧ�ڸ����������İ����������ֺ͵�ַ��Ϣ��hostent�ṹָ�롣
 */
struct hostent * gethostbyname(const char * hostname);

