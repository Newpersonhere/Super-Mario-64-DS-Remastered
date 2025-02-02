#ifdef EXT_OPTIONS_MENU
#ifdef DYNOS_INL

#include "sm64.h"
#include "engine/math_util.h"
#include "audio/external.h"
#include "game/camera.h"
#include "game/level_update.h"
#include "game/print.h"
#include "game/segment2.h"
#include "game/save_file.h"
#include "game/hud.h"
#include "game/bettercamera.h"
#include "game/mario_misc.h"
#include "game/game_init.h"
#include "game/cheats_menu.h"
#include "game/ingame_menu.h"
#include "game/options_menu.h"
#include "pc/pc_main.h"
#include "pc/cliopts.h"
#include "pc/cheats.h"
#include "pc/configfile.h"
#include "pc/controller/controller_api.h"
#include <stdbool.h>
#include "../../include/libc/stdlib.h"

#include "text/txtconv.h"
#include "text/text-loader.h"

u8 optmenu_open = 0;

static u8 optmenu_binding = 0;
static u8 optmenu_bind_idx = 0;

/* Keeps track of how many times the user has pressed L while in the options menu, so cheats can be unlocked */
static s32 l_counter = 0;

// How to add stuff:
// strings: add them to include/text_strings.h.in
//          and to menuStr[] / opts*Str[]
// options: add them to the relevant options list
// menus:   add a new submenu definition and a new
//          option to the optsMain list

static const u8 toggleStr[][20] = {
    "TEXT_OPT_DISABLED",
    "TEXT_OPT_ENABLED"
};

static const u8 menuStr[][32] = {
    "TEXT_OPT_HIGHLIGHT",
    "TEXT_OPT_BUTTON1",
    "TEXT_OPT_BUTTON2",
    "TEXT_OPT_OPTIONS",
    "TEXT_OPT_CAMERA",
    "TEXT_OPT_CONTROLS",
    "TEXT_OPT_VIDEO",
    "TEXT_OPT_AUDIO",
    "TEXT_EXIT_GAME",
    "TEXT_OPT_CHEATS",
    "TEXT_OPT_GAME",
    "TEXT_OPT_RT64"
};

static const u8 optsCameraStr[][32] = {
    "TEXT_OPT_CAMX",
    "TEXT_OPT_CAMY",
    "TEXT_OPT_INVERTX",
    "TEXT_OPT_INVERTY",
    "TEXT_OPT_CAMC",
    "TEXT_OPT_CAMP",
    "TEXT_OPT_ANALOGUE",
    "TEXT_OPT_MOUSE",
    "TEXT_OPT_CAMD",
    "TEXT_OPT_CAMON"
};

// TODO: Get dynamic languages
// This is only for testing

static const u8 optsGameStr[][32] = {
    "TEXT_OPT_LANGUAGE",
    "TEXT_OPT_PRECACHE",
    "TEXT_OPT_SWITCH_HUD",
    "TEXT_OPT_BILLBOARDS"
};

#ifdef RAPI_RT64
static const u8 optsRT64Str[][32] = {
    "TEXT_OPT_TARGETFPS",
    "TEXT_OPT_UPSCALER",
    "TEXT_OPT_UPSMODE",
    "TEXT_OPT_UPSSHARP",
    "TEXT_OPT_RESSCALE",
    "TEXT_OPT_MAXLIGHTS",
    "TEXT_OPT_SPHEREL",
    "TEXT_OPT_GI",
    "TEXT_OPT_DENOISER",
    "TEXT_OPT_MOTBLUR",
    "TEXT_OPT_UPSCALER_OFF",
    "TEXT_OPT_UPSCALER_AUTO",
    "TEXT_OPT_UPSCALER_DLSS",
    "TEXT_OPT_UPSCALER_FSR",
    "TEXT_OPT_UPSCALER_XESS",
    "TEXT_OPT_UPSCALER_ULTP",
    "TEXT_OPT_UPSCALER_PERF",
    "TEXT_OPT_UPSCALER_BAL",
    "TEXT_OPT_UPSCALER_QUAL",
    "TEXT_OPT_UPSCALER_ULTQ",
    "TEXT_OPT_UPSCALER_NAT"
};

static const u8 *upscalerChoices[] = {
    optsRT64Str[10],
    optsRT64Str[11],
    optsRT64Str[12],
    optsRT64Str[13],
    optsRT64Str[14]
};

static const u8 *upscalerModeChoices[] = {
    optsRT64Str[11],
    optsRT64Str[15],
    optsRT64Str[16],
    optsRT64Str[17],
    optsRT64Str[18],
    optsRT64Str[19],
    optsRT64Str[20]
};
#endif

