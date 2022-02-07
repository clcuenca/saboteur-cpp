#include<iostream>
#include<ProcessJ.hpp>

/*!
 * \def Align attribute
 * \brief Declares the aligned attribute
 */

//define Align(bytes) __attribute__ ((aligned(bytes)))

// Place me in namespace; We use this as an in-place address to allocate the
// ThreadRegistry
//static Align(16) int8_t RegistryPlaceholder[sizeof(ThreadRegistry)];

/*****
 * Based on gcc code
 */

/*!
 * x86-64 Assembly. Performs a syscall with the given number.
 * \param call The number corresponding with the desired system call
 * \return The result of the system call
 */
// internal_syscall; __sanitizer_common/sanitizer_syscall_linux_x86_64
//static uint64_t syscall(uint64_t call) {

  //  uint64_t result;

    //asm volatile("syscall" : "=a"(result) : "a"(call) : "rcx", "r11", "memory", "cc");

    //return result;

//}


//void someFunction() {

  //  std::cout << "Hello World!" << std::endl;

    //int index = 0;

    //index++;

//}

static void SomeOtherFunction(void) {

  std::cout << "Swapped!" << std::endl;


}

static void SomeFunction(void) {

  std::cout << "Counting" << std::endl;

  uint64_t index = 0;

  for(; index < 100; index++);

}


class DummyObserver : public ProcessJ::ThreadObserver {

public:


    /*!
     * Invoked when the ProcessJ::Thread is created.
     * \param thread The newly created ProcessJ::Thread
     */

    void OnCreated(void* thread) {

      std::cout << "OnCreated" << std::endl;

    }

    /*!
     * Invoked when the ProcessJ::Thread is in a waiting state.
     * It is possible that ThreadObserver::OnStarted(void*) is
     * called back immediately after (in the case that a return
     * address was assigned to the ProcessJ::Thread).
     * \param thread The waiting ProcessJ::Thread
     */

    void OnWaiting(void* thread) {

      std::cout << "OnWaiting" << std::endl;

    auto Function = reinterpret_cast<void*>(&SomeFunction);
    auto OtherFunction = reinterpret_cast<void*>(&SomeOtherFunction);
    std::cout << "SomeFunction Address: " << std::hex << Function << std::endl;
    std::cout << "SomeOtherFunction Address: " << std::hex << OtherFunction << std::endl;
    std::cout << "Setting Resume address to: " << Function << std::endl;
    ((ProcessJ::Thread*) thread)->setResumeAddress(Function);

    }

    /*!
     * Invoked when the ProcessJ::Thread has started execution
     * but before it has executed any associated code
     * \param thread The started ProcessJ::Thread
     */

    void OnStarted(void* thread) {


    }

    /*!
     * Invoked when the ProcessJ::Thread has successfully been suspended.
     * \param thread The suspended ProcessJ::Thread.
     */

    void OnSuspended(void* thread) {

      std::cout << "OnSuspended" << std::endl;

    }

    /*!
     * Invoked when the ProcessJ::Thread has been restarted from
     * a suspended state.
     * \param thread The restarted ProcessJ::Thread
     */

    void OnRestart(void* thread) {

      std::cout << "OnRestart" << std::endl;

    }

    /*!
     * Invoked when the ProcessJ::Thread has finished.
     * \param thread The ProcessJ::Thread that has finished.
     */

    void OnFinished(void* thread) {

      std::cout << "OnFinished" << std::endl;

    }


};

int main() {

    DummyObserver observer;
    ProcessJ::Thread thread((ProcessJ::ThreadObserver*)(&observer));

    // Yield every time we're here
    while(!thread.shouldTerminate()) Yield;

    return 0;


}
