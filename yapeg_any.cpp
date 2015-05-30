#include <yapeg_any.h>

namespace yapeg {

// CREATORS
Any::Any()
{
    resetNone();
}

Any::~Any()
{
    clear();
}

Any::Any(Any&& other)
    : d_typeInfo(other.d_typeInfo)
    , d_typeCategory(other.d_typeCategory)
    , d_data(other.d_data)
    , d_deleteFunc(other.d_deleteFunc)
    , d_cloneFunc(other.d_cloneFunc)
{
    other.resetNone();
}
    
Any::Any(const Any& other)
    : d_typeInfo(other.d_typeInfo)
    , d_typeCategory(other.d_typeCategory)
    , d_data(other.d_data)
    , d_deleteFunc(other.d_deleteFunc)
    , d_cloneFunc(other.d_cloneFunc)
{
    if(other.isObj())
    {
        d_data.p = other.clone();
    }
}

// MANIPULATORS
Any& Any::operator= (Any&& rhs)
{
    if(this != &rhs)
    {
        destroy();
        d_typeInfo = rhs.d_typeInfo;
        d_typeCategory = rhs.d_typeCategory;
        d_data = rhs.d_data;
        d_deleteFunc = rhs.d_deleteFunc;
        d_cloneFunc = rhs.d_cloneFunc;
        rhs.resetNone();
    }
    return *this;
}

Any& Any::operator= (const Any& rhs)
{
    if(this != &rhs)
    {
        Any tmp(rhs);
        operator=(std::move(tmp));
    }
    return *this;
}

void Any::resetNone()
{
    d_typeInfo = 0;
    d_typeCategory = TypeCategory::NONE;
    d_data.p = 0;
    d_deleteFunc = 0;
    d_cloneFunc = 0;
}

void Any::destroy()
{
    if(isObj())
    {
        assert(d_deleteFunc);
        d_deleteFunc(d_data.p);
        d_data.p = 0;
    }
}

void Any::setSimpleCommon()
{
    destroy();
    d_typeCategory = TypeCategory::SIMPLE;
    d_deleteFunc = 0;
    d_cloneFunc = 0;
}
    
void Any::clear()
{
    destroy();
    resetNone();
}

// ACCESSORS
void* Any::clone() const
{
    assert(isObj() && d_cloneFunc);
    return d_cloneFunc(d_data.p);
}

} // close namespace yapeg
