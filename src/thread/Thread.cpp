/*!
 * ProcessJ::Thread implementation
 *
 * \author: Carlos L. Cuenca
 */

#include<Thread.hpp>
#include<pthread.h> // Remove me
#include<cstring> // remove me
#include<iostream>

/// ----------------------------
/// Static Member Initialization

/*!
 * From the Documentation of clone(3):
 *
 * CLONE_CHILD_CLEAR_TID:
 *      Clear (zero) the child thread ID at the location pointed to by child_tid (clone())
 *      or cl_args.child_tid (clone3()) in child memory when the child exits, and do a wakeup
 *      on the futex at that address. The address involved may be changed by the set_tid_address(2)
 *      system call. This is used by threading libraries.
 *
 * CLONE_CHILD_SETTID:
 *      Store the child thread ID at the location pointed to by child_tid (clone())
 *      or cl_args.child_tid (clone3()) in the child's memory. The store operation
 *      completes before the clone call returns control to user space in the child
 *      process.
 *
 * CLONE_FILES:
 *      The calling process and the child process share the same file descriptor table.
 *      Any file descriptor created by the calling process or by the child process is
 *      also valid in the other process. Similarly, if one of the processes closes
 *      a file descriptor, or changes its associated flags (using fcntl(2) F_SETFD operation)
 *      the other process is also affected. If a process sharing a file descriptor table
 *      calls execve(2), its' file descriptor table is duplicated (unshared)
 *
 * CLONE_FS:
 *      If set, the caller and the child process share the same filesystem information. This includes
 *      the root of the filesystem, the current working directory, and the umask. Any call
 *      to chroot(2) chdir(2), or umask(2) performed by the calling process or the child
 *      process also affects the other process.
 *      If CLONE_FS is not set, thechild process works on a copy of the filesystem information
 *      of the calling process at the time of the clone call. Calls to chroot(2), chdir(2),
 *      or umask(2) performed later by one of the processes do not affect the other processes.
 *
 * CLONE_PARENT:
 *      If set then the parent of the new child (as returned by getppid(2)) will be the same as that
 *      of the calling process. If not set, (as with fork(2)) the child's parent is the calling process
 *      The parent process is signaled when the child terminates
 *
 * CLONE_PARENT_SETTID:
 *      Store the child thread ID at the location pointed to by parent_tid (clone()) or cl_args.parent_tid
 *      in the parent's memory.
 *
 *CLONE_SETTLS:
 *     The Thread Local Storage descriptor is set to tls.
 *
 * CLONE_SIGHAND:
 *      The parent and the child share the same table of signal handlers.
 *
 * CLONE_SYSVSEM:
 *      If set, then the child and the calling process share a single list of System V semaphor adjustment
 *      values
 *
 * CLONE_THREAD:
 *      If set, the child is placed in the same thread group as the calling process. A new
 *      thread created with CLONE_THREAD has the same parent process as the process that made
 *      the clone call (like CLONE_PARENT)
 *CLONE_VM:
 *      The calling process and the thread run in the same memroy space. Memory writes performed by
 *      the calling process or by the child process are also visibile in the other process. Any
 *      memory mapping (with mmap) by the thread or parent also affects the other process
 *      If not set, the thread runs in a separate copy of the memroy space of the parent
 *
 * // Additional notes:
 *
 *  - The thread does not share an I/O context with the parent (i.e. They don't share disk time) [CLONE_IO]
 *  - The thread is created in the same cgroup namespace as parent [CLONE_NEWCGROUP]
 *  - The thread id not created in a new IPC namespace, i.e., it is created in the same IPC namespace as the parent [CLONE_NEWIPC]
 *  - The thread is created in the same network namespace as the parent [CLONE_NEWNET]
 *  - The thread lives in the same mount namespace as the parent [CLONE_NEWNS]
 *  - The thread is created in the same PID namespace as the parent [CLONE_NEWPID]
 *  - The thread is created in the same user namespace as the parent [CLONE_NEWUSER] (Can't be used with CLONE_THREAD, CLONE_PARENT, CLONE_NEWUSER, CLONE_FS)
 *  - The thread is created in the same UTS namespace as the parent [CLONE_NEWUTS]
 *  - The thread does not have the same PID as the parent [CLONE_PID]
 *  - The thread's PID file descriptor referring to the child process is not allocated in the parent's memory [CLONE_PIDFD] (can't be used with CLONE_THREAD)
 *  - The thread is not being traced by default [CLONE_PTRACE]
 *  - The thread can be forced a CLONE_PTRACE from the tracing process [CLONE_UNTRACED]
 *  - The thread and the parent are schedulable after the call (if it is set, then the calling process is suspended until the child releases its virtual memory resources) [CLONE_VFORK]
 */

