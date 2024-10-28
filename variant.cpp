#include <iostream>
#include <array>
#include <variant>
#include <chrono>

constexpr int MAX_NUM = 10000;

class Base
{
public:
    Base() = default;
    virtual ~Base() = default;
    
    virtual void foo() = 0;
};

class Derived1: public Base
{
public:
    Derived1() = default;
    ~Derived1() = default;
    
    void foo() override
    {
        c = a + b;
    }

private:
    int a{1};
    int b{2};
    int c{0};
};

class Derived2: public Base
{
public:
    Derived2() = default;
    ~Derived2() = default;
    
    void foo() override
    {
        c = a * b;
    }

private:
    float a{1.f};
    float b{2.f};
    float c{0.f};
};

int main()
{
    {
        std::array<std::unique_ptr<Base>, MAX_NUM> arr;

        for(int i = 0; i < MAX_NUM; ++i)
        {
            if(i % 2 == 0)
            {
                arr[i] = std::make_unique<Derived1>();
            } else
            {
                arr[i] = std::make_unique<Derived2>();
            }
        }
        
        auto start = std::chrono::high_resolution_clock::now();

        for(auto& obj : arr)
        {
            obj->foo();
        }

        auto finish = std::chrono::high_resolution_clock::now();
        
        auto dur = std::chrono::duration_cast<std::chrono::microseconds>(finish - start);
        std::cout << "Using pointers: " << dur.count() << "us\n";
    }

    {
        std::array<std::variant<Derived1, Derived2>, MAX_NUM> arr;
        
        for(int i = 0; i < MAX_NUM; ++i)
        {
            if(i % 2 == 0)
            {
                arr[i] = Derived1();
            } else
            {
                arr[i] = Derived2();
            }
        }
        
        auto start = std::chrono::high_resolution_clock::now();

        for(auto& obj : arr)
        {
            if(std::holds_alternative<Derived1>(obj))
            {
                std::get<Derived1>(obj).foo();
            } else if(std::holds_alternative<Derived2>(obj))
            {
                std::get<Derived2>(obj).foo();
            }
        }

        auto finish = std::chrono::high_resolution_clock::now();
        
        auto dur = std::chrono::duration_cast<std::chrono::microseconds>(finish-start);
        std::cout << "Using std::variant: " << dur.count() << "us\n";
    }

    return 0;
}
