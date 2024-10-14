#pragma once

class NodeFactory
{
public:
    template<typename Type>
    static void* RegisterType()
    {
        return new Type();
    }

    using function = void* (*)();

    static std::map<std::string_view, function>& get();

    template<typename Type = void>
    static Type* Create(const std::string_view& name)
    {
        std::map<std::string_view, function>::const_iterator itFind = get().find(name);
        if (itFind == get().end())
            return nullptr;

        return (Type*)itFind->second();
    }

    template<typename Type>
    static bool Register(const std::string_view& name)
    {
        std::map<std::string_view, function>::const_iterator itFind = get().find(name);
        if (itFind != get().end())
            return false;

        get()[name] = &RegisterType<Type>;
        return true;
    }
};

#define STRINGS(x) #x

#define Register_Node(Type) \
namespace \
{\
	static bool registerName = NodeFactory::Register<Type>(STRINGS(Type));\
}
