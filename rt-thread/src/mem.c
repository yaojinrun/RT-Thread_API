/*
 * File      : mem.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2008 - 2012, RT-Thread Development Team
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
 * 2008-7-12      Bernard      the first version
 * 2010-06-09     Bernard      fix the end stub of heap
 *                             fix memory check in rt_realloc function
 * 2010-07-13     Bernard      fix RT_ALIGN issue found by kuronca
 * 2010-10-14     Bernard      fix rt_realloc issue when realloc a NULL pointer.
 * 2017-07-14     armink       fix rt_realloc issue when new size is 0
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *         Simon Goldschmidt
 *
 */

#include <rthw.h>
#include <rtthread.h>

#ifndef RT_USING_MEMHEAP_AS_HEAP

/* #define RT_MEM_DEBUG */
#define RT_MEM_STATS

#if defined (RT_USING_HEAP) && defined (RT_USING_SMALL_MEM)
#ifdef RT_USING_HOOK
static void (*rt_malloc_hook)(void *ptr, rt_size_t size);
static void (*rt_free_hook)(void *ptr);

/**
 * @addtogroup Hook
 */

/**@{*/

/**
 * @brief 设置分配钩子函数
 *
 * 该函数将设置一个钩子函数，当从堆内存中分配内存块时将调用该钩子函数。
 *
 * @param hook 钩子函数
 */
void rt_malloc_sethook(void (*hook)(void *ptr, rt_size_t size))
{
    rt_malloc_hook = hook;
}

/**
 * @brief 设置内存释放钩子函数
 *
 *  该函数将设置一个钩子函数，当内存块被释放会内存堆时将调用该钩子函数。
 *
 * @param hook 钩子函数
 */
void rt_free_sethook(void (*hook)(void *ptr))
{
    rt_free_hook = hook;
}

/**@}*/

#endif

#define HEAP_MAGIC 0x1ea0
struct heap_mem
{
    /* magic and used flag */
    rt_uint16_t magic;
    rt_uint16_t used;

    rt_size_t next, prev;

#ifdef RT_USING_MEMTRACE
    rt_uint8_t thread[4];   /* thread name */
#endif
};

/** 指向堆的指针：为了对齐，heap_ptr现在是指针而不是数组 */
static rt_uint8_t *heap_ptr;

/** 最后一个条目，不会被使用！*/
static struct heap_mem *heap_end;

#define MIN_SIZE 12
#define MIN_SIZE_ALIGNED     RT_ALIGN(MIN_SIZE, RT_ALIGN_SIZE)
#define SIZEOF_STRUCT_MEM    RT_ALIGN(sizeof(struct heap_mem), RT_ALIGN_SIZE)

static struct heap_mem *lfree;   /* pointer to the lowest free block */

static struct rt_semaphore heap_sem;
static rt_size_t mem_size_aligned;

#ifdef RT_MEM_STATS
static rt_size_t used_mem, max_mem;
#endif
#ifdef RT_USING_MEMTRACE
rt_inline void rt_mem_setname(struct heap_mem *mem, const char *name)
{
    int index;
    for (index = 0; index < sizeof(mem->thread); index ++)
    {
        if (name[index] == '\0') break;
        mem->thread[index] = name[index];
    }

    for (; index < sizeof(mem->thread); index ++)
    {
        mem->thread[index] = ' ';
    }
}
#endif

