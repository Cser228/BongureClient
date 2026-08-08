#ifndef MACRO_CONFIG_INT
#error "The config macros must be defined"
#define MACRO_CONFIG_INT(Name, ScriptName, Def, Min, Max, Save, Desc) ;
#define MACRO_CONFIG_COL(Name, ScriptName, Def, Save, Desc) ;
#define MACRO_CONFIG_STR(Name, ScriptName, Len, Def, Save, Desc) ;
#endif

//bonga
MACRO_CONFIG_INT(BcBongaVoice, bc_bonga_voice, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable Bongure voice assistant")
MACRO_CONFIG_INT(BcBongaDebug, bc_bonga_debug, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable Bongure Debug window")
//bonga

//Dummy arrow
MACRO_CONFIG_INT(BcDummyPointer, bc_dummy_pointer, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show arrow whos point on dummy")
MACRO_CONFIG_INT(BcDummyPointerSize, bc_dummy_pointer_size, 16, 1, 50, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Dummy arrow pointer size")
MACRO_CONFIG_INT(BcDummyPointerColorR, bc_dummy_pointer_color_r, 255, 0, 255, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Dummy arrow pointor color red")
MACRO_CONFIG_INT(BcDummyPointerColorG, bc_dummy_pointer_color_g, 255, 0, 255, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Dummy arrow pointor color green")
MACRO_CONFIG_INT(BcDummyPointerColorB, bc_dummy_pointer_color_b, 255, 0, 255, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Dummy arrow pointor color blue")
//Dummy arrow

//Edge info by 1 0
MACRO_CONFIG_INT(BcEdgeInfoBc, bc_edgeinfo, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Rushie edge info only by toggle, its 1 0")
//Edge info by 1 0

// Auto-mute system
MACRO_CONFIG_INT(BcAutoMute, bc_auto_mute, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Auto-mute players repeating the same message")
MACRO_CONFIG_INT(BcAutoMuteTimes, bc_auto_mute_times, 3, 2, 50, CFGFLAG_CLIENT | CFGFLAG_SAVE, "How many repeated messages before auto-mute")
MACRO_CONFIG_INT(BcAutoMuteVremya, bc_auto_mute_vremya, 0, 0, 3600, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Time window in seconds to track repeated messages for auto-mute (0 = disabled, track forever)")
// Auto-mute system

//Linux Colors
MACRO_CONFIG_INT(BcBongureMenuBgR, bc_bongure_menu_bg_r, 0, 0, 255, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Bongure menu background red")
MACRO_CONFIG_INT(BcBongureMenuBgG, bc_bongure_menu_bg_g, 0, 0, 255, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Bongure menu background green")
MACRO_CONFIG_INT(BcBongureMenuBgB, bc_bongure_menu_bg_b, 0, 0, 255, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Bongure menu background blue")
MACRO_CONFIG_INT(BcBongureMenuTextR, bc_bongure_menu_text_r, 0, 0, 255, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Bongure menu text red")
MACRO_CONFIG_INT(BcBongureMenuTextG, bc_bongure_menu_text_g, 255, 0, 255, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Bongure menu text green")
MACRO_CONFIG_INT(BcBongureMenuTextB, bc_bongure_menu_text_b, 0, 0, 255, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Bongure menu text blue")
//Linux Colors

//Auto-translate system
MACRO_CONFIG_INT(BcAutoTranslate, bc_auto_translate, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Auto-translate all message in dont russian")
//Auto-translate system

// Aspect ratio
MACRO_CONFIG_INT(BcCustomAspectRatioMode, bc_custom_aspect_ratio_mode, -1, -1, 2, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Aspect ratio mode (-1=legacy auto, 0=off, 1=preset, 2=custom)")
MACRO_CONFIG_INT(BcCustomAspectRatioApplyMode, bc_custom_aspect_ratio_apply_mode, 1, 0, 2, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Aspect ratio apply mode (0=game only, 1=full, 2=game no hud)")
MACRO_CONFIG_INT(BcCustomAspectRatio, bc_custom_aspect_ratio, 0, 0, 1000, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Aspect ratio value x100 (0=off, presets: 125=5:4, 133=4:3, 150=3:2, custom: 100-1000)")
MACRO_CONFIG_INT(BcCustomAspectRatioNum, bc_custom_aspect_ratio_num, 0, 0, 100000, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Custom aspect ratio numerator (width), 0=unset")
MACRO_CONFIG_INT(BcCustomAspectRatioDen, bc_custom_aspect_ratio_den, 0, 0, 100000, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Custom aspect ratio denominator (height), 0=unset")
// Aspect ratio
