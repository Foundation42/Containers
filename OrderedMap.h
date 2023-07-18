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

#ifndef Foundation42_OrderedMap_H
#define Foundation42_OrderedMap_H

#include <cstdint>
#include <functional>
#include <cassert>

// Template class for an ordered map.
template <typename Key_t, typename Value_t>
class OrderedMap
{
private:
    // Structure for a node in the map.
    struct Node
    {
        Key_t Key;
        Value_t Value;
        Node* Next { nullptr };
    };

    mutable Node* Head { nullptr }; // Head of the map.
    std::size_t ItemCount { 0 }; // Number of items in the map.

public:
    // Default constructor.
    OrderedMap() = default;

    // Copy constructor.
    OrderedMap(const OrderedMap& other)
    {
        // Copy each item from the other map.
        other.ForEach([this](const auto& lhs, const auto& rhs)
        {
            this->Set(lhs, rhs);
            return true;
        });
    }

    // Move constructor.
    OrderedMap(OrderedMap&& other) noexcept :
        Head(std::move(other.Head)),
        ItemCount(std::move(other.ItemCount))
    {
        // The assert(false) has been removed here.
    }

    // Destructor.
    ~OrderedMap()
    {
        // Clear the map.
        this->Clear();
    }

    // Get the head of the map.
    Node* GetHead() const
    {
        return this->Head;
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

    // Get the number of items in the map.
    std::size_t Count() const
    {
        return this->ItemCount;
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

    // Find the node with the given key, or create one if it does not exist.
    int FindOrCreate(const Key_t& key, const Value_t& value)
    {
        auto itemIndex { 0 };

        Node* current { this->Head };
        Node* previous { nullptr };
    
        // Search through the nodes.
        while (current != nullptr)
        {
            // Check if we found it.
            if (current->Key == key)
            {
                // Return the index if we found it.
                return itemIndex;
            }

            previous = current;
            current = current->Next;
            itemIndex++;
        }

        // We couldn't find it, so create it here.
        auto newNode { new Node() };
        newNode->Key = key;
        newNode->Value = value;

        // If this is the first node, set it as the head.
        // Otherwise, add it after the previous node.
        if (previous == nullptr)
        {
            this->Head = newNode;
        }
        else
        {
            previous->Next = newNode;
        }

        this->ItemCount++;

        return itemIndex;
    }

    // Find the node with the given key.
    Node* FindIt(const Key_t& key) const
    {
        Node* current { this->Head };
    
        // Search through the nodes.
        while (current != nullptr)
        {
            // Return the node if we found it.
            if (current->Key == key)
            {
                return current;
            }

            current = current->Next;
        }

        // We couldn't find it.
        return nullptr;
    }

    // Find the index of the node with the given key.
    int FindIndex(const Key_t& key) const
    {
        auto itemIndex { 0 };

        Node* current { this->Head };
    
        // Search through the nodes.
        while (current != nullptr)
        {
            // Return the index if we found it.
            if (current->Key == key)
            {
                return itemIndex;
            }

            current = current->Next;
            itemIndex++;
        }

        // We couldn't find it.
        return -1;
    }

    // Get the node at the given index in the map.
    Node* GetAt(std::size_t index)
    {
        Node* current { this->Head };

        for (auto i = 0u; i < index; ++i)
            current = current->Next;

        return current;
    }

    // Set the value for the given key in the map.
    std::size_t Set(const Key_t& key, const Value_t& value)
    {
        auto nodeIndex { this->FindOrCreate(key, value) };
        return nodeIndex;
    }

    // Get the value for the given key in the map.
    Value_t* Get(const Key_t& key) const
    {
        auto node { this->FindIt(key) };
        if (node == nullptr)
            return nullptr;

        return &node->Value;
    }

    // Check if the map contains the given key.
    bool Exists(const Key_t& key) const
    {
        auto nodeIndex { this->FindIndex(key) };
        return nodeIndex != -1;
    }

    // Overloaded << operator for merging another map into this one.
    OrderedMap& operator<<(const OrderedMap& other)
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
    OrderedMap& operator=(const OrderedMap& other)
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
    OrderedMap& operator=(OrderedMap&& other) noexcept
    {
        assert(&other != this);

        // Clear this map and then move the items from the other map.
        this->Clear();
        this->Head = std::move(other.Head);
        this->ItemCount = std::move(other.ItemCount);

        return *this;
    }
};

#endif // Foundation42_OrderedMap_H
