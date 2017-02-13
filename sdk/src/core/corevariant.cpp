/*
 * commvariant.cpp
 *
 *  Created on: May 13, 2013
 *      Author: trungbq
 */

#include <core/corevariant.h>
#include <utils/utilsprint.h>

#ifdef USE_LIBJSONC
#include <json-c/json.h>
#include <fstream>
#endif //USE_LIBJSONC
#include <stdio.h>

namespace iot {
namespace core {

class Variant::Private
{
public:
    Private():
        _type(Variant::TypeUnknown){
        _value.mapValue = NULL;
        _value.mapValue = NULL;
        _value.listValue = NULL;
        _value.stringValue = NULL;
        _value.boolValue = false;
        _value.intValue = 0;
        _value.uIntValue = 0;
        _value.longValue = 0;
        _value.uLongValue = 0;
        _value.doubleValue = 0.0;
        _value.floatValue = 0.0f;
    }

    Private(const Variant::Map &value):
        _type(Variant::TypeMap) {
        _value.mapValue = new Map(value);
    }

    Private(const Variant::List &value) :
        _type(Variant::TypeList) {
        _value.listValue = new List(value);
    }

    Private(const std::string &value):
        _type(Variant::TypeString) {
        _value.stringValue = new std::string(value);
    }

    Private(int value):
        _type(Variant::TypeInt) {
        _value.intValue = value;
    }

    Private(unsigned int value):
        _type(Variant::TypeUInt) {
        _value.uIntValue = value;
    }

    Private(bool value):
        _type(Variant::TypeBool) {
        _value.boolValue = value;
    }

    Private(long value):
        _type(Variant::TypeLong) {
        _value.longValue = value;
    }
    Private(unsigned long value):
        _type(Variant::TypeULong) {
        _value.uLongValue = value;
    }

    Private(double value):
        _type(Variant::TypeDouble) {
        _value.doubleValue = value;
    }

    Private(float value):
        _type(Variant::TypeFloat) {
        _value.floatValue = value;
    }

    Private(const Private &variant):
        _type(variant.type()) {
        switch(_type) {
        case Variant::TypeBool:
            _value.boolValue = variant._value.boolValue;
            break;
        case Variant::TypeInt:
            _value.intValue = variant._value.intValue;
            break;
        case Variant::TypeUInt:
            _value.uIntValue = variant._value.uIntValue;
            break;
        case Variant::TypeLong:
            _value.longValue = variant._value.longValue;
            break;
        case Variant::TypeULong:
            _value.uLongValue = variant._value.uLongValue;
            break;
        case Variant::TypeFloat:
            _value.floatValue = variant._value.floatValue;
            break;
        case Variant::TypeDouble:
            _value.doubleValue = variant._value.doubleValue;
            break;
        case Variant::TypeString:
            _type = Variant::TypeString;
            _value.stringValue = new std::string(*variant._value.stringValue);
            break;
        case Variant::TypeMap:
            _type = Variant::TypeMap;
            _value.mapValue = new Variant::Map(*variant._value.mapValue);
            break;
        case Variant::TypeList:
            _type = Variant::TypeList;
            _value.listValue = new Variant::List(*variant._value.listValue);
            break;
        default:
            _type = Variant::TypeUnknown;
            break;
        }
    }

    ~Private() {
        switch(_type) {
        case Variant::TypeList:
            delete _value.listValue;
            break;
        case Variant::TypeMap:
            delete _value.mapValue;
            break;
        case Variant::TypeString:
            delete _value.stringValue;
            _value.stringValue = NULL;
            break;
        default:
            break;
        }
    }

    inline Variant::Type type() const {
        return _type;
    }

    bool isInteger() const {
        return _type == Variant::TypeInt ||
                _type == Variant::TypeUInt ||
                _type == Variant::TypeLong ||
                _type == Variant::TypeULong;
    }

    bool toBool(bool *ok) const {
        if (_type != Variant::TypeBool) {
            if (ok != NULL)
                *ok= false;
        }
        if (ok != NULL)
            *ok = true;
        return _value.boolValue;
    }

