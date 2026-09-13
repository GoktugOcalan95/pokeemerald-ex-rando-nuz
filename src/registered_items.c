#include "global.h"
#include "registered_items.h"
#include "decompress.h"
#include "gpu_regs.h"
#include "item_icon.h"
#include "sprite.h"
#include "item.h"

s32 GetRegisteredItemSlot(u16 item)
{
    if (item != ITEM_NONE)
        for (u32 i = 0; i < REGISTERED_ITEMS_COUNT; i++)
            if (gSaveBlock1Ptr->registeredItems[i] == item)
                return i;
    return -1;
}

void UnregisterItem(u16 item)
{
    for (u32 i = 0; i < REGISTERED_ITEMS_COUNT; i++)
        if (gSaveBlock1Ptr->registeredItems[i] == item)
            gSaveBlock1Ptr->registeredItems[i] = ITEM_NONE;
}

static bool32 CanRegisterItem(u16 item)
{
    return item > ITEM_NONE && item < ITEMS_COUNT
        && GetItemPocket(item) == POCKET_KEY_ITEMS
        && GetItemFieldFunc(item) != NULL
        && CheckBagHasItem(item, 1);
}

bool32 RegisterItem(u32 slot, u16 item)
{
    if (slot >= REGISTERED_ITEMS_COUNT || !CanRegisterItem(item))
        return FALSE;
    UnregisterItem(item);
    gSaveBlock1Ptr->registeredItems[slot] = item;
    return TRUE;
}

u32 ValidateRegisteredItems(void)
{
    u32 count = 0;
    for (u32 i = 0; i < REGISTERED_ITEMS_COUNT; i++)
    {
        u16 item = gSaveBlock1Ptr->registeredItems[i];
        if (!CanRegisterItem(item) || GetRegisteredItemSlot(item) != i)
            gSaveBlock1Ptr->registeredItems[i] = ITEM_NONE;
        else
            count++;
    }
    return count;
}

s32 RegisteredItemSlotFromKeys(u16 keys)
{
    if (keys & DPAD_UP)
        return 0;
    if (keys & DPAD_RIGHT)
        return 1;
    if (keys & DPAD_DOWN)
        return 2;
    if (keys & DPAD_LEFT)
        return 3;
    return -1;
}

u16 RegisteredItemWheelInput(u16 newKeys)
{
    s32 slot;
    if (newKeys & (B_BUTTON | SELECT_BUTTON))
        return ITEMS_COUNT;
    slot = RegisteredItemSlotFromKeys(newKeys);
    if (slot >= 0)
        return gSaveBlock1Ptr->registeredItems[slot];
    return ITEM_NONE;
}

#define TAG_REGISTERED_ITEM_PANEL 0x9000
#define TAG_REGISTERED_ITEM_ICON(slot) (TAG_REGISTERED_ITEM_PANEL + 1 + (slot))

static EWRAM_DATA u8 sPanelSprites[REGISTERED_ITEMS_COUNT] = {0};
static EWRAM_DATA u8 sIconSprites[REGISTERED_ITEMS_COUNT] = {0};
static EWRAM_DATA struct SpriteTemplate sIconTemplates[REGISTERED_ITEMS_COUNT] = {0};
static EWRAM_DATA u16 sSavedWinOut = 0;
static EWRAM_DATA u16 sSavedWinIn = 0;
static EWRAM_DATA u16 sSavedBlendControl = 0;
static EWRAM_DATA u16 sSavedBlendAlpha = 0;
static EWRAM_DATA bool8 sSavedObjWindow = FALSE;

static const u32 sPanelGfx[] = INCGFX_U32("graphics/bag/key_item_box.png", ".4bpp");
static const u16 sPanelPalette[] = INCGFX_U16("graphics/bag/key_item_box.png", ".gbapal");
static const struct OamData sPanelOam = {
    .shape = SPRITE_SHAPE(32x32),
    .size = SPRITE_SIZE(32x32),
    .priority = 0,
    .objMode = ST_OAM_OBJ_BLEND,
};
static const struct SpriteTemplate sPanelTemplate = {
    .tileTag = TAG_REGISTERED_ITEM_PANEL,
    .paletteTag = TAG_REGISTERED_ITEM_PANEL,
    .oam = &sPanelOam,
    .anims = gDummySpriteAnimTable,
    .callback = SpriteCallbackDummy,
};

