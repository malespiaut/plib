#include "ulRTTI.h"

#ifdef UL_RTTI_EMULATION

static const rttiTypeInfo* RTTI_base_null_type[] = { 0 };

const rttiTypeInfo rttiTypeInfo::null_type("NULL", RTTI_base_null_type,0);

#endif