    int toInt(bool *ok) const  {
        switch(_type) {
        case Variant::TypeInt:
            if (ok != NULL)
                *ok = true;
            return _value.intValue;
        case Variant::TypeUInt:
            if (ok != NULL)
                *ok = true;
            return (int)_value.uIntValue;
        case Variant::TypeLong:
            if (ok != NULL)
                *ok = true;
            return (int) _value.longValue;
        case Variant::TypeULong:
            if (ok != NULL)
                *ok = true;
            return (int) _value.uLongValue;
        case Variant::TypeFloat:
            if (ok != NULL)
                *ok = true;
            return (int) _value.floatValue;
        case Variant::TypeDouble:
            if (ok != NULL)
                *ok = true;
            return (int) _value.doubleValue;
        default:
            break;
        }
        if (ok != NULL)
            *ok = false;
        return 0;
    }

    unsigned int toUInt(bool *ok) const {
        switch(_type) {
        case Variant::TypeInt:
            if (ok != NULL)
                *ok = true;
            return (unsigned int)_value.intValue;
        case Variant::TypeUInt:
            if (ok != NULL)
                *ok = true;
            return _value.uIntValue;
        case Variant::TypeLong:
            if (ok != NULL)
                *ok = true;
            return (unsigned int) _value.longValue;
        case Variant::TypeULong:
            if (ok != NULL)
                *ok = true;
            return (unsigned int) _value.uLongValue;
        case Variant::TypeFloat:
            if (ok != NULL)
                *ok = true;
            return (unsigned int) _value.floatValue;
        case Variant::TypeDouble:
            if (ok != NULL)
                *ok = true;
            return (unsigned int) _value.doubleValue;
        default:
            break;
        }
        if (ok != NULL)
            *ok = false;
        return 0;
    }


    long toLong(bool *ok) const {
        switch(_type) {
        case Variant::TypeInt:
            if (ok != NULL)
                *ok = true;
            return _value.intValue;
        case Variant::TypeUInt:
            if (ok != NULL)
                *ok = true;
            return (long)_value.uIntValue;
        case Variant::TypeLong:
            if (ok != NULL)
                *ok = true;
            return _value.longValue;
        case Variant::TypeULong:
            if (ok != NULL)
                *ok = true;
            return (long) _value.uLongValue;
        case Variant::TypeFloat:
            if (ok != NULL)
                *ok = true;
            return (long)_value.floatValue;
        case Variant::TypeDouble:
            if (ok != NULL)
                *ok = true;
            return (long)_value.doubleValue;
        default:
            break;
        }
        if (ok != NULL)
            *ok = false;
        return 0;
    }

    unsigned long toULong(bool *ok) const {
        switch(_type) {
        case Variant::TypeInt:
            if (ok != NULL)
                *ok = true;
            return (unsigned long) _value.intValue;
        case Variant::TypeUInt:
            if (ok != NULL)
                *ok = true;
            return (unsigned long)_value.uIntValue;
        case Variant::TypeLong:
            if (ok != NULL)
                *ok = true;
            return (unsigned long) _value.longValue;
        case Variant::TypeULong:
            if (ok != NULL)
                *ok = true;
            return (unsigned long) _value.longValue;
        case Variant::TypeFloat:
            if (ok != NULL)
                *ok = true;
            return (unsigned long) _value.floatValue;
        case Variant::TypeDouble:
            if (ok != NULL)
                *ok = true;
            return (unsigned long) _value.doubleValue;
        default:
            break;
        }
        if (ok != NULL)
            *ok = false;
        return 0;
    }
    double toDouble(bool *ok) const {
        switch(_type) {
        case Variant::TypeInt:
            if (ok != NULL)
                *ok = true;
            return (double)_value.intValue;
        case Variant::TypeUInt:
            if (ok != NULL)
                *ok = true;
            return (double)_value.uIntValue;
        case Variant::TypeLong:
            if (ok != NULL)
                *ok = true;
            return (double)_value.longValue;
        case Variant::TypeULong:
            if (ok != NULL)
                *ok = true;
            return (double) _value.uLongValue;
        case Variant::TypeFloat:
            if (ok != NULL)
                *ok = true;
            return (double)_value.floatValue;
        case Variant::TypeDouble:
            if (ok != NULL)
                *ok = true;
            return _value.doubleValue;
        default:
            break;
        }
        if (ok != NULL)
            *ok = false;
        return 0.0;
    }