static void plug_holes(struct heap_mem *mem)
{
    struct heap_mem *nmem;
    struct heap_mem *pmem;

    RT_ASSERT((rt_uint8_t *)mem >= heap_ptr);
    RT_ASSERT((rt_uint8_t *)mem < (rt_uint8_t *)heap_end);
    RT_ASSERT(mem->used == 0);

    /* plug hole forward */
    nmem = (struct heap_mem *)&heap_ptr[mem->next];
    if (mem != nmem &&
        nmem->used == 0 &&
        (rt_uint8_t *)nmem != (rt_uint8_t *)heap_end)
    {
        /* if mem->next is unused and not end of heap_ptr,
         * combine mem and mem->next
         */
        if (lfree == nmem)
        {
            lfree = mem;
        }
        mem->next = nmem->next;
        ((struct heap_mem *)&heap_ptr[nmem->next])->prev = (rt_uint8_t *)mem - heap_ptr;
    }

    /* plug hole backward */
    pmem = (struct heap_mem *)&heap_ptr[mem->prev];
    if (pmem != mem && pmem->used == 0)
    {
        /* if mem->prev is unused, combine mem and mem->prev */
        if (lfree == mem)
        {
            lfree = pmem;
        }
        pmem->next = mem->next;
        ((struct heap_mem *)&heap_ptr[mem->next])->prev = (rt_uint8_t *)pmem - heap_ptr;
    }
}

/**
 * @ingroup SystemInit
 *
 * @brief 系统内存堆的配置和初始化
 *
 * 该函数将初始化系统内存堆。
 *
 * @param begin_addr 系统内存堆的起始地址
 * @param end_addr 系统内存堆的结束地址。
 */
void rt_system_heap_init(void *begin_addr, void *end_addr)
{
    struct heap_mem *mem;
    rt_uint32_t begin_align = RT_ALIGN((rt_uint32_t)begin_addr, RT_ALIGN_SIZE);
    rt_uint32_t end_align = RT_ALIGN_DOWN((rt_uint32_t)end_addr, RT_ALIGN_SIZE);

    RT_DEBUG_NOT_IN_INTERRUPT;

    /* alignment addr */
    if ((end_align > (2 * SIZEOF_STRUCT_MEM)) &&
        ((end_align - 2 * SIZEOF_STRUCT_MEM) >= begin_align))
    {
        /* calculate the aligned memory size */
        mem_size_aligned = end_align - begin_align - 2 * SIZEOF_STRUCT_MEM;
    }
    else
    {
        rt_kprintf("mem init, error begin address 0x%x, and end address 0x%x\n",
                   (rt_uint32_t)begin_addr, (rt_uint32_t)end_addr);

        return;
    }

    /* point to begin address of heap */
    heap_ptr = (rt_uint8_t *)begin_align;

    RT_DEBUG_LOG(RT_DEBUG_MEM, ("mem init, heap begin address 0x%x, size %d\n",
                                (rt_uint32_t)heap_ptr, mem_size_aligned));

    /* initialize the start of the heap */
    mem        = (struct heap_mem *)heap_ptr;
    mem->magic = HEAP_MAGIC;
    mem->next  = mem_size_aligned + SIZEOF_STRUCT_MEM;
    mem->prev  = 0;
    mem->used  = 0;
#ifdef RT_USING_MEMTRACE
    rt_mem_setname(mem, "INIT");
#endif

    /* initialize the end of the heap */
    heap_end        = (struct heap_mem *)&heap_ptr[mem->next];
    heap_end->magic = HEAP_MAGIC;
    heap_end->used  = 1;
    heap_end->next  = mem_size_aligned + SIZEOF_STRUCT_MEM;
    heap_end->prev  = mem_size_aligned + SIZEOF_STRUCT_MEM;
#ifdef RT_USING_MEMTRACE
    rt_mem_setname(heap_end, "INIT");
#endif

    rt_sem_init(&heap_sem, "heap", 1, RT_IPC_FLAG_FIFO);

    /* initialize the lowest-free pointer to the start of the heap */
    lfree = (struct heap_mem *)heap_ptr;
}

/**
 * @addtogroup MM
 */

/**@{*/

/**
 * @brief 分配内存块
 *
 * 该函数将从内存堆上分配用户指定大小的内存块。
 *
 * @param size 需要分配的内存块的大小，单位为字节。
 *
 * @return 成功则返回分配的内存块地址；失败则返回RT_NULL
 */
