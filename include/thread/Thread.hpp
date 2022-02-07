/*!
 * \brief ProcessJ Thread class
 *
 * ProcessJ::Thread declaration. Defines the interface for a
 * ProcessJ::Thread that is to be used in conjuction with a scheduler.
 * The implementation of this class emphasizes the communication of
 * lifecycle events that correspond with the state of the thread.
 *
 * Additional functionalilty is included such as suspension and
 * swapping of resumption code.
 *
 * \author Carlos L. Cuenca
 * \version 0.9.0
 * \date 12/16/2021
 */

#ifndef PROCESS_J_THREAD_HPP
#define PROCESS_J_THREAD_HPP

/// --------
/// Includes

#include<sched.h>
#include<sys/syscall.h>
#include<sys/types.h>
#include<sys/ptrace.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/wait.h>
#include<unistd.h>
#include<Types.hpp>
#include<ThreadAttribute.hpp>
#include<ThreadObserver.hpp>

// We want it a little cleaner
namespace ProcessJ { class Thread; }

class ProcessJ::Thread {

    /// ---------------
    /// Private Members

private:

    /// -----------------------
    /// Static Member Variables

    static const ProcessJ::CloneFlags CloneFlags;

    /// ----------------
    /// Member Variables

    void*                       resumeAddress   ; /*< The address of the instruction the thread should resume from                              */
    void*                       returnAddress   ; /*< The address the ProcessJ::Thread should return to after successful execution              */
    void*                       exitAddress     ; /*< The address that corresponds to exiting execution                                         */
    ProcessJ::ThreadObserver*   observer        ; /*< The observer that receives callbacks from the ProcessJ::Thread instance                   */
    ProcessJ::ThreadID          threadID        ; /*< The thread id corresponding with the thread                                               */
    ProcessJ::Flag              terminate       ; /*< Flag denoting if the ProcessJ::Thread should terminate after the latest code execution    */
    ProcessJ::Flag              swapping        ; /*< Flag denoting if the ProcessJ::Thread is swapping its' resume and return addresses        */
    ProcessJ::Flag              waiting         ; /*< Flag denoting if the ProcessJ::Thread is in a waiting state                               */
    ProcessJ::Flag              started         ; /*< Flag denoting if the ProcessJ::Thread is in the started state                             */
    ProcessJ::Flag              finished        ; /*< Flag denoting if the ProcessJ::Thread is in the finished state                            */
    ProcessJ::Flag              suspended       ; /*< Flag denoting if the ProcessJ::Thread is in a suspended state.                            */

    // -----
    // Mutex

    ProcessJ::Mutex stateMutex ; /*< Mutex that corresponds to state changes    */
    ProcessJ::Mutex swapMutex  ; /*< Mutex that corresponds to swapping of data */

    /// --------------
    /// Static Methods

    /*!
     * The execution method of the ProcessJ::Thread. If the ProcessJ::Thread
     * was given a resume address at construction time, then the ProcessJ::Thread
     * executes the code immediately and notifies any ProcessJ::ThreadObserver that
     * it has started. Otherwise, it yields until a resume address is set so other
     * threads may get cpu time.
     * \return Status code
     */

    static /*ProcessJ::ReturnCode*/ void* Execution(void*);



    /// -------
    /// Methods

    /*!
     * Creates the thread of execution and notifies any ProcessJ::ThreadObserver
     * that it has been created from the child process. The execution of the
     * ProcessJ::Thread begins in ProcessJ::Thread::Execution. If the creation
     * of a new thread fails, then a ProcessJ::Thread::ThreadCreateFailureException
     * is thrown.
     */

    void create();

    /// --------------
    /// Public Members

public:

    /// -------
    /// Structs

    /*!
     * Struct that represents an execution link. This
     * is primarily used when swapping the code to be executed
     * by the ProcessJ::Thread
     */

    struct Link {

        void*           resumeAddress   = 0     ;
        void*           returnAddress   = 0     ;
        ProcessJ::Flag  terminate       = false ;

    };

    /// ------------
    /// Constructors

    /*!
     * Default Constructor. Initializes the ProcessJ::Thread
     * to its' default state.
     */

    Thread();

    /*!
     * Initializes the ProcessJ::Thread
     * to its' default state with the given ProcessJ::ThreadObserver.
     * \param observer The ProcessJ::ThreadObserver to bind to the
     * ProcessJ::Thread
     */

    Thread(ProcessJ::ThreadObserver*);

    /*!
     * Primary Constructor. Initializes the ProcessJ::Thread
     * with the given resume address. This constructor requires a pointer
     * \param address The address to initialize the ProcessJ::Thread with.
     */

    template<typename Address>
    Thread(Address);

    /*!
     * Primary Constructor. Initializes the ProcessJ::Thread
     * with the given resume address. This constructor requires a pointer
     * \param address The address to initialize the ProcessJ::Thread with.
     * \param observer The ProcessJ::ThreadObserver that receives callbacks
     * from the ProcessJ::Thread
     */

    template<typename Address>
    Thread(Address, ProcessJ::ThreadObserver*);

