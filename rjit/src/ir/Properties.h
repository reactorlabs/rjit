#ifndef IR_PROPERTIES_H
#define IR_PROPERTIES_H

namespace rjit {
namespace ir {

/** Pattern's result is always scalar.
 */
class ScalarResult {};

/** Pattern's result is scalar iff its inputs are scalar
 */
class ScalarResultIfInputsBinary {};





} // namespace ir
} // namespace rjit

#endif // IR_PROPERTIES_H
