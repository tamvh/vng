/*
 * variant.h
 *
 *  Created on: May 13, 2013
 *      Author: trungbq
 */

#ifndef COREVARIANT_H
#define COREVARIANT_H
#include <sdkdefs.h>
#include <list>
#include <map>
#include <string>

#ifdef USE_LIBJSONC
struct json_object;
#endif

namespace iot {
namespace core {

class SDKSHARED_EXPORT Variant {
public:
    typedef std::string Name;
    typedef std::map<Name, Variant> Map;
    typedef std::list<Variant> List;

    enum Type {
        TypeUnknown = 0,
        TypeMap = 1,
        TypeList = 2,
        TypeString = 3,
        TypeBool = 4,
        TypeInt = 5,
        TypeUInt = 6,
        TypeLong = 7,
        TypeULong = 8,
        TypeDouble = 9,
        TypeFloat = 10
    };
    Variant();
    Variant(const Map &value);
    Variant(const List &value);
	Variant(const std::string &value);
	Variant(int value);
    Variant(unsigned int value);
	Variant(bool value);
    Variant(long value);
    Variant(unsigned long value);
    Variant(double value);
	Variant(float value);
    Variant(const Variant &value);
	virtual ~Variant();

    Variant &operator = (const Variant &variant);

    Type type() const;
    bool isInteger() const;
    int toInt(bool *ok = 0) const;
    unsigned int toUInt(bool *ok = 0) const;
    long toLong(bool *ok = 0) const;
    unsigned long toULong(bool *ok = 0) const;
    bool toBool(bool *ok = 0) const;
	double toDouble(bool *ok = 0) const;
	float toFloat(bool *ok = 0) const;
    std::string toString() const;
    void toJson(std::string &json, int indents = 0) const;
    Map toMap() const;
    List toList() const;
    const Variant *value(const Name &name) const;
public:
#ifdef USE_LIBJSONC
    static Variant fromJson(json_object *json);
    static Variant loadJson(const std::string &path, bool *ok = NULL);
    static Variant loadJsonString(const std::string &content, bool *ok = NULL);
#endif
private:
    class Private;
    Private *_private;
};
} // namespace Core
} // namespace iot
#endif /* COREVARIANT_H */
