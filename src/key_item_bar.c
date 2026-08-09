#include "global.h"

#if MODULE_KEY_ITEM_DOCK_ENABLED
#include "event_object_lock.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "international_string_util.h"
#include "key_item_bar.h"
#include "main.h"
#include "item_menu.h"
#include "script.h"
#include "sound.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/songs.h"
#include "item.h"
#include "item_icon.h"
#include "sprite.h"

#define KEY_ITEM_BAR_VISIBLE_ICONS 5
#define KEY_ITEM_BAR_MAX_ITEMS BAG_KEYITEMS_COUNT

#define KEY_ITEM_BAR_TILE_TAG_BASE    0x7000
#define KEY_ITEM_BAR_PALETTE_TAG_BASE 0x7100

static void Task_KeyItemBarWaitForInput(u8 taskId);
static void CloseKeyItemBar(u8 taskId);
static void UseSelectedKeyItem(u8 taskId);
static void CreateKeyItemBarSprites(void);
static void RefreshKeyItemBarSprites(void);
static void DestroyKeyItemBarSprites(void);
static void UpdateKeyItemBarSelection(void);
static void CreateDockNameWindow(void);
static void RemoveDockNameWindow(void);
static void PrintSelectedKeyItemName(void);

static EWRAM_DATA u8 sKeyItemBarSpriteIds[KEY_ITEM_BAR_VISIBLE_ICONS] = {0};
static EWRAM_DATA u8 sKeyItemBarSpriteCount = 0;
static EWRAM_DATA u8 sKeyItemBarItemCount = 0;
static EWRAM_DATA enum Item sKeyItemBarItemIds[KEY_ITEM_BAR_MAX_ITEMS] = {0};
static EWRAM_DATA u8 sKeyItemBarSelected = 0;
static EWRAM_DATA u8 sKeyItemBarNameWindowId = 0;
static EWRAM_DATA bool8 sKeyItemBarNameWindowActive = FALSE;

static const u8 sDockNameTextColors[] =
{
    TEXT_COLOR_TRANSPARENT,
    TEXT_COLOR_WHITE,
    TEXT_COLOR_DARK_GRAY,
};

static const struct WindowTemplate sDockNameWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 7,
    .tilemapTop = 14,
    .width = 16,
    .height = 2,
    .paletteNum = 15,
    .baseBlock = 0x80,
};

enum
{
    KEY_ITEM_BAR_STATE_OPENING,
    KEY_ITEM_BAR_STATE_WAIT_RELEASE,
    KEY_ITEM_BAR_STATE_INPUT,
};

static void CreateKeyItemBarSprites(void)
{
    struct BagPocket *pocket = &gBagPockets[POCKET_KEY_ITEMS];
    u32 i;

    sKeyItemBarItemCount = 0;

    for (i = 0; i < pocket->capacity && sKeyItemBarItemCount < KEY_ITEM_BAR_MAX_ITEMS; i++)
    {
        enum Item itemId = GetBagItemId(POCKET_KEY_ITEMS, i);

        if (itemId == ITEM_NONE)
            continue;

        sKeyItemBarItemIds[sKeyItemBarItemCount++] = itemId;
    }

    if (sKeyItemBarItemCount == 0)
        return;

    sKeyItemBarSelected = 0;
    UpdateKeyItemBarSelection();
}

static u8 GetDockItemIndex(s8 offset)
{
    s16 itemIndex = sKeyItemBarSelected + offset;

    while (itemIndex < 0)
        itemIndex += sKeyItemBarItemCount;
    while (itemIndex >= sKeyItemBarItemCount)
        itemIndex -= sKeyItemBarItemCount;

    return itemIndex;
}