void *rt_malloc(rt_size_t size)
{
    rt_size_t ptr, ptr2;
    struct heap_mem *mem, *mem2;

    RT_DEBUG_NOT_IN_INTERRUPT;

    if (size == 0)
        return RT_NULL;

    if (size != RT_ALIGN(size, RT_ALIGN_SIZE))
        RT_DEBUG_LOG(RT_DEBUG_MEM, ("malloc size %d, but align to %d\n",
                                    size, RT_ALIGN(size, RT_ALIGN_SIZE)));
    else
        RT_DEBUG_LOG(RT_DEBUG_MEM, ("malloc size %d\n", size));

    /* alignment size */
    size = RT_ALIGN(size, RT_ALIGN_SIZE);

    if (size > mem_size_aligned)
    {
        RT_DEBUG_LOG(RT_DEBUG_MEM, ("no memory\n"));

        return RT_NULL;
    }

    /* every data block must be at least MIN_SIZE_ALIGNED long */
    if (size < MIN_SIZE_ALIGNED)
        size = MIN_SIZE_ALIGNED;

    /* take memory semaphore */
    rt_sem_take(&heap_sem, RT_WAITING_FOREVER);

    for (ptr = (rt_uint8_t *)lfree - heap_ptr;
         ptr < mem_size_aligned - size;
         ptr = ((struct heap_mem *)&heap_ptr[ptr])->next)
    {
        mem = (struct heap_mem *)&heap_ptr[ptr];

        if ((!mem->used) && (mem->next - (ptr + SIZEOF_STRUCT_MEM)) >= size)
        {
            /* mem is not used and at least perfect fit is possible:
             * mem->next - (ptr + SIZEOF_STRUCT_MEM) gives us the 'user data size' of mem */

            if (mem->next - (ptr + SIZEOF_STRUCT_MEM) >=
                (size + SIZEOF_STRUCT_MEM + MIN_SIZE_ALIGNED))
            {
                /* (in addition to the above, we test if another struct heap_mem (SIZEOF_STRUCT_MEM) containing
                 * at least MIN_SIZE_ALIGNED of data also fits in the 'user data space' of 'mem')
                 * -> split large block, create empty remainder,
                 * remainder must be large enough to contain MIN_SIZE_ALIGNED data: if
                 * mem->next - (ptr + (2*SIZEOF_STRUCT_MEM)) == size,
                 * struct heap_mem would fit in but no data between mem2 and mem2->next
                 * @todo we could leave out MIN_SIZE_ALIGNED. We would create an empty
                 *       region that couldn't hold data, but when mem->next gets freed,
                 *       the 2 regions would be combined, resulting in more free memory
                 */
                ptr2 = ptr + SIZEOF_STRUCT_MEM + size;

                /* create mem2 struct */
                mem2       = (struct heap_mem *)&heap_ptr[ptr2];
                mem2->magic = HEAP_MAGIC;
                mem2->used = 0;
                mem2->next = mem->next;
                mem2->prev = ptr;
#ifdef RT_USING_MEMTRACE
                rt_mem_setname(mem2, "    ");
#endif

                /* and insert it between mem and mem->next */
                mem->next = ptr2;
                mem->used = 1;

                if (mem2->next != mem_size_aligned + SIZEOF_STRUCT_MEM)
                {
                    ((struct heap_mem *)&heap_ptr[mem2->next])->prev = ptr2;
                }
#ifdef RT_MEM_STATS
                used_mem += (size + SIZEOF_STRUCT_MEM);
                if (max_mem < used_mem)
                    max_mem = used_mem;
#endif
            }
            else
            {
                /* (a mem2 struct does no fit into the user data space of mem and mem->next will always
                 * be used at this point: if not we have 2 unused structs in a row, plug_holes should have
                 * take care of this).
                 * -> near fit or excact fit: do not split, no mem2 creation
                 * also can't move mem->next directly behind mem, since mem->next
                 * will always be used at this point!
                 */
                mem->used = 1;
#ifdef RT_MEM_STATS
                used_mem += mem->next - ((rt_uint8_t *)mem - heap_ptr);
                if (max_mem < used_mem)
                    max_mem = used_mem;
#endif
            }
            /* set memory block magic */
            mem->magic = HEAP_MAGIC;
#ifdef RT_USING_MEMTRACE
            if (rt_thread_self())
                rt_mem_setname(mem, rt_thread_self()->name);
            else
                rt_mem_setname(mem, "NONE");
#endif

            if (mem == lfree)
            {
                /* Find next free block after mem and update lowest free pointer */
                while (lfree->used && lfree != heap_end)
                    lfree = (struct heap_mem *)&heap_ptr[lfree->next];

                RT_ASSERT(((lfree == heap_end) || (!lfree->used)));
            }

            rt_sem_release(&heap_sem);
            RT_ASSERT((rt_uint32_t)mem + SIZEOF_STRUCT_MEM + size <= (rt_uint32_t)heap_end);
            RT_ASSERT((rt_uint32_t)((rt_uint8_t *)mem + SIZEOF_STRUCT_MEM) % RT_ALIGN_SIZE == 0);
            RT_ASSERT((((rt_uint32_t)mem) & (RT_ALIGN_SIZE - 1)) == 0);

            RT_DEBUG_LOG(RT_DEBUG_MEM,
                         ("allocate memory at 0x%x, size: %d\n",
                          (rt_uint32_t)((rt_uint8_t *)mem + SIZEOF_STRUCT_MEM),
                          (rt_uint32_t)(mem->next - ((rt_uint8_t *)mem - heap_ptr))));

            RT_OBJECT_HOOK_CALL(rt_malloc_hook,
                                (((void *)((rt_uint8_t *)mem + SIZEOF_STRUCT_MEM)), size));

            /* return the memory data except mem struct */
            return (rt_uint8_t *)mem + SIZEOF_STRUCT_MEM;
        }
    }

    rt_sem_release(&heap_sem);

    return RT_NULL;
}
RTM_EXPORT(rt_malloc);