static const u8 optsVideoStr[][32] = {
    "TEXT_OPT_FSCREEN",
    "TEXT_OPT_TEXFILTER",
    "TEXT_OPT_NEAREST",
    "TEXT_OPT_LINEAR",
    "TEXT_OPT_RESETWND",
    "TEXT_OPT_VSYNC",
    "TEXT_OPT_AUTO",
    "TEXT_OPT_HUD",
    "TEXT_OPT_THREEPT",
    "TEXT_OPT_DRAWDIST",
    "TEXT_OPT_APPLY",
    "TEXT_OPT_60FPS",
    "TEXT_OPT_INTERNAL_BOOL",
    "TEXT_OPT_INTERNAL",
    "TEXT_OPT_WINDOW",
    "TEXT_OPT_FORCE4BY3"
};

static const u8 optsAudioStr[][32] = {
    "TEXT_OPT_MVOLUME",
    "TEXT_OPT_MUSVOLUME",
    "TEXT_OPT_SFXVOLUME",
    "TEXT_OPT_ENVVOLUME",
    "TEXT_OPT_MUSMUTE",
    "TEXT_OPT_SFXMUTE",
    "TEXT_OPT_ENVMUTE"
};

static const u8 optsCheatsStr[][64] = {
    "TEXT_OPT_CHEAT1",
    "TEXT_OPT_CHEAT2",
    "TEXT_OPT_CHEAT3",
    "TEXT_OPT_CHEAT4",
    "TEXT_OPT_CHEAT5",
    "TEXT_OPT_CHEAT6",
    "TEXT_OPT_CHEAT7",
    "TEXT_OPT_CHEAT8",
    "TEXT_OPT_CHEAT9"
};

static const u8 bindStr[][32] = {
    "TEXT_OPT_UNBOUND",
    "TEXT_OPT_PRESSKEY",
    "TEXT_BIND_A",
    "TEXT_BIND_B",
    "TEXT_BIND_START",
    "TEXT_BIND_L",
    "TEXT_BIND_R",
    "TEXT_BIND_Z",
    "TEXT_BIND_C_UP",
    "TEXT_BIND_C_DOWN",
    "TEXT_BIND_C_LEFT",
    "TEXT_BIND_C_RIGHT",
    "TEXT_BIND_UP",
    "TEXT_BIND_DOWN",
    "TEXT_BIND_LEFT",
    "TEXT_BIND_RIGHT",
    "TEXT_OPT_DEADZONE",
    "TEXT_OPT_RUMBLE",
    "TEXT_BIND_D_UP",
    "TEXT_BIND_D_DOWN",
    "TEXT_BIND_D_LEFT",
    "TEXT_BIND_D_RIGHT",
};

static const u8 *filterChoices[] = {
    optsVideoStr[2],
    optsVideoStr[3],
    optsVideoStr[8],
};

static const u8 *vsyncChoices[] = {
    toggleStr[0],
    toggleStr[1],
    optsVideoStr[6],
};

static const u8 optsInternalRes[][32] = {
    "TEXT_OPT_RES1", //320x240
    "TEXT_OPT_RES2", //640x480
    "TEXT_OPT_RES3", //960x720
    "TEXT_OPT_RES4", //1440x1080
    "TEXT_OPT_RES5", //1920x1440
    "TEXT_OPT_RES6", //640x360
    "TEXT_OPT_RES7", //848x480
    "TEXT_OPT_RES8", //1280x720
    "TEXT_OPT_RES9", //1600x900
    "TEXT_OPT_RES10", //1920x1080
    "TEXT_OPT_RES11", //2560x1440
    "TEXT_OPT_RES12" //3840x2160
};

static const u8 *internalChoices[] = {
    optsInternalRes[0],
    optsInternalRes[1],
    optsInternalRes[2],
    optsInternalRes[3],
    optsInternalRes[4],
    optsInternalRes[5],
    optsInternalRes[6],
    optsInternalRes[7],
    optsInternalRes[8],
    optsInternalRes[9],
    optsInternalRes[10],
    optsInternalRes[11]
};

//static const u8 optsWindowRes[][32] = {
//    "TEXT_OPT_WINDOW_RES1", //320x240
//    "TEXT_OPT_WINDOW_RES2", //640x480
//    "TEXT_OPT_WINDOW_RES3", //960x720
//    "TEXT_OPT_WINDOW_RES4", //1440x1080
//    "TEXT_OPT_WINDOW_RES5", //1920x1440
//    "TEXT_OPT_WINDOW_RES6", //640x360
//    "TEXT_OPT_WINDOW_RES7", //848x480
//    "TEXT_OPT_WINDOW_RES8", //1280x720
//    "TEXT_OPT_WINDOW_RES9", //1600x900
//    "TEXT_OPT_WINDOW_RES10", //1920x1080
//    "TEXT_OPT_WINDOW_RES11", //2560x1440
//    "TEXT_OPT_WINDOW_RES12" //3840x2160
//};
//
//static const u8 *windowChoices[] = {
//    optsWindowRes[0],
//    optsWindowRes[1],
//    optsWindowRes[2],
//    optsWindowRes[3],
//    optsWindowRes[4],
//    optsWindowRes[5],
//    optsWindowRes[6],
//    optsWindowRes[7],
//    optsWindowRes[8],
//    optsWindowRes[9],
//    optsWindowRes[10],
//    optsWindowRes[11]
//};