    float toFloat(bool *ok) const {
        switch(_type) {
        case Variant::TypeInt:
            if (ok != NULL)
                *ok = true;
            return (float) _value.intValue;
        case Variant::TypeUInt:
            if (ok != NULL)
                *ok = true;
            return (float)_value.uIntValue;
        case Variant::TypeLong:
            if (ok != NULL)
                *ok = true;
            return (float) _value.longValue;
        case Variant::TypeULong:
            if (ok != NULL)
                *ok = true;
            return (float) _value.uLongValue;
        case Variant::TypeFloat:
            if (ok != NULL)
                *ok = true;
            return _value.floatValue;
        case Variant::TypeDouble:
            if (ok != NULL)
                *ok = true;
            return (float) _value.doubleValue;
        default:
            break;
        }
        if (ok != NULL)
            *ok = false;
        return 0.0f;
    }

    std::string toString() const {
        if (_type != Variant::TypeString)
            return std::string();
        return *_value.stringValue;
    }

    std::map<std::string, Variant> toMap() const {
        if (_type != Variant::TypeMap)
            return Map();
        return *_value.mapValue;
    }

    std::list<Variant> toList() const {
        if (_type != Variant::TypeList)
            return List();
        return *_value.listValue;
    }

    void toJson(std::string &json, int indent) const {
        switch(_type) {
        case Variant::TypeMap:
            valueToJson(json, *_value.mapValue, indent);
            return;
        case Variant::TypeList:
            valueToJson(json, *_value.listValue, indent);
            return;
        case Variant::TypeString:
            json = *_value.stringValue;
            return;
        case Variant::TypeBool:
            json = _value.boolValue == true? "true": "false";
            return;
        case Variant::TypeInt: {
            char value[128];
            sprintf(value, "%d", _value.intValue);
            json = value;
            return;
        }
        case Variant::TypeUInt: {
            char value[128];
            sprintf(value, "%u", _value.uIntValue);
            json = value;
            return;
        }
        case Variant::TypeLong: {
            char value[128];
            sprintf(value, "%ld", _value.longValue);
            json = value;
            return;
        }
        case Variant::TypeULong: {
            char value[128];
            sprintf(value, "%ld", _value.uLongValue);
            json = value;
            return;
        }
        case Variant::TypeDouble: {
            char value[128];
            sprintf(value, "%f", _value.doubleValue);
            json = value;
            return;
        }
        case Variant::TypeFloat: {
            char value[128];
            sprintf(value, "%f", _value.floatValue);
            json = value;
            return;
        }
        case Variant::TypeUnknown:
            json = "";
            break;
        };
    }

    const Variant *lookup(const Variant::Name &name) const {
        if (_type != Variant::TypeMap)
            return NULL;


        Variant::Map::const_iterator found = _value.mapValue->find(name);
        if (found == _value.mapValue->end())
            return NULL;
        return &found->second;
    }
private:

    static void valueToJson(std::string &json,
                            const Variant::Map &map,
                            int indents) {
        json += "{\n";
        std::string tabs;
        for(int indent = 0; indent <= indents; indent++)
            tabs += '\t';
        json += tabs;
        Variant::Map::const_iterator end = map.end();
        for(Variant::Map::const_iterator iterator = map.begin();
            iterator != end;
            iterator++) {

            json += "\n\"" + tabs + iterator->first + "\": ";
            std::string value;
            iterator->second.toJson(value, indents + 1);
            json += value;
            json += ",";
        }
        if (map.size() > 1)
            json.erase(json.length() - 1, 1);
        json += "\n" + tabs + "}";
    }

    static void valueToJson(std::string &json,
                            const Variant::List &list,
                            int indents) {
        json += "[\n";
        std::string tabs;
        for(int indent = 0; indent <= indents; indent++)
            tabs += '\t';
        json += tabs;
        Variant::List::const_iterator end = list.end();
        for(Variant::List::const_iterator iterator = list.begin();
            iterator != end;
            iterator++) {
            std::string value;
            (*iterator).toJson(value, indents);
            json += value;
            json += ",";
        }

        if (list.size() > 1)
            json.erase(json.length() - 1, 1);
        for(int indent = 0; indent < indents; indent++)
            json += '\t';
        json += "]";
    }

private:
    union Value {