/**
 * @brief 重新分配内存块
 *
 * 此函数将在已分配内存块的基础上重新分配内存块的大小（增加或缩小），在进行重新分配内存块时，
 * 原来的内存块数据保持不变（缩小的情况下，后面的数据被自动截断）。
 *
 * @param rmem 指向已分配的内存块
 * @param newsize 重新分配的内存大小
 *
 * @return 成功则返回重新分配的内存块地址；失败则返回RT_NULL
 */
void *rt_realloc(void *rmem, rt_size_t newsize)
{
    rt_size_t size;
    rt_size_t ptr, ptr2;
    struct heap_mem *mem, *mem2;
    void *nmem;

    RT_DEBUG_NOT_IN_INTERRUPT;

    /* alignment size */
    newsize = RT_ALIGN(newsize, RT_ALIGN_SIZE);
    if (newsize > mem_size_aligned)
    {
        RT_DEBUG_LOG(RT_DEBUG_MEM, ("realloc: out of memory\n"));

        return RT_NULL;
    }
    else if (newsize == 0)
    {
        rt_free(rmem);
        return RT_NULL;
    }

    /* allocate a new memory block */
    if (rmem == RT_NULL)
        return rt_malloc(newsize);

    rt_sem_take(&heap_sem, RT_WAITING_FOREVER);

    if ((rt_uint8_t *)rmem < (rt_uint8_t *)heap_ptr ||
        (rt_uint8_t *)rmem >= (rt_uint8_t *)heap_end)
    {
        /* illegal memory */
        rt_sem_release(&heap_sem);

        return rmem;
    }

    mem = (struct heap_mem *)((rt_uint8_t *)rmem - SIZEOF_STRUCT_MEM);

    ptr = (rt_uint8_t *)mem - heap_ptr;
    size = mem->next - ptr - SIZEOF_STRUCT_MEM;
    if (size == newsize)
    {
        /* the size is the same as */
        rt_sem_release(&heap_sem);

        return rmem;
    }

    if (newsize + SIZEOF_STRUCT_MEM + MIN_SIZE < size)
    {
        /* split memory block */
#ifdef RT_MEM_STATS
        used_mem -= (size - newsize);
#endif

        ptr2 = ptr + SIZEOF_STRUCT_MEM + newsize;
        mem2 = (struct heap_mem *)&heap_ptr[ptr2];
        mem2->magic = HEAP_MAGIC;
        mem2->used = 0;
        mem2->next = mem->next;
        mem2->prev = ptr;
#ifdef RT_USING_MEMTRACE
        rt_mem_setname(mem2, "    ");
#endif
        mem->next = ptr2;
        if (mem2->next != mem_size_aligned + SIZEOF_STRUCT_MEM)
        {
            ((struct heap_mem *)&heap_ptr[mem2->next])->prev = ptr2;
        }

        plug_holes(mem2);

        rt_sem_release(&heap_sem);

        return rmem;
    }
    rt_sem_release(&heap_sem);

    /* expand memory */
    nmem = rt_malloc(newsize);
    if (nmem != RT_NULL) /* check memory */
    {
        rt_memcpy(nmem, rmem, size < newsize ? size : newsize);
        rt_free(rmem);
    }

    return nmem;
}
RTM_EXPORT(rt_realloc);