enum OptType {
    OPT_INVALID = 0,
    OPT_TOGGLE,
    OPT_CHOICE,
    OPT_SCROLL,
    OPT_SUBMENU,
    OPT_BIND,
    OPT_BUTTON,
};

struct SubMenu;

struct Option {
    enum OptType type;
    const u8 *label;
    union {
        u32 *uval;
        bool *bval;
    };
    union {
        struct {
            const u8 **choices;
            u32 numChoices;
        };
        struct {
            u32 scrMin;
            u32 scrMax;
            u32 scrStep;
        };
        struct SubMenu *nextMenu;
        void (*actionFn)(struct Option *, s32);
    };
};

struct SubMenu {
    struct SubMenu *prev; // this is set at runtime to avoid needless complication
    const u8 *label;
    struct Option *opts;
    s32 numOpts;
    s32 select;
    s32 scroll;
};

/* helper macros */

#define DEF_OPT_TOGGLE(lbl, bv) \
    { .type = OPT_TOGGLE, .label = lbl, .bval = bv }
#define DEF_OPT_SCROLL(lbl, uv, min, max, st) \
    { .type = OPT_SCROLL, .label = lbl, .uval = uv, .scrMin = min, .scrMax = max, .scrStep = st }
#define DEF_OPT_CHOICE(lbl, uv, ch) \
    { .type = OPT_CHOICE, .label = lbl, .uval = uv, .choices = ch, .numChoices = sizeof(ch) / sizeof(ch[0]) }
#define DEF_OPT_SUBMENU(lbl, nm) \
    { .type = OPT_SUBMENU, .label = lbl, .nextMenu = nm }
#define DEF_OPT_BIND(lbl, uv) \
    { .type = OPT_BIND, .label = lbl, .uval = uv }
#define DEF_OPT_BUTTON(lbl, act) \
    { .type = OPT_BUTTON, .label = lbl, .actionFn = act }
#define DEF_SUBMENU(lbl, opt) \
    { .label = lbl, .opts = opt, .numOpts = sizeof(opt) / sizeof(opt[0]) }

/* button action functions */

static void optmenu_act_exit(UNUSED struct Option *self, s32 arg) {
    if (!arg) game_exit(); // only exit on A press and not directions
}

static void optvideo_apply(UNUSED struct Option *self, s32 arg) {
    if (!arg) configWindow.settings_changed = true;
}

static void optcheat_disable_all_cheats(UNUSED struct Option *self, s32 arg) {
    if (!arg) memset(&Cheats, 0, sizeof(Cheats));
}

/* submenu option lists */

static struct Option optsCamera[] = {
    DEF_OPT_TOGGLE( optsCameraStr[9], &configEnableCamera ),
    DEF_OPT_TOGGLE( optsCameraStr[6], &configCameraAnalog ),
    DEF_OPT_TOGGLE( optsCameraStr[7], &configCameraMouse ),
    DEF_OPT_TOGGLE( optsCameraStr[2], &configCameraInvertX ),
    DEF_OPT_TOGGLE( optsCameraStr[3], &configCameraInvertY ),
    DEF_OPT_SCROLL( optsCameraStr[0], &configCameraXSens, 1, 100, 1 ),
    DEF_OPT_SCROLL( optsCameraStr[1], &configCameraYSens, 1, 100, 1 ),
    DEF_OPT_SCROLL( optsCameraStr[4], &configCameraAggr, 0, 100, 1 ),
    DEF_OPT_SCROLL( optsCameraStr[5], &configCameraPan, 0, 100, 1 ),
    DEF_OPT_SCROLL( optsCameraStr[8], &configCameraDegrade, 0, 100, 1 ),
};

#ifdef RAPI_RT64
// FIXME: These conditions are currently unused because the menu system doesn't support disabling options.
// The condition usage from sm64rt should be replicated if that functionality is added at some point.

extern bool gfx_rt64_dlss_supported();
extern bool gfx_rt64_fsr_supported();
extern bool gfx_rt64_xess_supported();

static bool opt_upscaler_enabled() {
    return gfx_rt64_dlss_supported() || gfx_rt64_fsr_supported() || gfx_rt64_xess_supported();
}

static bool opt_upscaler_in_use() {
    switch (configRT64Upscaler) {
    case 1:
        return opt_upscaler_enabled();
    case 2:
        return gfx_rt64_dlss_supported();
    case 3:
        return gfx_rt64_fsr_supported();
    case 4:
        return gfx_rt64_xess_supported();
    default:
        return false;
    }
}

static bool opt_upscaler_mode_enabled() {
    return opt_upscaler_in_use();
}

static bool opt_upscaler_sharpness_enabled() {
    return opt_upscaler_in_use() && (configRT64Upscaler != 4);
}

static bool opt_resolution_scale_enabled() {
    return !opt_upscaler_in_use();
}

