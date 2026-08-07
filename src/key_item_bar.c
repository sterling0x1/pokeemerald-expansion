#include "global.h"
#include "bg.h"
#include "event_object_lock.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "key_item_bar.h"
#include "main.h"
#include "menu.h"
#include "script.h"
#include "sound.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "constants/songs.h"
#include "item.h"
#include "item_icon.h"
#include "sprite.h"

#define KEY_ITEM_BAR_TOP_VISIBLE 14
#define KEY_ITEM_BAR_TOP_HIDDEN  20
#define KEY_ITEM_BAR_MOVE_DELAY  2
#define KEY_ITEM_BAR_MAX_ICONS 5

#define KEY_ITEM_BAR_TILE_TAG_BASE    0x7000
#define KEY_ITEM_BAR_PALETTE_TAG_BASE 0x7100

static void Task_KeyItemBarWaitForInput(u8 taskId);
static void ShowKeyItemBarWindow(void);
static void RemoveKeyItemBarWindow(void);
static void CloseKeyItemBar(u8 taskId);
static void MoveKeyItemBarWindow(u8 tilemapTop);
static EWRAM_DATA u8 sKeyItemBarSpriteIds[KEY_ITEM_BAR_MAX_ICONS] = {0};
static EWRAM_DATA u8 sKeyItemBarSpriteCount = 0;
static void CreateKeyItemBarSprites(void);
static void DestroyKeyItemBarSprites(void);
static EWRAM_DATA enum Item sKeyItemBarItemIds[KEY_ITEM_BAR_MAX_ICONS] = {0};
static EWRAM_DATA u8 sKeyItemBarSelected = 0;
static EWRAM_DATA u8 sKeyItemBarAffineMatrix = 0;
static EWRAM_DATA bool8 sKeyItemBarAffineAllocated = FALSE;

static EWRAM_DATA u8 sKeyItemBarWindowId = 0;
static EWRAM_DATA bool8 sKeyItemBarWindowActive = FALSE;

static void UpdateKeyItemBarSelection(void);
static void PrintSelectedKeyItemName(void);

static const struct WindowTemplate sKeyItemBarWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = KEY_ITEM_BAR_TOP_HIDDEN,
    .width = 28,
    .height = 5,
    .paletteNum = 15,
    .baseBlock = 0x80,
};

enum
{
    KEY_ITEM_BAR_STATE_OPENING,
    KEY_ITEM_BAR_STATE_WAIT_RELEASE,
    KEY_ITEM_BAR_STATE_INPUT,
    KEY_ITEM_BAR_STATE_CLOSING,
};

static void CreateKeyItemBarSprites(void)
{
    struct BagPocket *pocket = &gBagPockets[POCKET_KEY_ITEMS];
    u32 i;
    u8 count = 0;

    sKeyItemBarSpriteCount = 0;

    for (i = 0; i < pocket->capacity && count < KEY_ITEM_BAR_MAX_ICONS; i++)
    {
        enum Item itemId = GetBagItemId(POCKET_KEY_ITEMS, i);
        u8 spriteId;

        if (itemId == ITEM_NONE)
            continue;

        spriteId = AddItemIconSprite(
            KEY_ITEM_BAR_TILE_TAG_BASE + count,
            KEY_ITEM_BAR_PALETTE_TAG_BASE + count,
            itemId
        );

        if (spriteId == MAX_SPRITES)
            continue;

        sKeyItemBarSpriteIds[count] = spriteId;
        sKeyItemBarItemIds[count] = itemId;

        gSprites[spriteId].x = 56 + count * 32;
        gSprites[spriteId].y = 136;
        gSprites[spriteId].oam.priority = 0;

        count++;
    }

    sKeyItemBarSpriteCount = count;

    if (count == 0)
        return;

    // Start roughly in the middle of the dock.
    sKeyItemBarSelected = count / 2;


    UpdateKeyItemBarSelection();
}

