/*****************************************************************************
 *   Copyright(C)2009-2019 by VSF Team                                       *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *     http://www.apache.org/licenses/LICENSE-2.0                            *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 ****************************************************************************/

#ifndef __VSF_POOL_H__
#define __VSF_POOL_H__

/* example:

    // 0. Include vsf header file
    #include "vsf.h"

    // 1. Defining type of pool member
    typedef struct {
        ......
    } xxxx_t;

    // 2. Declare the pool with user type
    declare_vsf_pool(xxxx_pool, xxxx_t)

    // 3. Defining the pool  
    def_vsf_pool(xxxx_pool, xxxx_t)
    
    // 4. Implement the pool
    implement_vsf_pool(xxxx_pool, xxxx_t )

    // 4. Defining pool variable
    static NO_INIT vsf_pool(xxxx_pool) __xxxx_pool;

    void user_example_task(void)
    {
        ......

        // 5. Initialization user pool with size, attached object and code region .
        //    the attached object and code region can be omitted.
        VSF_POOL_INIT(xxxx_pool, &__xxxx_pool, 8);

        ......

        // 6. Alloc memory from user pool
        xxxx_t *ptarget = VSF_POOL_ALLOC(xxxx_pool, &__xxxx_pool);
        if (NULL != ptarget) {
            // 6.1. Do something when alloc successfully
            
            // 7. Free memory to user pool
            VSF_POOL_FREE(xxxx_pool, &__xxxx_pool, ptarget);
        } else {
            // 6.2. Do something when alloc failed
        }

        ......
    }
 */

/*============================ INCLUDES ======================================*/
#include "../compiler.h"
#include "./__class_pool.h"

/*============================ MACROS ========================================*/

#define __vsf_pool(__name)          __name##_pool_t
#define __vsf_pool_block(__name)    __name##_pool_item_t

#define __declare_vsf_pool(__name)                                              \
            typedef union __name##_pool_item_t __name##_pool_item_t;            \
            typedef struct __name##_pool_t __name##_pool_t;

#define __def_vsf_pool(__name, __type)                                          \
    union __name##_pool_item_t {                                                \
        implement(vsf_slist_t)                                                  \
        __type                  tMem;                                           \
    };                                                                          \
    struct __name##_pool_t {                                                    \
        implement(vsf_pool_t)                                                   \
    };                                                                          \
    extern void __name##_pool_init(__name##_pool_t *, vsf_pool_cfg_t *);        \
    extern bool __name##_pool_add_buffer(                                       \
        __name##_pool_t *, void *, uint_fast32_t );                             \
    extern __type *__name##_pool_alloc(__name##_pool_t *);                      \
    extern void __name##_pool_free(__name##_pool_t *, __type *);                \
    extern uint_fast32_t __name##_get_pool_item_count(__name##_pool_t *);       \
    extern code_region_t *__name##_pool_get_region(__name##_pool_t *);          \
    extern void *__name##_pool_get_target(__name##_pool_t *);                   \
    extern void *__name##_pool_set_target(__name##_pool_t *, void *);


#define __implement_vsf_pool(__name, __type)                                    \
                                                                                \
void __name##_pool_init(__name##_pool_t *pthis, vsf_pool_cfg_t *pcfg)           \
{                                                                               \
    vsf_pool_init((vsf_pool_t *)pthis, pcfg);                                   \
}                                                                               \
                                                                                \
