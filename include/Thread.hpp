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

#ifndef PROCESS_J_CHANNEL_HPP
#define PROCESS_J_CHANNEL_HPP

/// --------
/// Includes

#include<ProcessJ.hpp>

class ProcessJ::Thread {

    /// ---------------
    /// Private Members

private:

    void* resumeAddress; /*< The address of the instruction the thread should resume from */

    /// --------------
    /// Public Members

public:

    /*!
     * Default Constructor. Initializes the ProcessJ::Thread
     * to its' default state.
     */

    Thread() noexcept;

    /*!
     * Primary Constructor. Initializes the ProcessJ::Thread
     * with the given resume address. This constructor requires a pointer
     * \param address The address to initialize the ProcessJ::Thread with.
     */

    template<typename Address>
    Thread(const Address) noexcept;

    /*!
     * Deconstructor. Releases any resources used by the ProcessJ::Thread.
     * At the time of writing, no resources are used or released.
     */

    ~Thread() noexcept;

};

/*!
 * Default Constructor. Initializes the ProcessJ::Thread
 * to its' default state.
 */

ProcessJ::Thread::Thread() noexcept: resumeAddress(0) { /* Empty */ }

/*!
 * Primary Constructor. Initializes the ProcessJ::Thread
 * with the given resume address.
 * \param address The address to initialize the ProcessJ::Thread with.
 */

template<typename Address>
ProcessJ::Thread::Thread(const Address address) noexcept: resumeAddress(reinterpret_cast<void*>(address)) { /* Empty */ }

/*!
 * Deconstructor. Releases any resources used by the ProcessJ::Thread.
 * At the time of writing, no resources are used or released.
 */

ProcessJ::Thread::~Thread() noexcept { this->resumeAddress = 0; }

#endif
