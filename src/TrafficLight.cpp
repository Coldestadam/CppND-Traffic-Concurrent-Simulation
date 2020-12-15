#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.
    std::unique_lock<std::mutex> lck(mtx);
    _condition.wait(lck, [this]{ return !_queue.empty(); });
    T phase = std::move(_queue.front());
    _queue.pop_front();

    return phase;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lck(MessageQueue<T>::mtx);
    MessageQueue<T>::_queue.push_back(std::move(msg));
    _condition.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true){
        TrafficLightPhase phase = _messageQueue.receive();
        if(phase == TrafficLightPhase::green){
            return;
        }
    }
}

TrafficLight::TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class.
    TrafficLight::threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<int> distribution(4000, 6000);

    //Initializing Cycle Duration
    int cycle_duration = distribution(generator);

    auto start = std::chrono::high_resolution_clock::now();

    while (true)
    {
        //Thread sleeps for one millisecond
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        //Calculating the time from last cycle
        auto end = std::chrono::high_resolution_clock::now();
        int duration = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
        
        //If the time from last cycle is greater than the random cycle duration, then a cycle will occur
        if(duration >= cycle_duration){

            //If the currentphase is red, it turns to green and sends a message of change
            if(_currentPhase == TrafficLightPhase::red){
                _currentPhase = TrafficLightPhase::green;
                _messageQueue.send(std::move(TrafficLightPhase::green));
            }

            //If the currentphase is green, it turns to red and sends a message of change
            else {
                _currentPhase = TrafficLightPhase::red;
                _messageQueue.send(std::move(TrafficLightPhase::red));
            }
            
            
            //Resetting start time between cycles and cycle duration
            start = std::chrono::high_resolution_clock::now();
            cycle_duration = distribution(generator);
        }
    }
}

