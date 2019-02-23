/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "os/mynewt.h"
#include "console/console.h"
#include "testutil/testutil.h"

#include "runtest/runtest.h"
#include "runtest_priv.h"

#define RT_TASK_STATE_UNUSED   0
#define RT_TASK_STATE_ACTIVE   1
#define RT_TASK_STATE_DONE     2

static struct os_mutex rt_task_mtx;
static struct os_sem rt_task_sem;
static struct os_sem rt_task_block;

struct rt_task {
    os_task_func_t fn;
    struct os_task task;
    uint8_t state;
    char name[sizeof "taskX"];
    OS_TASK_STACK_DEFINE_NOSTATIC(stack, MYNEWT_VAL(RUNTEST_STACK_SIZE));
};

static struct rt_task rt_tasks[MYNEWT_VAL(RUNTEST_NUM_TASKS)];

static void
rt_task_lock(void)
{
    int rc;

    rc = os_mutex_pend(&rt_task_mtx, OS_TIMEOUT_NEVER);
    assert(rc == 0 || rc == OS_NOT_STARTED);
}

static void
rt_task_unlock(void)
{
    int rc;

    rc = os_mutex_release(&rt_task_mtx);
    assert(rc == 0 || rc == OS_NOT_STARTED);
}

static bool
rt_task_locked(void)
{
    struct os_task *owner;

    owner = rt_task_mtx.mu_owner;
    return owner != NULL && owner == os_sched_get_current_task();
}

#define RT_TASK_ASSERT_LOCKED() do \
{\
    if (os_started()) {\
        assert(rt_task_locked()); \
    }\
} while (0)

static int
rt_task_find_state(uint8_t state)
{
    int i;

    RT_TASK_ASSERT_LOCKED();

    for (i = 0; i < MYNEWT_VAL(RUNTEST_NUM_TASKS); i++) {
        if (rt_tasks[i].state == state) {
            return i;
        }
    }

    return -1;
}

static void
rt_task_wrapper(void *arg)
{
    struct rt_task *rt_task;

    rt_task = arg;

    /* Execute wrapped task handler. */
    rt_task->fn(NULL);

    rt_task_lock();

    /* Mark task done. */
    rt_task->state = RT_TASK_STATE_DONE;

    /* If this was the last running task, signal that the test is complete. */
    if (rt_task_find_state(RT_TASK_STATE_ACTIVE) == -1) {
        os_sem_release(&rt_task_sem);
    }

    rt_task_unlock();

    /* Block forever. */
    os_sem_pend(&rt_task_block, OS_WAIT_FOREVER);
}

struct os_task *
rt_task_new(os_task_func_t task_handler, uint8_t prio)
{
    struct rt_task *rt_task;
    int idx;
    int rc;

    rt_task_lock();

    idx = rt_task_find_state(RT_TASK_STATE_UNUSED);
    if (idx != -1) {
        rt_task = &rt_tasks[idx];
        rt_task->state = RT_TASK_STATE_ACTIVE;
    }

    rt_task_unlock();

    if (idx == -1) {
        TEST_ASSERT_FATAL(0, "No more test tasks");
        return NULL;
    }

    strcpy(rt_task->name, "task");
    rt_task->name[4] = '0' + idx;
    rt_task->name[5] = '\0';

    rc = os_task_init(&rt_task->task, rt_task->name, rt_task_wrapper, rt_task,
                      prio, OS_WAIT_FOREVER, rt_task->stack,
                      MYNEWT_VAL(RUNTEST_STACK_SIZE));
    TEST_ASSERT_FATAL(rc == 0);

    return &rt_task->task;
}

void
rt_task_reset(void)
{
    int rc;
    int i;

    rt_task_lock();

    for (i = 0; i < MYNEWT_VAL(RUNTEST_NUM_TASKS); i++) {
        if (rt_tasks[i].state != RT_TASK_STATE_UNUSED) {
            rc = os_task_remove(&rt_tasks[i].task);
            assert(rc == OS_OK);

            rt_tasks[i].state = RT_TASK_STATE_UNUSED;
        }
    }

    rt_task_unlock();
}

void
rt_task_wait(os_time_t max_ticks)
{
    int rc;

    rc = os_sem_pend(&rt_task_sem, max_ticks);
    TEST_ASSERT_FATAL(rc == 0,
                      "timeout waiting for tasks to terminate (%lu ticks)",
                      (unsigned long)max_ticks);
}

/*
 * Package init routine to register newtmgr "run" commands
 */
void
rt_task_init(void)
{
    int rc;
    int i;

    /* Ensure this function only gets called by sysinit. */
    SYSINIT_ASSERT_ACTIVE();

    rc = os_mutex_init(&rt_task_mtx);
    SYSINIT_PANIC_ASSERT(rc == 0);

    rc = os_sem_init(&rt_task_sem, 0);
    SYSINIT_PANIC_ASSERT(rc == 0);

    rc = os_sem_init(&rt_task_block, 0);
    SYSINIT_PANIC_ASSERT(rc == 0);

    for (i = 0; i < MYNEWT_VAL(RUNTEST_NUM_TASKS); i++) {
        rt_tasks[i].state = RT_TASK_STATE_UNUSED;
    }
}
