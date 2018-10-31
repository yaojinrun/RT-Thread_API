/*
 * File      : kservice.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2012, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2006-03-16     Bernard      the first version
 * 2006-05-25     Bernard      rewrite vsprintf
 * 2006-08-10     Bernard      add rt_show_version
 * 2010-03-17     Bernard      remove rt_strlcpy function
 *                             fix gcc compiling issue.
 * 2010-04-15     Bernard      remove weak definition on ICCM16C compiler
 * 2012-07-18     Arda         add the alignment display for signed integer
 * 2012-11-23     Bernard      fix IAR compiler error.
 * 2012-12-22     Bernard      fix rt_kprintf issue, which found by Grissiom.
 * 2013-06-24     Bernard      remove rt_kprintf if RT_USING_CONSOLE is not defined.
 * 2013-09-24     aozima       make sure the device is in STREAM mode when used by rt_kprintf.
 * 2015-07-06     Bernard      Add rt_assert_handler routine.
 */

#include <rtthread.h>
#include <rthw.h>

/* use precision */
#define RT_PRINTF_PRECISION

/**
 * @addtogroup Error
 */

/**@{*/

/* global errno in RT-Thread */
static volatile int __rt_errno;

#if defined(RT_USING_DEVICE) && defined(RT_USING_CONSOLE)
static rt_device_t _console_device = RT_NULL;
#endif

/**
 * @brief 获取错误号
 * 该函数将获取错误号
 *
 * @return 错误号
 */
rt_err_t rt_get_errno(void)
{
    rt_thread_t tid;

    if (rt_interrupt_get_nest() != 0)
    {
        /* it's in interrupt context */
        return __rt_errno;
    }

    tid = rt_thread_self();
    if (tid == RT_NULL)
        return __rt_errno;

    return tid->error;
}
RTM_EXPORT(rt_get_errno);

/**
 * @brief 设置错误号
 * 该函数将设置错误号
 *
 * @param error 将被设定的错误号
 */
void rt_set_errno(rt_err_t error)
{
    rt_thread_t tid;

    if (rt_interrupt_get_nest() != 0)
    {
        /* it's in interrupt context */
        __rt_errno = error;

        return;
    }

    tid = rt_thread_self();
    if (tid == RT_NULL)
    {
        __rt_errno = error;

        return;
    }

    tid->error = error;
}
RTM_EXPORT(rt_set_errno);

/*
 * 该函数返回错误号
 *
 * @return 返回系统的错误
 */
int *_rt_errno(void)
{
    rt_thread_t tid;

    if (rt_interrupt_get_nest() != 0)
        return (int *)&__rt_errno;

    tid = rt_thread_self();
    if (tid != RT_NULL)
        return (int *) & (tid->error);

    return (int *)&__rt_errno;
}
RTM_EXPORT(_rt_errno);

/**@}*/

/**
 * @addtogroup mem
 */

/**@{*/


/**
 * @brief 设置内存数据
 * 此函数将内存的内容设置为指定的值
 *
 * @param s 源内存的地址
 * @param c 将要设定的值
 * @param count 拷贝的数据长度
 *
 * @return 源内存的地址
 */
void *rt_memset(void *s, int c, rt_ubase_t count)
{
#ifdef RT_USING_TINY_SIZE
    char *xs = (char *)s;

    while (count--)
        *xs++ = c;

    return s;
#else
#define LBLOCKSIZE      (sizeof(rt_int32_t))
#define UNALIGNED(X)    ((rt_int32_t)X & (LBLOCKSIZE - 1))
#define TOO_SMALL(LEN)  ((LEN) < LBLOCKSIZE)

    int i;
    char *m = (char *)s;
    rt_uint32_t buffer;
    rt_uint32_t *aligned_addr;
    rt_uint32_t d = c & 0xff;

    if (!TOO_SMALL(count) && !UNALIGNED(s))
    {
        /* If we get this far, we know that n is large and m is word-aligned. */
        aligned_addr = (rt_uint32_t *)s;

        /* Store D into each char sized location in BUFFER so that
         * we can set large blocks quickly.
         */
        if (LBLOCKSIZE == 4)
        {
            buffer = (d << 8) | d;
            buffer |= (buffer << 16);
        }
        else
        {
            buffer = 0;
            for (i = 0; i < LBLOCKSIZE; i ++)
                buffer = (buffer << 8) | d;
        }

        while (count >= LBLOCKSIZE * 4)
        {
            *aligned_addr++ = buffer;
            *aligned_addr++ = buffer;
            *aligned_addr++ = buffer;
            *aligned_addr++ = buffer;
            count -= 4 * LBLOCKSIZE;
        }

        while (count >= LBLOCKSIZE)
        {
            *aligned_addr++ = buffer;
            count -= LBLOCKSIZE;
        }

        /* Pick up the remainder with a bytewise loop. */
        m = (char *)aligned_addr;
    }

    while (count--)
    {
        *m++ = (char)d;
    }

    return s;

#undef LBLOCKSIZE
#undef UNALIGNED
#undef TOO_SMALL
#endif
}
RTM_EXPORT(rt_memset);