static struct Option optsRT64[] = {
    DEF_OPT_SCROLL( optsRT64Str[0], &configRT64TargetFPS, 30, 360, 30 ),
    DEF_OPT_CHOICE( optsRT64Str[1], &configRT64Upscaler, upscalerChoices ),
    DEF_OPT_CHOICE( optsRT64Str[2], &configRT64UpscalerMode, upscalerModeChoices ),
    DEF_OPT_SCROLL( optsRT64Str[3], &configRT64UpscalerSharpness, 0, 100, 5 ),
    DEF_OPT_SCROLL( optsRT64Str[4], &configRT64ResScale, 10, 200, 1 ),
    DEF_OPT_SCROLL( optsRT64Str[5], &configRT64MaxLights, 1, 16, 1 ),
    DEF_OPT_TOGGLE( optsRT64Str[6], &configRT64SphereLights ),
    DEF_OPT_TOGGLE( optsRT64Str[7], &configRT64GI ),
    DEF_OPT_TOGGLE( optsRT64Str[8], &configRT64Denoiser ),
    DEF_OPT_SCROLL( optsRT64Str[9], &configRT64MotionBlurStrength, 0, 100, 5 ),
    DEF_OPT_BUTTON( optsVideoStr[10], optvideo_apply ),
};
#endif

static struct Option optsControls[] = {
    DEF_OPT_BIND( bindStr[ 2], configKeyA ),
    DEF_OPT_BIND( bindStr[ 3], configKeyB ),
    DEF_OPT_BIND( bindStr[ 4], configKeyStart ),
    DEF_OPT_BIND( bindStr[ 5], configKeyL ),
    DEF_OPT_BIND( bindStr[ 6], configKeyR ),
    DEF_OPT_BIND( bindStr[ 7], configKeyZ ),
    DEF_OPT_BIND( bindStr[ 8], configKeyCUp ),
    DEF_OPT_BIND( bindStr[ 9], configKeyCDown ),
    DEF_OPT_BIND( bindStr[10], configKeyCLeft ),
    DEF_OPT_BIND( bindStr[11], configKeyCRight ),
    DEF_OPT_BIND( bindStr[18], configKeyDUp ),
    DEF_OPT_BIND( bindStr[19], configKeyDDown ),
    DEF_OPT_BIND( bindStr[20], configKeyDLeft ),
    DEF_OPT_BIND( bindStr[21], configKeyDRight ),
    DEF_OPT_BIND( bindStr[12], configKeyStickUp ),
    DEF_OPT_BIND( bindStr[13], configKeyStickDown ),
    DEF_OPT_BIND( bindStr[14], configKeyStickLeft ),
    DEF_OPT_BIND( bindStr[15], configKeyStickRight ),
    // max deadzone is 31000; this is less than the max range of ~32768, but this
    // way, the player can't accidentally lock themselves out of using the stick
    DEF_OPT_SCROLL( bindStr[16], &configStickDeadzone, 0, 100, 1 ),
    DEF_OPT_SCROLL( bindStr[17], &configRumbleStrength, 0, 100, 1)
};

static struct Option optsVideo[] = {
    #ifndef TARGET_SWITCH
    DEF_OPT_TOGGLE( optsVideoStr[0], &configWindow.fullscreen ),
    DEF_OPT_TOGGLE( optsVideoStr[5], &configWindow.vsync ),
    //DEF_OPT_CHOICE( optsVideoStr[14], &configCustomWindowResolution, windowChoices ),
    #endif
    //DEF_OPT_TOGGLE( optsVideoStr[15], &configForce4by3 ),
    #ifndef RAPI_RT64
    DEF_OPT_TOGGLE( optsVideoStr[11], &config60FPS ),
    #endif
    #if defined(RAPI_D3D11)
    DEF_OPT_TOGGLE( optsVideoStr[12], &configInternalResolutionBool ),
    DEF_OPT_CHOICE( optsVideoStr[13], &configCustomInternalResolution, internalChoices ),
    #endif
    DEF_OPT_CHOICE( optsVideoStr[1], &configFiltering, filterChoices ),
    DEF_OPT_SCROLL( optsVideoStr[9], &configDrawDistance, 50, 509, 10 ),
    DEF_OPT_TOGGLE( optsVideoStr[7], &configHUD ),
    DEF_OPT_BUTTON( optsVideoStr[10], optvideo_apply ),
};

static struct Option optsGame[] = {
    { .type = OPT_CHOICE, .label = optsGameStr[0], .uval = &configLanguage, .choices = NULL, .numChoices = 0 },
    DEF_OPT_TOGGLE( optsGameStr[1], &configPrecacheRes ),
    #ifdef TARGET_SWITCH
    DEF_OPT_TOGGLE( optsGameStr[2], &configSwitchHud ),
    #endif
    DEF_OPT_TOGGLE( optsGameStr[3], &configBillboard ),
};

static struct Option optsAudio[] = {
    DEF_OPT_SCROLL( optsAudioStr[0], &configMasterVolume, 0, MAX_VOLUME, 1 ),
    DEF_OPT_SCROLL( optsAudioStr[1], &configMusicVolume, 0, MAX_VOLUME, 1),
    DEF_OPT_SCROLL( optsAudioStr[2], &configSfxVolume, 0, MAX_VOLUME, 1),
    DEF_OPT_SCROLL( optsAudioStr[3], &configEnvVolume, 0, MAX_VOLUME, 1),
    DEF_OPT_TOGGLE( optsAudioStr[4], &configMusicMute),
    DEF_OPT_TOGGLE( optsAudioStr[5], &configSfxMute),
    DEF_OPT_TOGGLE( optsAudioStr[6], &configEnvMute),
};

