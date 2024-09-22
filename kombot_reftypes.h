#pragma once

#define KOMBOT_PTR(type)           type *
#define KOMBOT_RPTR(type)          type * restrict
#define KOMBOT_CONSTVAL_RPTR(type) const type * restrict
#define KOMBOT_CONSTREF_RPTR(type) type * const restrict
#define KOMBOT_CONST_RPTR(type)    const type * const restrict

#define KOMBOT_FUNC_PTR(fname, rt, its) rt (*fname)(its)
