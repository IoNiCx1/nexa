#include "Type.h"

namespace nexa {

// One definition of every type singleton.
// All other TUs see the extern declarations from Type.h and link to these.
Type TYPE_INT    (TypeKind::Int);
Type TYPE_DOUBLE (TypeKind::Double);
Type TYPE_STRING (TypeKind::String);
Type TYPE_BOOL   (TypeKind::Bool);
Type TYPE_VOID   (TypeKind::Void);
Type TYPE_TENSOR (TypeKind::Tensor);
Type TYPE_INT_ARRAY(TypeKind::Array, &TYPE_INT);   // int[]

} // namespace nexa