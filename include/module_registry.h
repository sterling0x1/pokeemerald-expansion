#ifndef GUARD_MODULE_REGISTRY_H
#define GUARD_MODULE_REGISTRY_H

#include "module_manager.h"

// This is the single project composition point. The generic manager consumes
// descriptors from this table without including any feature-owned headers.
extern const struct ModuleDescriptor *const gModuleRegistry[MODULE_ID_COUNT];

#endif // GUARD_MODULE_REGISTRY_H
