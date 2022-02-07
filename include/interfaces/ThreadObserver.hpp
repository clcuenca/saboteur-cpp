/*!
 * \brief ProcessJ Thread Observer
 *
 * Defines an interface that should be implemented by any type
 * that is to observe a ProcessJ::Thread. This is the means
 * of communication from the ProcessJ::Thread about its'
 * lifecycle state.
 *
 * \author Carlos L. Cuenca
 * \version 0.9.0
 * \date 12/16/2021
 */

#ifndef PROCESS_J_THREAD_OBSERVER_HPP
#define PROCESS_J_THREAD_OBSERVER_HPP

/// --------
/// Includes

// We want it a little cleaner
namespace ProcessJ { class ThreadObserver; }

class ProcessJ::ThreadObserver {

    /// --------------
    /// Public Members

public:

    /*!
     * Empty deconstructor; required for polymorphism.
     */

    virtual ~ThreadObserver() noexcept { /* Empty */ }

    /// ---------
    /// Interface

    /*!
     * Invoked when the ProcessJ::Thread is created.
     * \param thread The newly created ProcessJ::Thread
     */

    virtual void OnCreated(void*) = 0;

    /*!
     * Invoked when the ProcessJ::Thread is in a waiting state.
     * It is possible that ThreadObserver::OnStarted(void*) is
     * called back immediately after (in the case that a return
     * address was assigned to the ProcessJ::Thread).
     * \param thread The waiting ProcessJ::Thread
     */

    virtual void OnWaiting(void*) = 0;

    /*!
     * Invoked when the ProcessJ::Thread has started execution
     * but before it has executed any associated code
     * \param thread The started ProcessJ::Thread
     */

    virtual void OnStarted(void*) = 0;

    /*!
     * Invoked when the ProcessJ::Thread has successfully been suspended.
     * \param thread The suspended ProcessJ::Thread.
     */

    virtual void OnSuspended(void*) = 0;

    /*!
     * Invoked when the ProcessJ::Thread has been restarted from
     * a suspended state.
     * \param thread The restarted ProcessJ::Thread
     */

    virtual void OnRestart(void*) = 0;

    /*!
     * Invoked when the ProcessJ::Thread has finished.
     * \param thread The ProcessJ::Thread that has finished.
     */

    virtual void OnFinished(void*) = 0;

};

#endif