const ProcessJ::CloneFlags ProcessJ::Thread::CloneFlags =
(CLONE_CHILD_CLEARTID  | CLONE_CHILD_SETTID   | CLONE_FILES   | CLONE_FS      |
 CLONE_PARENT          | CLONE_PARENT_SETTID  | CLONE_SETTLS  | CLONE_SIGHAND |
 CLONE_SYSVSEM         | CLONE_THREAD         | CLONE_VM      | 0);

/*!
 * Default Constructor. Initializes the ProcessJ::Thread
 * to its' default state.
 */

ProcessJ::Thread::Thread():
resumeAddress(0), returnAddress(0), exitAddress(0), observer(0), threadID(0), terminate(false),
swapping(false), waiting(false), started(false), finished(false), suspended(false), stateMutex(), swapMutex() {

    // Invoke Create
    this->create();

}

/*!
 * Initializes the ProcessJ::Thread
 * to its' default state with the given ProcessJ::ThreadObserver.
 * \param observer The ProcessJ::ThreadObserver to bind to the
 * ProcessJ::Thread
 */

ProcessJ::Thread::Thread(ProcessJ::ThreadObserver* observer):
resumeAddress(0), returnAddress(0), exitAddress(0), observer(observer), threadID(0), terminate(false),
swapping(false), waiting(false), started(false), finished(false), suspended(false), stateMutex(), swapMutex() {

    // Invoke Create
    this->create();

}

/*!
 * Deconstructor. Releases any resources used by the ProcessJ::Thread.
 * At the time of writing, no resources are used or released. The deconstructor
 * will make every attempt to gracefully terminate the ProcessJ::Thread
 */

ProcessJ::Thread::~Thread() noexcept {

    // First check if we have an exit address
    if(this->exitAddress) {

        // Suspend the ProcessJ::Thread
        this->suspend();

        // Set the registers

        // Restart the thread so it can exit
        this->restart();

    // No exit address was provided. We will attempt
    // to mark as terminated
    } else {

        ProcessJ::Flag tryTerminate = true;

        do {

            try {

                // Attempt to retrieve the value as long as it's not
                // marked to finish
                tryTerminate = !this->shouldTerminate();

                // If it's false, set the terminate flaf
                if(!tryTerminate) this->setShouldTerminate(true);

                // If we're here, nothing was thrown, so we should
                // Yield to give the thread a chance to terminate
                Yield;

            // Otherwise, some internal state is being resolved.
            } catch(const ProcessJ::Exception exception) {

                // Set the try flag to true and attempt to set it to terminate again
                tryTerminate = true;

            }

        } while(tryTerminate);

    }

    // We want to Yield until the thread has entered its' finished state
    ProcessJ::Flag isFinished = false;

    do {

        {

            // Lock the state mutex
            ProcessJ::Lock<ProcessJ::Mutex> lock(this->stateMutex);

            // Retrieve the value
            isFinished = this->finished;

        }

        // If it hasn't finished, yield
        if(!isFinished) Yield;

    // Loop to try again
    } while(!isFinished);

    // The thread should be finished now.
    // Clear out the thread state;
    this->resumeAddress = 0     ;
    this->returnAddress = 0     ;
    this->exitAddress   = 0     ;
    this->observer      = 0     ;
    this->threadID      = 0     ;
    this->terminate     = false ;
    this->swapping      = false ;
    this->waiting       = false ;
    this->started       = false ;
    this->finished      = false ;
    this->suspended     = false ;

}

/*!
 * Returns the value of the ProcessJ::Thread's resume address and clears
 * the ProcessJ::Thread's field before returning the value.
 * If this method is invoked while the ProcessJ::Thread's link data is being
 * swapped, it will throw a ProcessJ::Thread::ThreadIsSwappingException.
 * \return void pointer representing the address of the next
 * instruction to execute.
 */