/**
 * @brief 复制内存数据
 * 此功能将内存内容从源地址复制到目标地址。
 *
 * @param dst 目标内存的地址
 * @param src  源内存的地址
 * @param count 复制的数据长度
 *
 * @return 目标内存的地址
 */
void *rt_memcpy(void *dst, const void *src, rt_ubase_t count)
{
#ifdef RT_USING_TINY_SIZE
    char *tmp = (char *)dst, *s = (char *)src;
    rt_ubase_t len;

    if (tmp <= s || tmp > (s + count))
    {
        while (count--)
            *tmp ++ = *s ++;
    }
    else
    {
        for (len = count; len > 0; len --)
            tmp[len - 1] = s[len - 1];
    }

    return dst;
#else

#define UNALIGNED(X, Y)                                               \
                        (((rt_int32_t)X & (sizeof(rt_int32_t) - 1)) | \
                         ((rt_int32_t)Y & (sizeof(rt_int32_t) - 1)))
#define BIGBLOCKSIZE    (sizeof(rt_int32_t) << 2)
#define LITTLEBLOCKSIZE (sizeof(rt_int32_t))
#define TOO_SMALL(LEN)  ((LEN) < BIGBLOCKSIZE)

    char *dst_ptr = (char *)dst;
    char *src_ptr = (char *)src;
    rt_int32_t *aligned_dst;
    rt_int32_t *aligned_src;
    int len = count;

    /* If the size is small, or either SRC or DST is unaligned,
    then punt into the byte copy loop.  This should be rare. */
    if (!TOO_SMALL(len) && !UNALIGNED(src_ptr, dst_ptr))
    {
        aligned_dst = (rt_int32_t *)dst_ptr;
        aligned_src = (rt_int32_t *)src_ptr;

        /* Copy 4X long words at a time if possible. */
        while (len >= BIGBLOCKSIZE)
        {
            *aligned_dst++ = *aligned_src++;
            *aligned_dst++ = *aligned_src++;
            *aligned_dst++ = *aligned_src++;
            *aligned_dst++ = *aligned_src++;
            len -= BIGBLOCKSIZE;
        }

        /* Copy one long word at a time if possible. */
        while (len >= LITTLEBLOCKSIZE)
        {
            *aligned_dst++ = *aligned_src++;
            len -= LITTLEBLOCKSIZE;
        }

        /* Pick up any residual with a byte copier. */
        dst_ptr = (char *)aligned_dst;
        src_ptr = (char *)aligned_src;
    }

    while (len--)
        *dst_ptr++ = *src_ptr++;

    return dst;
#undef UNALIGNED
#undef BIGBLOCKSIZE
#undef LITTLEBLOCKSIZE
#undef TOO_SMALL
#endif
}
RTM_EXPORT(rt_memcpy);

/**
 * @brief 移动内存数据
 * 该函数将源内存中的内容移动到目标内存地址中
 * address.
 *
 * @param dest 目标内存的地址
 * @param src  源内存的地址
 * @param n 拷贝数据的长度
 *
 * @return 目标内存的地址
 */
void *rt_memmove(void *dest, const void *src, rt_ubase_t n)
{
    char *tmp = (char *)dest, *s = (char *)src;

    if (s < tmp && tmp < s + n)
    {
        tmp += n;
        s += n;

        while (n--)
            *(--tmp) = *(--s);
    }
    else
    {
        while (n--)
            *tmp++ = *s++;
    }

    return dest;
}
RTM_EXPORT(rt_memmove);

/**
 * @brief 比较内存数据
 * 该函数将比较两个区域内存里的内容是否一致。
 *
 * @param cs 一个内存区域
 * @param ct 另一块内存区域
 * @param count 内存区域的大小
 *
 * @return the result
 */
rt_int32_t rt_memcmp(const void *cs, const void *ct, rt_ubase_t count)
{
    const unsigned char *su1, *su2;
    int res = 0;

    for (su1 = cs, su2 = ct; 0 < count; ++su1, ++su2, count--)
        if ((res = *su1 - *su2) != 0)
            break;

    return res;
}
RTM_EXPORT(rt_memcmp);


