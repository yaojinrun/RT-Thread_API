/*
 * File      : rtservice.h
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
 * 2006-09-07     Bernard      move the kservice APIs to rtthread.h
 * 2007-06-27     Bernard      fix the rt_list_remove bug
 * 2012-03-22     Bernard      rename kservice.h to rtservice.h
 * 2017-11-15     JasonJia     Modify rt_slist_foreach to rt_slist_for_each_entry.
 *                             Make code cleanup.
 */

#ifndef __RT_SERVICE_H__
#define __RT_SERVICE_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup list
 */

/*@{*/

/**
 * @brief 获取type结构体中member成员在这个结构体中的偏移。
 */
#define rt_container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))


/**
 * @brief 初始化链表对象
 */
#define RT_LIST_OBJECT_INIT(object) { &(object), &(object) }

/**
 * @brief 初始化链表
 *
 * @param l 将被初始化的链表
 */
rt_inline void rt_list_init(rt_list_t *l)
{
    l->next = l->prev = l;
}

/**
 * @brief 链表头插
 *
 * @param l 操作的链表
 * @param n 将要被插入的新节点
 */
rt_inline void rt_list_insert_after(rt_list_t *l, rt_list_t *n)
{
    l->next->prev = n;
    n->next = l->next;

    l->next = n;
    n->prev = l;
}

/**
 * @brief 链表尾插
 *
 * @param n 将要被插入的新节点
 * @param l 操作链表
 */
rt_inline void rt_list_insert_before(rt_list_t *l, rt_list_t *n)
{
    l->prev->next = n;
    n->prev = l->prev;

    l->prev = n;
    n->next = l;
}

/**
 * @brief 移除一个链表节点
 * @param n 将要从链表中删除的节点
 */
rt_inline void rt_list_remove(rt_list_t *n)
{
    n->next->prev = n->prev;
    n->prev->next = n->next;

    n->next = n->prev = n;
}

/**
 * @brief 判断链表是否为空
 * @param l 被测试的链表
 */
rt_inline int rt_list_isempty(const rt_list_t *l)
{
    return l->next == l;
}

/**
 * @brief 获取链表长度
 * @param l 被获取的链表
 */
rt_inline unsigned int rt_list_len(const rt_list_t *l)
{
    unsigned int len = 0;
    const rt_list_t *p = l;
    while (p->next != l)
    {
        p = p->next;
        len ++;
    }

    return len;
}

/**
 * @brief 获取结构体变量的地址
 * @param node 入口点
 * @param type 结构体的类型
 * @param member 结构体中链表的成员名
 */
#define rt_list_entry(node, type, member) \
    rt_container_of(node, type, member)

/**
 * @brief 循环遍历head链表中每一个pos中的member成员
 * @param pos:    指向宿主结构的指针，在for循环中是一个迭代变量
 * @param head:   链表头
 * @param member: 结构体中链表的成员名
 */
#define rt_list_for_each_entry(pos, head, member) \
    for (pos = rt_list_entry((head)->next, typeof(*pos), member); \
         &pos->member != (head); \
         pos = rt_list_entry(pos->member.next, typeof(*pos), member))

/**
 * rt_list_for_each_entry_safe - 相比于list_for_each_entry而言，
 * list_for_each_entry_safe用指针n对链表的下一个数据结构进行了临时存储，
 * 所以如果在遍历链表的时候可能要删除链表中的当前项，用list_for_each_entry_safe
 * 可以安全的删除，而不会影响接下来的遍历过程（用n 指针可以继续完成接下来的遍历， 
 * 而list_for_each_entry则无法继续遍历）。
 * @param pos:    指向宿主结构的指针，在for循环中是一个迭代变量
 * @param n:      用于临时存储下一个数据结构的指针变量
 * @param head:   链表头
 * @param member: 结构体中链表的成员名
 */
#define rt_list_for_each_entry_safe(pos, n, head, member) \
    for (pos = rt_list_entry((head)->next, typeof(*pos), member), \
         n = rt_list_entry(pos->member.next, typeof(*pos), member); \
         &pos->member != (head); \
         pos = n, n = rt_list_entry(n->member.next, typeof(*n), member))