void* ProcessJ::Thread::getResumeAddress() {

    void* resumeAddress = 0;

    {

        // Lock the swap mutex
        ProcessJ::Lock<ProcessJ::Mutex> lock(this->swapMutex);

        // If the ProcessJ::Thread is being swapped, throw a
        // ProcessJ::Thread::ThreadIsSwappingException.
        if(this->swapping) throw ProcessJ::Thread::ThreadIsSwappingException();

        // Assign it
        resumeAddress = this->resumeAddress;

        // Clear it out. In case we're in ProcessJ::Thread::Execution,
        // we don't want to loop back around and re-execute
        this->resumeAddress = 0;

    }

    // Return it
    return resumeAddress;

}

/*!
 * Returns the value of the ProcessJ::Thread's return address.
 * If this method is invoked while the ProcessJ::Thread's link data is
 * being swapped, it will throw a ProcessJ::Thread::ThreadIsSwappingException.
 * \return void pointer representing the address the ProcessJ::Thread
 * should return to after executing code.
 */

void* ProcessJ::Thread::getReturnAddress() {

    void* returnAddress = 0;

    {

        // Lock the swap mutex
        ProcessJ::Lock<ProcessJ::Mutex> lock(this->swapMutex);

        // If the ProcessJ::Thread is being swapped, throw a
        // ProcessJ::Thread::ThreadIsSwappingException.
        if(this->swapping) throw ProcessJ::Thread::ThreadIsSwappingException();

        // Assign it
        returnAddress = this->returnAddress;

    }

    // Return it
    return returnAddress;

}


/*!
 * Returns a flag denoting if the ProcessJ::Thread should terminate
 * after execution of the current code, i.e. when it returns to
 * the ProcessJ::Thread::Execute function. By default, this is set
 * to false. If this method is invoked while the ProcessJ::Thread's link data is
 * being swapped, it will throw a ProcessJ::Thread::ThreadIsSwappingException.
 * \return Flag denoting if the ProcessJ::Thread should terminate.
 */

ProcessJ::Flag ProcessJ::Thread::shouldTerminate(){

    ProcessJ::Flag shouldTerminate = false;

    {

        // Lock the swap mutex
        ProcessJ::Lock<ProcessJ::Mutex> lock(this->swapMutex);

        // If the ProcessJ::Thread is being swapped, throw a
        // ProcessJ::Thread::ThreadIsSwappingException.
        if(this->swapping) throw ProcessJ::Thread::ThreadIsSwappingException();

        // Assign it
        shouldTerminate = this->terminate;

    }

    // Return the value
    return shouldTerminate;

}

/*!
 * Sets the terminate flag denoting if the ProcessJ::Thread should
 * terminate after execution of the current code, i.e. when it returns
 * to the ProcessJ::Thread::Execute function. By default, this is set
 * to false. If attempting to set the terminate flag while the ProcessJ::Thread
 * link data is being swapped, this will throw a ProcessJ::Thread::ThreadIsSwappingException.
 * The terminate flag can be set in any lifecycle state.
 * \param shouldTerminate Flag denoting if the Thread should terminate
 * after executing its' latest code.
 */

void ProcessJ::Thread::setShouldTerminate(ProcessJ::Flag shouldTerminate) {

    // Lock the swap mutex
    ProcessJ::Lock<ProcessJ::Mutex> swapLock(this->swapMutex);

    // Check if it's swapping
    if(this->swapping) throw ProcessJ::Thread::ThreadIsSwappingException();

    // Lock the state mutex here
    ProcessJ::Lock<ProcessJ::Mutex> stateLock(this->stateMutex);

    // If the thread is finished, throw a
    // ProcessJ::Thread::ThreadFinishedException
    if(this->finished) throw ProcessJ::Thread::ThreadFinishedException();

    // Otherwise, it's either in the waiting or started state. Suspend it.
    this->suspend();

    // Set the flag
    this->terminate = shouldTerminate;

    // Restart
    this->restart();

}

/*!
 * Swaps the ProcessJ::Thread's resume and return
 * address and returns the previous resum and return address
 * as contained in a ProcessJ::Thread::Link instance. This
 * method interrupts the ProcessJ::Thread, preserves the current
 * state of the pertinent data, sets the new data, and returns
 * a copy of the old data. Important. This method should be invoked
 * only by another thread, never the thread that corresponds with
 * the ProcessJ::Thread instance otherwise a ProcessJ::Thread::ThreadSwapSelf
 * exception is thrown. If this method is invoked while the ProcessJ::Thread
 * is in its' finished state, it will throw a ProcessJ::Thread::ThreadFinishedException.
 * \param link The pair of resume and return addresses to set
 * \return ProcessJ::Thread::Link instance containing the old
 * resume and return addresses.
 */

