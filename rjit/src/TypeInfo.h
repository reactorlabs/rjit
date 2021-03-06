#ifndef TYPE_INFO
#define TYPE_INFO

#include "EnumBitset.h"
#include <iostream>
#include <cassert>

#include "RDefs.h"

namespace rjit {

/* TypeInfo
 * holds type and shape information about a variable or register
 * Currently it contains: a set of possible types, whether it has attrs and the
 * size class (ie. if its scalar).
 */
class TypeInfo {
  public:
    // -- States to record

    enum class Type : int { Integer, Float, String, Vector, Bool, Any };

    enum class Size : uint8_t { Unknown, Scalar, Any };

    enum class Attrib : uint8_t { Unknown, Absent, Object, Any };

    // -- Constructors

    // Init unused bits to zero
    TypeInfo() : TypeInfo(0) {
        store.types_ = EnumBitset<Type>();
        store.size_ = Size::Unknown;
        store.attrib_ = Attrib::Unknown;
    }

    TypeInfo(Type type, Size size, Attrib attrib) : TypeInfo(0) {
        store.types_ = EnumBitset<Type>(type);
        store.size_ = size;
        store.attrib_ = attrib;
    }

    TypeInfo(SEXP from) : TypeInfo() { mergeAll(from); }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"

    TypeInfo(int base) { *reinterpret_cast<int*>(&store) = base; }

    explicit operator int() { return *reinterpret_cast<int*>(&store); }

#pragma GCC diagnostic pop

    // This only works if we make sure to zero initialize the unused values!
    bool operator!=(TypeInfo& other) {
        return static_cast<int>(*this) != static_cast<int>(other);
    }

    bool operator==(TypeInfo& other) {
        return static_cast<int>(*this) == static_cast<int>(other);
    }

    // -- getters
    static TypeInfo any() {
        TypeInfo any;
        any.addType(Type::Any);
        any.store.size_ = Size::Any;
        any.store.attrib_ = Attrib::Any;
        return any;
    }

    bool isBottom() {
        return types().empty() && attrib() == Attrib::Unknown &&
               size() == Size::Unknown;
    }

    bool isAny() {
        return types().has(Type::Any) && attrib() == Attrib::Any &&
               size() == Size::Any;
    }

    const EnumBitset<Type> types() { return EnumBitset<Type>(store.types_); }

    Attrib attrib() { return store.attrib_; }

    Size size() { return store.size_; }

    // -- setters

    bool hasType(Type t) { return types().has(t) || types().has(Type::Any); }

    bool hasOnlyType(Type t) const {
        return store.types_ == 1 << static_cast<int>(t);
    }

    const EnumBitset<Type> types(EnumBitset<Type> t) {
        store.types_ = t;
        return t;
    }

    Size size(Size s) {
        assert(s > Size::Unknown && s <= Size::Any);
        store.size_ = s;
        return s;
    }

    Attrib attrib(Attrib a) {
        assert(a > Attrib::Unknown && a <= Attrib::Any);
        store.attrib_ = a;
        return a;
    }

    // -- record a new runtime type instance

    const EnumBitset<Type> addType(SEXP value);
    void mergeAttrib(SEXP v);
    void mergeSize(SEXP v);
    void mergeAll(SEXP s);

    // -- merge two typeinfos

    bool mergeTypes(TypeInfo other) { return mergeTypes(other.types()); }
    bool mergeAttrib(TypeInfo other) { return mergeAttrib(other.attrib()); }
    bool mergeSize(TypeInfo other) { return mergeSize(other.size()); }

    bool mergeWith(TypeInfo other) {
        bool result = mergeTypes(other);
        result = mergeAttrib(other) or result;
        return mergeSize(other) or result;
    }

    static TypeInfo merge(TypeInfo l, TypeInfo r) {
        TypeInfo result(l);
        result.mergeWith(r);
        return result;
    }

    const EnumBitset<Type> addType(Type type) {
        auto t = types();
        t.insert(type);
        return types(t);
    }

  private:
    // -- merge helpers

    bool mergeAttrib(Attrib a) {
        if (a > attrib()) {
            attrib(a);
            return true;
        } else {
            return false;
        }
    }

    bool mergeSize(Size s) {
        if (s > size()) {
            size(s);
            return true;
        } else {
            return false;
        }
    }

    bool mergeTypes(EnumBitset<Type> t) {
        if ((t | types()) != types()) {
            types(t | types());
            return true;
        } else {
            return false;
        }
    }

    friend std::ostream& operator<<(std::ostream& out, TypeInfo& t);

    // -- Condensed bitmap store

    // TODO: How can we make Attrib and Size smaller than 8 and stop GCC from
    // whining about it???
    struct Store {
        int types_ : (int)Type::Any + 1;
        Attrib attrib_ : 8;
        Size size_ : 8;
    };

    static_assert(sizeof(Store) == sizeof(int), "Store must fit into int");

    Store store;
};

static_assert(sizeof(TypeInfo) == sizeof(int),
              "Typeinfo cannot be bigger than its Store");

std::ostream& operator<<(std::ostream& out, TypeInfo& info);
}

#endif
