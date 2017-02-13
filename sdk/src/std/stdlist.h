#ifndef STDLIST_H
#define STDLIST_H
#include <stdint.h>
#include <stddef.h>
namespace iot {
namespace std {
template<typename Item, typename Size = uint32_t>
class List
{
private:
    class Node
    {
    public:
        Node(const Item &item):
            _item(item),
            _previuos(NULL),
            _next(NULL) {

        }

        inline Node *next() const {
            return _next;
        }

        inline void setNext(Node *next) {
            _next = next;
        }

        inline Node *previous() const {
            return _previuos;
        }

        inline void setPrevious(Node *previous) {
            _previuos = previous;
        }

        inline Item &item() {
            return _item;
        }

    private:
        Item _item;
        Node *_previuos;
        Node *_next;
    };

public:
    class Iterator
    {
    private:
        Iterator(List *owner, Node *node = NULL):
            _owner(owner),
            _node(node) {

        }
    public:
        Iterator():
            _owner(NULL),
            _node(NULL) {

        }

        Iterator(const Iterator &iterator):
            _owner(iterator._owner),
            _node(iterator._node) {

        }

        inline Iterator &operator ++ () {
            _node = _node->next();
            return *this;
        }

        inline Iterator &operator ++ (int) {
            _node = _node->next();
            return *this;
        }

        inline Iterator &operator -- () {
            if (_node != NULL)
                _node = _node->previous();
            else
                _node = _owner->_head;
            return *this;
        }

        inline Iterator &operator -- (int) {
            if (_node != NULL)
                _node = _node->previous();
            else
                _node = _owner->_head;
            return *this;
        }

        inline Item &operator *() {
            return _node->item();
        }

        inline bool operator ==(const Iterator &iterator) const {
            return _node == iterator._node && _owner == iterator._owner;
        }

        inline bool operator != (const Iterator &iterator) const {
            return _node != iterator._node || _owner != iterator._owner;
        }
        inline Iterator &operator = (const Iterator &iterator) {
            _node = iterator._node;
            return *this;
        }
    private:
        List *_owner;
        Node *_node;
        friend class List;
    };

    /**
     * @brief List
     */
    List():
        _head(NULL),
        _tail(NULL),
        _size(0) {

    }

    virtual ~List() {

    }

    Iterator begin() const {
        return Iterator(this, _head);
    }

    Iterator begin() {
        return Iterator(this, _head);
    }

    Iterator end() {
        return Iterator(this, NULL);
    }

    Iterator end() const {
        return Iterator(this, NULL);
    }

    void remove(const Iterator &iterator) {
        remove(iterator._node);
    }

    Iterator enqueue(const Item &item) {
        Node *node = new Node(item);
        if (_head == NULL && _tail == NULL) {
            _head = node;
            _tail = node;
        } else {
            _tail->setNext(node);
            node->setPrevious(_tail);
            _tail = node;
        }
        _size += 1;
        return Iterator(this, node);
    }


    Item dequeue() {
        Node *head = _head;
        if(_tail == _head) {
            _tail = NULL;
            _head = NULL;
        } else {
            _head = _head->next();
            _head->setPrevious(NULL);
        }
        Item item = head->item();
        delete head;
        _size -= 1;
        return item;
    }

    Item head() {
        return _head->item();
    }

    inline const Size &size() const {
        return _size;
    }

    void clear() {
        while (_head != NULL) {
            Node *head = _head;
            _head = _head->next();
            delete head;
        }
        _tail = NULL;
        _size = 0;
    }

private:
    void remove(Node *node) {
        if (_head == _tail) {
            if (_head != node)
                return;
            _head = NULL;
            _tail = NULL;
            _size = 0;
            return;
        }

        if (node == _tail) {
            _tail = _tail->previous();
            _tail->setNext(NULL);
            return;
        } else if (node == _head) {
            _head = _head->next();
            _head->setPrevious(NULL);
        } else {
            node->previous()->setNext(node->next());
            node->next()->setPrevious(node->previous());
        }
        _size -= 1;
    }

private:
    Node *_head;
    Node *_tail;
    Size _size;
    friend class Iterator;

};
} // namespace std
} // namespace iot

#endif // CORELIST_H
