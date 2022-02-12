/*!
 * Opal::Saboteur implementation
 *
 * \author: Carlos L. Cuenca
 */

#include<Saboteur.hpp>

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
 *     The Saboteur Local Storage descriptor is set to tls.
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

/// ------------------------------
/// Static Variable Initialization

const Opal::CloneFlags Opal::Saboteur::CloneFlags =
(CLONE_CHILD_CLEARTID  | CLONE_CHILD_SETTID   | CLONE_FILES   | CLONE_FS      |
 CLONE_PARENT          | CLONE_PARENT_SETTID  | CLONE_SETTLS  | CLONE_SIGHAND |
 CLONE_SYSVSEM         | CLONE_THREAD         | CLONE_VM      | 0);

/// ------------
/// Constructors

/*!
 * Default Constructor. Initializes the Opal::Saboteur
 * to its' default state.
 */

Opal::Saboteur::Saboteur():
executionAddress(0), observer(0), threadID(0),
state(0), stack(), stackSize(), stateMutex() {

    //this->stack[513] = reinterpret_cast<uint64_t>(this)     ;
    //this->stack[512] = reinterpret_cast<uint64_t>(observer) ;

    std::cout << "Stack: " << this->stack << std::endl;

    this->stop = &kill; Lifecycle(this); }

/*!
 * Initializes the Opal::Saboteur
 * to its' default state with the given Opal::SaboteurObserver.
 * \param observer The Opal::SaboteurObserver to bind to the
 * Opal::Saboteur
 */

Opal::Saboteur::Saboteur(Opal::SaboteurObserver* observer):
executionAddress(0), observer(observer), threadID(0),
state(0), stack(), stackSize(), stateMutex() {

    this->stop = &kill;

     Lifecycle(this);

}

/*!
 * Deconstructor. Releases any resources used by the Opal::Saboteur.
 * At the time of writing, no resources are used or released. The deconstructor
 * will make every attempt to gracefully terminate the Opal::Saboteur
 */

Opal::Saboteur::~Saboteur() {

    // Set the Opal::Saboteur's state to terminated if it's not attached
    //if(!isIn(ATTACHED)) setStateTo(TERMINATE);

    // Relinquish the remaining cpu time as long as
    // the thread has not terminated.
    while(!isIn(TERMINATED)) Yield;

   // while(true) Yield;

    std::cout << "Why" << std::endl;
    // Clear out the thread state
    executionAddress  = 0     ;
    observer          = 0     ;
    threadID          = 0     ;
    state             = 0     ;

}

/// ------------------------
/// Private Static Functions

/*!
 * Resumes the thread on the operating system level.
 * If the thread resumption fails, this function will throw a
 * Opal::Saboteur::SaboteurResumeFailureException. If the given
 * handle to the Opal::Saboteur is null, this function will throw
 * a Opal::Saboteur::LostSaboteurException.
 */

void Opal::Saboteur::Resume(Opal::Saboteur* thread) {

    // If we're not in a suspended state, leave with no error
    //if(!thread->isIn(SUSPENDED)) return;

    // Set the Opal::Saboteur's state.
    thread->setStateTo(RESUMING);

    // Attempt to resume the thread, if it fails, throw an exception.
    if(ptrace(PTRACE_CONT, thread->threadID, 0, 0));
        //throw Opal::Saboteur::SaboteurResumeFailureException();

}

/*!
 * Stops the thread represented by the Opal::Saboteur.
 * If the suspension fails, this function will throw a
 * Opal::Saboteur::SaboteurSuspendFailureException. If the given
 * handle to the Opal::Saboteur is null, this function will
 * throw a Opal::Saboteur::LostSaboteurException.
 */

void Opal::Saboteur::Suspend(Opal::Saboteur* thread) {

    // If we're already suspended, leave with no error
    if(thread->isIn(SUSPENDED)) return;

    // Check for a self suspend
    //thread->checkErrorState(SELF_SUSPEND);

    //kill(thread->threadID, SIGSTOP);

    // Set the state
    thread->setStateTo(SUSPENDED);

}

struct Registers {

    void*       base    ;
    uint32_t    length  ;

};