static void UpdateKeyItemBarSelection(void)
{
    u8 i;

    if (sKeyItemBarSpriteCount == 0)
        return;

    for (i = 0; i < sKeyItemBarSpriteCount; i++)
    {
        struct Sprite *sprite = &gSprites[sKeyItemBarSpriteIds[i]];

        sprite->y = 136;

        if (i == sKeyItemBarSelected)
            sprite->y = 124;
    }

    PrintSelectedKeyItemName();
}

static void PrintSelectedKeyItemName(void)
{
    const u8 *name;

    if (sKeyItemBarSpriteCount == 0)
        return;

    name = gItemsInfo[sKeyItemBarItemIds[sKeyItemBarSelected]].name;

    FillWindowPixelRect(
        sKeyItemBarWindowId,
        PIXEL_FILL(1),
        0,
        0,
        224,
        16
    );

    AddTextPrinterParameterized(
        sKeyItemBarWindowId,
        FONT_NORMAL,
        name,
        8,
        2,
        TEXT_SKIP_DRAW,
        NULL
    );

    CopyWindowToVram(
        sKeyItemBarWindowId,
        COPYWIN_GFX
    );
}



static void DestroyKeyItemBarSprites(void)
{
    u8 i;

    for (i = 0; i < sKeyItemBarSpriteCount; i++)
    {
        u8 spriteId = sKeyItemBarSpriteIds[i];

        DestroySprite(&gSprites[spriteId]);

        FreeSpriteTilesByTag(
            KEY_ITEM_BAR_TILE_TAG_BASE + i
        );

        FreeSpritePaletteByTag(
            KEY_ITEM_BAR_PALETTE_TAG_BASE + i
        );

        sKeyItemBarSpriteIds[i] = 0;
        sKeyItemBarItemIds[i] = ITEM_NONE;
    }

    if (sKeyItemBarAffineAllocated)
    {
        FreeOamMatrix(sKeyItemBarAffineMatrix);
        sKeyItemBarAffineAllocated = FALSE;
        sKeyItemBarAffineMatrix = 0;
    }

    sKeyItemBarSpriteCount = 0;
    sKeyItemBarSelected = 0;
}




    

static void MoveKeyItemBarWindow(u8 tilemapTop)
{
    ClearWindowTilemap(sKeyItemBarWindowId);

    SetWindowAttribute(
        sKeyItemBarWindowId,
        WINDOW_TILEMAP_TOP,
        tilemapTop
    );

    PutWindowTilemap(sKeyItemBarWindowId);
    CopyWindowToVram(sKeyItemBarWindowId, COPYWIN_MAP);
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

ShowKeyItemBarWindow();

if (!sKeyItemBarWindowActive)
{
    ScriptUnfreezeObjectEvents();
    UnlockPlayerFieldControls();
    return FALSE;
}

CreateTask(Task_KeyItemBarWaitForInput, 0x50);
PlaySE(SE_WIN_OPEN);

return TRUE;
}

static void ShowKeyItemBarWindow(void)
{
    if (sKeyItemBarWindowActive)
        return;

        LoadMessageBoxAndBorderGfx();


 sKeyItemBarWindowId = AddWindow(
    &sKeyItemBarWindowTemplate
);

    if (sKeyItemBarWindowId == WINDOW_NONE)
        return;

    sKeyItemBarWindowActive = TRUE;

    FillWindowPixelBuffer(
        sKeyItemBarWindowId,
        PIXEL_FILL(1)
    );

    CopyWindowToVram(
        sKeyItemBarWindowId,
        COPYWIN_GFX
    );

    MoveKeyItemBarWindow(KEY_ITEM_BAR_TOP_HIDDEN);
}