static void RefreshKeyItemBarSprites(void)
{
    // Affine-double item sprites render 12 px to the right of their nominal
    // position, so offset the whole strip left to keep it visually centred.
    static const u8 sDockXPositions[KEY_ITEM_BAR_VISIBLE_ICONS] = {44, 76, 108, 140, 172};
    // Every icon shares a baseline. Selection is communicated entirely through
    // scale, so the active item grows in place like a macOS Dock icon.
    static const u8 sDockYPositions[KEY_ITEM_BAR_VISIBLE_ICONS] = {136, 136, 136, 136, 136};
    // OAM affine matrices use inverse scale values: smaller values render a
    // larger sprite. Keep the centre prominent and step down at the edges.
    static const u16 sDockScales[KEY_ITEM_BAR_VISIBLE_ICONS] = {0x120, 0x100, 0x90, 0x100, 0x120};
    u8 firstSlot;
    u8 visibleCount;
    u8 i;

    DestroyKeyItemBarSprites();

    if (sKeyItemBarItemCount == 0)
        return;

    visibleCount = min(sKeyItemBarItemCount, KEY_ITEM_BAR_VISIBLE_ICONS);
    firstSlot = (KEY_ITEM_BAR_VISIBLE_ICONS - visibleCount) / 2;

    for (i = 0; i < visibleCount; i++)
    {
        u8 dockSlot = firstSlot + i;
        enum Item itemId = sKeyItemBarItemIds[GetDockItemIndex(dockSlot - 2)];
        u8 spriteId = AddItemIconSprite(
            KEY_ITEM_BAR_TILE_TAG_BASE + i,
            KEY_ITEM_BAR_PALETTE_TAG_BASE + i,
            itemId);
        struct Sprite *sprite;
        u8 matrixNum;

        if (spriteId == MAX_SPRITES)
            continue;

        sprite = &gSprites[spriteId];
        sprite->x = sDockXPositions[dockSlot];
        sprite->y = sDockYPositions[dockSlot];
        sprite->oam.priority = 0;

        matrixNum = AllocOamMatrix();
        if (matrixNum != 0xFF)
        {
            // The doubled affine canvas prevents enlarged 32x32 icons from
            // clipping at their original sprite bounds.
            sprite->oam.affineMode = ST_OAM_AFFINE_DOUBLE;
            sprite->oam.matrixNum = matrixNum;
            SetOamMatrix(matrixNum, sDockScales[dockSlot], 0, 0, sDockScales[dockSlot]);
        }

        sKeyItemBarSpriteIds[sKeyItemBarSpriteCount++] = spriteId;
    }

}

static void UpdateKeyItemBarSelection(void)
{
    if (sKeyItemBarItemCount == 0)
        return;

    RefreshKeyItemBarSprites();
    PrintSelectedKeyItemName();
}



static void DestroyKeyItemBarSprites(void)
{
    u8 i;

    for (i = 0; i < sKeyItemBarSpriteCount; i++)
    {
        u8 spriteId = sKeyItemBarSpriteIds[i];

        FreeSpriteOamMatrix(&gSprites[spriteId]);
        DestroySprite(&gSprites[spriteId]);

        FreeSpriteTilesByTag(
            KEY_ITEM_BAR_TILE_TAG_BASE + i
        );

        FreeSpritePaletteByTag(
            KEY_ITEM_BAR_PALETTE_TAG_BASE + i
        );

        sKeyItemBarSpriteIds[i] = 0;
    }

    sKeyItemBarSpriteCount = 0;
}





bool32 TryOpenKeyItemBar(void)
{
    // Prevent duplicate instances if Select is processed twice.
    if (FindTaskIdByFunc(Task_KeyItemBarWaitForInput) != TASK_NONE)
        return FALSE;

    FreezeObjectEvents();
    PlayerFreeze();
    StopPlayerAvatar();
    LockPlayerFieldControls();

    CreateDockNameWindow();
    CreateKeyItemBarSprites();
    CreateTask(Task_KeyItemBarWaitForInput, 0x50);
    PlaySE(SE_WIN_OPEN);

    return TRUE;
}