// Note: RIP on the test machine was index 4 (register 5), rbp was index 6 (register 7)
//
void* Opal::Saboteur::RegistersOf(Opal::Saboteur* thread) {

    // Leave if the Opal::Saboteur is null
    if(!thread) return 0;

    uint64_t* user_regs = new uint64_t[32];

    struct Registers* registers = new Registers { &user_regs, sizeof(user_regs) };

    std::cout << "Retrieving Register Set" << std::endl;
    std::cout << "Stopping Process" << std::endl;

    // There's no guarantee the Opal::Saboteur is suspended, so
    // we attempt an invocation.
    Suspend(thread);

    // Retrieve the contents of the registers
    ptrace(PTRACE_GETREGSET, thread->threadID, 1, registers);

    std::cout << "Registers Retrieved. Returning register contents." << std::endl;

    // Return the contents
    return (void*) registers;

}

void Opal::Saboteur::SetRegistersOf(Opal::Saboteur* thread, void* registers) {

    // Leave if the Opal::Saboteur is null
    if(!thread) return;

    // There's no guarantee the Opal::Saboteur is suspended, so
    // we attempt an invocation.
    Suspend(thread);

    std::cout << "Setting Registers." << std::endl;

    // Set the register contents
    ptrace(PTRACE_SETREGSET, thread->threadID, 1, registers);

    std::cout << "Registers Set" << std::endl;

}

/*!
 * Creates the thread of execution and binds it to the given
 * Opal::Saboteur instance. This function ensures that a thread
 * is created, traced, and executed.
 * \param thread The Opal::Saboteur to bind
 */

void Opal::Saboteur::Create(Opal::Saboteur* thread) {

    // This is our OS thread 'handle'.
    uint32_t threadReference;

    thread->stack = new uint64_t[1024*1024];

    std::cout << std::hex << thread << " With stack: " << thread->stack <<  std::endl;

    int32_t stackSize = 1024*1024;
    pid_t processId;

    std::cout << "Invoking clone" << std::endl;
    if(clone(Opal::Saboteur::Execution, thread->stack + stackSize,
             CLONE_PARENT_SETTID | CLONE_PARENT | CLONE_VM | CLONE_SIGHAND
             | CLONE_FILES | CLONE_FS | CLONE_IO, (void*) thread, &processId) == -1) {

        perror("Opal::Saboteur");

    }

    std::cout << "Attempting to trace thread." << std::endl;

    if(ptrace(PTRACE_SEIZE, processId, NULL, NULL)) { perror("Failed Seize"); }

    std::cout << "Waiting" << std::endl;

    // Wait for the child process to stop itself in preparation
    // for execution.
    wait(0);

    std::cout << "Child stopped. Resuming child" << std::endl;

    Resume(thread);

    std::cout << "Saboteur trace success!" << std::endl;

    // Set the state
    thread->setStateTo(CREATED);

    std::cout << "Saboteur created." << std::endl;

}

/*!
 * The execution method of the Opal::Saboteur. Any Opal::Saboteur that's
 * in an intermediate state should execute here to determine the
 * next path of execution. This function expects a pointer to a
 * Opal::Saboteur, if it is null or invalid, a
 * Opal::Saboteur::LostSaboteurException is thrown. The function will
 * relinquish any remaining cpu time if either an error was thrown
 * when attempting to retrieve internal state information, or if
 * the Opal::Saboteur is in a incorrect state to continue.
 * \param arg_thread The Opal::Saboteur handle passed by the parent
 * process.
 */

