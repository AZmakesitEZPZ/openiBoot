#include "openiboot.h"
#include "openiboot-asmhelpers.h"
#include "tasks.h"
#include "event.h"
#include "clock.h"
#include "util.h"
#include "timer.h"

static uint64_t startTime = 0;

const TaskDescriptor bootstrapTaskInit = {
	TaskDescriptorIdentifier1,
	{0, 0},
	{0, 0},
	TASK_RUNNING,
	1,
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{{0, 0}, 0, 0, 0, 0},
	{0, 0},
	0,
	0,
	0,
	0,
	0,
	"bootstrap",
	TaskDescriptorIdentifier2
	};

const TaskDescriptor irqTaskInit = {
	TaskDescriptorIdentifier1,
	{0, 0},
	{0, 0},
	TASK_RUNNING,
	1,
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{{0, 0}, 0, 0, 0, 0},
	{0, 0},
	0,
	0,
	0,
	0,
	0,
	"irq",
	TaskDescriptorIdentifier2
	};

static TaskDescriptor bootstrapTask;
static TaskDescriptor irqTask;

TaskDescriptor *IRQTask = &irqTask;
TaskDescriptor *IRQBackupTask = NULL;

static void task_add_after(TaskDescriptor *_a, TaskDescriptor *_b)
{
	EnterCriticalSection();

	TaskDescriptor *prev = _b;
	TaskDescriptor *next = _b->taskList.next;

	prev->taskList.next = _a;
	_a->taskList.prev = prev;

	next->taskList.prev = _a;
	_a->taskList.next = next;

	LeaveCriticalSection();
}

static void task_add_before(TaskDescriptor *_a, TaskDescriptor *_b)
{
	EnterCriticalSection();

	TaskDescriptor *prev = _b->taskList.prev;
	TaskDescriptor *next = _b;

	prev->taskList.next = _a;
	_a->taskList.prev = prev;

	next->taskList.prev = _a;
	_a->taskList.next = next;

	LeaveCriticalSection();
}

static void task_remove(TaskDescriptor *_t)
{
	EnterCriticalSection();

	TaskDescriptor *prev = _t->taskList.prev;
	TaskDescriptor *next = _t->taskList.next;

	prev->taskList.next = next;
	next->taskList.prev = prev;

	LeaveCriticalSection();
}

int tasks_setup()
{
	memcpy(&bootstrapTask, &bootstrapTaskInit, sizeof(TaskDescriptor));
	memcpy(&irqTask, &irqTaskInit, sizeof(TaskDescriptor));
	bootstrapTask.taskList.next = &bootstrapTask;
	bootstrapTask.taskList.prev = &bootstrapTask;
	CurrentRunning = &bootstrapTask;
	return 0;
}

void task_init(TaskDescriptor *_td, char *_name)
{
	memset(_td, 0, sizeof(TaskDescriptor));
	_td->identifier1 = TaskDescriptorIdentifier1;
	_td->identifier2 = TaskDescriptorIdentifier2;
	_td->state = TASK_STOPPED;
	strcpy(_td->taskName, _name);
	_td->storage = malloc(TASK_STACK_SIZE);
	_td->storageSize = TASK_STACK_SIZE;

	bufferPrintf("tasks: Initialized %s.\n", _td->taskName);
}

void task_destroy(TaskDescriptor *_td)
{
	if(_td->storage)
		free(_td->storage);
}

void task_run(void (*_fn)(void*), void *_arg)
{
	LeaveCriticalSection();

	//bufferPrintf("tasks: New task started %s. 0x%08x(0x%08x).\r\n",
	//		CurrentRunning->taskName, _fn, _arg);

	_fn(_arg);

	//bufferPrintf("tasks: Task ending %s. 0x%08x(0x%08x).\r\n",
	//		CurrentRunning->taskName, _fn, _arg);

	EnterCriticalSection();
	task_stop();
}

int task_start(TaskDescriptor *_td, void *_fn, void *_arg)
{
	EnterCriticalSection();

	if(_td->state == TASK_STOPPED)
	{
		_td->criticalSectionNestCount = 1;

		_td->savedRegisters.r4 = (uint32_t)_fn;
		_td->savedRegisters.r5 = (uint32_t)_arg;
		_td->savedRegisters.lr = (uint32_t)&StartTask;
		_td->savedRegisters.sp = (uint32_t)(_td->storage + _td->storageSize);

		_td->state = TASK_RUNNING;

		if(CurrentRunning == IRQTask)
			task_add_after(_td, IRQBackupTask);
		else
		{
			task_add_before(_td, CurrentRunning);
			SwapTask(_td);
		}

		LeaveCriticalSection();

		//bufferPrintf("tasks: Added %s to tasks.\n", _td->taskName);
		return 1;
	}
		
	LeaveCriticalSection();

	return 0;
}

