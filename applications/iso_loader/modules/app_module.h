/* DreamShell ##version##

   module.c - ISO Loader app module
   Copyright (C) 2011 Superdefault
   Copyright (C) 2011-2023 SWAT

*/

#include <ds.h>

void isoLoader_DefaultPreset();

int isoLoader_LoadPreset();

int isoLoader_SavePreset();

void isoLoader_ResizeUI();

void isoLoader_toggleMemory(GUI_Widget *widget);

void isoLoader_toggleBootMode(GUI_Widget *widget);

void isoLoader_toggleIconSize(GUI_Widget *widget);

void isoLoader_Rotate_Image(GUI_Widget *widget);

void isoLoader_ShowPage(GUI_Widget *widget);

void isoLoader_ShowSettings(GUI_Widget *widget);

void isoLoader_ShowExtensions(GUI_Widget *widget);

void isoLoader_ShowGames(GUI_Widget *widget);

void isoLoader_ShowLink(GUI_Widget *widget);

void isoLoader_MakeShortcut(GUI_Widget *widget);

void isoLoader_toggleIconSize(GUI_Widget *widget);

void isoLoader_toggleLinkName(GUI_Widget *widget);

void isoLoader_toggleHideName(GUI_Widget *widget);

void isoLoader_toggleOptions(GUI_Widget *widget);

void isoLoader_togglePatchAddr(GUI_Widget *widget);

/* Switch to the selected volume */
void isoLoader_SwitchVolume(void *dir);

void isoLoader_toggleOS(GUI_Widget *widget);

void isoLoader_toggleAsync(GUI_Widget *widget);

void isoLoader_toggleDMA(GUI_Widget *widget);

void isoLoader_toggleCDDA(GUI_Widget *widget);

void isoLoader_toggleHeap(GUI_Widget *widget);

void isoLoader_toggleMemory(GUI_Widget *widget);

void isoLoader_toggleBootMode(GUI_Widget *widget);

void isoLoader_toggleExtension(GUI_Widget *widget);

void isoLoader_Run(GUI_Widget *widget);

void isoLoader_ItemClick(dirent_fm_t *fm_ent);

void isoLoader_ItemContextClick(dirent_fm_t *fm_ent);

void isoLoader_Init(App_t *app);

void isoLoader_ResizeUI();

void isoLoader_Shutdown(App_t *app);

void isoLoader_Exit(GUI_Widget *widget);
