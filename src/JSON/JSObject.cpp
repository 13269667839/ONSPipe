#include "JSObject.hpp"
#include "../Utility/Util.hpp"

#pragma mark -- JSObject
JSObject::JSObject()
{
    objectType = TokenType::Null;
}

std::string JSObject::toString()
{
    return "null";
}

std::ostream & operator << (std::ostream &os,JSObject *obj)
{
    if (obj)
    {
        os<<obj->toString();
    }
    return os;
}

#pragma mark -- JSString
JSString::JSString(std::string _str,TokenType _type)
{
    if (_type == TokenType::Array || _type == TokenType::Map)
    {
        throwError("can not convert to string");
    }
    
    strRef = _str.empty()?nullptr:new std::string(_str);
    objectType = _type;
}

JSString::~JSString()
{
    if (strRef)
    {
        strRef->clear();
        delete strRef;
        strRef = nullptr;
    }
}

std::string JSString::toString()
{
    auto res = std::string();
    if (strRef && !strRef->empty())
    {
        res = std::move(*strRef);
        
        if (objectType == TokenType::String)
        {
            res = "\"" + res + "\"";
        }
    }
    return res;
}

#pragma mark -- JSArray
JSArray::JSArray()
{
    arrayRef = nullptr;
    objectType = TokenType::Array;
}

JSArray::~JSArray()
{
    if (arrayRef)
    {
        for (auto obj : *arrayRef)
        {
            delete obj;
        }
        std::vector<JSObject *>().swap(*arrayRef);
        delete arrayRef;
        arrayRef = nullptr;
    }
}

void JSArray::addObject(JSObject *obj)
{
    if (!arrayRef)
    {
        arrayRef = new std::vector<JSObject *>();
    }
    arrayRef->push_back(obj);
}

std::string JSArray::toString()
{
    auto res = std::string();
    if (arrayRef && !arrayRef->empty())
    {
        for (auto obj : *arrayRef)
        {
            if (res.empty())
            {
                res += obj->toString();
            }
            else
            {
                res += "," + obj->toString();
            }
        }
    }
    return res.empty()?res:"[" + res + "]";
}

decltype(JSArray::arrayRef->begin()) JSArray::begin()
{
    return arrayRef->begin();
}

decltype(JSArray::arrayRef->end()) JSArray::end()
{
    return arrayRef->end();
}

#pragma mark -- JSMap
JSMap::JSMap()
{
    mapRef = nullptr;
    objectType = TokenType::Map;
}

JSMap::~JSMap()
{
    if (mapRef)
    {
        for (auto pair : *mapRef)
        {
            if (pair.second)
            {
                delete pair.second;
            }
        }
        mapRef->clear();
        delete mapRef;
        mapRef = nullptr;
    }
}

void JSMap::setObjectAndKey(std::string key,JSObject *obj)
{
    if (!mapRef)
    {
        mapRef = new std::map<std::string,JSObject *>();
    }
    mapRef->insert(std::make_pair(key, obj));
}

std::string JSMap::toString()
{
    auto res = std::string();
    if (mapRef && !mapRef->empty())
    {
        for (auto pair : *mapRef)
        {
            if (res.empty())
            {
                res += pair.first + ":" + pair.second->toString();
            }
            else
            {
                res += "," + pair.first + ":" + pair.second->toString();
            }
        }
    }
    return res.empty()?res:"{" + res + "}";
}
