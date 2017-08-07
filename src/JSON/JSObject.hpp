#ifndef JSObject_hpp
#define JSObject_hpp

#include <string>
#include <vector>
#include <map>
#include <ostream>

#include "JSONLexer.hpp"

class JSObject
{
public:
    JSObject();
    virtual ~JSObject() {}
    virtual std::string toString();
public:
    TokenType objectType;
};

std::ostream & operator << (std::ostream &os,JSObject *obj);

class JSString : public JSObject
{
public:
    JSString(std::string _str,TokenType _type);
    ~JSString() override;
    
    std::string toString() override;
    std::string *strRef;
};

class JSArray : public JSObject
{
public:
    JSArray();
    ~JSArray() override;
    
    void addObject(JSObject *obj);
    std::string toString() override;
    std::vector<JSObject *> *arrayRef;
    
    decltype(arrayRef->begin()) begin();
    decltype(arrayRef->end()) end();
};

class JSMap : public JSObject
{
public:
    JSMap();
    ~JSMap() override;
    
    std::string toString() override;
    void setObjectAndKey(std::string key,JSObject *obj);
    std::map<std::string,JSObject *> *mapRef;
};

#endif