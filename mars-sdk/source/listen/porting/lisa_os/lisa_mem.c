#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <lisa_mem.h>
#include "xutils.h"

#define TAG "lisa_mem"

/**
 * @brief 从堆上分配指定大小的内存
 * 
 * @param size 
 * @return void* 
 */
void *lisa_mem_alloc(uint32_t size)
{
	void *ptr = x_malloc(size, 1);
	return ptr;
}

/**
 * @brief 释放指针指向的堆上已分配内存区域
 * 
 * @param ptr 
 */
void lisa_mem_free(void *ptr)
{
	if (ptr) {
		x_free(ptr);
	}
}

/**
 * @brief 分配堆上指定的内存，并且将分配的内存初始化为零
 * 
 * @param count 
 * @param size 
 * @return void* 
 */
void *lisa_mem_calloc(uint32_t count, uint32_t size)
{
	void * ptr = x_malloc(count * size, 1);
	memset(ptr, 0, count * size);
	return ptr;
}

/**
 * @brief 给一个已经分配了地址的堆指针重新分配空间
 * 
 * @param ptr 
 * @param size 
 * @return void* 
 */
void *lisa_mem_realloc(void *ptr, uint32_t size)
{
	if (ptr) {
		x_free(ptr);
	}

	void *reptr = x_malloc(size, 1);
	return reptr;
}