static struct Option optsCheatsClassic[] = {
    DEF_OPT_TOGGLE(optsCheatsChtStrings[3], &Cheats.MoonJump),
    DEF_OPT_TOGGLE(optsCheatsChtStrings[4], &Cheats.GodMode),
    DEF_OPT_TOGGLE(optsCheatsChtStrings[5], &Cheats.InfiniteLives),
    DEF_OPT_TOGGLE(optsCheatsChtStrings[6], &Cheats.Responsive),
    DEF_OPT_TOGGLE(optsCheatsChtStrings[7], &Cheats.MoonGravity),
    DEF_OPT_TOGGLE(optsCheatsChtStrings[8], &Cheats.DebugMove),
    DEF_OPT_TOGGLE(optsCheatsChtStrings[9], &Cheats.SuperCopter),
    DEF_OPT_TOGGLE(optsCheatsChtStrings[10], &Cheats.AutoWallKick),
    DEF_OPT_TOGGLE(optsCheatsChtStrings[11], &Cheats.NoHoldHeavy),
};

static struct Option optsCheatsModifiers[] = {
    DEF_OPT_CHOICE(optsCheatsChtStrings[12], &Cheats.SpeedModifier, optsCheatsModifierChoices),
    DEF_OPT_CHOICE(optsCheatsChtStrings[13], &Cheats.JumpModifier, optsCheatsModifierChoices),
    DEF_OPT_CHOICE(optsCheatsChtStrings[14], &Cheats.SwimModifier, optsCheatsModifierChoices),
    DEF_OPT_CHOICE(optsCheatsChtStrings[15], &Cheats.SizeModifier, optsCheatsSizeChoices),
    DEF_OPT_TOGGLE(optsCheatsChtStrings[16], &Cheats.CapModifier),
    DEF_OPT_TOGGLE(optsCheatsChtStrings[17], &Cheats.SuperWingCap),
    DEF_OPT_CHOICE(optsCheatsChtStrings[18], &Cheats.PlayAs, PlayAsCheatChoices),
    DEF_OPT_TOGGLE(optsCheatsChtStrings[19], &Cheats.Jukebox),
    DEF_OPT_CHOICE(optsCheatsChtStrings[20], &Cheats.JukeboxMusic, optsCheatsJukeboxChoices),
    DEF_OPT_TOGGLE(optsCheatsChtStrings[21], &Cheats.SpeedDisplay),
};

static struct Option optsCheatsTimeSpace[] = {
    DEF_OPT_CHOICE(optsCheatsChtStrings[22], &Cheats.BLJAnywhere, bljCheatChoices),
    DEF_OPT_TOGGLE(optsCheatsChtStrings[23], &Cheats.SwimAnywhere),
    DEF_OPT_TOGGLE(optsCheatsChtStrings[24], &Cheats.ExitAnywhere),
    DEF_OPT_TOGGLE(optsCheatsChtStrings[25], &Cheats.WalkOnHazards),
    DEF_OPT_TOGGLE(optsCheatsChtStrings[26], &Cheats.NoDeathBarrier),
    DEF_OPT_TOGGLE(optsCheatsChtStrings[27], &Cheats.NoBounds),
    DEF_OPT_CHOICE(optsCheatsChtStrings[28], &Cheats.WaterLevel, optsCheatsWaterLevelChoices),
    DEF_OPT_TOGGLE(optsCheatsChtStrings[29], &Cheats.CoinsMagnet),
    DEF_OPT_TOGGLE(optsCheatsChtStrings[30], &Cheats.TimeStop),
    DEF_OPT_BIND  (optsCheatsBtnStrings[0],  CheatsControls.TimeStopButton),
    DEF_OPT_TOGGLE(optsCheatsChtStrings[31], &Cheats.QuickEnding),
    DEF_OPT_CHOICE(optsCheatsChtStrings[32], &Cheats.HurtMario, optsCheatsHurtChoices),
};

static struct SubMenu menuCheatsClassic   = DEF_SUBMENU(optsCheatsCatStrings[0], optsCheatsClassic);
static struct SubMenu menuCheatsModifiers = DEF_SUBMENU(optsCheatsCatStrings[1], optsCheatsModifiers);
static struct SubMenu menuCheatsTimeSpace = DEF_SUBMENU(optsCheatsCatStrings[2], optsCheatsTimeSpace);

