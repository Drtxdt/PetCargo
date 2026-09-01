#ifndef __LISA_MEM__
#define __LISA_MEM__

#include <stddef.h>
#include <stdint.h>

/**
 * @brief malloc
 * @param  size             
 * @return void* 
 */
void *lisa_mem_alloc(uint32_t size);

/**
 * @brief realloc
 * @param  ptr              
 * @param  size             
 * @return void* 
 */
void *lisa_mem_realloc(void *ptr, uint32_t size);

/**
 * @brief calloc
 * @param  count            
 * @param  size             
 * @return void* 
 */
void *lisa_mem_calloc(uint32_t count, uint32_t size);

/**
 * @brief free
 * @param  ptr              
 */
void lisa_mem_free(void *ptr);

#endif  // __LISA_MEM__