bool __name##_pool_add_buffer(                                                  \
    __name##_pool_t *pthis, void *ptBuffer, uint_fast32_t tSize)                \
{                                                                               \
    return vsf_pool_add_buffer((vsf_pool_t *)pthis, ptBuffer,                   \
                            tSize, sizeof(__name##_pool_item_t));               \
}                                                                               \
                                                                                \
__type *__name##_pool_alloc(__name##_pool_t *pthis)                             \
{                                                                               \
    return (__type *)vsf_pool_alloc((vsf_pool_t *)pthis);                       \
}                                                                               \
                                                                                \
void __name##_pool_free(__name##_pool_t *pthis, __type *ptItem)                 \
{                                                                               \
    vsf_pool_free((vsf_pool_t *)pthis, (void *)ptItem);                         \
}                                                                               \
SECTION(".text." #__name "_get_pool_item_count")                                \
uint_fast32_t __name##_get_pool_item_count(__name##_pool_t *pthis)              \
{                                                                               \
    return vsf_pool_get_count((vsf_pool_t *)pthis);                             \
}                                                                               \
SECTION(".text." #__name "_pool_get_region")                                    \
code_region_t *__name##_pool_get_region(__name##_pool_t *pthis)                 \
{                                                                               \
    return vsf_pool_get_region((vsf_pool_t *)pthis);                            \
}                                                                               \
SECTION(".text." #__name "_pool_get_target")                                    \
void *__name##_pool_get_target(__name##_pool_t *pthis)                          \
{                                                                               \
    return vsf_pool_get_tag((vsf_pool_t *)pthis);                               \
}                                                                               \
SECTION(".text." #__name "_pool_set_target")                                    \
void *__name##_pool_set_target(__name##_pool_t *pthis, void *ptag)              \
{                                                                               \
    return vsf_pool_set_tag((vsf_pool_t *)pthis, ptag);                         \
}




//! \name pool normal access
//! @{
#define vsf_pool(__name)            __vsf_pool(__name)
#define vsf_pool_block(__name)      __vsf_pool_block(__name)

#define declare_vsf_pool(__name)        /* the name of the pool */              \
            __declare_vsf_pool(__name)

#define def_vsf_pool(__name,            /* the name of the pool */              \
                     __type)            /* the type of the pool */              \
            __def_vsf_pool(__name, __type)

#define implement_vsf_pool(__name,            /* the name of the pool */        \
                     __type)            /* the type of the pool */              \
            __implement_vsf_pool(__name, __type)


#define VSF_POOL_INIT(__NAME,           /* the name of the pool */              \
                      __VSF_POOL,       /* the address of the pool */           \
                      __SIZE,           /* the total size of the pool */        \
                      ...)              /* the address of an attached object */ \
                                        /* the address of the code region obj */\
            do {                                                                \
                vsf_pool_cfg_t cfg = {__VA_ARGS__};                             \
                __NAME##_pool_init((__VSF_POOL), &cfg);                         \
                static NO_INIT __NAME##_pool_item_t s_tBuffer[__SIZE];          \
                vsf_pool_add_buffer(  (vsf_pool_t *)(__VSF_POOL),               \
                                    s_tBuffer,                                  \
                                    sizeof(s_tBuffer),                          \
                                    sizeof(__NAME##_pool_item_t));              \
            } while(0)                                                                         

#define VSF_POOL_ADD_BUFFER(__NAME,     /* the name of the pool */              \
                            __VSF_POOL, /* the address of the pool */           \
                            __BUFFER,   /* the address of the buffer */         \
                            __SIZE)     /* the size of the buffer */            \
            __NAME##_pool_add_buffer((__VSF_POOL), (__BUFFER), (__SIZE))

#define VSF_POOL_FREE(__NAME,           /* the name of the pool */              \
                      __VSF_POOL,       /* the address of the pool */           \
                      __ITEM)           /* the address of the memory block */   \
                                        /* to be released */                    \
            __NAME##_pool_free((__VSF_POOL), (__ITEM))

#define VSF_POOL_ALLOC(__NAME,          /* the name of the pool */              \
                       __VSF_POOL)      /* the address of the pool */           \
            __NAME##_pool_alloc((__VSF_POOL))

#define VSF_POOL_ITEM_COUNT(__NAME,     /* the name of the pool */              \
                            __VSF_POOL) /* the address of the pool */           \
            __NAME##_get_pool_item_count((__VSF_POOL))

#define VSF_POOL_GET_REGION(__NAME,      /* the name of the pool */             \
                           __VSF_POOL)   /* the address of the pool */          \
            __NAME##_pool_get_region((__VSF_POOL))

#define VSF_POOL_GET_TAG(__NAME,        /* the name of the pool */              \
                         __VSF_POOL)    /* the address of the pool */           \
            __NAME##_pool_get_target((__VSF_POOL))

#define VSF_POOL_SET_TAG(__NAME,        /* the name of the pool */              \
                         __VSF_POOL,    /* the address of the pool */           \
                         __TARGET)      /* the address of the target */         \
            __NAME##_pool_set_target((__VSF_POOL), (__TARGET))