static struct Option optsCheats[] = {
    DEF_OPT_TOGGLE (optsCheatsChtStrings[0], &Cheats.EnableCheats),
    DEF_OPT_SUBMENU(optsCheatsCatStrings[3], &menuCheatsClassic),
    DEF_OPT_SUBMENU(optsCheatsCatStrings[4], &menuCheatsModifiers),
    DEF_OPT_SUBMENU(optsCheatsCatStrings[5], &menuCheatsTimeSpace),
    DEF_OPT_TOGGLE (optsCheatsChtStrings[1], &Cheats.Spamba),
    DEF_OPT_BIND   (optsCheatsBtnStrings[1], CheatsControls.SpambaControls),
    DEF_OPT_TOGGLE (optsCheatsChtStrings[2], &Cheats.ChaosMode),
    DEF_OPT_BUTTON (optsCheatsCatStrings[6], optcheat_disable_all_cheats),
};

/* submenu definitions */

static struct SubMenu menuCamera   = DEF_SUBMENU( menuStr[4], optsCamera );
#ifdef RAPI_RT64
static struct SubMenu menuRT64     = DEF_SUBMENU( menuStr[11], optsRT64 );
#endif
static struct SubMenu menuGame     = DEF_SUBMENU( menuStr[10], optsGame );
static struct SubMenu menuControls = DEF_SUBMENU( menuStr[5], optsControls );
static struct SubMenu menuVideo    = DEF_SUBMENU( menuStr[6], optsVideo );
static struct SubMenu menuAudio    = DEF_SUBMENU( menuStr[7], optsAudio );
static struct SubMenu menuCheats   = DEF_SUBMENU( menuStr[9], optsCheats );

/* main options menu definition */

static struct Option optsMain[] = {
    DEF_OPT_SUBMENU( menuStr[10], &menuGame ),
    DEF_OPT_SUBMENU( menuStr[4], &menuCamera ),
#ifdef RAPI_RT64
    DEF_OPT_SUBMENU( menuStr[11], &menuRT64 ),
#endif
    DEF_OPT_SUBMENU( menuStr[5], &menuControls ),
    DEF_OPT_SUBMENU( menuStr[6], &menuVideo ),
    DEF_OPT_SUBMENU( menuStr[7], &menuAudio ),
    DEF_OPT_BUTTON ( menuStr[8], optmenu_act_exit ),
    // NOTE: always keep cheats the last option here because of the half-assed way I toggle them
    DEF_OPT_SUBMENU( menuStr[9], &menuCheats )
};

static struct SubMenu menuMain = DEF_SUBMENU( menuStr[3], optsMain );

/* implementation */

static s32 optmenu_option_timer = 0;
static u8 optmenu_hold_count = 0;

static struct SubMenu *currentMenu = &menuMain;

static inline s32 wrap_add(s32 a, const s32 b, const s32 min, const s32 max) {
    s32 c = a + b;
    if (c < min) {
        return max;
    }
    else if (c > max) {
        return min;
    }
    else {
        return c;
    }
}

static void uint_to_hex(u32 num, u8 *dst) {
    u8 places = 4;
    while (places--) {
        const u32 digit = num & 0xF;
        dst[places] = digit;
        num >>= 4;
    }
    dst[4] = DIALOG_CHAR_TERMINATOR;
}

//Displays a box.
static void optmenu_draw_box(s16 x1, s16 y1, s16 x2, s16 y2, u8 r, u8 g, u8 b) {
    gDPPipeSync(gDisplayListHead++);
    gDPSetRenderMode(gDisplayListHead++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
    gDPSetCycleType(gDisplayListHead++, G_CYC_FILL);
    gDPSetFillColor(gDisplayListHead++, GPACK_RGBA5551(r, g, b, 255));
    gDPFillRectangle(gDisplayListHead++, x1, y1, x2 - 1, y2 - 1);
    gDPPipeSync(gDisplayListHead++);
    gDPSetCycleType(gDisplayListHead++, G_CYC_1CYCLE);
}

static void optmenu_draw_text(s16 x, s16 y, const u8 *str, u8 col) {
    const u8 textX = get_str_x_pos_from_center(x, (u8*)str, 10.0f);
    gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, 255);
    print_generic_string(textX+1, y-1, str);
    if (col == 0) {
        gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
    } else {
        gDPSetEnvColor(gDisplayListHead++, 255, 32, 32, 255);
    }
    print_generic_string(textX, y, str);
}