/**
 * @brief 分配多内存块
 *
 * 此函数将从内存堆中分配连续内存地址的多个内存块，每个空间对象的内存大小为字节，并返回指向已分配内存的指针。
 *
 * @param count 内存块数量
 * @param size 内存块大小
 *
 * @return 成功则指向指向第一个内存块地址的指针，并且所有分配的内存块都被初始化成零；如果失败则返回 NULL。
 */
void *rt_calloc(rt_size_t count, rt_size_t size)
{
    void *p;

    RT_DEBUG_NOT_IN_INTERRUPT;

    /* allocate 'count' objects of size 'size' */
    p = rt_malloc(count * size);

    /* zero the memory */
    if (p)
        rt_memset(p, 0, count * size);

    return p;
}
RTM_EXPORT(rt_calloc);

/**
 * @brief 释放内存块
 *
 * 用户线程使用完从内存分配器中申请的内存后，必须及时调用该函数释放，否则会造成内存泄漏，
 * 系统会把待释放的内存还回给堆管理器中。
 *
 * @param rmem 待释放的内存块指针
 */
void rt_free(void *rmem)
{
    struct heap_mem *mem;

    RT_DEBUG_NOT_IN_INTERRUPT;

    if (rmem == RT_NULL)
        return;
    RT_ASSERT((((rt_uint32_t)rmem) & (RT_ALIGN_SIZE - 1)) == 0);
    RT_ASSERT((rt_uint8_t *)rmem >= (rt_uint8_t *)heap_ptr &&
              (rt_uint8_t *)rmem < (rt_uint8_t *)heap_end);

    RT_OBJECT_HOOK_CALL(rt_free_hook, (rmem));

    if ((rt_uint8_t *)rmem < (rt_uint8_t *)heap_ptr ||
        (rt_uint8_t *)rmem >= (rt_uint8_t *)heap_end)
    {
        RT_DEBUG_LOG(RT_DEBUG_MEM, ("illegal memory\n"));

        return;
    }

    /* Get the corresponding struct heap_mem ... */
    mem = (struct heap_mem *)((rt_uint8_t *)rmem - SIZEOF_STRUCT_MEM);

    RT_DEBUG_LOG(RT_DEBUG_MEM,
                 ("release memory 0x%x, size: %d\n",
                  (rt_uint32_t)rmem,
                  (rt_uint32_t)(mem->next - ((rt_uint8_t *)mem - heap_ptr))));


    /* protect the heap from concurrent access */
    rt_sem_take(&heap_sem, RT_WAITING_FOREVER);

    /* ... which has to be in a used state ... */
    if (!mem->used || mem->magic != HEAP_MAGIC)
    {
        rt_kprintf("to free a bad data block:\n");
        rt_kprintf("mem: 0x%08x, used flag: %d, magic code: 0x%04x\n", mem, mem->used, mem->magic);
    }
    RT_ASSERT(mem->used);
    RT_ASSERT(mem->magic == HEAP_MAGIC);
    /* ... and is now unused. */
    mem->used  = 0;
    mem->magic = HEAP_MAGIC;
#ifdef RT_USING_MEMTRACE
    rt_mem_setname(mem, "    ");
#endif

    if (mem < lfree)
    {
        /* the newly freed struct is now the lowest */
        lfree = mem;
    }

#ifdef RT_MEM_STATS
    used_mem -= (mem->next - ((rt_uint8_t *)mem - heap_ptr));
#endif

    /* finally, see if prev or next are free also */
    plug_holes(mem);
    rt_sem_release(&heap_sem);
}
RTM_EXPORT(rt_free);