//! @}

/*============================ TYPES =========================================*/

typedef struct {
    void *ptarget;
    code_region_t *pregion;
} vsf_pool_cfg_t;

typedef
void
vsf_pool_item_init_evt_handler_t(   void *ptarget, 
                                    void *pitem, 
                                    uint_fast32_t item_size);

//! \name vsf pool interface
//! @{
def_interface(i_pool_t)
    void (*Init)            (vsf_pool_t *pthis, vsf_pool_cfg_t *pcfg);
    struct {
        bool (*AddEx)       (vsf_pool_t *pthis,
                             void *pbuffer,
                             uint32_t buffer_size,
                             uint32_t item_size,
                             vsf_pool_item_init_evt_handler_t *fnhandler_init);
        bool (*Add)         (vsf_pool_t *pthis,
                             void *pbuffer,
                             uint32_t buffer_size,
                             uint32_t item_size);
    }Buffer;

    void *(*Allocate)       (vsf_pool_t *pthis);
    void (*Free)            (vsf_pool_t *pthis, void *pitem);
    uint_fast16_t  (*Count) (vsf_pool_t *pthis);

    struct {
        void *(*Get)        (vsf_pool_t *pthis);
        void *(*Set)        (vsf_pool_t *pthis, void *ptag);
    }Tag;

end_def_interface(i_pool_t)
//! @}
/*============================ GLOBAL VARIABLES ==============================*/
extern const i_pool_t VSF_POOL;

/*============================ PROTOTYPES ====================================*/

/*! \brief initialise target pool
 *! \param pthis address of the target pool
 *! \param pcfg configurations
 *! \return none
 */
extern void vsf_pool_init(vsf_pool_t *pthis, vsf_pool_cfg_t *pcfg);

/*! \brief add memory to pool
 *! \param pthis            address of the target pool
 *! \param pbuffer          address of the target memory
 *! \param buffer_size      the size of the target memory
 *! \param item_size        memory block size of the pool
 *! \param fnhandler_init   block initialisation handler
 *! \retval false           the buffer size is too small or invalid parameters
 *! \retval true            buffer is added
 */
extern bool
vsf_pool_add_buffer_ex( vsf_pool_t *pthis,
                        void *pbuffer,
                        uint32_t buffer_size,
                        uint32_t item_size,
                        vsf_pool_item_init_evt_handler_t *fnhandler_init);

/*! \brief add memory to pool
 *! \param pthis        address of the target pool
 *! \param pbuffer      address of the target memory
 *! \param buffer_size  the size of the target memory
 *! \param item_size    memory block size of the pool
 *! \retval false       the buffer size is too small or invalid parameters
 *! \retval true        buffer is added
 */
extern bool vsf_pool_add_buffer(  vsf_pool_t *pthis,
                                void *pbuffer,
                                uint32_t buffer_size,
                                uint32_t item_size);

/*! \brief try to fetch a memory block from the target pool
 *! \param pthis    address of the target pool
 *! \retval NULL    the pool is empty
 *! \retval !NULL   address of the allocated memory block
 */
extern void *vsf_pool_alloc(vsf_pool_t *pthis);

/*! \brief return a memory block to the target pool
 *! \param pthis    address of the target pool
 *! \param pitem    target memory block
 *! \return none
 */
extern void vsf_pool_free(vsf_pool_t *pthis, void *pitem);

/*! \brief get the number of memory blocks available in the target pool
 *! \param pthis    address of the target pool
 *! \return the number of memory blocks
 */
extern uint_fast16_t vsf_pool_get_count(vsf_pool_t *pthis);

/*! \brief get the address of the object which is attached to the pool
 *! \param pthis    address of the target pool
 *! \return the address of the object
 */
extern void *vsf_pool_get_tag(vsf_pool_t *pthis);

/*! \brief set the address of the object which is attached to the pool
 *! \param pthis    address of the target pool
 *! \return the address of the object
 */
extern void *vsf_pool_set_tag(vsf_pool_t *pthis, void *ptag);

/*! \brief get the address of the code region used by this pool
 *! \param pthis    address of the target pool
 *! \return the address of the code region
 */
extern code_region_t *vsf_pool_get_region(vsf_pool_t *pthis);

#endif
