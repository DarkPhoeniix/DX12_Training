#pragma once

class ComponentFactory
{
public:
    using function = void* (*)();

    template<typename Type>
    static void* RegisterType()
    {
        return new Type();
    }

    static std::map<std::string, function>& get();

    template<typename Type = void>
    static Type* Create(const std::string& name)
    {
        std::map<std::string, function>::const_iterator itFind = get().find(name);

        if (itFind == get().end())
        {
            return nullptr;
        }

        return (Type*)itFind->second();
    }

    template<typename Type>
    static bool Register(const std::string& name)
    {
        std::map<std::string, function>::const_iterator itFind = get().find(name);

        if (itFind != get().end())
        {
            return false;
        }

        get()[name] = &RegisterType<Type>;

        return true;
    }
};

#define STRINGS1(x) STRINGS(x)
#define STRINGS(x) #x

#define Register_Component(A) \
namespace \
{\
	static bool registerName = ComponentFactory::Register<A>(STRINGS1(A));\
}