/**
 * @brief 获取链表中的第一个元素
 * @param ptr:    链表头
 * @param type:   结构体类型
 * @param member: 结构体中链表的成员名
 *
 * 请注意，该链表不能为空。
 */
#define rt_list_first_entry(ptr, type, member) \
    rt_list_entry((ptr)->next, type, member)

/*@}*/



/**
 * @addtogroup slist
 */

/*@{*/

#define RT_SLIST_OBJECT_INIT(object) { RT_NULL }

/**
 * @brief 初始化一个单链表
 *
 * @param l 将要被初始化的单向链表
 */
rt_inline void rt_slist_init(rt_slist_t *l)
{
    l->next = RT_NULL;
}

/**
 * @brief 在单向链表l的尾部追加一个节点
 *
 * @param l 被操作的链表
 * @param n 将要被追加的节点
 */
rt_inline void rt_slist_append(rt_slist_t *l, rt_slist_t *n)
{
    struct rt_slist_node *node;

    node = l;
    while (node->next) node = node->next;

    /* append the node to the tail */
    node->next = n;
    n->next = RT_NULL;
}

/**
 * @brief 向单向链表节点l后插入一一个节点n
 *
 * @param l 被插入的链表节点
 * @param n 将要被插入的节点
 */
rt_inline void rt_slist_insert(rt_slist_t *l, rt_slist_t *n)
{
    n->next = l->next;
    l->next = n;
}

/**
 * @brief 获取单向链表的长度
 *
 * @param l 单向链表的链表头
 */
rt_inline unsigned int rt_slist_len(const rt_slist_t *l)
{
    unsigned int len = 0;
    const rt_slist_t *list = l->next;
    while (list != RT_NULL)
    {
        list = list->next;
        len ++;
    }

    return len;
}

/**
 * @brief 从单向链表l中移除入一个节点n
 *
 * @param l 单向链表的链表头
 * @param n 将要被移除的节点
 */
rt_inline rt_slist_t *rt_slist_remove(rt_slist_t *l, rt_slist_t *n)
{
    /* remove slist head */
    struct rt_slist_node *node = l;
    while (node->next && node->next != n) node = node->next;

    /* remove node */
    if (node->next != (rt_slist_t *)0) node->next = node->next->next;

    return l;
}

/**
 * @brief 获取单向链表的第一个节点
 *
 * @param l 单向链表的链表头
 */
rt_inline rt_slist_t *rt_slist_first(rt_slist_t *l)
{
    return l->next;
}

/**
 * @brief 获取单向链表节点n的下一个节点
 *
 * @param n 单向链表的节点
 */
rt_inline rt_slist_t *rt_slist_next(rt_slist_t *n)
{
    return n->next;
}

/**
 * @brief 判断单向链表是否为空
 *
 * @param l 单向链表的链表头
 */
rt_inline int rt_slist_isempty(rt_slist_t *l)
{
    return l->next == RT_NULL;
}

/**
 * @brief 获取单向链表节点的数据结构
 * @param node 入口点
 * @param type 结构体类型
 * @param member 结构体中链表的成员名
 */
#define rt_slist_entry(node, type, member) \
    rt_container_of(node, type, member)

/**
 * rt_slist_for_each_entry  -   循环遍历单向链表head中每一个pos中的member成员
 * @param pos:    指向宿主结构的指针，在for循环中是一个迭代变量
 * @param head:   单链表的链表头
 * @param member: 结构体中链表的成员名
 */
#define rt_slist_for_each_entry(pos, head, member) \
    for (pos = rt_slist_entry((head)->next, typeof(*pos), member); \
         &pos->member != (RT_NULL); \
         pos = rt_slist_entry(pos->member.next, typeof(*pos), member))

/**
 * rt_slist_first_entry - 获取链表中的第一个元素
 * @param ptr:    链表头
 * @param type:   结构体类型
 * @param member: 结构体中链表的成员名
 *
 * 请注意，该链表不能为空。
 */
#define rt_slist_first_entry(ptr, type, member) \
    rt_slist_entry((ptr)->next, type, member)

/*@}*/

#ifdef __cplusplus
}
#endif

#endif
