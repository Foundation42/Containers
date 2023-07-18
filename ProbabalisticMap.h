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

// Template class for a probabilistic map.
template <typename Key_t, typename Value_t>
class ProbabalisticMap
{
private:
    // Structure for a node in the map.
    struct Node
    {
        Key_t Key;
        Value_t Value;
        std::size_t Probability { 0 };
        Node* Next { nullptr };
    };

    mutable Node* Head { nullptr }; // Head of the map.
    std::size_t ItemCount { 0 }; // Number of items in the map.

public:
    // Default constructor.
    ProbabalisticMap() = default;

    // Copy constructor.
    ProbabalisticMap(const ProbabalisticMap& other)
    {
        // Copy each item from the other map.
        other.ForEach([this](const auto& lhs, const auto& rhs)
        {
            this->Set(lhs, rhs);
            return true;
        });
    }

    // Move constructor.
    ProbabalisticMap(ProbabalisticMap&& other) noexcept :
        Head(std::move(other.Head)),
        ItemCount(std::move(other.ItemCount))
    {
        // The assert(false) has been removed here.
    }

    // Destructor.
    ~ProbabalisticMap()
    {
        // Clear the map.
        this->Clear();
    }

    // Clear all items from the map.
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

    // Using declaration for a function that takes a key.
    using keyCallback = std::function<void (const Key_t& key)>;

    // Apply the given function to each key in the map.
    void ForEachKey(keyCallback callback) const
    {
        Node* current { this->Head };

        while (current)
        {
            callback(current->Key);
            current = current->Next;
        }
    }

    // Using declaration for a function that takes a key and a value.
    using kvCallback = std::function<bool (const Key_t& key, const Value_t& value)>;

    // Apply the given function to each key-value pair in the map.
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

    // Get the node at the given index in the map.
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

    // Insert a new node with the given key at the front of the map.
    Node* PushNodeAtFront(const Key_t& key) 
    {
        auto newNode { new Node() };
        newNode->Key = key;
        newNode->Next = this->Head;
        this->Head = newNode;
        this->ItemCount++;

        return newNode;
    }

    // Find the node with the given key in the map.
    Node* Find(const Key_t& key) const
    {
        Node* current { this->Head };
        Node* previous { nullptr };
    
        // Search through the nodes.
        while (current != nullptr)
        {
            // Check if we found it.
            if (current->Key == key)
            {
                // If node at the front we are done.
                if (previous == nullptr)
                    return current;

                // Update the probability.
                current->Probability++;

                // Move it to the front if its probability is higher than the front node.
                if (current->Probability < this->Head->Probability)
                    return current;

                previous->Next = current->Next;
                current->Next = this->Head;
                this->Head = current;

                return current;
            }

            previous = current;
            current = current->Next;
        }

        // We couldn't find it.
        return nullptr;
    }

    // Get the number of items in the map.
    std::size_t Count() const
    {
        return this->ItemCount;
    }

    // Find the node with the given key, or create one if it does not exist.
    Node* FindOrCreate(const Key_t& key)
    {
        auto node { this->Find(key) };

        if (node != nullptr)
            return node;

        // If we couldn't find it, create one at the front.
        return PushNodeAtFront(key);
    }

    // Set the value for the given key in the map.
    void Set(const Key_t& key, const Value_t& value)
    {
        auto node { this->FindOrCreate(key) };
        node->Value = value;
    }

    // Get the value for the given key in the map.
    Value_t* Get(const Key_t& key) const
    {
        auto node { this->Find(key) };
        if (node == nullptr)
            return nullptr;

        return &node->Value;
    }

    // Overloaded << operator for merging another map into this one.
    ProbabalisticMap& operator<<(const ProbabalisticMap& other)
    {
        assert(&other != this);

        // Merge each item from the other map into this one.
        other.ForEach([this](const auto& lhs, const auto& rhs)
        {
            this->Set(lhs, rhs);
            return true;
        });

        return *this;
    }

    // Overloaded = operator for copying another map into this one.
    ProbabalisticMap& operator=(const ProbabalisticMap& other)
    {
        assert(&other != this);
        
        // Clear this map and then copy each item from the other map.
        this->Clear();
        other.ForEach([this](const auto& lhs, const auto& rhs)
        {
            this->Set(lhs, rhs);
            return true;
        });

        return *this;
    }

    // Overloaded = operator for moving another map into this one.
    ProbabalisticMap& operator=(ProbabalisticMap&& other) noexcept
    {
        assert(&other != this);

        // Clear this map and then move the items from the other map.
        this->Clear();
        this->Head = std::move(other.Head);
        this->ItemCount = std::move(other.ItemCount);

        return *this;
    }
};

#endif // SUBSTRATE_ProbabalisticMap_H
