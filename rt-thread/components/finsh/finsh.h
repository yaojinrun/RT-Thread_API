/*
 * File      : finsh.h
 * COPYRIGHT (C) 2006 - 2018, RT-Thread Development Team
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
 * 2010-03-22     Bernard      first version
 */
#ifndef __FINSH_H__
#define __FINSH_H__

#include <rtthread.h>
#include "finsh_api.h"

/* -- the beginning of option -- */
#define FINSH_NAME_MAX          16      /* max length of identifier */
#define FINSH_NODE_MAX          16      /* max number of node */

#define FINSH_HEAP_MAX          128     /* max length of heap */
#define FINSH_STRING_MAX        128     /* max length of string */
#define FINSH_VARIABLE_MAX      8       /* max number of variable */

#define FINSH_STACK_MAX         64      /* max stack size */
#define FINSH_TEXT_MAX          128     /* max text segment size */

#define HEAP_ALIGNMENT          4       /* heap alignment */

#define FINSH_GET16(x)    (*(x)) | (*((x)+1) << 8)
#define FINSH_GET32(x)    (rt_uint32_t)(*(x)) | ((rt_uint32_t)*((x)+1) << 8) | \
    ((rt_uint32_t)*((x)+2) << 16) | ((rt_uint32_t)*((x)+3) << 24)

#define FINSH_SET16(x, v)           \
    do                              \
    {                               \
        *(x)     = (v) & 0x00ff;    \
        (*((x)+1)) = (v) >> 8;      \
    } while ( 0 )

#define FINSH_SET32(x, v)                                       \
    do                                                          \
    {                                                           \
        *(x)     = (rt_uint32_t)(v)  & 0x000000ff;              \
        (*((x)+1)) = ((rt_uint32_t)(v) >> 8) & 0x000000ff;      \
        (*((x)+2)) = ((rt_uint32_t)(v) >> 16) & 0x000000ff;     \
        (*((x)+3)) = ((rt_uint32_t)(v) >> 24);                  \
    } while ( 0 )

/* -- the end of option -- */

/* std header file */
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define FINSH_VERSION_MAJOR         1
#define FINSH_VERSION_MINOR         0

/*
 * @addtogroup finsh
 */
/*@{*/
#define FINSH_ERROR_OK              0   /**< @brief 无错误             */
#define FINSH_ERROR_INVALID_TOKEN   1   /**< @brief 无效令牌	        */
#define FINSH_ERROR_EXPECT_TYPE     2   /**< @brief 已知类型           */
#define FINSH_ERROR_UNKNOWN_TYPE    3   /**< @brief 未知类型           */
#define FINSH_ERROR_VARIABLE_EXIST  4   /**< @brief 变量存在           */
#define FINSH_ERROR_EXPECT_OPERATOR 5   /**< @brief Expect a operator  */
#define FINSH_ERROR_MEMORY_FULL     6   /**< @brief 内存已满           */
#define FINSH_ERROR_UNKNOWN_OP      7   /**< @brief Unknown operator   */
#define FINSH_ERROR_UNKNOWN_NODE    8   /**< @brief Unknown node       */
#define FINSH_ERROR_EXPECT_CHAR     9   /**< @brief Expect a character */
#define FINSH_ERROR_UNEXPECT_END    10  /**< @brief Unexpect end       */
#define FINSH_ERROR_UNKNOWN_TOKEN   11  /**< @brief Unknown token      */
#define FINSH_ERROR_NO_FLOAT        12  /**< @brief 不支持浮点运算     */
#define FINSH_ERROR_UNKNOWN_SYMBOL  13  /**< @brief Unknown symbol     */
#define FINSH_ERROR_NULL_NODE       14  /**< @brief 空节点             */
/*@}*/

/* system call item */
struct finsh_syscall_item
{
    struct finsh_syscall_item* next;    /* next item */
    struct finsh_syscall syscall;       /* syscall */
};
extern struct finsh_syscall_item *global_syscall_list;

/* system variable table */
struct finsh_sysvar
{
    const char*     name;       /* the name of variable */
#if defined(FINSH_USING_DESCRIPTION) && defined(FINSH_USING_SYMTAB)
    const char*     desc;       /* description of system variable */
#endif
    uint8_t      type;      /* the type of variable */
    void*        var ;      /* the address of variable */
};