int/*Opal::ReturnCode*/ Opal::Saboteur::Execution(void* arg_thread) {

    std::cout << "Starting child" << std::endl;

    // Fatal error, missing thread handle, so throw an exception
    if(!arg_thread) throw Opal::Saboteur::LostSaboteurException();

    // Cast it as a saboteur
    Opal::Saboteur* thread = (Opal::Saboteur*) arg_thread;

    std::cout << "Handle retrieved: "   << thread << std::endl;
    std::cout << "Setting thread id."   << std::endl;
    std::cout << "State: "              << thread->state << std::endl;

    // We don't want to stop the process
    // to do the setup again after the thread has been created,
    // so we wrap this stuff here
    if(!thread->state) {

        uint64_t sysResult = 0;
        GetProcessId(sysResult);
        thread->threadID = sysResult;

        std::cout << "Saboteur Id retrieved."             << std::endl;
        Suspend(thread);

        std::cout << "Request successful " << std::endl;

    }

    std::cout << std::hex << thread->threadID  <<  " Waiting" << std::endl;

    // Waiting state = No terminate and no execution address
    // Both method invocations may throw an exception that indicate
    // an undetermined state.
    while(!(thread->setStateTo(WAITING).getExecutionAddress()) &&
          !(thread->isIn(TERMINATE))) Yield;

    std::cout << "Finished waiting" << std::endl;

    // Started state = execution address, the terminate state
    // is a don't-care
    if(thread->getExecutionAddress()) {

        // We're going to need this when we come back
        Push(thread);

        std::cout << "Return address: " << Indirect(&Opal::Saboteur::Execution) << std::endl;

        // Push the return address (here)
        Push(Indirect(Opal::Saboteur::Execution));

        // Set the state and execute the code.
        Traverse(thread->setStateTo(STARTED).getExecutionAddress());

    }
    std::cout << "Terminating" << std::endl;
    // Otherwise, the thread is set to terminate, and there is no more
    // code to execute. Remove the thread handle from the stack and
    // set the thread to the corresponding state.
    thread->setStateTo(TERMINATED);

    // Leave
    return 0;

}

/// ---------------
/// Private Methods

/*!
 * Checks the internal state for any matches of the given value.
 * If the value matches, then a corresponding exception is thrown.
 * \param state The Opal::State to check.
 */

void Opal::Saboteur::checkErrorState(Opal::State state) {

    // Acquire the lock
    Opal::Lock<Opal::Mutex> stateLock(stateMutex);

    // If the Opal::Saboteur is being swapped, throw a
    // Opal::Saboteur::SaboteurIsSwappingException.
    if(state & SWAPPING) throw Opal::Saboteur::SaboteurIsSwappingException();

    // Check if the process is finished
    if(state & TERMINATED) throw Opal::Saboteur::SaboteurFinishedException();

    uint64_t sysResult = 0;GetProcessId(sysResult)

    // Check thread id against the instances thread id here
    // If the thread id's match, throw a Opal::Saboteur::SaboteurSwapSelfException
    if(threadID == sysResult) throw Opal::Saboteur::SaboteurSwapSelfException();

    // Check if the thread is calling this itself
    if(threadID == sysResult)
        throw Opal::Saboteur::SaboteurSuspendSelfException();

}

/*!
 * Sets the current state of the Opal::Saboteur. If the Opal::Saboteur
 * has an observer, the Opal::Saboteur will notify it of the state
 * mutation.
 * \param state the Opal::State value to set.
 */

Opal::Saboteur& Opal::Saboteur::setStateTo(Opal::State state) {

    // Acquire the lock
    Opal::Lock<Opal::Mutex> stateLock(stateMutex);

    if(this->state == state) return *this;

    std::cout << "Setting State: " << std::hex << this->state << " to " << state << std::endl;
    // Check if the Opal::Saboteur is resuming
    if(!(state ^ RESUMING)) {

        this->state = STARTED;

        if(observer)
            observer->OnResume(Indirect(this));

        return *this;

    }

    // This will clear the other flags and persist the appropriate
    // flags.
    this->state = state;

    // Check if there's an observer to notify
    if(observer) switch(state) {

        case CREATED    : observer->OnCreated(Indirect(this))   ; break;

        case WAITING    : observer->OnWaiting(Indirect(this))   ; break;

        case STARTED    : observer->OnStarted(Indirect(this))   ; break;

        case SUSPENDED  : observer->OnSuspended(Indirect(this)) ; break;

        case SUICIDE    : observer->OnSuicide(Indirect(this))   ; break;

        case TERMINATED : observer->OnTerminated(Indirect(this)); break;

        default: break;

    }

    return *this;

}

/*!
 * Returns a Opal::Flag indicating if the Opal::Saboteur
 * is in the given state.
 * \param state The state to inspect
 * \return Opal::Flag denoting if the Opal::Saboteur is
 * in the given state.
 */

