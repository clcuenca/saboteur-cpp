/*!
 * \brief Saboteur class
 *
 * Opal::Saboteur declaration. Defines the interface for a
 * Opal::Saboteur that is to be used in conjuction with a scheduler.
 * The implementation of this class emphasizes the communication of
 * lifecycle events that correspond with the state of the thread.
 *
 * Additional functionality is included such as suspension and
 * swapping of resumption code.
 *
 * \author Carlos L. Cuenca
 * \version 0.9.0
 * \date 02/07/2022
 */

#ifndef OPAL_SABOTEUR_HPP
#define OPAL_SABOTEUR_HPP

/// --------
/// Includes

#include<pthread.h> // Remove me
#include<cstring> // remove me
#include<iostream>
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
#include<SaboteurObserver.hpp>

namespace Opal { class Saboteur; }

extern "C" void Lifecycle(void*);

/// -----------------------
/// Opal::Saboteur States

/*!
 * \def CLEAR
 * \brief Clear Mask
 */

#define CLEAR 0x0000000000000000

/*!
 * \def CREATED
 * \brief Value that denotes a created state.
 */

#define CREATED 0x0000000000000801

/*!
 * \def WAITING
 * \brief Value that denotes a waiting state.
 */

#define WAITING 0x0000000000000803

/*!
 * \def STARTED
 * \brief Value that denotes a started state.
 */

#define STARTED 0x0000000000000805

/*!
 * \def TERMINATED
 * \brief Value that denotes a terminated state.
 */

#define TERMINATED 0x0000000000000809

/*!
 * \def TERMINATE
 * \brief Value that denotes a terminate state.
 */

#define TERMINATE 0x0000000000000811

/*!
 * \def SUSPENDED
 * \brief Value that denotes a suspended state.
 */

#define SUSPENDED 0x0000000000000821

/*!
 * \def RESUMING
 * \brief Value that denotes a resumption state.
 */

#define RESUMING 0x0000000000000841

/*!
 * \def SWAPPING
 * \brief Value that denotes a swapping state.
 */

#define SWAPPING 0x0000000000000881

/*!
 * \def SELF_SWAP
 * \brief Value that denotes a self swapping state.
 */

#define SELF_SWAP 0x0000000000000901

/*!
 * \def SELF_SUSPEND
 * \brief Value that denotes a self suspend state.
 */

#define SELF_SUSPEND 0x0000000000000a01

/*!
 * \def SUICIDE
 * \brief Value that denotes a suicide state.
 */

#define SUICIDE 0x0000000000000c09

/*!
 * \def ATTACHED
 * \brief Value that denotes an attached state.
 */

#define ATTACHED 0x0000000000000801

/// --------------
/// General Macros

/*!
 * \def Indirect(data)
 * \brief Returns the data as a void pointer
 */

#define Indirect(data)             \
    reinterpret_cast<void*>(data)  \

/// ---------------
/// Inline Assembly

#define Assembly __asm__ __volatile__

/// --------------------
/// Architecture: x86-64

/*!
 * \def Push(data)
 * \brief Pushes the given data onto the stack.
 */

#define Push(data)                              \
    Assembly(                                   \
             "push %[input]     \n"             \
             :: [input] "r" (Indirect(data)))   \

/*!
 * \def Pop(data)
 * \brief Pops the most recent data off the stack.
 */

#define Pop(data)                       \
    Assembly(                           \
            "pop %[output]  \n"         \
            : [output] "=r" (data):)    \

/*!
 * \def GetLastStack(data)
 * \brief Retrieves the data on top of the
 * stack without modifying the stack's state.
 */

#define GetLastStack(data)                      \
    Assembly(                                   \
            "pop   %[output]   \n"              \
            "push  %[input]    \n"              \
            : [output] "=r" (data)              \
            : [input]  "r"  (Indirect(data)))   \

/*!
 * \def SetFirstParameter(parameter)
 * \brief Sets the first parameter based on
 * the target architecture's standard calling convention.
 */

#define SetFirstParameter(parameter)            \
    Assembly(                                   \
             "mov rdi, %[input] \n"             \
             :: [input]  "r" (Indirect(data)))  \

/*!
 * \def Traverse(path)
 * \brief Effectively continues execution at the specified address
 * A little hacky, but extremely effective. This will not modify the
 * the current Saboteur's stack.
 */

#define Traverse(path)                            \
    Assembly(                                     \
             "push %[address]      \n"            \
             "ret                  \n"            \
             :: [address] "r" (Indirect(path)))   \


/// -----------------
/// Class Declaration

class Opal::Saboteur {

    /// ---------------
    /// Private Members

private:

    /// -----------------------
    /// Static Member Variables

    static const Opal::CloneFlags CloneFlags;

