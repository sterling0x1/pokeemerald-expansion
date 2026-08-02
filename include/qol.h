#ifndef GUARD_QOL_H
#define GUARD_QOL_H

#include "global.h"
#include "config/modules.h"

#if MODULE_QOL_ENABLED

bool32 Qol_IsAutoRunEnabled(void);
bool32 Qol_IsRunningIndoorsEnabled(void);
u8 Qol_GetItemDescriptionsMode(void);
bool32 Qol_IsRepelPromptEnabled(void);
bool32 Qol_AreTmsReusable(void);
bool32 Qol_IsFieldPoisonEnabled(void);
bool32 Qol_IsTrainerEscapeEnabled(void);
bool32 Qol_IsBoxShortcutEnabled(void);
void Qol_MarkTrainerEscape(u16 trainerA, u16 trainerB);
bool32 Qol_ConsumeTrainerEscape(u16 *trainerA, u16 *trainerB);
bool32 Qol_ShouldSuppressTrainerApproach(void);
void Qol_ResetRuntimeState(void);

#else

static inline bool32 Qol_IsAutoRunEnabled(void) { return FALSE; }
static inline bool32 Qol_IsRunningIndoorsEnabled(void) { return FALSE; }
static inline u8 Qol_GetItemDescriptionsMode(void) { return 0; }
static inline bool32 Qol_IsRepelPromptEnabled(void) { return FALSE; }
static inline bool32 Qol_AreTmsReusable(void) { return FALSE; }
// With the module absent, preserve Expansion's configured field-poison rules.
static inline bool32 Qol_IsFieldPoisonEnabled(void) { return TRUE; }
static inline bool32 Qol_IsTrainerEscapeEnabled(void) { return FALSE; }
static inline bool32 Qol_IsBoxShortcutEnabled(void) { return FALSE; }
static inline void Qol_MarkTrainerEscape(u16 trainerA, u16 trainerB) {}
static inline bool32 Qol_ConsumeTrainerEscape(u16 *trainerA, u16 *trainerB) { return FALSE; }
static inline bool32 Qol_ShouldSuppressTrainerApproach(void) { return FALSE; }
static inline void Qol_ResetRuntimeState(void) {}

#endif // MODULE_QOL_ENABLED

#endif // GUARD_QOL_H