bool32 ShowRegisteredItemWheel(bool32 inBag)
{
    static const s8 offsets[REGISTERED_ITEMS_COUNT][2] = {
        {0, -32}, {32, 0}, {0, 32}, {-32, 0},
    };
    struct SpriteSheet sheet = {sPanelGfx, sizeof(sPanelGfx), TAG_REGISTERED_ITEM_PANEL};
    struct SpritePalette palette = {sPanelPalette, TAG_REGISTERED_ITEM_PANEL};
    s16 centerX = inBag ? 172 : 120;
    s16 centerY = inBag ? 88 : 80;

    sSavedWinOut = GetGpuReg(REG_OFFSET_WINOUT);
    sSavedWinIn = GetGpuReg(REG_OFFSET_WININ);
    sSavedBlendControl = GetGpuReg(REG_OFFSET_BLDCNT);
    sSavedBlendAlpha = GetGpuReg(REG_OFFSET_BLDALPHA);
    SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT2_BG0 | BLDCNT_TGT2_BG1 | BLDCNT_TGT2_BG2 | BLDCNT_TGT2_BG3 | BLDCNT_TGT2_BD);
    SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(8, 8));
    SetGpuRegBits(REG_OFFSET_WININ, WININ_WIN0_CLR | WININ_WIN1_CLR);
    SetGpuRegBits(REG_OFFSET_WINOUT, WINOUT_WIN01_CLR | WINOUT_WINOBJ_CLR);
    sSavedObjWindow = (GetGpuReg(REG_OFFSET_DISPCNT) & DISPCNT_OBJWIN_ON) != 0;
    if (!inBag)
    {
        SetGpuRegBits(REG_OFFSET_DISPCNT, DISPCNT_OBJWIN_ON);
        SetGpuRegBits(REG_OFFSET_WINOUT, WINOUT_WINOBJ_OBJ);
    }
    for (u32 i = 0; i < REGISTERED_ITEMS_COUNT; i++)
    {
        sPanelSprites[i] = MAX_SPRITES;
        sIconSprites[i] = MAX_SPRITES;
    }
    LoadSpriteSheet(&sheet);
    if (GetSpriteTileStartByTag(sheet.tag) == TAG_NONE || LoadSpritePalette(&palette) == 0xFF)
        goto fail;
    for (u32 i = 0; i < REGISTERED_ITEMS_COUNT; i++)
    {
        u16 item = gSaveBlock1Ptr->registeredItems[i];
        s16 x = centerX + offsets[i][0];
        s16 y = centerY + offsets[i][1];
        u8 matrixNum;
        u8 spriteId = CreateSpriteUnchecked(&sPanelTemplate, x, y, 1);

        if (spriteId == MAX_SPRITES)
            goto fail;
        sPanelSprites[i] = spriteId;
        matrixNum = AllocOamMatrix();
        if (matrixNum == 0xFF)
            goto fail;
        gSprites[spriteId].oam.affineMode = ST_OAM_AFFINE_NORMAL;
        gSprites[spriteId].oam.matrixNum = matrixNum;
        gSprites[spriteId].affineAnimPaused = TRUE;
        gSprites[spriteId].copyToObjWin = !inBag;
        SetOamMatrixRotationScaling(matrixNum, 0x100, 0x100, (u16)(-i * 0x4000));
        if (item != ITEM_NONE)
        {
            u32 iconGfx[0x120 / sizeof(u32)];
            u32 iconTiles[0x200 / sizeof(u32)] = {0};
            struct SpriteTemplate *template = &sIconTemplates[i];

            DecompressDataWithHeaderWram(GetItemIconPic(item), iconGfx);
            CopyItemIconPicTo4x4Buffer(iconGfx, iconTiles);
            sheet = (struct SpriteSheet){iconTiles, sizeof(iconTiles), TAG_REGISTERED_ITEM_ICON(i)};
            palette = (struct SpritePalette){GetItemIconPalette(item), TAG_REGISTERED_ITEM_ICON(i)};
            LoadSpriteSheet(&sheet);
            if (GetSpriteTileStartByTag(sheet.tag) == TAG_NONE || LoadSpritePalette(&palette) == 0xFF)
                goto fail;
            *template = gItemIconSpriteTemplate;
            template->tileTag = template->paletteTag = TAG_REGISTERED_ITEM_ICON(i);
            template->callback = SpriteCallbackDummy;
            spriteId = CreateSpriteUnchecked(template, x + 4, y + 4, 0);
            if (spriteId == MAX_SPRITES)
                goto fail;
            sIconSprites[i] = spriteId;
            gSprites[spriteId].oam.priority = 0;
        }
    }
    return TRUE;

fail:
    CloseRegisteredItemWheel();
    return FALSE;
}

void CloseRegisteredItemWheel(void)
{
    for (u32 i = 0; i < REGISTERED_ITEMS_COUNT; i++)
    {
        if (sIconSprites[i] != MAX_SPRITES)
            DestroySprite(&gSprites[sIconSprites[i]]);
        if (sPanelSprites[i] != MAX_SPRITES)
        {
            if (gSprites[sPanelSprites[i]].oam.affineMode == ST_OAM_AFFINE_NORMAL)
                FreeOamMatrix(gSprites[sPanelSprites[i]].oam.matrixNum);
            DestroySprite(&gSprites[sPanelSprites[i]]);
        }
        FreeSpriteTilesByTag(TAG_REGISTERED_ITEM_ICON(i));
        FreeSpritePaletteByTag(TAG_REGISTERED_ITEM_ICON(i));
        sIconSprites[i] = sPanelSprites[i] = MAX_SPRITES;
    }
    FreeSpriteTilesByTag(TAG_REGISTERED_ITEM_PANEL);
    FreeSpritePaletteByTag(TAG_REGISTERED_ITEM_PANEL);
    SetGpuReg(REG_OFFSET_WINOUT, sSavedWinOut);
    SetGpuReg(REG_OFFSET_WININ, sSavedWinIn);
    SetGpuReg(REG_OFFSET_BLDCNT, sSavedBlendControl);
    SetGpuReg(REG_OFFSET_BLDALPHA, sSavedBlendAlpha);
    if (!sSavedObjWindow)
        ClearGpuRegBits(REG_OFFSET_DISPCNT, DISPCNT_OBJWIN_ON);
}