/**@}*/

/**
 * @addtogroup str
 */

/**@{*/

/**
 * @brief 判断字符串
 * 该函数判断s1字符串中是否存在s2字符串。
 *
 * @param s1 源字符串
 * @param s2 寻找的字符串
 *
 * @return 返回在s1里出现的第一个字符串s2的地址，未找到则返回RT_NULL。
 */
char *rt_strstr(const char *s1, const char *s2)
{
    int l1, l2;

    l2 = rt_strlen(s2);
    if (!l2)
        return (char *)s1;
    l1 = rt_strlen(s1);
    while (l1 >= l2)
    {
        l1 --;
        if (!rt_memcmp(s1, s2, l2))
            return (char *)s1;
        s1 ++;
    }

    return RT_NULL;
}
RTM_EXPORT(rt_strstr);

/**
 * @brief 忽略大小写比较字符串
 * 此函数将比较两个字符串，同时忽略大小写的差异。
 *
 * @param a 被比较的字符串
 * @param b 被比较的字符串
 *
 * @return 0 相等；大于0 a大于b;小于0 a小于b
 */
rt_uint32_t rt_strcasecmp(const char *a, const char *b)
{
    int ca, cb;

    do
    {
        ca = *a++ & 0xff;
        cb = *b++ & 0xff;
        if (ca >= 'A' && ca <= 'Z')
            ca += 'a' - 'A';
        if (cb >= 'A' && cb <= 'Z')
            cb += 'a' - 'A';
    }
    while (ca == cb && ca != '\0');

    return ca - cb;
}
RTM_EXPORT(rt_strcasecmp);

/**
 * @brief 复制字符串
 * 该函数将复制不超过那个字节的字符串。
 *
 * @param dst 复制的字符串地址
 * @param src 源的字符串地址
 * @param n 最大拷贝的字符串长度
 *
 * @return 复制后字符串地址
 */
char *rt_strncpy(char *dst, const char *src, rt_ubase_t n)
{
    if (n != 0)
    {
        char *d = dst;
        const char *s = src;

        do
        {
            if ((*d++ = *s++) == 0)
            {
                /* NUL pad the remaining n-1 bytes */
                while (--n != 0)
                    *d++ = 0;
                break;
            }
        } while (--n != 0);
    }

    return (dst);
}
RTM_EXPORT(rt_strncpy);

/**
 * @brief 比较指定长度字符串
 * 此函数将比较具有指定最大长度的两个字符串
 *
 * @param cs 被比较的源字符串
 * @param ct 被比较的目标字符串
 * @param count 字符串被比较的最大长度
 *
 * @return 0 相等；大于0 a大于b;小于0 a小于b
 */
rt_int32_t rt_strncmp(const char *cs, const char *ct, rt_ubase_t count)
{
    register signed char __res = 0;

    while (count)
    {
        if ((__res = *cs - *ct++) != 0 || !*cs++)
            break;
        count --;
    }

    return __res;
}
RTM_EXPORT(rt_strncmp);

/**
 * @brief 比较字符串
 * 此函数将比较两个没有指定长度的字符串
 *
 * @param cs 被比较的字符串
 * @param ct 被比较的字符串
 *
 * @return the result
 */
rt_int32_t rt_strcmp(const char *cs, const char *ct)
{
    while (*cs && *cs == *ct)
        cs++, ct++;

    return (*cs - *ct);
}
RTM_EXPORT(rt_strcmp);
/**
 * @brief 计算字符串长度
 * strnlen()函数返回s指向的字符串中的字符长度，不包括终止空字节（'\ 0'），
 * 但最多只能是maxlen。 在这样做时，strnlen()只查看s指向的字符串中的第一个
 * maxlen字符，并且永远不会超出s + maxlen。
 *
 * @param s 字符串
 * @param maxlen 字符串的最大长度
 * @return 字符串的长度
 */
rt_size_t rt_strnlen(const char *s, rt_ubase_t maxlen)
{
    const char *sc;

    for (sc = s; *sc != '\0' && sc - s < maxlen; ++sc) /* nothing */
        ;

    return sc - s;
}
/**
 * @brief 计算字符串长度
 * 此函数将返回字符串的长度，其终止符为null。
 *
 * @param s 指向字符串的指针
 *
 * @return 字符串的长度
 */