static void Task_KeyItemBarWaitForInput(u8 taskId)
{
    struct Task *task = &gTasks[taskId];

    switch (task->data[0])
    {
    case KEY_ITEM_BAR_STATE_OPENING:
        task->data[0] = KEY_ITEM_BAR_STATE_WAIT_RELEASE;
        break;

    case KEY_ITEM_BAR_STATE_WAIT_RELEASE:
        if (!(gMain.heldKeys & SELECT_BUTTON))
            task->data[0] =
                KEY_ITEM_BAR_STATE_INPUT;
        break;

case KEY_ITEM_BAR_STATE_INPUT:
    if (JOY_NEW(DPAD_LEFT))
    {
        if (sKeyItemBarItemCount != 0)
        {
            if (sKeyItemBarSelected == 0)
                sKeyItemBarSelected = sKeyItemBarItemCount - 1;
            else
                sKeyItemBarSelected--;

            PlaySE(SE_SELECT);
            UpdateKeyItemBarSelection();
        }
    }
    else if (JOY_NEW(DPAD_RIGHT))
    {
        if (sKeyItemBarItemCount != 0)
        {
            sKeyItemBarSelected++;

            if (sKeyItemBarSelected >= sKeyItemBarItemCount)
                sKeyItemBarSelected = 0;

            PlaySE(SE_SELECT);
            UpdateKeyItemBarSelection();
        }
    }
    else if (JOY_NEW(B_BUTTON | SELECT_BUTTON))
    {
        PlaySE(SE_SELECT);

        DestroyKeyItemBarSprites();
        CloseKeyItemBar(taskId);
    }
    else if (JOY_NEW(A_BUTTON) && sKeyItemBarItemCount != 0)
    {
        PlaySE(SE_SELECT);
        UseSelectedKeyItem(taskId);
    }
    break;

    }
}

static void CloseKeyItemBar(u8 taskId)
{
    RemoveDockNameWindow();
    ScriptUnfreezeObjectEvents();
    UnlockPlayerFieldControls();

    DestroyTask(taskId);
}

static void UseSelectedKeyItem(u8 taskId)
{
    u8 itemTaskId;
    enum Item itemId = sKeyItemBarItemIds[sKeyItemBarSelected];

    // Keep the player locked: Key Item field-use tasks expect the same state as
    // the normal Select-button shortcut and will release it when they finish.
    DestroyKeyItemBarSprites();
    RemoveDockNameWindow();
    gSpecialVar_ItemId = itemId;
    itemTaskId = CreateTask(GetItemFieldFunc(itemId), 8);
    gTasks[itemTaskId].data[3] = TRUE;

    DestroyTask(taskId);
}

static void CreateDockNameWindow(void)
{
    if (sKeyItemBarNameWindowActive)
        return;

    sKeyItemBarNameWindowId = AddWindow(&sDockNameWindowTemplate);
    if (sKeyItemBarNameWindowId == WINDOW_NONE)
        return;

    sKeyItemBarNameWindowActive = TRUE;
    FillWindowPixelBuffer(sKeyItemBarNameWindowId, PIXEL_FILL(TEXT_COLOR_TRANSPARENT));
    PutWindowTilemap(sKeyItemBarNameWindowId);
    CopyWindowToVram(sKeyItemBarNameWindowId, COPYWIN_FULL);
}

static void RemoveDockNameWindow(void)
{
    if (!sKeyItemBarNameWindowActive)
        return;

    ClearWindowTilemap(sKeyItemBarNameWindowId);
    CopyWindowToVram(sKeyItemBarNameWindowId, COPYWIN_MAP);
    RemoveWindow(sKeyItemBarNameWindowId);
    sKeyItemBarNameWindowActive = FALSE;
}

static void PrintSelectedKeyItemName(void)
{
    const u8 *name;
    u8 x;

    if (!sKeyItemBarNameWindowActive || sKeyItemBarItemCount == 0)
        return;

    name = gItemsInfo[sKeyItemBarItemIds[sKeyItemBarSelected]].name;
    x = GetStringCenterAlignXOffset(FONT_NORMAL, name, 16 * 8);

    FillWindowPixelBuffer(sKeyItemBarNameWindowId, PIXEL_FILL(TEXT_COLOR_TRANSPARENT));
    AddTextPrinterParameterized3(sKeyItemBarNameWindowId, FONT_NORMAL, x, 1,
                                 sDockNameTextColors, TEXT_SKIP_DRAW, name);
    CopyWindowToVram(sKeyItemBarNameWindowId, COPYWIN_GFX);
}

#endif // MODULE_KEY_ITEM_DOCK_ENABLED
