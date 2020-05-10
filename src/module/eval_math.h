#pragma once
#include "core.h"

VL_Expr* VL_Core_apply_infix(VL_Core* self, const VL_Expr* expr);

#define DECLARE_FN(X)       \
X(int) X(float)             \
X(add) X(sub) X(mul) X(div) \
X(lte) X(lt) X(gte) X(gt)   \
X(and) X(or) X(not)         \
X(eq) X(neq)                \

#define DECLARE(X) void VL_Core_apply_ ## X (VL_Core* self, const VL_Expr* expr);

DECLARE_FN(DECLARE)

#undef DECLARE
#undef DECLARE_FN