rt_size_t rt_strlen(const char *s)
{
    const char *sc;

    for (sc = s; *sc != '\0'; ++sc) /* nothing */
        ;

    return sc - s;
}
RTM_EXPORT(rt_strlen);

#ifdef RT_USING_HEAP
/**
 * @brief 复制字符串
 * 此函数将复制一个字符串。
 *
 * @param s 将要被复制的字符串
 *
 * @return 返回复制后的字符串地址
 */
char *rt_strdup(const char *s)
{
    rt_size_t len = rt_strlen(s) + 1;
    char *tmp = (char *)rt_malloc(len);

    if (!tmp)
        return RT_NULL;

    rt_memcpy(tmp, s, len);

    return tmp;
}
RTM_EXPORT(rt_strdup);
#endif

/**
 * 该函数将打印 rt-thread rtos 的版本。
 */
void rt_show_version(void)
{
    rt_kprintf("\n \\ | /\n");
    rt_kprintf("- RT -     Thread Operating System\n");
    rt_kprintf(" / | \\     %d.%d.%d build %s\n",
               RT_VERSION, RT_SUBVERSION, RT_REVISION, __DATE__);
    rt_kprintf(" 2006 - 2018 Copyright by rt-thread team\n");
}
RTM_EXPORT(rt_show_version);

/* private function */
#define isdigit(c)  ((unsigned)((c) - '0') < 10)

rt_inline rt_int32_t divide(rt_int32_t *n, rt_int32_t base)
{
    rt_int32_t res;

    /* optimized for processor which does not support divide instructions. */
    if (base == 10)
    {
        res = ((rt_uint32_t) * n) % 10U;
        *n = ((rt_uint32_t) * n) / 10U;
    }
    else
    {
        res = ((rt_uint32_t) * n) % 16U;
        *n = ((rt_uint32_t) * n) / 16U;
    }

    return res;
}

rt_inline int skip_atoi(const char **s)
{
    register int i = 0;
    while (isdigit(**s))
        i = i * 10 + *((*s)++) - '0';

    return i;
}

#define ZEROPAD     (1 << 0)    /* pad with zero */
#define SIGN        (1 << 1)    /* unsigned/signed long */
#define PLUS        (1 << 2)    /* show plus */
#define SPACE       (1 << 3)    /* space if plus */
#define LEFT        (1 << 4)    /* left justified */
#define SPECIAL     (1 << 5)    /* 0x */
#define LARGE       (1 << 6)    /* use 'ABCDEF' instead of 'abcdef' */

#ifdef RT_PRINTF_PRECISION
static char *print_number(char *buf,
                          char *end,
                          long  num,
                          int   base,
                          int   s,
                          int   precision,
                          int   type)
#else
static char *print_number(char *buf,
                          char *end,
                          long  num,
                          int   base,
                          int   s,
                          int   type)
#endif
{
    char c, sign;
#ifdef RT_PRINTF_LONGLONG
    char tmp[32];
#else
    char tmp[16];
#endif
    const char *digits;
    static const char small_digits[] = "0123456789abcdef";
    static const char large_digits[] = "0123456789ABCDEF";
    register int i;
    register int size;

    size = s;

    digits = (type & LARGE) ? large_digits : small_digits;
    if (type & LEFT)
        type &= ~ZEROPAD;

    c = (type & ZEROPAD) ? '0' : ' ';

    /* get sign */
    sign = 0;
    if (type & SIGN)
    {
        if (num < 0)
        {
            sign = '-';
            num = -num;
        }
        else if (type & PLUS)
            sign = '+';
        else if (type & SPACE)
            sign = ' ';
    }

#ifdef RT_PRINTF_SPECIAL
    if (type & SPECIAL)
    {
        if (base == 16)
            size -= 2;
        else if (base == 8)
            size--;
    }
#endif

    i = 0;
    if (num == 0)
        tmp[i++] = '0';
    else
    {
        while (num != 0)
            tmp[i++] = digits[divide(&num, base)];
    }

#ifdef RT_PRINTF_PRECISION
    if (i > precision)
        precision = i;
    size -= precision;
#else
    size -= i;
#endif

    if (!(type & (ZEROPAD | LEFT)))
    {
        if ((sign) && (size > 0))
            size--;

        while (size-- > 0)
        {
            if (buf <= end)
                *buf = ' ';
            ++ buf;
        }
    }

    if (sign)
    {
        if (buf <= end)
        {
            *buf = sign;
            -- size;
        }
        ++ buf;
    }

#ifdef RT_PRINTF_SPECIAL
    if (type & SPECIAL)
    {
        if (base == 8)
        {
            if (buf <= end)
                *buf = '0';
            ++ buf;
        }
        else if (base == 16)
        {
            if (buf <= end)
                *buf = '0';
            ++ buf;
            if (buf <= end)
            {
                *buf = type & LARGE ? 'X' : 'x';
            }
            ++ buf;
        }
    }
#endif

    /* no align to the left */
    if (!(type & LEFT))
    {
        while (size-- > 0)
        {
            if (buf <= end)
                *buf = c;
            ++ buf;
        }
    }

#ifdef RT_PRINTF_PRECISION
    while (i < precision--)
    {
        if (buf <= end)
            *buf = '0';
        ++ buf;
    }
#endif

    /* put number in the temporary buffer */
    while (i-- > 0)
    {
        if (buf <= end)
            *buf = tmp[i];
        ++ buf;
    }

    while (size-- > 0)
    {
        if (buf <= end)
            *buf = ' ';
        ++ buf;
    }

    return buf;
}

