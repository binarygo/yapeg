#ifndef INCLUDED_YAPEG_ANY_H
#define INCLUDED_YAPEG_ANY_H

#include <utility>
#include <typeinfo>
#include <type_traits>
#include <exception>
#include <cassert>

namespace yapeg {

namespace any_impl {
    
template<typename T>
struct IsObj: public std::true_type {};
template<>
struct IsObj<char>: public std::false_type {};
template<>
struct IsObj<short>: public std::false_type {};
template<>
struct IsObj<unsigned short>: public std::false_type {};
template<>
struct IsObj<int>: public std::false_type {};
template<>
struct IsObj<unsigned int>: public std::false_type {};
template<>
struct IsObj<float>: public std::false_type {};
template<>
struct IsObj<double>: public std::false_type {};
template<>
struct IsObj<long long>: public std::false_type {};
template<>
struct IsObj<unsigned long long>: public std::false_type {};
    
} // close namespace any_impl
    
class Any
{
public:
    // TYPES
    class TypeMismatch: public std::exception {};
    
private:
    // TYPES
    using DeleteFunc = void(*)(void*);
    using CloneFunc = void*(*)(void*);
    enum class TypeCategory { NONE, SIMPLE, OBJ };

    // DATA
    const std::type_info* d_typeInfo;
    TypeCategory d_typeCategory;
    union
    {
        char c;
        short s;
        unsigned short us;
        int i;
        unsigned int ui;
        float f;
        double d;
        long long ll;
        unsigned long long ull;
        void* p;
    } d_data;
    DeleteFunc d_deleteFunc;
    CloneFunc d_cloneFunc;

private:
    // CLASS METHODS
    template<typename T>
    static void deleteT(void* p)
    {
        if(p) delete static_cast<T*>(p);
    }

    template<typename T>
    static void* cloneT(void* p)
    {
        assert(p);
        T* tp = new T(*(static_cast<T*>(p)));
        assert(tp);
        return tp;
    }
    
    // MANIPULATORS
    void resetNone();
    void destroy();
    void setSimpleCommon();

    template<typename T>
    void setTypeInfo()
    {
        d_typeInfo = &(typeid(T));
        assert(d_typeInfo);
    }

    // ACCESSORS
    void* clone() const;
    
    template<typename T>
    void checkTypeInfo() const
    {
        if(!is<T>()) throw TypeMismatch();
    }
    
public:
    // CREATORS
    Any();
    ~Any();
    Any(Any&& other);
    Any(const Any& other);
    
    // MANIPULATORS
    Any& operator= (Any&& rhs);
    Any& operator= (const Any& rhs);

    void clear();

    template<typename T>
    typename std::enable_if<std::is_same<T,char>::value, void>::type
    set(T t)
    {
        setTypeInfo<T>();
        setSimpleCommon();
        d_data.c = t;
    }

    template<typename T>
    typename std::enable_if<std::is_same<T,short>::value, void>::type
    set(T t)
    {
        setTypeInfo<T>();
        setSimpleCommon();
        d_data.s = t;
    }

    template<typename T>
    typename std::enable_if<std::is_same<T,unsigned short>::value, void>::type
    set(T t)
    {
        setTypeInfo<T>();
        setSimpleCommon();
        d_data.us = t;
    }

    template<typename T>
    typename std::enable_if<std::is_same<T,int>::value, void>::type
    set(T t)
    {
        setTypeInfo<T>();
        setSimpleCommon();
        d_data.i = t;
    }
    
    template<typename T>
    typename std::enable_if<std::is_same<T,unsigned int>::value, void>::type
    set(T t)
    {
        setTypeInfo<T>();
        setSimpleCommon();
        d_data.ui = t;
    }

    template<typename T>
    typename std::enable_if<std::is_same<T,float>::value, void>::type
    set(T t)
    {
        setTypeInfo<T>();
        setSimpleCommon();
        d_data.f = t;
    }

    template<typename T>
    typename std::enable_if<std::is_same<T,double>::value, void>::type
    set(T t)
    {
        setTypeInfo<T>();
        setSimpleCommon();
        d_data.d = t;
    }

    template<typename T>
    typename std::enable_if<std::is_same<T,long long>::value, void>::type
    set(T t)
    {
        setTypeInfo<T>();
        setSimpleCommon();
        d_data.ll = t;
    }

    template<typename T>
    typename std::enable_if<std::is_same<T,unsigned long long>::value, void>::type
    set(T t)
    {
        setTypeInfo<T>();
        setSimpleCommon();
        d_data.ull = t;
    }
    
    template<typename T>
    typename std::enable_if<any_impl::IsObj<T>::value, void>::type
    set(T&& t)
    {
        T* p = new T(std::forward<T>(t));
        assert(p);
        destroy();
        setTypeInfo<T>();
        d_typeCategory = TypeCategory::OBJ;
        d_data.p = p;
        d_deleteFunc = &deleteT<T>;
        d_cloneFunc = &cloneT<T>;
    }

    // ACCESSORS
    template<typename T>
    typename std::enable_if<std::is_same<T,char>::value, T>::type
    get() const
    {
        checkTypeInfo<T>();
        assert(isSimple());
        return d_data.c;
    }

    template<typename T>
    typename std::enable_if<std::is_same<T,short>::value, T>::type
    get() const
    {
        checkTypeInfo<T>();
        assert(isSimple());
        return d_data.s;
    }

    template<typename T>
    typename std::enable_if<std::is_same<T,unsigned short>::value, T>::type
    get() const
    {
        checkTypeInfo<T>();
        assert(isSimple());
        return d_data.us;
    }
    
    template<typename T>
    typename std::enable_if<std::is_same<T,int>::value, T>::type
    get() const
    {
        checkTypeInfo<T>();
        assert(isSimple());
        return d_data.i;
    }

    template<typename T>
    typename std::enable_if<std::is_same<T,unsigned int>::value, T>::type
    get() const
    {
        checkTypeInfo<T>();
        assert(isSimple());
        return d_data.ui;
    }
    
    template<typename T>
    typename std::enable_if<std::is_same<T,float>::value, T>::type
    get() const
    {
        checkTypeInfo<T>();
        assert(isSimple());
        return d_data.f;
    }    

    template<typename T>
    typename std::enable_if<std::is_same<T,double>::value, T>::type
    get() const
    {
        checkTypeInfo<T>();
        assert(isSimple());
        return d_data.d;
    }

    template<typename T>
    typename std::enable_if<std::is_same<T,long long>::value, T>::type
    get() const
    {
        checkTypeInfo<T>();
        assert(isSimple());
        return d_data.ll;
    }

    template<typename T>
    typename std::enable_if<std::is_same<T,unsigned long long>::value, T>::type
    get() const
    {
        checkTypeInfo<T>();
        assert(isSimple());
        return d_data.ull;
    }
    
    template<typename T>
    typename std::enable_if<any_impl::IsObj<T>::value, const T&>::type
    get() const
    {
        assert(isObj() && d_data.p);
        return *static_cast<T*>(d_data.p);
    }

    template<typename T>
    bool is() const
    { return d_typeInfo && *d_typeInfo == typeid(T); }
    
    bool isNone() const
    { return TypeCategory::NONE == d_typeCategory; }
    
    bool isSimple() const
    { return TypeCategory::SIMPLE == d_typeCategory; }
    
    bool isObj() const
    { return TypeCategory::OBJ == d_typeCategory; }
};

} // close namespace yapeg

#endif // INCLUDED_YAPEG_ANY_H