static void Task_KeyItemBarWaitForInput(u8 taskId)
{
    struct Task *task = &gTasks[taskId];

    switch (task->data[0])
    {
case KEY_ITEM_BAR_STATE_OPENING:
    if (++task->data[1] < KEY_ITEM_BAR_MOVE_DELAY)
        break;

    task->data[1] = 0;

    if (GetWindowAttribute(
            sKeyItemBarWindowId,
            WINDOW_TILEMAP_TOP
        ) > KEY_ITEM_BAR_TOP_VISIBLE)
    {
        u8 top = GetWindowAttribute(
            sKeyItemBarWindowId,
            WINDOW_TILEMAP_TOP
        );

        MoveKeyItemBarWindow(top - 1);
    }
    else
    {
        DrawStdWindowFrame(
            sKeyItemBarWindowId,
            FALSE
        );

        CopyWindowToVram(
            sKeyItemBarWindowId,
            COPYWIN_FULL
        );

        CreateKeyItemBarSprites();

        task->data[0] =
            KEY_ITEM_BAR_STATE_WAIT_RELEASE;
    }

    break;

    case KEY_ITEM_BAR_STATE_WAIT_RELEASE:
        if (!(gMain.heldKeys & SELECT_BUTTON))
            task->data[0] =
                KEY_ITEM_BAR_STATE_INPUT;
        break;

case KEY_ITEM_BAR_STATE_INPUT:
    if (JOY_NEW(DPAD_LEFT))
    {
        if (sKeyItemBarSpriteCount != 0)
        {
            if (sKeyItemBarSelected == 0)
                sKeyItemBarSelected = sKeyItemBarSpriteCount - 1;
            else
                sKeyItemBarSelected--;

            PlaySE(SE_SELECT);
            UpdateKeyItemBarSelection();
        }
    }
    else if (JOY_NEW(DPAD_RIGHT))
    {
        if (sKeyItemBarSpriteCount != 0)
        {
            sKeyItemBarSelected++;

            if (sKeyItemBarSelected >= sKeyItemBarSpriteCount)
                sKeyItemBarSelected = 0;

            PlaySE(SE_SELECT);
            UpdateKeyItemBarSelection();
        }
    }
    else if (JOY_NEW(B_BUTTON | SELECT_BUTTON))
    {
        PlaySE(SE_SELECT);

        DestroyKeyItemBarSprites();

        ClearStdWindowAndFrameToTransparent(
            sKeyItemBarWindowId,
            FALSE
        );

        ClearWindowTilemap(sKeyItemBarWindowId);
        ScheduleBgCopyTilemapToVram(0);

        task->data[1] = 0;
        task->data[0] =
            KEY_ITEM_BAR_STATE_CLOSING;
    }
    break;

    case KEY_ITEM_BAR_STATE_CLOSING:
        if (++task->data[1] < KEY_ITEM_BAR_MOVE_DELAY)
            break;

        task->data[1] = 0;

        if (GetWindowAttribute(
                sKeyItemBarWindowId,
                WINDOW_TILEMAP_TOP
            ) < KEY_ITEM_BAR_TOP_HIDDEN)
        {
            u8 top = GetWindowAttribute(
                sKeyItemBarWindowId,
                WINDOW_TILEMAP_TOP
            );

            MoveKeyItemBarWindow(top + 1);
        }
        else
        {
            CloseKeyItemBar(taskId);
        }
        break;
    }
}

static void RemoveKeyItemBarWindow(void)
{
    if (!sKeyItemBarWindowActive)
        return;

    ClearWindowTilemap(sKeyItemBarWindowId);
    CopyWindowToVram(sKeyItemBarWindowId, COPYWIN_MAP);
    RemoveWindow(sKeyItemBarWindowId);

    sKeyItemBarWindowId = 0;
    sKeyItemBarWindowActive = FALSE;

    ScheduleBgCopyTilemapToVram(0);
}

static void CloseKeyItemBar(u8 taskId)
{
    RemoveKeyItemBarWindow();

    ScriptUnfreezeObjectEvents();
    UnlockPlayerFieldControls();

    DestroyTask(taskId);
}