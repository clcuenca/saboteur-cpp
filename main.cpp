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

int main() {

    //std::thread thread(someFunction);

    return 0;


}
