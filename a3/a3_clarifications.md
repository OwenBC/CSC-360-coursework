# CSC360 A3 clarifications - July 8

## Pre-emption:

The current task (the one in current_task) may be _pre-empted_.  If the scheduler discovers that there is a task on a higher-priority queue than the current task, it must pre-empt the current task.  The current task is enqueued back on to the queue from which it was taken, and the higher-priority task becomes the current task.

- Example 1: Task A is taken from Queue 2 (q=4) and made the current task.  The next tick, Task B arrives in the top-priority Queue 1 (q=2).  The scheduler must take task A and enqueue it on to Queue 2, and make task B the current task.
- Example 2: Task C is taken from Queue 1 (q=2) and made the current task.  The next tick, Task D arrives in the top-priority Queue 1 (q=2).  Since task C was from Queue 1, and task D was from Queue 1, there is no pre-emption, and nothing needs to be done.

## Boost:

- The mlfq.c file comments describe the expected behaviour for Tasks on the Queues.
- The current task should be dealt with in the following way:  It should be treated as if it had been dequeued from Queue 1 (it's queue data field should be updated accordingly).  Its remaining quantum should be reduced to 2 if it is currently higher than 2, or left alone if it is 2 or less.  Since the current_task has its queue set to 1, there is no way for it to be pre-empted by any other task during the boost.
  - Example 3: Current Task E was taken from Queue 3 and has a remaining quantum of 5.  The boost occurs.  Task E's queue variable is updated to 1, and its remaining quantum is set to 2.
  - Example 4: Current Task F was taken from Queue 3 and has a remaining quantum of 1.  The boost occurs.  Task F's queue variable is updated to 1, and it's remaining quantum remains at 1.
- Tasks that are neither on a queue, nor are the current task are not affected by the boost at all.
  - Example 5: Task G is not the current task, and is not on a queue - it exists only on the task_table.  The boost occurs.  None of Task G's variable values are changed - in particular, its current_queue variable is unchanged.

## Queue

- Due to a bug in the queue code we've given you, queue_size() has some... issues (Task_t's next data member is not explicitly initialized to null).  As a result, queue_size() doesn't work properly for queues with 3 or more Tasks.  This doesn't affect you at all, unless you're calling queue_size() in your own code and expecting it to work.  You shouldn't need to call queue_size() at all - you can complete the boost() function simply by dequeuing over and over and checking is_empty() instead.  My solution does not call queue_size().  Sorry for the error.

