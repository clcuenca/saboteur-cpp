/*!
 * \brief Opal Saboteur Observer
 *
 * Defines an interface that should be implemented by any type
 * that is to observe a Opal::Saboteur. This is the means
 * of communication from the Opal::Saboteur about its'
 * lifecycle state.
 *
 * \author Carlos L. Cuenca
 * \version 0.9.0
 * \date 12/16/2021
 */

#ifndef OPAL_SABOTEUR_OBSERVER_HPP
#define OPAL_SABOTEUR_OBSERVER_HPP

/// --------
/// Includes

#include<cinttypes>

// We want it a little cleaner
namespace Opal { class SaboteurObserver; }

class Opal::SaboteurObserver {

    /// ---------------
    /// Private Members

private:

    // Omit from documentation
    #define Indirect(data) reinterpret_cast<uint64_t*>(data)

    // Omit from documentation
    // We have this so we can more easily dispatch the callbacks
    // as defined by derived classes from the assembly code.
    uint64_t* vtable[7];

    // Omit from documentation
    // We initialize the vtable here
    template<typename Type>
    void initializeVTable() {

        using Callback = void (Type::*)(void*);

        struct {

            Callback callback[7];

        } indirection = { &Type::OnCreated, &Type::OnResume,    &Type::OnStarted,
                          &Type::OnSuicide, &Type::OnSuspended, &Type::OnTerminated,
                          &Type::OnWaiting };

        // Supposedly the compiler orders these alphabetically
        // but we don't want to run the risk of it doing something else
        this->vtable[0] = *(reinterpret_cast<uint64_t**>(&indirection));
        this->vtable[1] = *(reinterpret_cast<uint64_t**>(&indirection) + 1);
        this->vtable[2] = *(reinterpret_cast<uint64_t**>(&indirection) + 2);
        this->vtable[3] = *(reinterpret_cast<uint64_t**>(&indirection) + 3);
        this->vtable[4] = *(reinterpret_cast<uint64_t**>(&indirection) + 4);
        this->vtable[5] = *(reinterpret_cast<uint64_t**>(&indirection) + 5);
        this->vtable[6] = *(reinterpret_cast<uint64_t**>(&indirection) + 6);

    }

    /// -----------------
    /// Protected Members

protected:

    SaboteurObserver() { this->initializeVTable<SaboteurObserver>(); }

    /// --------------
    /// Public Members

public:

    /*!
     * Template Constructor. Any classes implementing this interface
     * require an invocation of this constructor for Opal::Saboteur
     * to callback correctly.
     * \param type Pointer to the Type that implements this interface.
     */

    template<typename Type>
    SaboteurObserver(Type* type) { this->initializeVTable<Type>(); }

    /*!
     * Empty deconstructor; required for polymorphism.
     */

    virtual ~SaboteurObserver() noexcept { /* Empty */ }

    /// ---------
    /// Interface

    /*!
     * Invoked when the Opal::Saboteur is created.
     * \param thread The newly created Opal::Saboteur
     */

    virtual void OnCreated(void*) { /* Empty */ }

    /*!
     * Invoked when the Opal::Saboteur is in a waiting state.
     * It is possible that SaboteurObserver::OnStarted(void*) is
     * called back immediately after (in the case that a return
     * address was assigned to the Opal::Saboteur).
     * \param thread The waiting Opal::Saboteur
     */

    virtual void OnWaiting(void*) { /* Empty */ }

    /*!
     * Invoked when the Opal::Saboteur has started execution
     * but before it has executed any associated code
     * \param thread The started Opal::Saboteur
     */

    virtual void OnStarted(void*) { /* Empty */ }

    /*!
     * Invoked when the Opal::Saboteur has successfully been suspended.
     * \param thread The suspended Opal::Saboteur.
     */

    virtual void OnSuspended(void*) { /* Empty */ }

    /*!
     * Invoked when the Opal::Saboteur has been resumed from
     * a suspended state.
     * \param thread The restarted Opal::Saboteur
     */

    virtual void OnResume(void*) { /* Empty */ }

    /*!
     * Invoked when the Opal::Saboteur is destroying
     * itself.
     * \param thread Thre Opal::Saboteur that is committing
     * suicide
     */

    virtual void OnSuicide(void*) { /* Empty */ }

    /*!
     * Invoked when the Opal::Saboteur has terminated.
     * \param thread The Opal::Saboteur that has finished.
     */

    virtual void OnTerminated(void*) { /* Empty */ }

};

#endif