rt_int32_t rt_vsnprintf(char       *buf,
                        rt_size_t   size,
                        const char *fmt,
                        va_list     args)
{
#ifdef RT_PRINTF_LONGLONG
    unsigned long long num;
#else
    rt_uint32_t num;
#endif
    int i, len;
    char *str, *end, c;
    const char *s;

    rt_uint8_t base;            /* the base of number */
    rt_uint8_t flags;           /* flags to print number */
    rt_uint8_t qualifier;       /* 'h', 'l', or 'L' for integer fields */
    rt_int32_t field_width;     /* width of output field */

#ifdef RT_PRINTF_PRECISION
    int precision;      /* min. # of digits for integers and max for a string */
#endif

    str = buf;
    end = buf + size - 1;

    /* Make sure end is always >= buf */
    if (end < buf)
    {
        end  = ((char *) - 1);
        size = end - buf;
    }

    for (; *fmt ; ++fmt)
    {
        if (*fmt != '%')
        {
            if (str <= end)
                *str = *fmt;
            ++ str;
            continue;
        }

        /* process flags */
        flags = 0;

        while (1)
        {
            /* skips the first '%' also */
            ++ fmt;
            if (*fmt == '-') flags |= LEFT;
            else if (*fmt == '+') flags |= PLUS;
            else if (*fmt == ' ') flags |= SPACE;
            else if (*fmt == '#') flags |= SPECIAL;
            else if (*fmt == '0') flags |= ZEROPAD;
            else break;
        }

        /* get field width */
        field_width = -1;
        if (isdigit(*fmt)) field_width = skip_atoi(&fmt);
        else if (*fmt == '*')
        {
            ++ fmt;
            /* it's the next argument */
            field_width = va_arg(args, int);
            if (field_width < 0)
            {
                field_width = -field_width;
                flags |= LEFT;
            }
        }

#ifdef RT_PRINTF_PRECISION
        /* get the precision */
        precision = -1;
        if (*fmt == '.')
        {
            ++ fmt;
            if (isdigit(*fmt)) precision = skip_atoi(&fmt);
            else if (*fmt == '*')
            {
                ++ fmt;
                /* it's the next argument */
                precision = va_arg(args, int);
            }
            if (precision < 0) precision = 0;
        }
#endif
        /* get the conversion qualifier */
        qualifier = 0;
#ifdef RT_PRINTF_LONGLONG
        if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L')
#else
        if (*fmt == 'h' || *fmt == 'l')
#endif
        {
            qualifier = *fmt;
            ++ fmt;
#ifdef RT_PRINTF_LONGLONG
            if (qualifier == 'l' && *fmt == 'l')
            {
                qualifier = 'L';
                ++ fmt;
            }
#endif
        }

        /* the default base */
        base = 10;

        switch (*fmt)
        {
        case 'c':
            if (!(flags & LEFT))
            {
                while (--field_width > 0)
                {
                    if (str <= end) *str = ' ';
                    ++ str;
                }
            }

            /* get character */
            c = (rt_uint8_t)va_arg(args, int);
            if (str <= end) *str = c;
            ++ str;

            /* put width */
            while (--field_width > 0)
            {
                if (str <= end) *str = ' ';
                ++ str;
            }
            continue;

        case 's':
            s = va_arg(args, char *);
            if (!s) s = "(NULL)";

            len = rt_strlen(s);
#ifdef RT_PRINTF_PRECISION
            if (precision > 0 && len > precision) len = precision;
#endif

            if (!(flags & LEFT))
            {
                while (len < field_width--)
                {
                    if (str <= end) *str = ' ';
                    ++ str;
                }
            }

            for (i = 0; i < len; ++i)
            {
                if (str <= end) *str = *s;
                ++ str;
                ++ s;
            }

            while (len < field_width--)
            {
                if (str <= end) *str = ' ';
                ++ str;
            }
            continue;

        case 'p':
            if (field_width == -1)
            {
                field_width = sizeof(void *) << 1;
                flags |= ZEROPAD;
            }
#ifdef RT_PRINTF_PRECISION
            str = print_number(str, end,
                               (long)va_arg(args, void *),
                               16, field_width, precision, flags);
#else
            str = print_number(str, end,
                               (long)va_arg(args, void *),
                               16, field_width, flags);
#endif
            continue;

        case '%':
            if (str <= end) *str = '%';
            ++ str;
            continue;

        /* integer number formats - set up the flags and "break" */
        case 'o':
            base = 8;
            break;

        case 'X':
            flags |= LARGE;
        case 'x':
            base = 16;
            break;

        case 'd':
        case 'i':
            flags |= SIGN;
        case 'u':
            break;

        default:
            if (str <= end) *str = '%';
            ++ str;

            if (*fmt)
            {
                if (str <= end) *str = *fmt;
                ++ str;
            }
            else
            {
                -- fmt;
            }
            continue;
        }

#ifdef RT_PRINTF_LONGLONG
        if (qualifier == 'L') num = va_arg(args, long long);
        else if (qualifier == 'l')
#else
        if (qualifier == 'l')
#endif
        {
            num = va_arg(args, rt_uint32_t);
            if (flags & SIGN) num = (rt_int32_t)num;
        }
        else if (qualifier == 'h')
        {
            num = (rt_uint16_t)va_arg(args, rt_int32_t);
            if (flags & SIGN) num = (rt_int16_t)num;
        }
        else
        {
            num = va_arg(args, rt_uint32_t);
            if (flags & SIGN) num = (rt_int32_t)num;
        }
#ifdef RT_PRINTF_PRECISION
        str = print_number(str, end, num, base, field_width, precision, flags);
#else
        str = print_number(str, end, num, base, field_width, flags);
#endif
    }

    if (str <= end) *str = '\0';
    else *end = '\0';

    /* the trailing null byte doesn't count towards the total
    * ++str;
    */
    return str - buf;
}
RTM_EXPORT(rt_vsnprintf);

