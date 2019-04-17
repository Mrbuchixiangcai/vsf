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
/*============================ INCLUDES ======================================*/
#include "vsf.h"
#include <stdio.h>

#include "ooc_demo/class_demo.h"
#include "ooc_demo/class_simple_demo.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

#if VSF_USE_KERNEL_THREAD_MODE == ENABLED
declare_vsf_thread(user_task_t)

def_vsf_thread(user_task_t, 1024,

    features_used(
        mem_sharable( using_grouped_evt; )
        mem_nonsharable( )
    )
    
    def_params(
#if VSF_CFG_SYNC_EN
        vsf_sem_t *psem;
#endif
    ));
#endif

#if VSF_CFG_SYNC_EN == ENABLED
declare_grouped_evts(user_grouped_evts_t)

def_grouped_evts(user_grouped_evts_t)
    def_evt     (user_grouped_evts_t, timer0_evt),
    def_evt     (user_grouped_evts_t, timer1_evt),
    def_evt     (user_grouped_evts_t, timer2_evt),
    def_evt     (user_grouped_evts_t, timer3_evt),
    def_evt     (user_grouped_evts_t, timer4_evt),
    def_adapter (user_grouped_evts_t, sem_evt),
end_def_grouped_evts(user_grouped_evts_t)
  
#endif

#if VSF_USE_KERNEL_PT_MODE == ENABLED

declare_vsf_pt(user_pt_bmpevt_demo_slave_t);
declare_vsf_pt(user_pt_bmpevt_demo_thread_t);

def_vsf_pt(user_pt_bmpevt_demo_slave_t,
    features_used(
        mem_sharable( using_grouped_evt; )
        mem_nonsharable( )
    )
    
    def_params(
        user_grouped_evts_t         *pgroup_evts;
        enum_of_user_grouped_evts_t mask;
    ))
    


def_vsf_pt(user_pt_bmpevt_demo_thread_t,

    features_used(
        mem_sharable( using_grouped_evt; )
        mem_nonsharable( )
    )
    
    def_params(
        user_grouped_evts_t         *pgroup_evts;
        enum_of_user_grouped_evts_t mask;
        
        vsf_pt(user_pt_bmpevt_demo_slave_t) slave;
    ))
#endif

struct user_msg_t {
    implement(vsf_slist_node_t);
    int index;
};
typedef struct user_msg_t user_msg_t;
declare_vsf_pool(user_msg_pool_t)
def_vsf_pool(user_msg_pool_t, user_msg_t)

#if VSF_CFG_BMPEVT_EN == ENABLED
declare_vsf_task(bmevt_demo_t)
def_vsf_task(bmevt_demo_t,
    features_used(
        mem_sharable( using_grouped_evt; )
        mem_nonsharable( )
    )
)
#endif
#if VSF_CFG_TIMER_EN == ENABLED
declare_vsf_task(timer_example_t)

def_vsf_task(timer_example_t)

#endif

#if VSF_CFG_QUEUE_EN == ENABLED

declare_vsf_task(msg_queue_example_t)

def_vsf_task(msg_queue_example_t)

#endif

static NO_INIT class_demo_t class_demo;
static NO_INIT class_simple_demo_t class_simple_demo;

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

// user code below
#if VSF_CFG_TIMER_EN == ENABLED
static NO_INIT timer_example_t __timer_example[5];

#endif

#if VSF_CFG_SYNC_EN == ENABLED
static NO_INIT vsf_sem_t user_sem;

#if VSF_CFG_BMPEVT_EN == ENABLED
static NO_INIT bmevt_demo_t __bmevt_demo;


implement_grouped_evts(user_grouped_evts_t,
    add_sync_adapter( &user_sem, sem_evt_msk ),
)

static NO_INIT user_grouped_evts_t __user_grouped_evts;
#endif

#endif

#if VSF_CFG_QUEUE_EN == ENABLED
vsf_pool(user_msg_pool_t) user_msg_pool;
static NO_INIT vsf_queue_t user_msgq;

static NO_INIT msg_queue_example_t __msg_queue_example_receiver;

implement_vsf_pool( user_msg_pool_t, user_msg_t)