    /*!
     * Deconstructor. Releases any resources used by the ProcessJ::Thread.
     * At the time of writing, no resources are used or released.
     */

    ~Thread();

    /*!
     * Returns the value of the ProcessJ::Thread's resume address
     * \return void pointer representing the address of the next
     * instruction to execute.
     */

    void* getResumeAddress();

    /*!
     * Returns the value of the ProcessJ::Thread's return address
     * \return void pointer representing the address the ProcessJ::Thread
     * should return to after executing code.
     */

    void* getReturnAddress();

    /*!
     * Returns a flag denoting if the ProcessJ::Thread should terminate
     * after execution of the current code, i.e. when it returns to
     * the ProcessJ::Thread::Execute function. By default, this is set
     * to false.
     * \return Flag denoting if the ProcessJ::Thread should terminate.
     */

    ProcessJ::Flag shouldTerminate();

    /*!
     * Mutates the value of the ProcessJ::Thread's resumeAddress.
     * The given address must be a pointer.
     * \param address The desired resume address.
     */

    template<typename Address>
    void setResumeAddress(Address);

    /*!
     * Mutates the value of the ProcessJ::Thread's returnAddress.
     * The given address must be a pointer.
     * \param address The desired return address.
     */

    template<typename Address>
    void setReturnAddress(Address);

    /*!
     * Sets the terminate flag denoting if the ProcessJ::Thread should
     * terminate after execution of the current code, i.e. when it returns
     * to the ProcessJ::Thread::Execute function. By default, this is set
     * to false.
     * \param shouldTerminate Flag denoting if the Thread should terminate
     * after executing its' latest code.
     */

    void setShouldTerminate(ProcessJ::Flag);

    /*!
     * Swaps the ProcessJ::Thread's resume and return
     * address and returns the previous resum and return address
     * as contained in a ProcessJ::Thread::Link instance. This
     * method interrupts the ProcessJ::Thread, preserves the current
     * state of the pertinent data, sets the new data, and returns
     * a copy of the old data. Important. This method should be invoked
     * only by another thread, never the thread that corresponds with
     * the ProcessJ::Thread instance otherwise a ProcessJ::Thread::ThreadSwapSelf
     * exception is thrown.
     * \param link The pair of resume and return addresses to set
     * \return ProcessJ::Thread::Link instance containing the old
     * resume and return addresses.
     */

    Link swap(const Link&);

    /*!
     * Suspends the thread. This method should be invoked by another
     * thread. Once the ProcessJ::Thread is suspended, it will notify
     * the ProcessJ::ThreadObserver of the suspended state.
     * If the thread that corresponds with this object invokes this method,
     * this method will throw a ProcessJ::Thread::ThreadSuspendSelfException.
     */

    void suspend();

    /*!
     * Attempts to continue the execution of the ProcessJ::Thread
     * if it's in a suspended state. If the ProcessJ::Thread is
     * restarted, it will notify the ProcessJ::ThreadObserver of
     * the state mutation.
     */

    void restart();

    /// ----------
    /// Exceptions

    /*!
     * Exception that gets thrown when a ProcessJ::Thread creation
     * fails.
     */

    class ThreadCreateFailureException : public ProcessJ::Exception {

        ProcessJ::StringLiteral what() const throw() {

            return "Error: Thread creation failed.";

        }

    };

    /*!
     * Exception that gets thrown when a ProcessJ::Thread was not
     * given as an argument when executing.
     */

    class ThreadHandleNullException : public ProcessJ::Exception {

        ProcessJ::StringLiteral what() const throw() {

            return "Error: Thread handle was not provided.";

        }

    };

    /*!
     * Exception that gets thrown when a ProcessJ::Thread attempts
     * to invoke ProcessJ::Thread::swap on itself
     */

    class ThreadSwapSelfException : public ProcessJ::Exception {

        ProcessJ::StringLiteral what() const throw() {

            return "Error: Thread attempted to swap itself.";

        }

    };

    /*!
     * Exception that gets thrown when a ProcessJ::Thread attempts
     * to invoke ProcessJ::Thread::suspend on itself
     */

    class ThreadSuspendSelfException : public ProcessJ::Exception {

        ProcessJ::StringLiteral what() const throw() {

            return "Error: Thread attempted to suspend itself.";

        }

    };

    /*!
     * Exception that gets thrown when attempting to set either
     * the resume or return address of a ProcessJ::Thread while
     * it is in the process of being swapped.
     */

    class ThreadIsSwappingException : public ProcessJ::Exception {

        ProcessJ::StringLiteral what() const throw() {

            return "Error: Cannot set address, thread is being swapped.";

        }

    };

    /*!
     * Exception that gets thrown when attempting to modify link
     * data while the ProcessJ::Thread is finished.
     */

    class ThreadFinishedException : public ProcessJ::Exception {

        ProcessJ::StringLiteral what() const throw() {

            return "Error: Thread is finished.";

        }

    };

    /*!
     * Exception that gets thrown when process attachement has failed
     */

