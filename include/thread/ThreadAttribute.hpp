/*!
 * \brief ProcessJ Thread Attribute Union
 *
 * As required by clone
 *
 * \author Carlos L. Cuenca
 * \version 0.9.0
 * \date 12/16/2021
 */

#ifndef PROCESS_J_THREAD_ATTRIBUTE_HPP
#define PROCESS_J_THREAD_ATTRIBUTE_HPP

/// --------
/// Includes

#include<Types.hpp>

/// X86-64:
/// 3 TLS entries, two of which are accessbile via fs and gs registers
/// fs is used by glibc and in IA32 fs is used by wine and gs by glibc
/// glibc makes its TLS entry point to a struct pthread that contains internal strucures
/// on x86-64 struct pthread starts with tcbhead_t
/// needed even when there is a single thread
///
/// Limitation c++ programs, thread-local variables must not require a
/// static constructor
/// x86-64 uses the %fs segment register as opposed to the %gs register in IA-32 (virtually the same)
/// tls_get_address(tls_index* ti);
/// defined as
/// typedef struct {
/// unsigned long int ti_module;
/// unsigned long int ti_offset;
/// } tls_index;
/// Thread pointer is retrieved by movl %fs:0, %rax
/// To access TLS blocks, the tlsoffset must be known. The value must be subtracted
/// from the thread register value
/// See: https://akkadia.org/drepper/tls.pdf
/// Linux Task struct  & thread struct as returned by get_current. using the gs/fs inline
/// assembly above Should we rewrite? maybe.
/// I don't want to include the headers, but do want to minimize the code

namespace ProcessJ {

    union   ThreadAttributeUnion ;
    struct  SchedulerParameters  ;
    struct  ThreadAttribute      ;

    /*!
     * Describes CPU Mask
     * CPU set size / N CPU Bits
     * Affinity Map
     */

    typedef struct {

        uint64_t bits[1024 / (8*sizeof(uint64_t))];

    } CPUSet;

}

union ProcessJ::ThreadAttributeUnion {

    ProcessJ::Byte      size[56]    ;
    ProcessJ::Integer32 align       ;

};

struct ProcessJ::SchedulerParameters {

    ProcessJ::Integer32 schedulerPriority;

};



struct ProcessJ::ThreadAttribute {

    ProcessJ::SchedulerParameters   schedulerParameters ;
    ProcessJ::Integer32             schedulerPolicy     ;
    ProcessJ::Integer32             flags               ;
    size_t                          guardSize           ;
    void*                           stackAddress        ;
    size_t                          stackSize           ;
    ProcessJ::CPUSet*               cpuSet              ;
    size_t                          cpuSetSize          ;

};

#endif
