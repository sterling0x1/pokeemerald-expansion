#ifndef GUARD_CONFIG_MODULES_H
#define GUARD_CONFIG_MODULES_H

// Optional gameplay modules registered with the shared module lifecycle.
// These switches currently control registration and lifecycle ownership.
// Runtime hook isolation is migrated module-by-module before a switch can
// guarantee that all code and UI belonging to that module are omitted.
#define MODULE_RANDOMIZER_ENABLED TRUE
#define MODULE_NUZLOCKE_ENABLED   TRUE
#define MODULE_CHEATS_ENABLED     TRUE

#endif // GUARD_CONFIG_MODULES_H