static implement_vsf_task(msg_queue_example_t)
{
    vsf_sync_reason_t reason;
    vsf_slist_node_t *pnode;
    user_msg_t *pmsg;

    switch (evt) {
    case VSF_EVT_INIT:
        recv_again:
        if (!vsf_eda_queue_recv(&user_msgq, &pnode, -1)) {
            goto msg_rcvd;
        }
        break;
//    case VSF_EVT_TIMER:
    case VSF_EVT_SYNC:
    case VSF_EVT_SYNC_CANCEL:
        reason = vsf_eda_queue_get_reason(&user_msgq, evt, &pnode);
        if (reason == VSF_SYNC_GET) {
        msg_rcvd:
            pmsg = (user_msg_t *)pnode;
            printf("msg receviced from %d\r\n", pmsg->index);
            VSF_POOL_FREE(user_msg_pool_t, &user_msg_pool, pmsg);
            goto recv_again;
        }
    }
    

    return fsm_rt_wait_for_evt;
}
#endif

#if VSF_CFG_TIMER_EN == ENABLED

static implement_vsf_task(timer_example_t)
{

    int index = (timer_example_t *)ptThis - __timer_example;
    int delay = 2000 * (1 + index);
    user_msg_t *pmsg;

	switch (evt) {
    case VSF_EVT_TIMER:
        printf("%d: timer post\r\n ", index);
#if VSF_CFG_BMPEVT_EN == ENABLED
        set_grouped_evts( &__user_grouped_evts,  1 << (index + timer0_evt_idx));
#endif
        pmsg = VSF_POOL_ALLOC(user_msg_pool_t, &user_msg_pool);
        if (pmsg != NULL) {
            pmsg->index = index;
            if (vsf_eda_queue_send(&user_msgq, &pmsg->use_as__vsf_slist_node_t)) {
                VSF_POOL_FREE(user_msg_pool_t, &user_msg_pool, pmsg);
            }
        }
    case VSF_EVT_INIT:
        vsf_teda_set_timer(delay);
        break;
	}

    return fsm_rt_wait_for_evt;
}
#endif



static void __code_region_example_on_enter(void *pObj, void *pLocal)
{
    printf("-------enter-------\r\n");
}

static void __code_region_example_on_leave(void *pObj,void *pLocal)
{
    printf("-------leave-------\r\n");
}

const static i_code_region_t __example_code_region = {
    .OnEnter = __code_region_example_on_enter,
    .OnLeave = __code_region_example_on_leave,
};


#if VSF_USE_KERNEL_THREAD_MODE == ENABLED
implement_vsf_thread(user_task_t) 
{

    /*! you can define your own code region and use them */
    static const code_region_t user_region = {
        NULL,
        (i_code_region_t *)&__example_code_region,
    };

    code_region(&user_region){
        printf("\tbody\r\n");
    }

    while (1) {
#if VSF_CFG_TIMER_EN
        vsf_delay(1000);
#endif
        printf("user_thread post user sem:\r\n");
#if VSF_CFG_SYNC_EN
        vsf_sem_post(this.psem);
#endif
    }
}
#endif


#if VSF_CFG_BMPEVT_EN == ENABLED

#if VSF_USE_KERNEL_PT_MODE == ENABLED

static implement_vsf_pt(user_pt_bmpevt_demo_slave_t)
{
    vsf_pt_begin();
    
    vsf_pt_wait_until(
        wait_for_one(this.pgroup_evts, this.mask, -1){
            printf("get sem in pt slave thread\r\n");
        } 
    );

    vsf_pt_wait_until(
        vsf_sem_pend(&user_sem, 100){
            printf("get user sem in pt slave thread\r\n");
        }
        on_sem_timeout() {
            printf("get user sem TIMEOUT pt slave thread\r\n");
        }
    );

    vsf_pt_end();
}



static implement_vsf_pt(user_pt_bmpevt_demo_thread_t)
{
    vsf_pt_begin();

    while (1) {
    
        this.slave.mask = this.mask;
        this.slave.pgroup_evts = this.pgroup_evts;
        vsf_pt_call(user_pt_bmpevt_demo_slave_t, &this.slave);
            vsf_pt_on_call_return(fsm_rt_err) {
                printf("error detected\r\n");
            }
    
        vsf_pt_wait_until(
            wait_for_one(this.pgroup_evts, this.mask, -1){
                printf("get sem in pt master thread\r\n");
            }
        );
        
        vsf_pt_wait_until(
            vsf_sem_pend(&user_sem, 100){
                printf("get user sem in pt master thread\r\n");
            }
            on_sem_timeout() {
                printf("get user sem TIMEOUT pt master thread\r\n");
            }
        );
        
        printf("delay start...\r\n");
        vsf_pt_wait_until(
            vsf_delay(2000) {
                printf("delay completed...\r\n");
            }
        );
    }

    vsf_pt_end();
}
#endif

