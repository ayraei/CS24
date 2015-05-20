#
# Keep a pointer to the main scheduler context.
# This variable should be initialized to %esp;
# which is done in the __sthread_start routine.
#
        .data
        .align 4
scheduler_context:      .long   0

#
# __sthread_schedule is the main entry point for the thread
# scheduler.  It has three parts:
#
#    1. Save the context of the current thread on the stack.
#       The context includes only the integer registers
#       and EFLAGS.
#    2. Call __sthread_scheduler (the C scheduler function),
#       passing the context as an argument.  The scheduler
#       stack *must* be restored by setting %esp to the
#       scheduler_context before __sthread_scheduler is
#       called.
#    3. __sthread_scheduler will return the context of
#       a new thread.  Restore the context, and return
#       to the thread.
#
        .text
        .align 4
        .globl __sthread_schedule
__sthread_schedule:

        # Save the process state onto its stack
        pushfl  # Pushes eflags
        pusha   # Pushes all general-purpose registers.

        # Call the high-level scheduler with the current context as an argument
        movl    %esp, %eax
        movl    scheduler_context, %esp
        pushl   %eax
        call    __sthread_scheduler

        # The scheduler will return a context to start.
        # Restore the context to resume the thread.
__sthread_restore:

        movl %eax, %esp     # Restore the stack pointer.
        popa                # Restore general-purpose registers
        popfl               # Restore eflags

        ret

#
# Initialize a process context, given:
#    1. the stack for the process
#    2. the function to start
#    3. its argument
# The context should be consistent with the context
# saved in the __sthread_schedule routine.
#
# A pointer to the newly initialized context is returned to the caller.
# (This is the stack-pointer after the context has been set up.)
#
# This function is described in more detail in sthread.c.
#
#
        .globl __sthread_initialize_context
__sthread_initialize_context:

        movl 4(%esp), %eax      # Store process's stack pointer to eax.
        movl 8(%esp), %ecx      # Store the function to start in ecx.
        movl 12(%esp), %edx     # Store the function's arg to edx.
        
        # Now we need to set up the thread's context.
        # thread_schedule pushes 8 registers and eflags; each is 4 bytes.
        # So we set aside (8+1)*4 = 36 bytes on the stack for that.
        # Then, we need to put the function to execute, its return address,
        # and its argument on the stack.
        movl %ecx, 36(%eax)     # The function to execute
        movl $__sthread_finish, 40(%eax)    # The return address
        movl %edx, 44(%eax)     # The function's argument
        
        # Return value: eax = stack pointer

        ret

#
# The start routine initializes the scheduler_context variable,
# and calls the __sthread_scheduler with a NULL context.
# The scheduler will return a context to resume.
#
        .globl __sthread_start
__sthread_start:
        # Remember the context
        movl    %esp, scheduler_context

        # Call the scheduler with no context
        pushl   $0
        call    __sthread_scheduler

        # Restore the context returned by the scheduler
        jmp     __sthread_restore
