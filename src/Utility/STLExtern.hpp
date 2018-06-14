#ifndef STLExtern_hpp
#define STLExtern_hpp

#include <vector>
#include <map>
#include <memory>

namespace STLExtern
{
template <typename ValueType>
void releaseVector(std::vector<ValueType> &vec)
{
    if (vec.empty())
    {
        return;
    }

    for (auto val : vec)
    {
        if (val)
        {
            delete val;
            val = nullptr;
        }
    }

    std::vector<ValueType>().swap(vec);
}

template <typename ValueType>
void releaseVector(std::vector<ValueType> *vec)
{
    if (!vec || vec->empty())
    {
        return;
    }

    for (auto ite = vec->begin(); ite != vec->end(); ++ite)
    {
        auto val = *ite;
        if (val != nullptr)
        {
            delete val;
            val = nullptr;
        }
    }

    std::vector<ValueType>().swap(*vec);
}

template <typename ValueType>
void releaseVector(std::vector<std::shared_ptr<ValueType>> *vec)
{
    if (!vec || vec->empty())
    {
        return;
    }

    for (auto ite = vec->begin(); ite != vec->end(); ++ite)
    {
        auto val = *ite;
        if (val != nullptr)
        {
            val.reset();
            val = nullptr;
        }
    }

    std::vector<std::shared_ptr<ValueType>>().swap(*vec);
}

template <typename keyType, typename valueType>
void releaseMap(std::map<keyType, valueType> *dic)
{
    if (!dic || dic->empty())
    {
        return;
    }

    for (auto ite = dic->begin(); ite != dic->end(); ++ite)
    {
        if (ite->second != nullptr)
        {
            delete ite->second;
            ite->second = nullptr;
        }
    }

    dic->clear();
}

template <typename keyType, typename valueType>
void releaseMap(std::map<keyType, std::shared_ptr<valueType>> *dic)
{
    if (!dic || dic->empty())
    {
        return;
    }

    for (auto ite = dic->begin(); ite != dic->end(); ++ite)
    {
        if (ite->second != nullptr)
        {
            ite->second.reset();
            ite->second = nullptr;
        }
    }

    dic->clear();
}
};

#endif