implement_vsf_task(bmevt_demo_t)
{
    vsf_task_wait_until(
        wait_for_one(&__user_grouped_evts, timer4_evt_msk, -1){
            printf("get timer 4 in eda task\r\n");
            return fsm_rt_yield;         //! do this again
        }
    );
    
    return fsm_rt_wait_for_evt;
}

#endif



int main(void)
{
    static_task_instance(
        features_used(
            mem_sharable( using_grouped_evt; )
            mem_nonsharable(
            )
        )
    )

    class_demo.public_param_base = 100;
    class_demo_init(&class_demo, 1, 2);
    class_demo_get_param(&class_demo);
    class_demo_get_base_param(&class_demo);

    class_demo.public_param_demo = 200;
    class_simple_demo_init(&class_simple_demo, 1, 2);
    class_simple_demo_get_param(&class_simple_demo);
    class_simple_demo_get_base_param(&class_simple_demo);



    extern void stdout_init(void);
    stdout_init();

#if VSF_CFG_SYNC_EN == ENABLED
    // initialize adapter
    do {
        vsf_sem_init(&user_sem, 0);

#   if VSF_CFG_BMPEVT_EN == ENABLED
        init_grouped_evts(user_grouped_evts_t, &__user_grouped_evts, timer4_evt_msk);

        init_vsf_task(bmevt_demo_t, &__bmevt_demo, vsf_priority_0);
#       if VSF_USE_KERNEL_PT_MODE == ENABLED
        do {
            static user_pt_bmpevt_demo_thread_t __pt_demo = {
                .mask = timer4_evt_msk,
                .pgroup_evts = &__user_grouped_evts,
            };
            init_vsf_pt(user_pt_bmpevt_demo_thread_t, &__pt_demo, vsf_priority_inherit);
        } while(0);
#       endif
#   endif
    } while(0);
#endif

    do {
        VSF_POOL_INIT(  user_msg_pool_t, 
                        &user_msg_pool, 
                        16, 
                        &user_msgq, 
                        (code_region_t *)&VSF_SCHED_SAFE_CODE_REGION);
        vsf_eda_queue_init(&user_msgq, 4);
        
        init_vsf_task(  msg_queue_example_t, 
                        &__msg_queue_example_receiver, 
                        vsf_priority_inherit);
        /*
        user_msg_receiver.evthandler = user_msg_receiver_evthandler;
    #if VSF_CFG_EVTQ_EN == ENABLED
        user_msg_receiver.on_terminate = NULL;
    #endif
        vsf_eda_init(&user_msg_receiver, vsf_priority_inherit, false);
        */
        
    } while(0);
    
#if VSF_CFG_TIMER_EN == ENABLED
    for (int i = 0; i < dimof(__timer_example); i++) {
        init_vsf_task(  timer_example_t,            //!< vst_task type
                        &__timer_example[i],        //!< vsf_task object
                        vsf_priority_0,             //!< priority
                        &__timer_example[i]);       //!< target object (pthis)
    }
#endif


#if VSF_USE_KERNEL_THREAD_MODE == ENABLED
    do {
        static NO_INIT user_task_t __user_task;
#   if VSF_CFG_SYNC_EN == ENABLED
        __user_task.psem = &user_sem;
#   endif
        init_vsf_thread(user_task_t, &__user_task, vsf_priority_0);
    } while(0);
#endif



#if VSF_CFG_BMPEVT_EN == ENABLED && VSF_USE_KERNEL_THREAD_MODE == ENABLED
    while (1) {
        wait_for_all(   &__user_grouped_evts, 
                        all_evts_msk_of_user_grouped_evts_t &~timer4_evt_msk,
                        -1) {
            //! when all the grouped events are set
            reset_grouped_evts( &__user_grouped_evts, 
                                all_evts_msk_of_user_grouped_evts_t &~timer4_evt_msk);
            printf("\r\n--------------barrier--------------: \r\n");
        } on_bmevt_timeout() {
            //! when timeout happened
            printf("\r\n============== barrier timeout ============: \r\n");
        }

        vsf_delay(1000);
    }
#endif

}

/* EOF */