/**
 * @brief 格式化字符串
 * 该函数将用格式化后的字符串填充缓冲区。
 *
 * @param buf 保存格式化后字符串的缓冲区
 * @param size 缓冲区的大小
 * @param fmt 格式化的格式
 */
rt_int32_t rt_snprintf(char *buf, rt_size_t size, const char *fmt, ...)
{
    rt_int32_t n;
    va_list args;

    va_start(args, fmt);
    n = rt_vsnprintf(buf, size, fmt, args);
    va_end(args);

    return n;
}
RTM_EXPORT(rt_snprintf);

/**
 * @brief 格式化字符串
 * 此函数将以格式化后的字符串填充缓冲区
 *
 * @param buf 保存格式化字符串的缓冲区
 * @param arg_ptr 参数指针
 * @param format 格式化字符串的格式
 */
rt_int32_t rt_vsprintf(char *buf, const char *format, va_list arg_ptr)
{
    return rt_vsnprintf(buf, (rt_size_t) - 1, format, arg_ptr);
}
RTM_EXPORT(rt_vsprintf);

/**
 * @brief 格式化字符串
 * 此函数将以格式化后的字符串填充缓冲区
 *
 * @param buf 保存格式化字符串的缓冲区
 * @param format 格式化字符串的格式
 */
rt_int32_t rt_sprintf(char *buf, const char *format, ...)
{
    rt_int32_t n;
    va_list arg_ptr;

    va_start(arg_ptr, format);
    n = rt_vsprintf(buf, format, arg_ptr);
    va_end(arg_ptr);

    return n;
}
RTM_EXPORT(rt_sprintf);

#ifdef RT_USING_CONSOLE

#ifdef RT_USING_DEVICE
/**
 * @brief 获取控制台设备句柄
 * 该函数将返回控制台的设备对象句柄
 *
 * @return 控制台的设备对象句柄
 */
rt_device_t rt_console_get_device(void)
{
    return _console_device;
}
RTM_EXPORT(rt_console_get_device);

/**
 *@brief 设置控制台设备句柄
 * 此功能将设置控制台设备。 将设备设置为控制台后，
 * rt_kprintf的所有输出都将重定向到此新设备。
 *
 * @param name 新控制台设备的名称
 *
 * @return 旧控制台设备句柄
 */
