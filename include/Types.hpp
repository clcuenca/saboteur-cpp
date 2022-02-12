/*!
 * \brief Opal types declaration. Declares non-class and non-struct
 * types that are used throughout the runtime.
 *
 * \author Carlos L. Cuenca
 * \version 0.1.0
 * \date 02/05/2022
 */

#ifndef OPAL_TYPES_HPP
#define OPAL_TYPES_HPP

/// --------
/// Includes

#include<cinttypes>
#include<exception>
#include<mutex>

namespace Opal {

    /// -----------------
    /// Macro Definitions

    /*!
     * \def #define Expect(value, expected) __builtin_expect(value, expected)
     * \brief Macro Definition for expect. Platform-dependent, Linux x86-64
     */

    #define Expect(value, expected) __builtin_expect(value, expected)

    /*!
     * \def #define Yield sched_yield()
     * \brief Macro Definition for a sched_yield syscall.
     * Platform-dependant, Linux x86-64.
     */

    #define Yield sched_yield()

    /*!
     * \def Macro definition to stop a process
     * \brief Tested with linux arm64
     */

    #define StopProcess(processId) kill(processId, SIGSTOP)

    /*!
     * \def Macro definition to stop the current calling process
     * \brief Tested with linux arm64
     */

    #define StopThisProcess StopProcess(GetProcessId);

    /*!
     * \def Macro Definition for retrieving a thread id
     * \brief Tested with linux arm64
     */

    /// !!!//syscall(186) // x86-64 gettid is 186
    #define GetProcessId(data)  \
        __asm__ __volatile__( \
                             "mov rax, 0x00ba  \n"     \
                             "syscall       \n"     \
                             "mov %[output], rax\n" \
                             : [output] "=r" (data) :); \


    /// ----------------
    /// Type Definitions

    /*!
     * \var typedef uint64_t ReturnCode;
     * \brief Type definition for a return code
     */

    typedef uint64_t ReturnCode;

    /*!
     * \var typedef int32_t Status;
     * \brief Type definition for a return code
     */

    typedef int32_t Status;

    /*!
     * \var typedef int32_t Integer32;
     * \brief Type definition for a 32-bit integer signed
     */

    typedef int32_t Integer32;

    /*!
     * \var typedef int8_t Byte;
     * \brief Type definition for a byte signed
     */

    typedef int8_t Byte;

    /*!
     * \var typedef int32_t CloneFlags;
     * \brief Type definition for clone flags.
     */

    typedef uint64_t CloneFlags;

    /*!
     * \var typedef int64_t ThreadID;
     * \brief Type definition for a ThreadID
     */

    typedef uint64_t ThreadID;

    /*!
     * \var typedef uint64_t State;
     * \bried Type definition for a State
     */

    typedef uint64_t State;

    /*!
     * \var typedef bool Flag;
     * \brief Type definition for a flag
     */

    typedef bool Flag;

    /*!
     * \var typedef const char* StringLiteral;
     * \brief Type definition for a String Literal.
     */

    typedef const char* StringLiteral;

    /*!
     * \var typedef std::exception Exception;
     * \brief Type definition for an Exception
     */

    typedef std::exception Exception;

    /*!
     * \var typedef std::mutex Mutex;
     * \brief Type definition for a Mutex.
     */

    typedef std::mutex Mutex;

    /*!
     * \var template<typename Type> using Lock<Type> = std::lock_guard<Type>
     * \brief Alias for a mutex lock.
     */

    template<typename Type>
    using Lock = std::lock_guard<Type>;

}

#endif
