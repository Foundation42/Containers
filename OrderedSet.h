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

#ifndef Foundation42_OrderedSet_H
#define Foundation42_OrderedSet_H

#include <cstdint>
#include <functional>
#include <cassert>

#include <cstdint>
#include <functional>
#include <cassert>

// Template class for an ordered set.
template <typename Key_t>
class OrderedSet
{
private:
    // Structure for a node in the set.
    struct Node
    {
        Key_t Key;
        Node* Next { nullptr };
    };

    mutable Node* Head { nullptr }; // Head of the set.
    std::size_t ItemCount { 0 }; // Number of items in the set.

public:
    // Default constructor.
    OrderedSet() = default;

    // Copy constructor.
    OrderedSet(const OrderedSet& other)
    {
        // Copy each item from the other set.
        other.ForEach([this](const auto& lhs)
        {
            this->Add(lhs);
            return true;
        });
    }

    // Move constructor.
    OrderedSet(OrderedSet&& other) noexcept :
        Head(std::move(other.Head)),
        ItemCount(std::move(other.ItemCount))
    {
        // The assert(false) has been removed here.
    }

    // Destructor.
    ~OrderedSet()
    {
        // Clear the set.
        this->Clear();
    }

    // Get the head of the set.
    Node* GetHead() const
    {
        return this->Head;
    }

    // Clear all items from the set.
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

    // Get the number of items in the set.
    std::size_t Count() const
    {
        return this->ItemCount;
    }

    // Using declaration for a function that takes a key.
    using keyCallback = std::function<void (const Key_t& key)>;

    // Apply the given function to each key in the set.
    void ForEach(keyCallback callback) const
    {
        Node* current { this->Head };

        while (current)
        {
            callback(current->Key);
            current = current->Next;
        }
    }

    // Using declaration for a function that takes a mutable key.
    using mutableKeyCallback = std::function<void (Key_t& key)>;

    // Apply the given function to each key in the set.
    void MutableForEach(mutableKeyCallback callback)
    {
        Node* current { this->Head };

        while (current)
        {
            callback(current->Key);
            current = current->Next;
        }
    }

    // Find the node with the given key, or create one if it does not exist.
    int FindOrCreate(const Key_t& key)
    {
        auto itemIndex { 0 };

        Node* current { this->Head };
        Node* previous { nullptr };
    
        // Search through the nodes.
        while (current != nullptr)
        {
            // Return the index if we found it.
            if (current->Key == key)
            {
                return itemIndex;
            }

            previous = current;
            current = current->Next;
            itemIndex++;
        }

        // We couldn't find it, so create it here.
        auto newNode { new Node() };
        newNode->Key = key;

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

    // Find the index of the node with the given key.
    int Find(const Key_t& key) const
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

    // Get the node at the given index in the set.
    Node* GetAt(std::size_t index)
    {
        Node* current { this->Head };

        for (auto i = 0u; i < index; ++i)
            current = current->Next;

        return current;
    }

    // Add the given key to the set.
    std::size_t Add(const Key_t& key)
    {
        auto nodeIndex { this->FindOrCreate(key) };
        return nodeIndex;
    }

    // Free the given node.
    void FreeNode(Node* node)
    {
        delete node;
    }

    // Add the given node to the front of the set.
    void PushFront(Node* node)
    {
        node->Next = this->Head;
        this->Head = node;
        this->ItemCount++;
    }

    // Remove the first node from the set and return it.
    Node* PopFront()
    {
        auto node { this->Head };

        if (node == nullptr)
            return nullptr;

        this->Head = node->Next;
        this->ItemCount--;

        return node;
    }

    // Insert the given key into the set in sorted order.
    void InsertSorted(const Key_t& key)
    {
        auto node { new Node() };
        node->Key = key;
        this->InsertNodeSorted(node);
    }

    // Insert the given node into the set in sorted order.
    void InsertNodeSorted(Node* node)
    {
        Node* current { this->Head };
        Node* previous { nullptr };
    
        // Find the insert point.
        while (current != nullptr)
        {
            // Break if we found the insert point.
            if (node->Key < current->Key)
                break;

            previous = current;
            current = current->Next;
        }

        // If this is the first node, set it as the head.
        // Otherwise, add it after the previous node.
        if (previous == nullptr)
            this->Head = node;
        else
            previous->Next = node;

        node->Next = current;
        this->ItemCount++;
    }

    // Using declaration for a function that takes a key and returns a bool.
    using KeyPredicate = std::function<bool (const Key_t& key)>;

    // Delete nodes for which the predicate returns true.
    void DeleteNodes(const KeyPredicate predicate)
    {
        Node* current { this->Head };
        Node* previous { nullptr };
    
        while (current != nullptr)
        {
            if (predicate(current->Key))
            {
                if (previous == nullptr)
                {
                    this->Head = nullptr;
                }
                else
                {
                    previous->Next = current->Next;
                }

                current = current->Next;
                delete current;
                this->ItemCount--;
                continue;
            }

            previous = current;
            current = current->Next;
        }
    }

    // Check if the set contains the given key.
    bool Exists(const Key_t& key) const
    {
        auto nodeIndex { this->Find(key) };
        return nodeIndex != -1;
    }

    // Overloaded = operator for copying another set into this one.
    OrderedSet& operator=(const OrderedSet& other)
    {
        assert(&other != this);
        
        // Clear this set and then copy each item from the other set.
        this->Clear();
        other.ForEach([this](const auto& lhs)
        {
            this->Add(lhs);
            return true;
        });

        return *this;
    }

    // Overloaded = operator for moving another set into this one.
    OrderedSet& operator=(OrderedSet&& other) noexcept
    {
        assert(&other != this);

        // Clear this set and then move the items from the other set.
        this->Clear();
        this->Head = std::move(other.Head);
        this->ItemCount = std::move(other.ItemCount);

        return *this;
    }
};

#endif // Foundation42_OrderedSet_H