rt_device_t rt_console_set_device(const char *name)
{
    rt_device_t new, old;

    /* save old device */
    old = _console_device;

    /* find new console device */
    new = rt_device_find(name);
    if (new != RT_NULL)
    {
        if (_console_device != RT_NULL)
        {
            /* close old console device */
            rt_device_close(_console_device);
        }

        /* set new console device */
        rt_device_open(new, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_STREAM);
        _console_device = new;
    }

    return old;
}
RTM_EXPORT(rt_console_set_device);
#endif

RT_WEAK void rt_hw_console_output(const char *str)
{
    /* empty console output */
}
RTM_EXPORT(rt_hw_console_output);

/**
 * @brief 打印字符串
 * 该函数将打印字符串到控制台。
 *
 * @param str 字符串。
 */
void rt_kputs(const char *str)
{
    if (!str) return;

#ifdef RT_USING_DEVICE
    if (_console_device == RT_NULL)
    {
        rt_hw_console_output(str);
    }
    else
    {
        rt_uint16_t old_flag = _console_device->open_flag;

        _console_device->open_flag |= RT_DEVICE_FLAG_STREAM;
        rt_device_write(_console_device, 0, str, rt_strlen(str));
        _console_device->open_flag = old_flag;
    }
#else
    rt_hw_console_output(str);
#endif
}

/**
 * @brief 打印格式化字符串
 * 该函数将打印格式化的字符串到系统控制台。
 *
 * @param fmt 格式化字符串的格式
 */
void rt_kprintf(const char *fmt, ...)
{
    va_list args;
    rt_size_t length;
    static char rt_log_buf[RT_CONSOLEBUF_SIZE];

    va_start(args, fmt);
    /* the return value of vsnprintf is the number of bytes that would be
     * written to buffer had if the size of the buffer been sufficiently
     * large excluding the terminating null byte. If the output string
     * would be larger than the rt_log_buf, we have to adjust the output
     * length. */
    length = rt_vsnprintf(rt_log_buf, sizeof(rt_log_buf) - 1, fmt, args);
    if (length > RT_CONSOLEBUF_SIZE - 1)
        length = RT_CONSOLEBUF_SIZE - 1;
#ifdef RT_USING_DEVICE
    if (_console_device == RT_NULL)
    {
        rt_hw_console_output(rt_log_buf);
    }
    else
    {
        rt_uint16_t old_flag = _console_device->open_flag;

        _console_device->open_flag |= RT_DEVICE_FLAG_STREAM;
        rt_device_write(_console_device, 0, rt_log_buf, length);
        _console_device->open_flag = old_flag;
    }
#else
    rt_hw_console_output(rt_log_buf);
#endif
    va_end(args);
}
RTM_EXPORT(rt_kprintf);
#endif

/**@}*/


/**
 * @addtogroup mem
 */

/**@{*/


#ifdef RT_USING_HEAP
/**
 * @brief 对齐分配内存
 * 该函数将分配一各内存块，该地址与指定的对其大小对齐。
 *
 * @param size 分配的内存块大小
 * @param align 内存对齐大小
 *
 * @return 成功则返回分配的内存块地址，失败则返回RT_NULL。
 */
void *rt_malloc_align(rt_size_t size, rt_size_t align)
{
    void *align_ptr;
    void *ptr;
    rt_size_t align_size;

    /* align the alignment size to 4 byte */
    align = ((align + 0x03) & ~0x03);

    /* get total aligned size */
    align_size = ((size + 0x03) & ~0x03) + align;
    /* allocate memory block from heap */
    ptr = rt_malloc(align_size);
    if (ptr != RT_NULL)
    {
        /* the allocated memory block is aligned */
        if (((rt_uint32_t)ptr & (align - 1)) == 0)
        {
            align_ptr = (void *)((rt_uint32_t)ptr + align);
        }
        else
        {
            align_ptr = (void *)(((rt_uint32_t)ptr + (align - 1)) & ~(align - 1));
        }

        /* set the pointer before alignment pointer to the real pointer */
        *((rt_uint32_t *)((rt_uint32_t)align_ptr - sizeof(void *))) = (rt_uint32_t)ptr;

        ptr = align_ptr;
    }

    return ptr;
}
RTM_EXPORT(rt_malloc_align);

/**
 * @brief 释放对齐分配的内存
 * 该函数将释放由rt_malloc_align函数分配的并且地址对齐的内存块。
 *
 * @param ptr 将要释放的内存块指针
 */