Opal::Flag Opal::Saboteur::isIn(Opal::State state) {

    // Acquire the lock
    Opal::Lock<Opal::Mutex> stateLock(stateMutex);

    // Return the masked state
    return !(this->state ^ state);

}

/*!
 * Returns the value of the last address the Opal::Saboteur
 * resumed execution or will continue execution from a suspended
 * state. This value is updated when either the link data is swapped
 * or the Opal::Saboteur is suspended. This method will check the
 * internal state of the Opal::Saboteur for a SWAPPING state, and
 * if it matches, it will throw a Opal::Saboteur::SaboteurIsSwappingException.
 * \return void pointer representing the address of the last
 * address the Opal::Saboteur resumed execution, or will resume
 * execution from a suspended state.
 */

void* Opal::Saboteur::getExecutionAddress() {

    // Otherwise, return the value.
    return executionAddress;

}

/*!
 * Replaces the current executing code with the given code.
 * The Opal::Saboteur must be suspended before retrieving
 * results.
 * \param executionAddress The address of the instruction
 * to replace.
 */

void Opal::Saboteur::setExecutionAddress(void* executionAddress) { /* Empty */ }

/// --------------
/// Public Methods

/*!
 * Swaps the Opal::Saboteur's resume and return
 * address and returns the previous resume and return address
 * as contained in a Opal::Saboteur::Link instance. This
 * method interrupts the Opal::Saboteur, preserves the current
 * state of the data, sets the new data, and returns
 * a copy of the previous data. Important. This method should be invoked
 * only by another thread, never the thread that corresponds with
 * the Opal::Saboteur instance otherwise a Opal::Saboteur::SaboteurSwapSelf
 * exception is thrown. If this method is invoked while the Opal::Saboteur
 * is in its' finished state, it will throw a Opal::Saboteur::SaboteurFinishedException.
 * \param The execution address to swap with the current execution address.
 * \param resume Opal::Flag denoting if the thread should continue execution
 * after swapping the execution address.
 * \return previous execution address
 */

void* Opal::Saboteur::swap(void* executionAddress, Opal::Flag resume) {

    // Check for terminated, self swap, self suspend, and suicide
    checkErrorState(TERMINATED | SELF_SWAP | SELF_SUSPEND | SUICIDE);

    // If the Opal::Saboteur is not suspended, suspend it.
    if(!isIn(SUSPENDED)) Suspend(this);

    // Get the previous execution address
    void* previous = getExecutionAddress();

    // Overwrite the current execution address.
    setExecutionAddress(executionAddress);

    // Resume execution
    if(resume) Resume(this);

    // Finally, return the previous data
    return previous;

}

/*!
 * Pushes the given execution address to the highest priority (current)
 * position, effectively holding off on executing the code
 * that was being executed at the time of invocation, if
 * the Opal::Saboteur was executing. This method will
 * suspend the Opal::Saboteur.
 * \param executionAddress The execution address to push
 * \param resume Opal::Flag denoting if the Opal::Saboteur
 * should be resumed after the completion of the operation.
 */