ProcessJ::Thread::Link ProcessJ::Thread::swap(const ProcessJ::Thread::Link& link) {

    // Lock the state mutex for the entire scope
    ProcessJ::Lock<ProcessJ::Mutex> stateLock(this->stateMutex);

    // Check if the process is finished
    if(this->finished) throw ProcessJ::Thread::ThreadFinishedException();

    // Check thread id against the instances thread id here
    // If the thread id's match, throw a ProcessJ::Thread::ThreadSwapSelfException
    if(this->threadID == GetProcessId) throw ProcessJ::Thread::ThreadSwapSelfException();

    {

        // Lock the swap mutex; unlocks when we exit the
        // anonymous scope
        ProcessJ::Lock<ProcessJ::Mutex> swapLock(this->swapMutex);

        // Set the swap flag
        this->swapping = true;

    }

    // Otherwise, Stop the process before we adjust anything
    this->suspend();

    // Create a new ProcessJ::Thread::Link instance
    struct ProcessJ::Thread::Link old;

    // Retrieve the current data
    old.resumeAddress   = this->resumeAddress   ;
    old.returnAddress   = this->returnAddress   ;
    old.terminate       = this->terminate       ;

    // If the thread is not executing, set the resume address
    if(!this->started)
        this->resumeAddress = link.resumeAddress        ;

    // Otherwise, modify the registers directly
    else {

        // Clear the resume address
        this->resumeAddress = 0;

        // Set the registers

    }

    // Set the current data
    this->returnAddress     = link.returnAddress        ;
    this->terminate         = link.terminate            ;

    // Restart the thread
    this->restart();

    {

        // Lock the swap mutex; unlocks when we exit the
        // anonymous scope
        ProcessJ::Lock<ProcessJ::Mutex> lock(this->swapMutex);

        // Reset the swap flag
        this->swapping = false;

    }

    // Finally, return the old data
    return old;

}

/*!
 * Suspends the thread. This method should be invoked by another
 * thread. Once the ProcessJ::Thread is suspended, it will notify
 * the ProcessJ::ThreadObserver of the suspended state.
 * If the thread that corresponds with this object invokes this method,
 * this method will throw a ProcessJ::Thread::ThreadSuspendSelfException.
 */

void ProcessJ::Thread::suspend() {

    // We don't want to block the managing thread, so just return
    // if it's already suspended
    if(this->suspended) return;

    // Check if the thread is calling this itself
    if(this->threadID == GetProcessId)
        throw ProcessJ::Thread::ThreadSuspendSelfException();

    // Stop the thread
    StopProcess(this->threadID);

    // Set the suspended state
    this->suspended = true;

    // Notify the ProcessJ::ThreadObserver if there is one registered
    if(this->observer) this->observer->OnSuspended(reinterpret_cast<void*>(this));

}

/*!
 * Attempts to continue the execution of the ProcessJ::Thread
 * if it's in a suspended state. If the ProcessJ::Thread is
 * restarted, it will notify the ProcessJ::ThreadObserver of
 * the state mutation. If the ProcessJ::Thread is not suspended,
 * leave
 */

void ProcessJ::Thread::restart() {

    // If we're not suspended, just leave
    if(!this->suspended) return;

    // Otherwise, restart
    ptrace(PTRACE_CONT, this->threadID, 0, 0);

    // Clear the suspended state
    this->suspended = false;

    // And notify the ProcessJ::ThreadObserver if there is one registered.
    if(this->observer) this->observer->OnRestart(reinterpret_cast<void*>(this));

}

/*!
 * The execution method of the ProcessJ::Thread. If the ProcessJ::Thread
 * was given a resume address at construction time, then the ProcessJ::Thread
 * executes the code immediately and notifies any ProcessJ::ThreadObserver that
 * it has started. Otherwise, it yields until a resume address is set so other
 * threads may get cpu time. This
 * \return Status code
 */

