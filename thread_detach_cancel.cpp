#include <iostream>
#include <thread>
#include <future>
#include <chrono>

void threadPrint(const std::string& msg)
{
    static std::mutex lock;
    std::lock_guard guard(lock);
    std::cout << msg << std::endl;
}

void threadFunction(std::future<void> future, const std::string& name)
{
    threadPrint("Starting thread " + name);
    const auto startTime = std::chrono::high_resolution_clock::now();
    
    std::chrono::milliseconds sleepTime{1000};
    
    if (future.wait_for(std::chrono::milliseconds(sleepTime)) == std::future_status::timeout)
    {
        threadPrint("Execute thread " + name);
    } else
    {
        threadPrint("Cancel thread " + name);
    }
    
    // Careful with prints, the main thread might end before prints are executed (e.g. remove T2 and T3)!
    const auto endTime = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    threadPrint("Exit thread " + name + " after " + std::to_string(duration) + "ms");
}

class Thread
{
public:
    Thread(const std::string& name)
    {
        std::future<void> future = m_promise.get_future();
        std::thread(&threadFunction, std::move(future), name).detach();
    }
    
    ~Thread() {}
    
    void cancel()
    {
        m_promise.set_value(); // Call only once otherwise crash!
    }

private:
    std::promise<void> m_promise;
};


int main()
{
    Thread t1{"T1"};
    Thread t2{"T2"};
    Thread t3{"T3"};
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    t1.cancel(); // t1 not executed
    
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    t3.cancel(); // t3 not executed
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    t2.cancel(); // t2 is executed, cancel is too late

    return 0;
}