#ifdef RT_MEM_STATS
void rt_memory_info(rt_uint32_t *total,
                    rt_uint32_t *used,
                    rt_uint32_t *max_used)
{
    if (total != RT_NULL)
        *total = mem_size_aligned;
    if (used  != RT_NULL)
        *used = used_mem;
    if (max_used != RT_NULL)
        *max_used = max_mem;
}

#ifdef RT_USING_FINSH
#include <finsh.h>

void list_mem(void)
{
    rt_kprintf("total memory: %d\n", mem_size_aligned);
    rt_kprintf("used memory : %d\n", used_mem);
    rt_kprintf("maximum allocated memory: %d\n", max_mem);
}
FINSH_FUNCTION_EXPORT(list_mem, list memory usage information)

#ifdef RT_USING_MEMTRACE
int memcheck(void)
{
    int position;
    rt_uint32_t level;
    struct heap_mem *mem;
    level = rt_hw_interrupt_disable();
    for (mem = (struct heap_mem *)heap_ptr; mem != heap_end; mem = (struct heap_mem *)&heap_ptr[mem->next])
    {
        position = (rt_uint32_t)mem - (rt_uint32_t)heap_ptr;
        if (position < 0) goto __exit;
        if (position > mem_size_aligned) goto __exit;
        if (mem->magic != HEAP_MAGIC) goto __exit;
        if (mem->used != 0 && mem->used != 1) goto __exit;
    }
    rt_hw_interrupt_enable(level);

    return 0;
__exit:
    rt_kprintf("Memory block wrong:\n");
    rt_kprintf("address: 0x%08x\n", mem);
    rt_kprintf("  magic: 0x%04x\n", mem->magic);
    rt_kprintf("   used: %d\n", mem->used);
    rt_kprintf("  size: %d\n", mem->next - position - SIZEOF_STRUCT_MEM);
    rt_hw_interrupt_enable(level);

    return 0;
}
MSH_CMD_EXPORT(memcheck, check memory data);

int memtrace(int argc, char **argv)
{
    struct heap_mem *mem;

    list_mem();

    rt_kprintf("\nmemory heap address:\n");
    rt_kprintf("heap_ptr: 0x%08x\n", heap_ptr);
    rt_kprintf("lfree   : 0x%08x\n", lfree);
    rt_kprintf("heap_end: 0x%08x\n", heap_end);

    rt_kprintf("\n--memory item information --\n");
    for (mem = (struct heap_mem *)heap_ptr; mem != heap_end; mem = (struct heap_mem *)&heap_ptr[mem->next])
    {
        int position = (rt_uint32_t)mem - (rt_uint32_t)heap_ptr;
        int size;

        rt_kprintf("[0x%08x - ", mem);

        size = mem->next - position - SIZEOF_STRUCT_MEM;
        if (size < 1024)
            rt_kprintf("%5d", size);
        else if (size < 1024 * 1024)
            rt_kprintf("%4dK", size / 1024);
        else
            rt_kprintf("%4dM", size / (1024 * 1024));

        rt_kprintf("] %c%c%c%c", mem->thread[0], mem->thread[1], mem->thread[2], mem->thread[3]);
        if (mem->magic != HEAP_MAGIC)
            rt_kprintf(": ***\n");
        else
            rt_kprintf("\n");
    }

    return 0;
}
MSH_CMD_EXPORT(memtrace, dump memory trace information);
#endif /* end of RT_USING_MEMTRACE */
#endif /* end of RT_USING_FINSH    */

#endif

/**@}*/

#endif /* end of RT_USING_HEAP */
#endif /* end of RT_USING_MEMHEAP_AS_HEAP */