void*/*ProcessJ::ReturnCode*/ ProcessJ::Thread::Execution(void* arg_thread) {

    std::cout << "Entered Worker" << std::endl;
    std::cout << "Requesting parent trace" << std::endl;
    // First we send a trace request; If this fails, throw an error
    if(ptrace(PTRACE_TRACEME, GetProcessId, 0, 0) < 0) {

        // Print the error
        perror("ProcessJ Error");

        // Throw the exception
        throw ProcessJ::Thread::ProcessAttachFailureException();

    }
    std::cout << "Success. Stopping Thread" << std::endl;
    // Stop this process so the tracer process can get the acknowledgement
    //StopThisProcess;
    std::cout << "Attempting to retrieve thread" << std::endl;
    // We came back from a suspension, first we retrieve the ProcessJ::Thread
    // pointer that was given to us by the parent placed in the stack.
    ProcessJ::Thread* thread = (ProcessJ::Thread*) arg_thread;
    std::cout << "How are we here?" << std::endl;
    // The argument we retrieve from our tiny stack should not be null
    // Otherwise, we throw this exception.
    if(!thread)
        throw ProcessJ::Thread::ThreadHandleNullException();
    std::cout << "No, really, how are we here? " << std::endl;
    // We then notify the ProcessJRuntime::ThreadObserver
    // that the thread has been created.
    if(thread->observer) thread->observer->OnCreated(reinterpret_cast<void*>(thread));

    // We will store the return address here
    void* resumeAddress = 0;
    ProcessJ::Flag shouldTerminate = false;
    std::cout << "Entering Waiting State" << std::endl;
WAIT:

    {

        // Lock the state mutex
        ProcessJ::Lock<ProcessJ::Mutex> lock(thread->stateMutex);

        // Mark the thread as waiting and reset the started state
        thread->waiting = true  ;
        thread->started = false ;

    }

    // Notify the ProcessJRuntime::ThreadObserver that the thread
    // has entered a waiting state.
    if(thread->observer) thread->observer->OnWaiting(reinterpret_cast<void*>(thread));

    // We use this flag to determine breaking out of the waiting loop
    ProcessJ::Flag shouldWait = true;

    do {

        // We're going to attempt to grab values from critical sections
        try {

            // If the resume address is null, or the thread should not terminate
            shouldWait = !(resumeAddress = thread->getResumeAddress()) &&
                         !(shouldTerminate = thread->shouldTerminate());

            std::cout << "Thread in waiting state; shouldWait = " << shouldWait << " yielding" << std::endl;
            // We yield if the values reflected a wait.
            if(shouldWait) Yield;

        // Otherwise, some intermediate state is being resolved.
        } catch(const ProcessJ::Exception& exception) {

            // Set the wait flag
            shouldWait = true;

            // And yield
            Yield;

        }

    // Check if we should continue waiting.
    } while(shouldWait);

    std::cout << "Thread has a resume address, or it should terminate" << std::endl;
    std::cout << "Resume Address: " << resumeAddress << std::endl;
    std::cout << "Should Terminate: " << shouldTerminate << std::endl;

    // If the ProcessJ::Thread doesn't have an address we must have gotten a terminate signal.
    // Jump to the exit, we don't even want to notify the ProcessJ::ThreadObserver
    // that we started since we didn't.
    if(!resumeAddress) goto EXIT;

    std::cout << "Transitioning to started state" << std::endl;

    {

        // The thread is about to execute code, update the state
        ProcessJ::Lock<ProcessJ::Mutex> lock(thread->stateMutex);

        // Reset the waiting state and set the started state
        thread->waiting = false ;
        thread->started = true  ;

    }

    // Otherwise, check the terminate flag when setting the ProcessJ::Thread's
    // return address.
    // There's some information that discourages the use of jumping to the
    // Address of labels for portability reasons. This case might be an
    // exception.
    thread->setReturnAddress((shouldTerminate) ? &&EXIT : &&WAIT);

    std::cout << "Return Address set to: " << thread->getReturnAddress() << std::endl;
    std::cout << "Wait Location: " << &&WAIT << std::endl;
    std::cout << "Exit Location: " << &&EXIT << std::endl;

    // If we're here, we are about to start, so notify the ProcessJ::ThreadObserver
    if(thread->observer) thread->observer->OnStarted(reinterpret_cast<void*>(thread));

    std::cout << "Attempting to Jump to " << std::hex << resumeAddress << std::endl;

    // x86-64 Assembly, we want to change this eventually depending on the
    // architecture the compilation is for.

    //__asm__ __volatile__ (
    //    ".intel_syntax\n"
   //     "jmp qword[%0]\n"
    //    ".att_syntax  \n"
        //"%0           "
    //    :: "r" (resumeAddress) );

    __asm__ __volatile__ (

                          ".intel_syntax                \n"
                          "push %[returnAddress]        \n"
                          "push %[address]              \n"
                          "ret                          \n"
                          ".att_syntax                  \n"
                          :: [address] "r" (resumeAddress),
                             [returnAddress] "r" (thread->getReturnAddress()));

EXIT:

    std::cout << "Terminating thread" << std::endl;

    {

        // Lock the state mutex
        ProcessJ::Lock<ProcessJ::Mutex> lock(thread->stateMutex);

        // Mark the thread as finished and reset the started & waiting state.
        thread->waiting  = false    ;
        thread->started  = false    ;
        thread->finished = true     ;

    }

    // Finally, if we're here, we let the ProcessJ::ThreadObserver that
    // the thread has finished.
    if(thread->observer) thread->observer->OnFinished(reinterpret_cast<void*>(thread));

    // Return success
    return arg_thread;

}