        Variant::Map *mapValue;
        Variant::List *listValue;
        std::string *stringValue;
        bool boolValue;
        int intValue;
        unsigned int uIntValue;
        long longValue;
        unsigned long uLongValue;
        double doubleValue;
        float floatValue;
    } _value;
    Variant::Type _type;

};

Variant::Variant():
    _private(new Private)
{

}

Variant::Variant(const Map &value) :
    _private(new Private(value))
{
}

Variant::Variant(const List &value) :
    _private(new Private(value))
{
}

Variant::Variant(const std::string &value):
    _private(new Private(value))
{
}

Variant::Variant(int value):
    _private(new Private(value))
{
}

Variant::Variant(unsigned int value):
    _private(new Private(value))
{
}

Variant::Variant(bool value):
    _private(new Private(value))
{
}

Variant::Variant(long value):
    _private(new Private(value))
{
}

Variant::Variant(unsigned long value):
    _private(new Private(value))
{
}

Variant::Variant(double value):
    _private(new Private(value))
{
}

Variant::Variant(float value):
    _private(new Private(value))
{
}

Variant::Variant(const Variant &value):
    _private(new Private(*value._private))

{
}

Variant::~Variant()
{
    delete _private;
}

Variant::Type Variant::type() const
{
    return _private == NULL? TypeUnknown: _private->type();
}

bool Variant::isInteger() const
{
    return _private == NULL? false: _private->isInteger();
}

bool Variant::toBool(bool *ok) const
{
    return _private->toBool(ok);
}

int Variant::toInt(bool *ok) const
{
    return _private->toInt(ok);
}

unsigned int Variant::toUInt(bool *ok) const
{
    return _private->toUInt(ok);
}


long Variant::toLong(bool *ok) const
{
    return _private->toLong(ok);
}

unsigned long Variant::toULong(bool *ok) const
{
    return _private->toULong(ok);
}
double Variant::toDouble(bool *ok) const
{
    return _private->toDouble(ok);
}

float Variant::toFloat(bool *ok) const
{
    return _private->toFloat(ok);
}

std::string Variant::toString() const
{
    return _private->toString();
}

void Variant::toJson(std::string &json, int indents) const
{
    _private->toJson(json, indents);
}
Variant::Map Variant::toMap() const
{
    return _private->toMap();
}

Variant::List Variant::toList() const
{
    return _private->toList();
}


Variant &Variant::operator = (const Variant &variant)
{

    delete _private;
    _private = new Private(*variant._private);
    return *this;
}

const Variant *Variant::value(const Name &name) const
{
    return _private->lookup(name);
}

#ifdef USE_LIBJSONC
Variant Variant::fromJson(json_object *json)
{
    json_object_get_type(json);
    switch (json_object_get_type(json)) {
    case json_type_boolean:
        return Variant(json_object_get_boolean(json)? true: false);
    case json_type_int:
        return Variant(json_object_get_int(json));
    case json_type_double:
        return Variant(json_object_get_double(json));
    case json_type_string:
        return Variant(std::string(json_object_get_string(json)));
    case json_type_array: {
        Variant::List list;
        int length = json_object_array_length(json);
        for (int index = 0; index < length; index++) {

            json_object *object = json_object_array_get_idx(json, index);
            list.push_back(Variant::fromJson(object));
        }
        return Variant(list);
    }
    case json_type_object: {
        Variant::Map map;
        json_object_object_foreach(json, key, value) {
            map[key] = Variant::fromJson(value);
        }
        return Variant(map);
    }
    default:
        break;
    }
    return Variant();

}

Variant Variant::loadJson(const std::string &path, bool *ok)
{
    std::ifstream stream(path.c_str());
    if (!stream.is_open()) {
        if (ok != NULL)
            *ok = false;
        return Variant();
    }
    std::string content((std::istreambuf_iterator<char>(stream)),
                        std::istreambuf_iterator<char>());
    return loadJsonString(content, ok);
}

Variant Variant::loadJsonString(const std::string &content, bool *ok)
{
    json_tokener *tokener = json_tokener_new();
    if (tokener == NULL) {
        if (ok != NULL)
            *ok = false;
        return Variant();
    }

    json_object *json = json_tokener_parse_ex(tokener, content.c_str(), -1);
    json_tokener_error error = tokener->err;

    json_tokener_free(tokener);
    if(error != json_tokener_success) {
        if (json != NULL)
            json_object_put(json);
        if (ok != NULL)
            *ok = false;
        return Variant();
    }

    Variant variant = fromJson(json);
    json_object_put(json);
    if (ok != NULL)
        *ok = true;
    return variant;
}
#endif //USE_LIBJSONC

} // namespace Core
} // namespace iot