void task_stop()
{
	EnterCriticalSection();

	if(CurrentRunning == IRQTask)
	{
		LeaveCriticalSection();

		bufferPrintf("tasks: Cannot stop IRQ task.\r\n");
		return;
	}

	TaskDescriptor *next = CurrentRunning->taskList.next;
	if(next == CurrentRunning)
	{
		LeaveCriticalSection();
		bufferPrintf("tasks: Cannot stop last task! Expect hell to break loose now!\n");
		return;
	}

	// Remove us from the queue
	CurrentRunning->state = TASK_STOPPED;
	task_remove(CurrentRunning);

	//bufferPrintf("tasks: %s died, swapping to %s.\r\n",
	//		CurrentRunning->taskName, next->taskName);

	// Swap onto next task
	SwapTask(next);
	
	// Can't ever reach here, code will continue in task_yield after
	// the call to SwapTask... -- Ricky26
}

int task_yield()
{
	EnterCriticalSection();
	if(CurrentRunning == IRQTask)
	{
		LeaveCriticalSection();

		bufferPrintf("tasks: Can't swap during ISR!\r\n");
		return -1;
	}

	TaskDescriptor *next = CurrentRunning->taskList.next;
	if(next != CurrentRunning)
	{
		// We have another thread to schedule! -- Ricky26
		
		//bufferPrintf("tasks: Swapping from %s to %s.\n", CurrentRunning->taskName, next->taskName);
		SwapTask(next);
		//bufferPrintf("tasks: Swapped from %s to %s.\n", CurrentRunning->taskName, next->taskName);

		LeaveCriticalSection();
		return 1;
	}

	LeaveCriticalSection();
	return 0; // didn't yield
}

void tasks_run()
{
	while(1)
	{
		if(!task_yield())
		{
			//bufferPrintf("WFI\r\n");
			WaitForInterrupt(); // Nothing to do, wait for an interrupt.
		}
	}
}

void task_wake_event(Event *_evt, void *_obj)
{
	TaskDescriptor *task = _obj;
	//bufferPrintf("tasks: Resuming sleeping task %p.\n", task);
	
	if(CurrentRunning == IRQTask)
		task_add_before(task, IRQBackupTask);
	else
		task_add_before(task, CurrentRunning); // Add us back to the queue.
}

int task_sleep(int _ms)
{
	EnterCriticalSection();

	if(CurrentRunning == IRQTask)
	{
		LeaveCriticalSection();

		bufferPrintf("tasks: You can't suspend an ISR.\r\n");
		return -1;
	}

	TaskDescriptor *next = CurrentRunning->taskList.next;
	if(next == CurrentRunning)
	{
		LeaveCriticalSection();

		if (!startTime) {
			bufferPrintf("tasks: Last thread cannot sleep!\n");
			startTime = timer_get_system_microtime();
		}

		if (!has_elapsed(startTime, (_ms * 1000)))
			task_sleep(_ms);

		startTime = 0;

		return 0;
	}

	uint32_t ticks = _ms * 1000;
	TaskDescriptor *task = CurrentRunning;

	//bufferPrintf("tasks: Putting task %p to sleep for %d ms.\n", task, _ms);
	task_remove(task);
	event_add(&task->sleepEvent, ticks, &task_wake_event, task);
	SwapTask(next);

	LeaveCriticalSection();

	return 0;
}

void task_suspend()
{
	EnterCriticalSection();

	if(CurrentRunning == IRQTask)
	{
		LeaveCriticalSection();

		bufferPrintf("tasks: You can't suspend an ISR.\r\n");
		return;
	}

	TaskDescriptor *next = CurrentRunning->taskList.next;
	if(next == CurrentRunning)
	{
		LeaveCriticalSection();

		bufferPrintf("tasks: Last task cannot be suspended!\r\n");
		return;
	}

	task_remove(CurrentRunning);
	SwapTask(next);
	LeaveCriticalSection();
}

void task_wait()
{
	CurrentRunning->wasWoken = 0;
	task_suspend();
}

int task_wait_timeout(int _ms)
{
	CurrentRunning->wasWoken = 0;
	task_sleep(_ms);

	return CurrentRunning->wasWoken;
}

void task_wake(TaskDescriptor *_task)
{
	EnterCriticalSection();
	_task->wasWoken = 1;
	task_add_after(_task, CurrentRunning);
	SwapTask(_task);
	LeaveCriticalSection();
}
