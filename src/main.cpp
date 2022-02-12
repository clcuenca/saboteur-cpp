#include<iostream>
#include<Opal.hpp>

static void SomeOtherFunction(void) {

  std::cout << "Swapped!" << std::endl;


}

static void SomeFunction(void) {

  std::cout << "Counting" << std::endl;

  uint64_t index = 0;

  for(; index < 100; index++);

  std::cout << "Finished counting" << std::endl;

}


class DummyObserver : public Opal::SaboteurObserver {

public:


    /*!
     * Invoked when the Opal::Saboteur is created.
     * \param thread The newly created Opal::Saboteur
     */

    void OnCreated(void* thread) {

      std::cout << "OnCreated: " << thread << std::endl;

    }

    /*!
     * Invoked when the Opal::Saboteur is in a waiting state.
     * It is possible that SaboteurObserver::OnStarted(void*) is
     * called back immediately after (in the case that a return
     * address was assigned to the Opal::Saboteur).
     * \param thread The waiting Opal::Saboteur
     */

    void OnWaiting(void* thread) {

        std::cout << "OnWaiting: " << thread << std::endl;

    }

    /*!
     * Invoked when the Opal::Saboteur has started execution
     * but before it has executed any associated code
     * \param thread The started Opal::Saboteur
     */

    void OnStarted(void* thread) {

        std::cout << "OnStarted: " << thread << std::endl;


    }

    /*!
     * Invoked when the Opal::Saboteur has successfully been suspended.
     * \param thread The suspended Opal::Saboteur.
     */

    void OnSuspended(void* thread) {

      std::cout << "OnSuspended: " << thread << std::endl;

    }

    /*!
     * Invoked when the Opal::Saboteur has been restarted from
     * a suspended state.
     * \param thread The restarted Opal::Saboteur
     */

    void OnResume(void* thread) {

      std::cout << "OnResume: " << thread << std::endl;

    }

    /*!
     * Invoked when the Opal::Saboteur has finished.
     * \param thread The Opal::Saboteur that has finished.
     */

    void OnFinished(void* thread) {

      std::cout << "OnFinished: " << thread << std::endl;

    }

};

int main() {

    // Create an observer.
    DummyObserver observer;

    uint64_t* array = new uint64_t[64];

    std::cout << "test: " << array << std::endl;

    // Create the thread.
    Opal::Saboteur thread((Opal::SaboteurObserver*)(&observer));

    bool pushed = false;

    while(!thread.isWaiting()) Yield;

    // We wait until the thread has completed.
    while(true) {

      if(!pushed) {

        auto Function = reinterpret_cast<void*>(SomeFunction);
        auto OtherFunction = reinterpret_cast<void*>(&SomeOtherFunction);

        std::cout << "Setting execution address to: " << Function << std::endl;

        thread.push(Function, true);

        pushed = true;

      } else Yield;

    }

    // Finally, return
    return 0;

}
