#include <iostream>
#include <array>
#include <variant>
#include <chrono>
#include <cstring>

constexpr int MAX_NUM = 10000;
constexpr int NUM_LOOPS = 10;

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
        for(int i = 0; i < NUM_LOOPS; ++i)
        {
            c = a + b + i;
        }
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
        for(int i = 0; i < NUM_LOOPS; ++i)
        {
            c = a * b * i;
        }
    }

private:
    float a{1.f};
    float b{2.f};
    float c{0.f};
};

int main()
{
    // Test std::array of std::unique_ptrs
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

    // Test std::array of std::variants
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

    // Test std::array of pointers to objects in other arrays
    {
        std::array<Base*, MAX_NUM> arr;
        std::array<std::unique_ptr<Derived1>, MAX_NUM> der1s;
        std::array<std::unique_ptr<Derived2>, MAX_NUM> der2s;

        for(int i = 0; i < MAX_NUM; ++i)
        {
            der1s[i] = std::make_unique<Derived1>();
            der2s[i] = std::make_unique<Derived2>();
        }

        for(int i = 0; i < MAX_NUM; ++i)
        {
            if(i % 2 == 0)
            {
                arr[i] = der1s[i].get();
            } else
            {
                arr[i] = der2s[i].get();
            }
        }
        
        auto start = std::chrono::high_resolution_clock::now();

        for(auto& obj : arr)
        {
            obj->foo();
        }

        auto finish = std::chrono::high_resolution_clock::now();
        
        auto dur = std::chrono::duration_cast<std::chrono::microseconds>(finish - start);
        std::cout << "Using pointers to objects: " << dur.count() << "us\n";
    }
    
    // Test union
    {
        struct UnionOfDerived
        {
            bool isDer1;
            
            union
            {
                Derived1 der1;
                Derived2 der2;
            };
            
            UnionOfDerived() { std::memset(this, 0, sizeof(UnionOfDerived)); }
            ~UnionOfDerived() {}
            
            void selectDer1()
            {
                isDer1 = true;
                der1 = Derived1();
            }
            
            void selectDer2()
            {
                isDer1 = false;
                der2 = Derived2();
            }
        };
        
        std::array<UnionOfDerived, MAX_NUM> arr;
        
        for(int i = 0; i < MAX_NUM; ++i)
        {
            if(i % 2 == 0)
            {
                arr[i].selectDer1();
            } else
            {
                arr[i].selectDer2();
            }
        }
        
        auto start = std::chrono::high_resolution_clock::now();

        for(auto& obj : arr)
        {
            if(obj.isDer1)
            {
                obj.der1.foo();
            } else
            {
                obj.der2.foo();
            }
        }

        auto finish = std::chrono::high_resolution_clock::now();
        
        auto dur = std::chrono::duration_cast<std::chrono::microseconds>(finish - start);
        std::cout << "Using union: " << dur.count() << "us\n";
    }

    return 0;
}