static void optmenu_draw_opt(const struct Option *opt, s16 x, s16 y, u8 sel) {
    u8 buf[32] = { 0 };
    u8 * choice;    

    if (opt->type == OPT_SUBMENU || opt->type == OPT_BUTTON)
        y -= 6;

    optmenu_draw_text(x, y, get_key_string(opt->label), sel);
	
    s16 sx = 0;
    s16 sy = 0;
    s16 sw = 0;
    s16 sh = 0;

    switch (opt->type) {
        case OPT_TOGGLE:
            optmenu_draw_text(x, y-13, get_key_string(toggleStr[(int)*opt->bval]), sel);
            break;

        case OPT_CHOICE:
            if(strcmp(opt->label, optsGameStr[0]) != 0){
                choice = get_key_string(opt->choices[*opt->uval]);
                optmenu_draw_text(x, y-13, choice, sel);
            }else{
                choice = getTranslatedText(languages[*opt->uval]->name);
                optmenu_draw_text(x, y-13, choice, sel);
                free(choice);
            }
            break;

        case OPT_SCROLL:
            sx = x - 127 / 2;
            sy = 209 - (y - 35);            
            sw = sx + (127.0 * (((*opt->uval * 1.0)) / (opt->scrMax * 1.0)));
            sh = sy + 7;

            render_hud_rectangle(sx - 1, sy - 1, (sx + 126), sh + 1, 180, 180, 180);
            render_hud_rectangle(sx, sy, sw - 2, sh, 255, 255, 255);
            break;

        case OPT_BIND:
            x = 112;
            for (u8 i = 0; i < MAX_BINDS; ++i, x += 48) {
                const u8 white = (sel && (optmenu_bind_idx == i));
                // TODO: button names
                if (opt->uval[i] == VK_INVALID) {
                    if (optmenu_binding && white)
                        optmenu_draw_text(x, y-13, get_key_string(bindStr[1]), 1);
                    else
                        optmenu_draw_text(x, y-13, get_key_string(bindStr[0]), white);
                } else {
                    uint_to_hex(opt->uval[i], buf);
                    optmenu_draw_text(x, y-13, buf, white);
                }
            }
            break;

        default: break;
    };
}

static void optmenu_opt_change(struct Option *opt, s32 val) {
    switch (opt->type) {
        case OPT_TOGGLE:
            *opt->bval = !*opt->bval;
            break;

        case OPT_CHOICE:
            *opt->uval = wrap_add(*opt->uval, val, 0, strcmp(opt->label, optsGameStr[0]) == 0 ? get_num_languages() - 1: (s32) opt->numChoices - 1);
            set_language(languages[configLanguage]);
            break;

        case OPT_SCROLL:
            *opt->uval = wrap_add(*opt->uval, opt->scrStep * val, opt->scrMin, opt->scrMax);
            break;

        case OPT_SUBMENU:
            opt->nextMenu->prev = currentMenu;
            currentMenu = opt->nextMenu;
            break;

        case OPT_BUTTON:
            if (opt->actionFn)
                opt->actionFn(opt, val);
            break;

        case OPT_BIND:
            if (val == 0xFF) {
                // clear the bind
                opt->uval[optmenu_bind_idx] = VK_INVALID;
            } else if (val == 0) {
                opt->uval[optmenu_bind_idx] = VK_INVALID;
                optmenu_binding = 1;
                controller_get_raw_key(); // clear the last key, which is probably A
            } else {
                optmenu_bind_idx = wrap_add(optmenu_bind_idx, val, 0, MAX_BINDS - 1);
            }
            break;

        default: break;
    }
}

static inline s16 get_hudstr_centered_x(const s16 sx, const u8 *str) {
    const u8 *chr = str;
    s16 len = 0;
    while (*chr != GLOBAR_CHAR_TERMINATOR) ++chr, ++len;
    return sx - len * 6; // stride is 12
}

//Options menu
void optmenu_draw(void) {
    s16 scroll;
    s16 scrollpos;

    u8* label = get_key_string(currentMenu->label);

    const s16 labelX = get_hudstr_centered_x(160, label);
    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
    print_hud_lut_string(HUD_LUT_GLOBAL, labelX, 40, label);
    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_end);

    if (currentMenu->numOpts > 4) {
        optmenu_draw_box(272, 90, 280, 208, 0x80, 0x80, 0x80);
        scrollpos = 54 * ((f32)currentMenu->scroll / (currentMenu->numOpts-4));
        optmenu_draw_box(272, 90+scrollpos, 280, 154+scrollpos, 0xFF, 0xFF, 0xFF);
    }

    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    gDPSetScissor(gDisplayListHead++, G_SC_NON_INTERLACE, 0, 80, SCREEN_WIDTH, SCREEN_HEIGHT);
    for (u8 i = 0; i < currentMenu->numOpts; i++) {
        scroll = 140 - 32 * i + currentMenu->scroll * 32;
        // FIXME: just start from the first visible option bruh
        if (scroll <= 140 && scroll > 32)
            optmenu_draw_opt(&currentMenu->opts[i], 160, scroll, (currentMenu->select == i));
    }

    gDPSetScissor(gDisplayListHead++, G_SC_NON_INTERLACE, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
    // gSPDisplayList(gDisplayListHead++, dl_rgba16_text_begin);
    // print_hud_lut_string(HUD_LUT_GLOBAL, 80, 90 + (32 * (currentMenu->select - currentMenu->scroll)), get_key_string(menuStr[0]));
    // print_hud_lut_string(HUD_LUT_GLOBAL, 224, 90 + (32 * (currentMenu->select - currentMenu->scroll)), get_key_string(menuStr[0]));
    // gSPDisplayList(gDisplayListHead++, dl_rgba16_text_end);
}

//This has been separated for interesting reasons. Don't question it.
void optmenu_draw_prompt(void) {
    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    optmenu_draw_text(264, 212, get_key_string(menuStr[1 + optmenu_open]), 0);
    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);
}

