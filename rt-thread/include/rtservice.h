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
 * @brief ��ȡtype�ṹ����member��Ա������ṹ���е�ƫ�ơ�
 */
#define rt_container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))


/**
 * @brief ��ʼ���������
 */
#define RT_LIST_OBJECT_INIT(object) { &(object), &(object) }

/**
 * @brief ��ʼ������
 *
 * @param l ������ʼ��������
 */
rt_inline void rt_list_init(rt_list_t *l)
{
    l->next = l->prev = l;
}

/**
 * @brief ����ͷ��
 *
 * @param l ����������
 * @param n ��Ҫ��������½ڵ�
 */
rt_inline void rt_list_insert_after(rt_list_t *l, rt_list_t *n)
{
    l->next->prev = n;
    n->next = l->next;

    l->next = n;
    n->prev = l;
}

/**
 * @brief ����β��
 *
 * @param n ��Ҫ��������½ڵ�
 * @param l ��������
 */
rt_inline void rt_list_insert_before(rt_list_t *l, rt_list_t *n)
{
    l->prev->next = n;
    n->prev = l->prev;

    l->prev = n;
    n->next = l;
}

/**
 * @brief �Ƴ�һ������ڵ�
 * @param n ��Ҫ��������ɾ���Ľڵ�
 */
rt_inline void rt_list_remove(rt_list_t *n)
{
    n->next->prev = n->prev;
    n->prev->next = n->next;

    n->next = n->prev = n;
}

/**
 * @brief �ж������Ƿ�Ϊ��
 * @param l �����Ե�����
 */
rt_inline int rt_list_isempty(const rt_list_t *l)
{
    return l->next == l;
}

/**
 * @brief ��ȡ������
 * @param l ����ȡ������
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
 * @brief ��ȡ�ṹ������ĵ�ַ
 * @param node ��ڵ�
 * @param type �ṹ�������
 * @param member �ṹ��������ĳ�Ա��
 */
#define rt_list_entry(node, type, member) \
    rt_container_of(node, type, member)

/**
 * @brief ѭ������head������ÿһ��pos�е�member��Ա
 * @param pos:    ָ�������ṹ��ָ�룬��forѭ������һ����������
 * @param head:   ����ͷ
 * @param member: �ṹ��������ĳ�Ա��
 */
#define rt_list_for_each_entry(pos, head, member) \
    for (pos = rt_list_entry((head)->next, typeof(*pos), member); \
         &pos->member != (head); \
         pos = rt_list_entry(pos->member.next, typeof(*pos), member))

/**
 * rt_list_for_each_entry_safe - �����list_for_each_entry���ԣ�
 * list_for_each_entry_safe��ָ��n���������һ�����ݽṹ��������ʱ�洢��
 * ��������ڱ��������ʱ�����Ҫɾ�������еĵ�ǰ���list_for_each_entry_safe
 * ���԰�ȫ��ɾ����������Ӱ��������ı������̣���n ָ����Լ�����ɽ������ı����� 
 * ��list_for_each_entry���޷�������������
 * @param pos:    ָ�������ṹ��ָ�룬��forѭ������һ����������
 * @param n:      ������ʱ�洢��һ�����ݽṹ��ָ�����
 * @param head:   ����ͷ
 * @param member: �ṹ��������ĳ�Ա��
 */
#define rt_list_for_each_entry_safe(pos, n, head, member) \
    for (pos = rt_list_entry((head)->next, typeof(*pos), member), \
         n = rt_list_entry(pos->member.next, typeof(*pos), member); \
         &pos->member != (head); \
         pos = n, n = rt_list_entry(n->member.next, typeof(*n), member))

/**
 * @brief ��ȡ�����еĵ�һ��Ԫ��
 * @param ptr:    ����ͷ
 * @param type:   �ṹ������
 * @param member: �ṹ��������ĳ�Ա��
 *
 * ��ע�⣬��������Ϊ�ա�
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
 * @brief ��ʼ��һ��������
 *
 * @param l ��Ҫ����ʼ���ĵ�������
 */
rt_inline void rt_slist_init(rt_slist_t *l)
{
    l->next = RT_NULL;
}

/**
 * @brief �ڵ�������l��β��׷��һ���ڵ�
 *
 * @param l ������������
 * @param n ��Ҫ��׷�ӵĽڵ�
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
 * @brief ��������ڵ�l�����һһ���ڵ�n
 *
 * @param l �����������ڵ�
 * @param n ��Ҫ������Ľڵ�
 */
rt_inline void rt_slist_insert(rt_slist_t *l, rt_slist_t *n)
{
    n->next = l->next;
    l->next = n;
}

/**
 * @brief ��ȡ��������ĳ���
 *
 * @param l �������������ͷ
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
 * @brief �ӵ�������l���Ƴ���һ���ڵ�n
 *
 * @param l �������������ͷ
 * @param n ��Ҫ���Ƴ��Ľڵ�
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
 * @brief ��ȡ��������ĵ�һ���ڵ�
 *
 * @param l �������������ͷ
 */
rt_inline rt_slist_t *rt_slist_first(rt_slist_t *l)
{
    return l->next;
}

/**
 * @brief ��ȡ��������ڵ�n����һ���ڵ�
 *
 * @param n ��������Ľڵ�
 */
rt_inline rt_slist_t *rt_slist_next(rt_slist_t *n)
{
    return n->next;
}

/**
 * @brief �жϵ��������Ƿ�Ϊ��
 *
 * @param l �������������ͷ
 */
rt_inline int rt_slist_isempty(rt_slist_t *l)
{
    return l->next == RT_NULL;
}

/**
 * @brief ��ȡ��������ڵ�����ݽṹ
 * @param node ��ڵ�
 * @param type �ṹ������
 * @param member �ṹ��������ĳ�Ա��
 */
#define rt_slist_entry(node, type, member) \
    rt_container_of(node, type, member)

/**
 * rt_slist_for_each_entry  -   ѭ��������������head��ÿһ��pos�е�member��Ա
 * @param pos:    ָ�������ṹ��ָ�룬��forѭ������һ����������
 * @param head:   �����������ͷ
 * @param member: �ṹ��������ĳ�Ա��
 */
#define rt_slist_for_each_entry(pos, head, member) \
    for (pos = rt_slist_entry((head)->next, typeof(*pos), member); \
         &pos->member != (RT_NULL); \
         pos = rt_slist_entry(pos->member.next, typeof(*pos), member))

/**
 * rt_slist_first_entry - ��ȡ�����еĵ�һ��Ԫ��
 * @param ptr:    ����ͷ
 * @param type:   �ṹ������
 * @param member: �ṹ��������ĳ�Ա��
 *
 * ��ע�⣬��������Ϊ�ա�
 */
#define rt_slist_first_entry(ptr, type, member) \
    rt_slist_entry((ptr)->next, type, member)

/*@}*/

#ifdef __cplusplus
}
#endif

#endif
