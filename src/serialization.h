#ifndef AS_SERIALIZATION_H_
#define AS_SERIALIZATION_H_

#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/variant.hpp>

#include "term.h"

namespace boost {
namespace serialization {

template <class Archive>
void serialize(Archive &ar, as::Variable &variable, const unsigned int version) {
    ar & variable.name;
}

template <class Archive>
void serialize(Archive &ar, as::BeliefAtom &atom, const unsigned int version) {
    ar & atom.functor;
    ar & atom.terms;
}

template <class Archive, typename Op>
void serialize(Archive &ar, as::UnaryOp<Op> &op, const unsigned int version) {
    ar & op.operand;
}

template <class Archive, typename Op>
void serialize(Archive &ar, as::BinaryOp<Op> &op, const unsigned int version) {
    ar & op.left;
    ar & op.right;
}

}  // namespace serialization
}  // namespace boost

#endif  // AS_SERIALIZATION_H_
