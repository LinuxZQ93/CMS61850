/**
*@file LinkedList.h
*@brief 侵入式链表，相比std::list在插入及删除上效率更高，但使用会复杂。若不是存在大量链表的插入，删除等操作，建议用std::list,参考chromium
*
*Version:1.0
*
*Date:2022/04
*
*@author linuxzq93@163.com
*/

#ifndef __FRAMEWORK_INCLUDE_LIST_LINKEDLIST_H__
#define __FRAMEWORK_INCLUDE_LIST_LINKEDLIST_H__

namespace base {

template <typename T>
class TLinkNode{
public:
    TLinkNode() : previous(NULL), next(NULL) {}
    TLinkNode(LinkNode<T> *previous, LinkNode<T> *next)
        : previous(previous), next(next) {}

    // Insert |this| into the linked list, before |e|.
    void insertBefore(LinkNode<T> *e)
    {
        this->next = e;
        this->previous = e->previous;
        e->previous->next = this;
        e->previous = this;
    }

    // Insert |this| into the linked list, after |e|.
    void insertAfter(LinkNode<T> *e)
    {
        this->next = e->next;
        this->previous = e;
        e->next->previous = this;
        e->next = this;
    }

    // Remove |this| from the linked list.
    void removeFromList()
    {
        this->previous->next = this->next;
        this->next->previous = this->previous;
        // next() and previous() return non-NULL if and only this node is not in any
        // list.
        this->next = NULL;
        this->previous = NULL;
    }

    LinkNode<T> *previous() const
    {
        return previous;
    }

    LinkNode<T> *next() const
    {
        return next;
    }

    // Cast from the node-type to the value type.
    const T *value() const
    {
        return static_cast<const T *>(this);
    }

    T *value()
    {
        return static_cast<T *>(this);
    }

private:
    TLinkNode<T> *previous;
    TLinkNode<T> *next;

    TLinkNode(const TLinkNode &) = delete;
    TLinkNode operator=(const TLinkNode &) = delete;
};

template <typename T>
class TLinkedList {
public:
    // The "root" node is self-referential, and forms the basis of a circular
    // list (root_.next() will point back to the start of the list,
    // and root_->previous() wraps around to the end of the list).
    TLinkedList() : root(&root, &root) {}

    // Appends |e| to the end of the linked list.
    void append(LinkNode<T> *e)
    {
        e->insertBefore(&root);
    }

    LinkNode<T> *head() const
    {
        return root.next();
    }

    LinkNode<T> *tail() const
    {
        return root.previous();
    }

    const LinkNode<T> *end() const
    {
        return &root;
    }

    bool empty() const { return head() == end(); }

private:
    LinkNode<T> root;
    TLinkedList(const TLinkedList &) = delete;
    TLinkedList operator=(const TLinkedList &) = delete;
};

}  // namespace base

#endif //__FRAMEWORK_INCLUDE_LIST_LINKEDLIST_H__