    /// ----------------
    /// Member Variables

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    /****
     *
    /// FOR FUCKS SAKE, IF THIS CLASS IS EXTENDED, UPDATE THE OFFSETS IN THE ASSEMBLY IMPLEMENTATION
    /// THERE IS NO WAY FOR ANYONE TO KNOW WHERE THE MEMBERS ARE AND A VTABLE WILL HOSE EVERYTHING.
    /// EVERYTHING WILL BREAK AND WE WILL ALL BE OUT OF A JOB.
    ///
    /// If I could make this in big bold red colors with flashing pop-ups 90's style, I would.
    /// Important: These need to stay in this order. If they must be changed, update the assembly implementation files.
    /// 64 Bytes long
    /// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    */
    Opal::ThreadID              threadID            ; /*< The thread id corresponding with the thread                               */ // 8 Bytes
    Opal::State                 state               ; /*< The value that denotes the current state of the Opal::Saboteur          */ // 8 Bytes
    uint64_t*                   stack               ; /*< The stack that is allocated for this thread                               */ // 8 Bytes
    uint64_t                    stackSize           ; /*< The size of the stack                                                     */ // 8 Bytes
    Opal::SaboteurObserver*     observer            ; /*< The observer that receives callbacks from the Opal::Saboteur instance   */
    int (*stop)(int32_t, int32_t)                   ;
    Opal::Mutex                 stateMutex          ; /*< Mutex that corresponds to state changes                                   */
    void*                       executionAddress    ; /*< The address of the instruction the thread should resume from              */ // 8 Bytes

    /// --------------
    /// Static Methods

    /*!
     * Resumes the thread on the operating system level.
     * If the thread resumption fails, this function will throw a
     * Opal::Saboteur::SaboteurResumeFailureException. If the given
     * handle to the Opal::Saboteur is null, this function will throw
     * a Opal::Saboteur::LostSaboteurException.
     */

    static void Resume(Saboteur*);

    /*!
     * Stops the thread represented by the Opal::Saboteur.
     * If the suspension fails, this function will throw a
     * Opal::Saboteur::SaboteurSuspendFailureException. If the given
     * handle to the Opal::Saboteur is null, this function will
     * throw a Opal::Saboteur::LostSaboteurException.
     */

    static void Suspend(Saboteur*);

    /*!
     * Creates the thread of execution and binds it to the given
     * Opal::Saboteur instance. This function ensures that a thread
     * is created, traced, and executed.
     * \param thread The Opal::Saboteur to bind
     */

    static void Create(Saboteur*);

    static void* RegistersOf(Saboteur*);
    static void SetRegistersOf(Saboteur*, void*);

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

    static /*Opal::ReturnCode*/ int Execution(void*);

    /// -------
    /// Methods

    /*!
     * Checks the internal state for any matches of the given value.
     * If the value matches, then a corresponding exception is thrown.
     * \param state The Opal::State to check.
     */

    void checkErrorState(Opal::State);

    /*!
     * Sets the current state of the Opal::Saboteur.
     * \param state the Opal::State value to set.
     */

    Saboteur& setStateTo(Opal::State);

    /*!
     * Returns a Opal::Flag indicating if the Opal::Saboteur
     * is in the given state.
     * \param state The state to inspect
     * \return Opal::Flag denoting if the Opal::Saboteur is
     * in the given state.
     */

    Opal::Flag isIn(Opal::State);

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

    void* getExecutionAddress();

    /*!
     * Replaces the current executing code with the given code.
     * The Opal::Saboteur must be suspended before retrieving
     * results.
     * \param executionAddress The address of the instruction
     * to replace.
     */

    void setExecutionAddress(void*);

    /// --------------
    /// Public Members

public:

    /// ------------
    /// Constructors

    /*!
     * Default Constructor. Initializes the Opal::Saboteur
     * to its' default state.
     */

    Saboteur();

    /*!
     * Initializes the Opal::Saboteur
     * to its' default state with the given Opal::SaboteurObserver.
     * \param observer The Opal::SaboteurObserver to bind to the
     * Opal::Saboteur
     */

    Saboteur(Opal::SaboteurObserver*);

    /*!
     * Primary Constructor. Initializes the Opal::Saboteur
     * with the given resume address. This constructor requires a pointer
     * \param address The address to initialize the Opal::Saboteur with.
     */

    template<typename Address>
    Saboteur(Address);

    /*!
     * Primary Constructor. Initializes the Opal::Saboteur
     * with the given resume address. This constructor requires a pointer
     * \param address The address to initialize the Opal::Saboteur with.
     * \param observer The Opal::SaboteurObserver that receives callbacks
     * from the Opal::Saboteur
     */

    template<typename Address>
    Saboteur(Address, Opal::SaboteurObserver*);

    /*!
     * Deconstructor. Releases any resources used by the Opal::Saboteur.
     * At the time of writing, no resources are used or released.
     */

    ~Saboteur();

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

    void* swap(void*, Opal::Flag=false);

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

    void push(void*, Opal::Flag=false);

    /*!
     * Cancels the currently executing code (highest priority) the
     * Opal::Saboteur was executing and returns the address of the
     * instruction at the moment the Opal::thread
     * was suspended. This method will suspend the Opal::Saboteur.
     * \param executionAddress The execution address to push
     * \param resume Opal::Flag denoting if the Opal::Saboteur
     * should be resumed after the completion of the operation.
     */