void Opal::Saboteur::push(void* executionAddress, Opal::Flag resume) {

    // Ideally we:
    // 1. Get the register contents of the Opal::Saboteur
    // 2. Extract the value of the rip
    // 3. Update the PathDeterminant with the record corresponding with the execution
    // 4. Create a new record with the given execution address
    // 5. Update the Opal::Saboteur's rip register with the execution address
    // 6. Resume the thread.
    // This is all fine and dandy when the thread is executing, but when the Opal::Saboteur
    // is in a waiting state, we simply acquire a state lock and push the new execution address
    // onto the PathDeterminant. Why? Because we want Opal::Saboteur::Execution to be pushed
    // onto the stack in case execution of code is completed and the thread goes back to
    // its' waiting state.

    // Throw an error if this thread invokes this function
    std::cout << "Pushing execution address" << std::endl;

    // Retrieve the register set
    void* registers = RegistersOf(this);

    std::cout << "Setting rip" << std::endl;

    // Grab the RIP and update the latest record in the PathDeterminant.

    // Push the given execution address to the front so it can be
    // executed immediately. This will cause the Opal::Saboteur
    // to execute as soon as its' resumed since it's either in its'
    // waiting state (It will transition to starting) or its' running.
    // Before we leave, we check for those states and update the instruction
    // register if it's in a running state so it can execute where we
    // specified. Important caveat: This is where we would be update
    // the activation records for each process.

    // Check the thread's state first; If it's waiting, we only update the
    // PathDeterminant. No mutex since the Opal::Saboteur is suspended
    if(isIn(WAITING) || true) {

        // Note to self: Store previous state before suspension.
        std::cout << "Setting execution address" << std::endl;

        this->executionAddress = executionAddress;

        std::cout << this->executionAddress << std::endl;

    }

    else {

        // Set the rip register to the code that should be executed.
        ((uint64_t**) (((Registers*) registers)->base))[4] = (uint64_t*) executionAddress;

        std::cout << "Set rip: " << ((uint64_t**) (((Registers*) registers)->base))[4] <<  std::endl;

        // Set the registers after modification.
        SetRegistersOf(this, registers);

    }

    std::cout << "Resuming process" << std::endl;

    // We know that the Opal::Saboteur is suspended at this point.
    // The user could have wanted to keep the Opal::Saboteur suspended
    // and it would be real annoying to start it without their discretion.
    if(resume) Resume(this);

}

/*!
 * Places the given execution address at what might be considered
 * the lowest priority of the Opal::Saboteur's PathDeterminant.
 * For example, if a queue is being used as the underlying structure
 * of the Opal::PathDeterminant, then the execution address
 * will be placed last.
 * \param executionAddress The execution address to push
 * \param resume Opal::Flag denoting if the Opal::Saboteur
 * should be resumed after the completion of the operation.
 */

void* Opal::Saboteur::place(void* executionAddress, Opal::Flag resume) { /* Empty */ }

/*!
 * Cancels the currently executing code (highest priority) the
 * Opal::Saboteur was executing and returns the address of the
 * instruction at the moment the Opal::thread
 * was suspended. This method will suspend the Opal::Saboteur.
 * \param executionAddress The execution address to push
 * \param resume Opal::Flag denoting if the Opal::Saboteur
 * should be resumed after the completion of the operation.
 */

void* Opal::Saboteur::pop(Opal::Flag resume) { /* Empty */ }

/*!
 * Suspends the thread. This method should be invoked by another
 * thread. This method stores the current instruction address to
 * be executed when the Opal::Saboteur is resumed.
 * Once the Opal::Saboteur is suspended, it will notify
 * the Opal::SaboteurObserver of the suspended state (if any).
 * This method checks the internal state of the Opal::Saboteur.
 * If it finds a match to SELF_SUSPEND, it will throw a
 * Opal::Saboteur::SaboteurSuspendSelfException.
 */

void Opal::Saboteur::suspend() { Suspend(this); }

/*!
 * Attempts to continue the execution of the Opal::Saboteur
 * if it's in a suspended state. If the Opal::Saboteur is
 * resumeed, it will notify the Opal::SaboteurObserver of
 * the state mutation.
 */

void Opal::Saboteur::resume() { Resume(this); }

/*!
 * Returns a flag denoting if the Opal::Saboteur will terminate
 * after executing all of the assigned code.
 * \return Opal::Flag denoting if the Opal::Saboteur will
 * terminate after reaching the endpoint.
 */

Opal::Flag Opal::Saboteur::willTerminate() { return isIn(TERMINATE); }

/*!
 * Returns a flag denoting if the Opal::Saboteur has terminated.
 * \return Opal::Flag denoting if the Opal::Saboteur has terminated.
 */

Opal::Flag Opal::Saboteur::isTerminated() { return isIn(TERMINATED); }

/*!
 * Returns a flag denoting if the Opal::Saboteur is
 * currently suspended.
 * \return Opal::Flag denoting if the Opal::Saboteur is
 * suspended.
 */

Opal::Flag Opal::Saboteur::isSuspended() { return isIn(SUSPENDED); }

/*!
 * Returns a flag denoting if the Opal::Saboteur is
 * waiting.
 * \return Opal::Flag denoting if the Opal::Saboteur
 * is waiting.
 */

Opal::Flag Opal::Saboteur::isWaiting() { return isIn(WAITING); }
