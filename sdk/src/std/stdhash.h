#ifndef STDHASH_H
#define STDHASH_H
#include <stddef.h>

namespace iot {
namespace std {
template <typename Key, typename Size = uint16_t>
class HashFunction
{
public:
    Size operator ()(const Key &key, const Size &size) const {
        return Size(key) % size;
    }
};

template <typename Key,
          typename Value,
          typename Size = uint16_t,
          typename Function = HashFunction<Key, Size> >
class Hash
{
private:
    class Node
    {
    public:
        Node(const Key &key, const Value &value):
            _key(key),
            _value(value),
            _next(NULL) {

        }

        const Key &key() const {
            return _key;
        }


        const Value &value() const {
            return _value;
        }

        void setValue(const Value &value) {
            _value = value;
        }

        Node *next() {
            return _next;
        }

        void setNext(Node *next) {
            _next = next;
        }

    private:
        Size _size;
        Key _key;
        Value _value;
        Node *_next;
    };
public:

    Hash(const Size &size):
        _slots(new Slot[size]),
        _size(size) {
        for(Size index = 0; index < size; index++)
            _slots[index] = NULL;
    }

    ~Hash() {
        clear();
        delete []_slots;
    }

    void set(const Key &key, const Value &value) {
        Size index = _function(key, _size);
        Slot previous = NULL;
        Slot slot = _slots[index];
        while (slot != NULL && slot->key() != key) {
            previous = slot;
            slot = slot->next();
        }
        if (slot == NULL) {
            slot = new Node(key, value);
            if (previous == NULL)
                _slots[index] = slot;
            else {
                previous->setNext(slot);
            }
        } else {
            // just update the value
            slot->setValue(value);
        }
    }

    bool get(const Key &key, Value &value) const {
        Size index = _function(key, _size);
        Slot slot = _slots[index];

        while (slot != NULL) {
            if (slot->key() == key) {
                value = slot->value();
                return true;
            }
            slot = slot->next();
        }
        return false;
    }

    void erase(const Key &key) {
        Size index = _function(key, _size);
        Slot previous = NULL;
        Slot slot = _slots[index];

        while (slot != NULL && slot->key() != key) {
            previous = slot;
            slot = slot->next();
        }

        if (slot == NULL)
            return;

        if (previous == NULL)
            _slots[index] = slot->next();
        else
            previous->setNext(slot->next());
        delete slot;
    }

    void clear() {
        // destroy all buckets one by one
        for (Size index = 0; index < _size; index ++) {
            Slot slot = _slots[index];
            while (slot != NULL) {
                Slot previous = slot;
                slot = slot->next();
                delete previous;
            }
            _slots[index] = NULL;
        }
    }

private:
    typedef Node *Slot;
    Slot *_slots;
    Size _size;
    Function _function;
};

} // namespace std
} // namespace iot

#endif // STDHASH_H