    class ProcessAttachFailureException : public ProcessJ::Exception {

        ProcessJ::StringLiteral what() const throw() {

            return "Error: Failed to attach process";

        }

    };

};

/*!
 * Primary Constructor. Initializes the ProcessJ::Thread
 * with the given resume address.
 * \param address The address to initialize the ProcessJ::Thread with.
 */

template<typename Address>
ProcessJ::Thread::Thread(Address address):
resumeAddress(0), returnAddress(0), exitAddress(0), observer(0), threadID(0), terminate(false),
swapping(false), waiting(false), started(false), finished(false), suspended(false), stateMutex(), swapMutex()  {

    // We add a level of indirection so we can cast the given
    // address into a function pointer
    Address indirection[1] { address };

    // Set the memeber
    this->resumeAddress = reinterpret_cast<void*>(indirection);

    // Invoke Create
    this->create();

}

/*!
 * Primary Constructor. Initializes the ProcessJ::Thread
 * with the given resume address. This constructor requires a pointer
 * \param address The address to initialize the ProcessJ::Thread with.
 * \param observer The ProcessJ::ThreadObserver that receives callbacks
 * from the ProcessJ::Thread
 */

template<typename Address>
ProcessJ::Thread::Thread(Address address, ProcessJ::ThreadObserver* observer):
resumeAddress(0), returnAddress(0), exitAddress(0), observer(observer), threadID(0), terminate(false),
swapping(false), waiting(false), started(false), finished(false), suspended(false), stateMutex(), swapMutex()  {

    // We add a level of indirection so we can cast the given
    // address into a function pointer
    Address indirection[1] { address };

    // Set the memeber
    this->resumeAddress = reinterpret_cast<void*>(indirection);

    // Invoke Create
    this->create();

}

/*!
 * Mutates the value of the ProcessJ::Thread's resumeAddress.
 * The given address must be a pointer. If this method is invoked
 * while the ProcessJ::Thread is being swapped, this will throw
 * a ProcessJ::Thread::ThreadIsSwappingException. If this method
 * is invoked when the ProcessJ::Thread is in its' finished state,
 * it will throw a ProcessJ::Thread::ThreadFinishedException.
 * \param address The desired resume address.
 */
#include<iostream>
template<typename Address>
void ProcessJ::Thread::setResumeAddress(Address address) {

    // Lock the swap mutex here
    ProcessJ::Lock<ProcessJ::Mutex> swapLock(this->swapMutex);

    // If the ProcessJ::Thread is being swapped, throw a
    // ProcessJ::Thread::ThreadIsSwappingException
    if(this->swapping) throw ProcessJ::Thread::ThreadIsSwappingException();

    // Lock the state mutex here
    ProcessJ::Lock<ProcessJ::Mutex> stateLock(this->stateMutex);

    // If the thread is finished, throw a
    // ProcessJ::Thread::ThreadFinishedException
    if(this->finished) throw ProcessJ::Thread::ThreadFinishedException();

    std::cout << "Attempting Suspend to set resume address: " << address << std::endl;

    // Otherwise, it's either in the waiting or started state. Suspend it.
    this->suspend();

    // If it's in the waiting state, set the resume address
    // to the member
    if(this->waiting) this->resumeAddress = reinterpret_cast<void*>(address);

    // Otherwise, it's in the started state. Set the register directly
    else { /* empty */ }

    std::cout << "Resume address set: " << this->waiting << std::endl;
    std::cout << "Restarting" << std::endl;
    // Restart the thread
    this->restart();
}

/*!
 * Mutates the value of the ProcessJ::Thread's returnAddress.
 * The given address must be a pointer. If this method is invoked
 * while the ProcessJ::Thread is being swapped, this will throw
 * a ProcessJ::Thread::ThreadIsSwappingException. If this method
 * is invoked when the ProcessJ::Thread is in its' finished state,
 * it will throw a ProcessJ::Thread::ThreadFinishedException.
 * \param address The desired return address.
 */

template<typename Address>
void ProcessJ::Thread::setReturnAddress(Address address) {

    // Lock the swap mutex here
    ProcessJ::Lock<ProcessJ::Mutex> swapLock(this->swapMutex);

    // If the thread is being swapped, throw a
    // ProcessJ::Thread::ThreadIsSwappingException
    if(this->swapping) throw ProcessJ::Thread::ThreadIsSwappingException();

    // Lock the state mutex here
    ProcessJ::Lock<ProcessJ::Mutex> stateLock(this->stateMutex);

    // If the thread is finished, throw a
    // ProcessJ::Thread::ThreadFinishedException
    if(this->finished) throw ProcessJ::Thread::ThreadFinishedException();

    // Before we suspend the thread, create some indirection so
    // we can cast the given address to a void pointer.
    // Normally, we are recieving function pointers
    Address indirection[1] = { address };

    // Otherwise, it's either in the waiting or started state. Suspend it.
    this->suspend();

    // Set the address
    this->returnAddress = reinterpret_cast<void*>(*indirection);

    // Restart the thread
    this->restart();

}

#endif
