/*************************************************************************
 * 
 * Foundation42. CONFIDENTIAL
 * ===========================
 * 
 *  Copyright (C) [2013] - [2023] Foundation42.
 *  All Rights Reserved.
 * 
 * NOTICE:  All information contained herein is, and remains
 * the property of Foundation42. and its suppliers, if any.
 * The intellectual and technical concepts contained herein are
 * proprietary to Foundation42. and its suppliers and may be
 * covered by European, U.S. and/or Foreign Patents, patents in process, and
 * are protected by trade secret or copyright law.
 * 
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from an authorized Officer of Foundation42.
 * 
*************************************************************************/

#ifndef SUBSTRATE_ProbabalisticMap_H
#define SUBSTRATE_ProbabalisticMap_H

#include <cstdint>
#include <functional>
#include <cassert>

template <typename Key_t, typename Value_t>
class ProbabalisticMap
{
private:
    struct Node
    {
        Key_t Key;
        Value_t Value;
        std::size_t Probability { 0 };
        Node* Next { nullptr };
    };

    mutable Node* Head { nullptr };
    std::size_t ItemCount { 0 };

public:
    ProbabalisticMap() = default;

    ProbabalisticMap(const ProbabalisticMap& other)
    {
        other.ForEach([this](const auto& lhs, const auto& rhs)
        {
            this->Set(lhs, rhs);
            return true;
        });
    }

    ProbabalisticMap(ProbabalisticMap&& other) :
        Head(std::move(other.Head)),
        ItemCount(std::move(other.ItemCount))
    {
        assert(false);
    }

    ~ProbabalisticMap()
    {
        this->Clear();
    }

    void Clear()
    {
        Node* current { this->Head };

        while (current)
        {
            auto next { current->Next };
            delete current;

            current = next;
        }

        this->Head = nullptr;
        this->ItemCount = 0;
    }

    typedef std::function<void (const Key_t& key)> keyCallback;

    void ForEachKey(keyCallback callback) const
    {
        Node* current { this->Head };

        while (current)
        {
            callback(current->Key);

            current = current->Next;
        }
    }

    typedef std::function<bool (const Key_t& key, const Value_t& value)> kvCallback;

    void ForEach(kvCallback callback) const
    {
        Node* current { this->Head };

        while (current)
        {
            if (!callback(current->Key, current->Value))
                break;

            current = current->Next;
        }
    }

    Node* GetAt(std::size_t index)
    {
        Node* current { this->Head };

        for (std::size_t i = 0; i <= index; i++)
        {
            if (current == nullptr)
                break;
            
            if (i == index)
                return current;
            
            current = current->Next;
        }

        return nullptr;           
    }

    Node* PushNodeAtFront(const Key_t& key) 
    {
        auto newNode { new Node() };
        newNode->Key = key;
        newNode->Next = this->Head;
        this->Head = newNode;
        this->ItemCount++;

        return newNode;
    }

    Node* Find(const Key_t& key) const
    {
        Node* current { this->Head };
        Node* previous { nullptr };
    
        // search through the nodes
        while (current != nullptr)
        {
            // check if we found it
            if (current->Key == key)
            {
                // great, we found it
                
                // if node at the front we are done
                if (previous == nullptr)
                    return current;

                // update the probability
                current->Probability++;

                // check if probability is higher
                // than the front node

                if (current->Probability < this->Head->Probability)
                    return current;

                // move it to the front
                previous->Next = current->Next;
                current->Next = this->Head;
                this->Head = current;

                return current;
            }

            previous = current;
            current = current->Next;
        }

        // we couldn't find it
        return nullptr;
    }

    std::size_t Count() const
    {
        return this->ItemCount;
    }

    Node* FindOrCreate(const Key_t& key)
    {
        auto node { this->Find(key) };

        if (node != nullptr)
            return node;

        // we couldn't find it
        // so create one at the front
        return PushNodeAtFront(key);
    }

    void Set(const Key_t& key, const Value_t& value)
    {
        auto node { this->FindOrCreate(key) };
        node->Value = value;
    }

    Value_t* Get(const Key_t& key) const
    {
        auto node { this->Find(key) };
        if (node == nullptr)
            return nullptr;

        return &node->Value;
    }

    ProbabalisticMap& operator<<(const ProbabalisticMap& other)
    {
        assert(&other != this);

        other.ForEach([this](const auto& lhs, const auto& rhs)
        {
            this->Set(lhs, rhs);
            return true;
        });

        return *this;
    }

    ProbabalisticMap& operator=(const ProbabalisticMap& other)
    {
        assert(&other != this);
        
        this->Clear();

        other.ForEach([this](const auto& lhs, const auto& rhs)
        {
            this->Set(lhs, rhs);
            return true;
        });

        return *this;
    }

    ProbabalisticMap& operator=(ProbabalisticMap&& other)
    {
        assert(&other != this);

        this->Clear();

        this->Head = std::move(other.Head);
        this->ItemCount = std::move(other.ItemCount);

        return *this;
    }
};

#endif // SUBSTRATE_ProbabalisticMap_H