    void* pop(Opal::Flag=false);

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

    void* place(void*, Opal::Flag=false);

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

    void suspend();

    /*!
     * Attempts to continue the execution of the Opal::Saboteur
     * if it's in a suspended state. If the Opal::Saboteur is
     * resumeed, it will notify the Opal::SaboteurObserver of
     * the state mutation.
     */

    void resume();

    /*!
     * Returns a flag denoting if the Opal::Saboteur should terminate
     * after execution of the current code, i.e. when it links to
     * the Opal::Saboteur::Terminate function. By default, this is set
     * to false. This method checks the internal state of the Opal::Saboteur
     * for a SWAPPING state, and if it matches, it will throw a
     * Opal::Saboteur::SaboteurIsSwappingException.
     * \return Opal::Flag denoting if the Opal::Saboteur will
     * terminate after reaching the endpoint.
     */

    Opal::Flag willTerminate();

    /*!
     * Returns a flag denoting if the Opal::Saboteur has terminated.
     * \return Opal::Flag denoting if the Opal::Saboteur has terminated.
     */

    Opal::Flag isTerminated();

    /*!
     * Returns a flag denoting if the Opal::Saboteur is
     * currently suspended.
     * \return Opal::Flag denoting if the Opal::Saboteur is
     * suspended.
     */

    Opal::Flag isSuspended();

    /*!
     * Returns a flag denoting if the Opal::Saboteur is
     * waiting.
     * \return Opal::Flag denoting if the Opal::Saboteur
     * is waiting.
     */

    Opal::Flag isWaiting();

    /// ----------
    /// Exceptions

    /*!
     * Exception that gets thrown when a Opal::Saboteur creation
     * fails.
     */

    class SaboteurCreateFailureException : public Opal::Exception {

        Opal::StringLiteral what() const throw() {

            return "Error: Saboteur creation failed.";

        }

    };

    /*!
     * Exception that gets thrown when a Opal::Saboteur was not
     * given as an argument when executing.
     */

    class SaboteurHandleNullException : public Opal::Exception {

        Opal::StringLiteral what() const throw() {

            return "Error: Saboteur handle was not provided.";

        }

    };

    /*!
     * Exception that gets thrown when a Opal::Saboteur attempts
     * to invoke Opal::Saboteur::swap on itself
     */

    class SaboteurSwapSelfException : public Opal::Exception {

        Opal::StringLiteral what() const throw() {

            return "Error: Saboteur attempted to swap itself.";

        }

    };

    /*!
     * Exception that gets thrown when a Opal::Saboteur attempts
     * to invoke Opal::Saboteur::suspend on itself
     */

    class SaboteurSuspendSelfException : public Opal::Exception {

        Opal::StringLiteral what() const throw() {

            return "Error: Saboteur attempted to suspend itself.";

        }

    };

    /*!
     * Exception that gets thrown when the thread represented by the
     * Opal::Saboteur fails to resume.
     */

    class SaboteurResumeFailureException : public Opal::Exception {

        Opal::StringLiteral what() const throw() {

            return "Error: Saboteur resume failed.";

        }

    };

    /*!
     * Exception that gets thrown when attempting to set either
     * the resume or return address of a Opal::Saboteur while
     * it is in the process of being swapped.
     */

    class SaboteurIsSwappingException : public Opal::Exception {

        Opal::StringLiteral what() const throw() {

            return "Error: Cannot set address, thread is being swapped.";

        }

    };

    /*!
     * Exception that gets thrown when attempting to modify link
     * data while the Opal::Saboteur is finished.
     */

    class SaboteurFinishedException : public Opal::Exception {

        Opal::StringLiteral what() const throw() {

            return "Error: Saboteur is finished.";

        }

    };

    /*!
     * Exception that gets thrown when process attachement has failed
     */

    class ProcessAttachFailureException : public Opal::Exception {

        Opal::StringLiteral what() const throw() {

            return "Error: Failed to attach process";

        }

    };

    /*!
     * Exception that gets thrown when process attachement has failed
     */

    class LostSaboteurException : public Opal::Exception {

        Opal::StringLiteral what() const throw() {

            return "Error: Saboteur handle lost.";

        }

    };

};

/*!
 * Primary Constructor. Initializes the Opal::Saboteur
 * with the given resume address.
 * \param address The address to initialize the Opal::Saboteur with.
 */

template<typename Address>
Opal::Saboteur::Saboteur(Address address):
executionAddress(Indirect(address)), observer(0), threadID(0),
state(0), stateMutex() { Create(this); }

/*!
 * Primary Constructor. Initializes the Opal::Saboteur
 * with the given resume address. This constructor requires a pointer
 * \param address The address to initialize the Opal::Saboteur with.
 * \param observer The Opal::SaboteurObserver that receives callbacks
 * from the Opal::Saboteur
 */

template<typename Address>
Opal::Saboteur::Saboteur(Address address, Opal::SaboteurObserver* observer):
executionAddress(Indirect(address)), observer(observer), threadID(0),
state(0), stateMutex() { Create(this); }

#endif
