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
    Derived1(int a = 1, int b = 2, int c = 0): m_a(a), m_b(b), m_c(c) {}
    ~Derived1() = default;
    
    void foo() override
    {
        for(int i = 0; i < NUM_LOOPS; ++i)
        {
            m_c = m_a + m_b + i;
        }
    }

private:
    int m_a;
    int m_b;
    int m_c;
};

class Derived2: public Base
{
public:
    Derived2(float a = 1.0f, float b = 2.0f, float c = 0.0f): m_a(a), m_b(b), m_c(c) {}
    ~Derived2() = default;
    
    void foo() override
    {
        for(int i = 0; i < NUM_LOOPS; ++i)
        {
            m_c = m_a * m_b * i;
        }
    }

private:
    float m_a;
    float m_b;
    float m_c;
};

// Static pool, no deallocation method!
template<size_t SIZE>
class MemoryPool
{
public:
    MemoryPool()
    {
        m_used = 0;
    }

    uint64_t* allocate(size_t size)
    {
        uint8_t* ptr = m_pool + m_used;
        m_used += size;

        if(m_used > SIZE)
        {
            throw std::bad_alloc();
        }

        return reinterpret_cast<uint64_t*>(ptr);
    }
    
private:
    uint8_t m_pool[SIZE];
    size_t m_used;
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
                arr[i] = std::make_unique<Derived1>(i, 2 * i, 10);
            } else
            {
                arr[i] = std::make_unique<Derived2>(2.0f * i, 3.0f, 0.0f);
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
    
    // Test std::array of ptrs with a memory pool
    {
        constexpr size_t poolSize = MAX_NUM * std::max(sizeof(Derived1), sizeof(Derived2));
        
        MemoryPool<poolSize> pool;
        std::array<Base*, MAX_NUM> arr;

        for(int i = 0; i < MAX_NUM; ++i)
        {
            if(i % 2 == 0)
            {
                uint64_t* buffer = pool.allocate(sizeof(Derived1));
                arr[i] = new (buffer) Derived1(1, 2, 0);
            } else
            {
                uint64_t* buffer = pool.allocate(sizeof(Derived2));
                arr[i] = new (buffer) Derived2(0.0f, 0.0f, 0.0f);
            }
        }
        
        auto start = std::chrono::high_resolution_clock::now();

        for(auto& obj : arr)
        {
            obj->foo();
        }

        auto finish = std::chrono::high_resolution_clock::now();
        
        auto dur = std::chrono::duration_cast<std::chrono::microseconds>(finish - start);
        std::cout << "Using memory pool: " << dur.count() << "us\n";
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