void optmenu_toggle(void) {
    if (optmenu_open == 0) {
        #ifndef nosound
        play_sound(SOUND_MENU_CHANGE_SELECT, gDefaultSoundArgs);
        #endif

        // HACK: hide the last option in main if cheats are disabled
        menuMain.numOpts = sizeof(optsMain) / sizeof(optsMain[0]);
        if (!Cheats.EnableCheats) {
            menuMain.numOpts--;
            if (menuMain.select >= menuMain.numOpts) {
                menuMain.select = 0; // don't bother
                menuMain.scroll = 0;
            }
        }

        currentMenu = &menuMain;
        optmenu_open = 1;
        
        /* Resets l_counter to 0 every time the options menu is open */
        l_counter = 0;
    } else {
        #ifndef nosound
        r96_stop_jingle();
        r96_stop_music();
        play_sound(SOUND_MENU_MARIO_CASTLE_WARP2, gDefaultSoundArgs);
        #endif
        optmenu_open = 0;
        newcam_init_settings(); // load bettercam settings from config vars
        controller_reconfigure(); // rebind using new config values
        configfile_save(configfile_name());
    }
}

void optmenu_check_buttons(void) {
    if (optmenu_binding) {
        u32 key = controller_get_raw_key();
        if (key != VK_INVALID) {
            #ifndef nosound
            play_sound(SOUND_MENU_CHANGE_SELECT, gDefaultSoundArgs);
            #endif
            currentMenu->opts[currentMenu->select].uval[optmenu_bind_idx] = key;
            optmenu_binding = 0;
            optmenu_option_timer = 12;
        }
        return;
    }

    if (gPlayer1Controller->buttonPressed & R_TRIG)
        optmenu_toggle();
    
    /* Enables cheats if the user press the L trigger 3 times while in the options menu. Also plays a sound. */
    
    if ((gPlayer1Controller->buttonPressed & L_TRIG) && !Cheats.EnableCheats) {
        if (l_counter == 2) {
                Cheats.EnableCheats = true;
                play_sound(SOUND_MENU_STAR_SOUND, gDefaultSoundArgs);
                l_counter = 0;
        } else {
            l_counter++;
        }
    }
    
    if (!optmenu_open) return;

    u8 allowInput = 0;

    optmenu_option_timer--;
    if (optmenu_option_timer <= 0) {
        if (optmenu_hold_count == 0) {
            optmenu_hold_count++;
            optmenu_option_timer = 10;
        } else {
            optmenu_option_timer = 3;
        }
        allowInput = 1;
    }

    if (ABS(gPlayer1Controller->stickY) > 60) {
        if (allowInput) {
            #ifndef nosound
            play_sound(SOUND_MENU_CHANGE_SELECT, gDefaultSoundArgs);
            #endif

            if (gPlayer1Controller->stickY >= 60) {
                currentMenu->select--;
                if (currentMenu->select < 0)
                    currentMenu->select = currentMenu->numOpts-1;
            } else {
                currentMenu->select++;
                if (currentMenu->select >= currentMenu->numOpts)
                    currentMenu->select = 0;
            }

            if (currentMenu->select < currentMenu->scroll)
                currentMenu->scroll = currentMenu->select;
            else if (currentMenu->select > currentMenu->scroll + 3)
                currentMenu->scroll = currentMenu->select - 3;
        }
    } else if (ABS(gPlayer1Controller->stickX) > 60) {
        if (allowInput) {
            #ifndef nosound
            play_sound(SOUND_MENU_CHANGE_SELECT, gDefaultSoundArgs);
            #endif
            if (gPlayer1Controller->stickX >= 60)
                optmenu_opt_change(&currentMenu->opts[currentMenu->select], 1);
            else
                optmenu_opt_change(&currentMenu->opts[currentMenu->select], -1);
        }
    } else if (gPlayer1Controller->buttonPressed & A_BUTTON) {
        if (allowInput) {
            #ifndef nosound
            play_sound(SOUND_MENU_CHANGE_SELECT, gDefaultSoundArgs);
            #endif
            optmenu_opt_change(&currentMenu->opts[currentMenu->select], 0);
        }
    } else if (gPlayer1Controller->buttonPressed & B_BUTTON) {
        if (allowInput) {
            if (currentMenu->prev) {
                #ifndef nosound
                play_sound(SOUND_MENU_CHANGE_SELECT, gDefaultSoundArgs);
                #endif
                currentMenu = currentMenu->prev;
            } else {
                // can't go back, exit the menu altogether
                optmenu_toggle();
            }
        }
    } else if (gPlayer1Controller->buttonPressed & Z_TRIG) {
        // HACK: clear binds with Z
        if (allowInput && currentMenu->opts[currentMenu->select].type == OPT_BIND)
            optmenu_opt_change(&currentMenu->opts[currentMenu->select], 0xFF);
    } else if (gPlayer1Controller->buttonPressed & START_BUTTON) {
        if (allowInput) optmenu_toggle();
    } else {
        optmenu_hold_count = 0;
        optmenu_option_timer = 0;
    }
}

#endif // EXT_OPTIONS_MENU
#endif