void rt_free_align(void *ptr)
{
    void *real_ptr;

    real_ptr = (void *) * (rt_uint32_t *)((rt_uint32_t)ptr - sizeof(void *));
    rt_free(real_ptr);
}
RTM_EXPORT(rt_free_align);
#endif

/**@}*/

#ifndef RT_USING_CPU_FFS
const rt_uint8_t __lowest_bit_bitmap[] =
{
    /* 00 */ 0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* 10 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* 20 */ 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* 30 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* 40 */ 6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* 50 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* 60 */ 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* 70 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* 80 */ 7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* 90 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* A0 */ 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* B0 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* C0 */ 6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* D0 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* E0 */ 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* F0 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
};

/**
 * 此函数在值中查找第一个位集（从最低有效位开始）并返回该位的索引。
 *
 * 位从第位1位开始编号（最低有效位）。 任何这些函数的返回值为零意味着参数为零。
 *
 * @return 返回第一个位集的索引。 如果value为0，则此函数应返回0。
 */
int __rt_ffs(int value)
{
    if (value == 0) return 0;

    if (value & 0xff)
        return __lowest_bit_bitmap[value & 0xff] + 1;

    if (value & 0xff00)
        return __lowest_bit_bitmap[(value & 0xff00) >> 8] + 9;

    if (value & 0xff0000)
        return __lowest_bit_bitmap[(value & 0xff0000) >> 16] + 17;

    return __lowest_bit_bitmap[(value & 0xff000000) >> 24] + 25;
}
#endif

#ifdef RT_DEBUG
/* RT_ASSERT(EX)'s hook */
void (*rt_assert_hook)(const char *ex, const char *func, rt_size_t line);
/**
 * 该函数将设置一个钩子函数到RT_ASSERT(EX)，当表达式为假时运行。
 *
 * @param hook the hook function
 */
void rt_assert_set_hook(void (*hook)(const char *ex, const char *func, rt_size_t line))
{
    rt_assert_hook = hook;
}

/**
 * 断言函数
 *
 * @param ex 断言函数的条件字符串
 * @param func 断言发生时的函数名称。
 * @param line 断言发生时的文件行数。
 */
void rt_assert_handler(const char *ex_string, const char *func, rt_size_t line)
{
    volatile char dummy = 0;

    if (rt_assert_hook == RT_NULL)
    {
#ifdef RT_USING_MODULE
        if (rt_module_self() != RT_NULL)
        {
            /* unload assertion module */
            rt_module_unload(rt_module_self());

            /* re-schedule */
            rt_schedule();
        }
        else
#endif
        {
            rt_kprintf("(%s) assertion failed at function:%s, line number:%d \n", ex_string, func, line);
            while (dummy == 0);
        }
    }
    else
    {
        rt_assert_hook(ex_string, func, line);
    }
}
RTM_EXPORT(rt_assert_handler);
#endif /* RT_DEBUG */

#if !defined (RT_USING_NEWLIB) && defined (RT_USING_MINILIBC) && defined (__GNUC__)
#include <sys/types.h>
void *memcpy(void *dest, const void *src, size_t n) __attribute__((weak, alias("rt_memcpy")));
void *memset(void *s, int c, size_t n) __attribute__((weak, alias("rt_memset")));
void *memmove(void *dest, const void *src, size_t n) __attribute__((weak, alias("rt_memmove")));
int   memcmp(const void *s1, const void *s2, size_t n) __attribute__((weak, alias("rt_memcmp")));

size_t strlen(const char *s) __attribute__((weak, alias("rt_strlen")));
char *strstr(const char *s1, const char *s2) __attribute__((weak, alias("rt_strstr")));
int strcasecmp(const char *a, const char *b) __attribute__((weak, alias("rt_strcasecmp")));
char *strncpy(char *dest, const char *src, size_t n) __attribute__((weak, alias("rt_strncpy")));
int strncmp(const char *cs, const char *ct, size_t count) __attribute__((weak, alias("rt_strncmp")));
#ifdef RT_USING_HEAP
char *strdup(const char *s) __attribute__((weak, alias("rt_strdup")));
#endif

int sprintf(char *buf, const char *format, ...) __attribute__((weak, alias("rt_sprintf")));
int snprintf(char *buf, rt_size_t size, const char *fmt, ...) __attribute__((weak, alias("rt_snprintf")));
int vsprintf(char *buf, const char *format, va_list arg_ptr) __attribute__((weak, alias("rt_vsprintf")));

#endif

/**@}*/
