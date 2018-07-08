#include "JSObject.hpp"
#include "../Utility/STLExtern.hpp"

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

std::ostream & operator << (std::ostream &os,JSObject &obj)
{
    os<<obj.toString();
    return os;
}

#pragma mark -- JSNumber
JSNumber::JSNumber(std::string _str,TokenType _type)
{
    if (_type == TokenType::Integer)
    {
        objectType = _type;
        numberVal.intVal = atol(_str.c_str());
    }
    else if (_type == TokenType::Float)
    {
        objectType = _type;
        numberVal.floatVal = atof(_str.c_str());
    }
    else if (_type == TokenType::Boolean)
    {
        objectType = _type;
        numberVal.intVal = _str == "true" ? 1 : 0;
    }
    else 
    {
        throwError("unexcept JSNumber initial params");
    }
}

std::string JSNumber::toString()
{
    auto strRef = std::string();

    switch (objectType)
    {
        case TokenType::Integer:
            strRef += std::to_string(numberVal.intVal);
            break;
        case TokenType::Float:
            strRef += std::to_string(numberVal.floatVal);
            break;
        case TokenType::Boolean:
            strRef += numberVal.intVal == 1 ? "true" : "false";
            break;
        default:
            break;
    }

    return strRef;
}

#pragma mark -- JSString
JSString::JSString(std::string _str)
{
    strRef = _str.empty()?nullptr:new std::string(_str);
    objectType = TokenType::String;
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
        STLExtern::releaseVector(arrayRef);
        delete arrayRef;
        arrayRef = nullptr;
    }
}

void JSArray::addObject(std::shared_ptr<JSObject> obj)
{
    if (!obj) 
    {
        return;
    }

    if (!arrayRef)
    {
        arrayRef = new std::vector<std::shared_ptr<JSObject>>();
    }
    
    arrayRef->emplace_back(obj);
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
        STLExtern::releaseMap(mapRef);
        delete mapRef;
        mapRef = nullptr;
    }
}

void JSMap::setObjectAndKey(std::string key,std::shared_ptr<JSObject> value)
{
    if (key.empty() || !value) 
    {
        return;
    }

    if (!mapRef)
    {
        mapRef = new std::map<std::string,std::shared_ptr<JSObject>>();
    }

    mapRef->emplace(std::make_pair(key, value));
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