#define THREAD_CREATE_JOINABLE 0
#define THREAD_CREATE_DETACHED 1

/*!
 * Creates the thread of execution and notifies any ProcessJ::ThreadObserver
 * that it has been created from the child process. The execution of the
 * ProcessJ::Thread begins in ProcessJ::Thread::Execution. If the creation
 * of a new thread fails, then a ProcessJ::Thread::ThreadCreateFailureException
 * is thrown.
 */

void ProcessJ::Thread::create() {

    // Declare the stack address
    //void* stackAddress = 0;
    std::cout << "Creating Thread" << std::endl;
    // This was originally set to the ThreadAttributeUnion
    const struct ProcessJ::ThreadAttribute* threadAttributes = (struct ProcessJ::ThreadAttribute*) 0;
    struct ProcessJ::ThreadAttribute defaultAttributes;

    ProcessJ::Flag freeCPUSet   = false;
    ProcessJ::Flag C11          = false;

    // Declare the user register structs first
    // so we don't hose our locals
    struct { uint64_t regs[32]; } user_regs;

    // We are passing this to the ptrace request
    struct {

        void*       base        ;
        uint32_t    length      ;

    } registers = { &user_regs, sizeof(user_regs.regs) };

    pthread_t thread;

    pthread_create(&thread, NULL, &ProcessJ::Thread::Execution, (void*) this);

    uint64_t threadId = 0;
    std::cout << "Retrieving Thread Id" << std::endl;
    memcpy(&threadId, &thread, sizeof(threadId) < sizeof(thread) ? sizeof(threadId) : sizeof(thread));

    this->threadID = threadId;
    std::cout << "Thread ID: " << this->threadID << std::endl;

    // Attempt to create a new thread. If the invocation fails, throw a
    // ProcessJ::Thread::ThreadCreateFailureException
    //if(Expect((this->threadID = clone(&ProcessJ::Thread::Execution, , ProcessJ::Thread::CloneFlags, )) == -1, 0))
        //throw ProcessJ::Thread::ThreadCreateFailureException();

    // To hold our status
    ProcessJ::Status status;
    std::cout << "Attempting to trace thread. Waiting" << std::endl;
    // Since we're tracing the process (thread) we wait for the new
    // process to signal the kernel.
    wait(0);
    //waitpid(-1, &status, WUNTRACED);
    std::cout << "Thread trace success!" << std::endl;
    std::cout << "Retrieving RegSet" << std::endl;
    // Retrieve the contents of the registers
    ptrace(PTRACE_GETREGSET, this->threadID, 1, &registers);
    std::cout << "Regset Retrieved." << std::endl;
    // Set thread address to the register
    user_regs.regs[20] = (uint64_t) reinterpret_cast<void*>(this);
    std::cout << "Setting Registers" << std::endl;
    // Set the modified register contents
    ptrace(PTRACE_SETREGSET, this->threadID, 1, &registers);
    std::cout << "Registers set, resuming process" << std::endl;
    // If we're here, the kernel responded. Continue the worker
    ptrace(PTRACE_CONT, this->threadID, 0, 0);
    std::cout << "Leaving" << std::endl;
}
