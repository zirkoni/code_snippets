// ex 1, 2
#include <iostream>
#include <print>
#include <string>
// ex 3
#include <stacktrace>

// Compile with: g++ -std=c++23


//////////////////////////////////////////////////////////////
// Explicit object parameters
//////////////////////////////////////////////////////////////
class Dog
{
public:
    Dog(const std::string& name): m_name(name) {}

    template<typename Self>
    auto&& name(this Self&& self)
    {
        return std::forward<Self>(self).m_name;
    }
    
private:
    std::string m_name;
};

void example1()
{
    Dog d("Koira");
    d.name() = "Musti";
    std::println("{}", d.name());
    
    const Dog& constDog = d;
    std::println("{}", constDog.name());
}
//////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////
// Monadic operations for std::optional
//////////////////////////////////////////////////////////////
std::optional<int> stringToInteger(const std::string& s)
{
    try
    {
        return std::stoi(s);
    } catch(...)
    {
        return {};
    }
}

void example2()
{
    std::string input;
    std::print("Enter a number: ");
    std::getline(std::cin, input);

    auto result = stringToInteger(input)
        .and_then  ( [](int value) -> std::optional<int> { return value * value; } )
        .transform ( [](int value) { return "Squared: " + std::to_string(value); } )
        .or_else   ( [] { return std::optional<std::string>("That's not a number!"); });

    std::println("{}", *result);
}
//////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////
// Stacktrace
//////////////////////////////////////////////////////////////
size_t foo(const std::string& s)
{
    return s.length();
}

void bar(const std::string s, size_t len)
{
    std::println("Length of {} is {}", s, len);

    // Not compiling with GCC 14.2.1
    //auto trace{ std::stacktrace::current() };
    //std::println("{}", trace);
}

void fooBar()
{
    std::string s("fooBar");
    auto len = foo(s);
    bar(s, len);
}

void example3()
{
    fooBar();
}

int main()
{
    example1();
    example2();
    example3();
}