#if defined(_MSC_VER) || (defined(__GNUC__) && defined(__x86_64__))
struct finsh_syscall* finsh_syscall_next(struct finsh_syscall* call);
struct finsh_sysvar* finsh_sysvar_next(struct finsh_sysvar* call);
#define FINSH_NEXT_SYSCALL(index)  index=finsh_syscall_next(index)
#define FINSH_NEXT_SYSVAR(index)   index=finsh_sysvar_next(index)
#else
#define FINSH_NEXT_SYSCALL(index)  index++
#define FINSH_NEXT_SYSVAR(index)   index++
#endif

/* system variable item */
struct finsh_sysvar_item
{
    struct finsh_sysvar_item *next;     /* next item */
    struct finsh_sysvar sysvar;         /* system variable */
};
extern struct finsh_sysvar *_sysvar_table_begin, *_sysvar_table_end;
extern struct finsh_sysvar_item* global_sysvar_list;

/* find out system variable, which should be implemented in user program */
struct finsh_sysvar* finsh_sysvar_lookup(const char* name);


struct finsh_token
{
    char eof;
    char replay;

    int  position;
    uint8_t current_token;

    union {
        char char_value;
        int int_value;
        long long_value;
    } value;
    uint8_t string[FINSH_STRING_MAX];

    uint8_t* line;
};

#define FINSH_IDTYPE_VAR        0x01
#define FINSH_IDTYPE_SYSVAR     0x02
#define FINSH_IDTYPE_SYSCALL    0x04
#define FINSH_IDTYPE_ADDRESS    0x08
struct finsh_node
{
    uint8_t node_type;  /* node node_type */
    uint8_t data_type;  /* node data node_type */
    uint8_t idtype;     /* id node information */

    union {         /* value node */
        char    char_value;
        short   short_value;
        int     int_value;
        long    long_value;
        void*   ptr;
    } value;
    union
    {
        /* point to variable identifier or function identifier */
        struct finsh_var    *var;
        struct finsh_sysvar *sysvar;
        struct finsh_syscall*syscall;
    }id;

    /* sibling and child node */
    struct finsh_node *sibling, *child;
};

struct finsh_parser
{
    uint8_t* parser_string;

    struct finsh_token token;
    struct finsh_node* root;
};

/**
 *
 *
 * finsh shell的基本数据类型
 */
enum finsh_type {
    finsh_type_unknown = 0, /**< unknown data type */
    finsh_type_void,        /**< void           */
    finsh_type_voidp,       /**< void pointer   */
    finsh_type_char,        /**< char           */
    finsh_type_uchar,       /**< unsigned char  */
    finsh_type_charp,       /**< char pointer   */
    finsh_type_short,       /**< short          */
    finsh_type_ushort,      /**< unsigned short */
    finsh_type_shortp,      /**< short pointer  */
    finsh_type_int,         /**< int            */
    finsh_type_uint,        /**< unsigned int   */
    finsh_type_intp,        /**< int pointer    */
    finsh_type_long,        /**< long           */
    finsh_type_ulong,       /**< unsigned long  */
    finsh_type_longp        /**< long pointer   */
};

/* init finsh environment */
int finsh_init(struct finsh_parser* parser);
/* flush finsh node, text segment */
int finsh_flush(struct finsh_parser* parser);
/* reset all of finsh */
int finsh_reset(struct finsh_parser* parser);
#ifdef RT_USING_DEVICE
void finsh_set_device(const char* device_name);
#endif

/* run finsh parser to generate abstract synatx tree */
void finsh_parser_run (struct finsh_parser* parser, const unsigned char* string);
/* run compiler to compile abstract syntax tree */
int finsh_compiler_run(struct finsh_node* node);
/* run finsh virtual machine */
void finsh_vm_run(void);

/* get variable value */
struct finsh_var* finsh_var_lookup(const char* name);
/* get bottom value of stack */
long finsh_stack_bottom(void);

/* get error number of finsh */
uint8_t finsh_errno(void);
/* get error string */
const char* finsh_error_string(uint8_t type);

#ifdef RT_USING_HEAP
/**
 * @ingroup finsh
 *
 * @brief 添加调用函数
 *
 * 该函数将添加一个系统调用到FinSH的运行环境中去。
 * @param name 系统调用的名称
 * @param func 系统调用的函数指针
 */
void finsh_syscall_append(const char* name, syscall_func func);

/**
 * @ingroup finsh
 *
 * @brief 添加环境变量
 *
 * 该函数将添加一个系统变量到FinSH的运行环境中去。
 * @param name 系统变量的名称
 * @param type 系统变量的数据类型
 * @param addr 系统变量的地址
 */
void finsh_sysvar_append(const char* name, uint8_t type, void* addr);
#endif
#endif
