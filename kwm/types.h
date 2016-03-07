#ifndef TYPES_H
#define TYPES_H

#include <Carbon/Carbon.h>

#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <queue>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libproc.h>
#include <signal.h>

#include <pthread.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

struct hotkey;
struct modifiers;
struct container_offset;
struct color;

struct window_info;
struct window_role;
struct screen_info;
struct space_info;
struct node_container;
struct tree_node;

struct kwm_mach;
struct kwm_border;
struct kwm_hotkeys;
struct kwm_prefix;
struct kwm_toggles;
struct kwm_path;
struct kwm_focus;
struct kwm_screen;
struct kwm_tiling;
struct kwm_mode;
struct kwm_thread;

#ifdef DEBUG_BUILD
    #define DEBUG(x) std::cout << x << std::endl;
    #define Assert(Expression, Function) if(!(Expression)) \
                                         {\
                                            std::cout << "Assertion failed: " << Function << std::endl;\
                                            *(volatile int*)0 = 0;\
                                         }
#else
    #define DEBUG(x)
    #define Assert(Expression, Function)
#endif

#define BSP_WINDOW_EVENT_CALLBACK(name) void name(window_info *Window, int OpenWindows)
typedef BSP_WINDOW_EVENT_CALLBACK(OnBSPWindowCreate);
typedef BSP_WINDOW_EVENT_CALLBACK(OnBSPWindowDestroy);

typedef std::chrono::time_point<std::chrono::steady_clock> kwm_time_point;

#define CGSSpaceTypeUser 0
extern "C" int CGSGetActiveSpace(int cid);
extern "C" int CGSSpaceGetType(int cid, int sid);
extern "C" bool CGSManagedDisplayIsAnimating(const int cid, CFStringRef display);
extern "C" CFStringRef CGSCopyManagedDisplayForSpace(const int cid, int space);
extern "C" CFStringRef CGSCopyBestManagedDisplayForRect(const int cid, CGRect rect);
extern "C" CFArrayRef CGSCopyManagedDisplaySpaces(const int cid);

#define CGSDefaultConnection _CGSDefaultConnection()
extern "C" int _CGSDefaultConnection(void);

// Undocumented private OS X API call
// return is CGWindowID*, second arg
extern "C" AXError _AXUIElementGetWindow(AXUIElementRef, int *);

enum focus_option
{
    FocusModeAutofocus,
    FocusModeAutoraise,
    FocusModeStandby,
    FocusModeDisabled
};

enum cycle_focus_option
{
    CycleModeScreen,
    CycleModeAll,
    CycleModeDisabled
};

enum space_tiling_option
{
    SpaceModeBSP,
    SpaceModeMonocle,
    SpaceModeFloating,
    SpaceModeDefault
};

enum hotkey_state
{
    HotkeyStateNone,
    HotkeyStateInclude,
    HotkeyStateExclude
};

enum container_type
{
    ContainerRoot,
    ContainerLeft,
    ContainerRight,
    ContainerUpper,
    ContainerLower
};

enum split_mode
{
    SplitModeOptimal=-1,
    SplitModeUnset,
    SplitModeVertical,
    SplitModeHorizontal,
};

struct modifiers
{
    bool CmdKey;
    bool AltKey;
    bool CtrlKey;
    bool ShiftKey;
};

struct hotkey
{
    std::vector<std::string> List;
    bool IsSystemCommand;
    hotkey_state State;

    modifiers Mod;
    CGKeyCode Key;
    bool Prefixed;

    std::string Command;
};

struct bound_rect
{
    double X, Y;
    double Width, Height;
};

struct container_offset
{
    double VerticalGap, HorizontalGap;
};

struct padding_offset
{
    double PaddingLeft, PaddingRight, PaddingTop, PaddingBottom;
};

struct color
{
    double Red;
    double Green;
    double Blue;
    double Alpha;

    std::string Format;
};

struct node_container
{
    bound_rect Boundary;
    container_type Type;
    split_mode SplitMode;
    double SplitRatio;
};

struct tree_node
{
    int WindowID;
    node_container Container;
    tree_node *Parent;
    tree_node *LeftChild;
    tree_node *RightChild;
};

struct window_info
{
    std::string Name;
    std::string Owner;
    int PID, WID;
    int Layer;
    bound_rect Boundary;
    CFTypeRef Role, SubRole;
    AXUIElementRef Reference;
    bool Floating;
    bool FloatToggle;
    bool ZoomToggle;
};

struct window_role
{
    CFTypeRef Role;
    CFTypeRef SubRole;
};

struct space_info
{
    bool Initialized;
    bool Managed;

    bound_rect Boundary;
    container_offset Offset;
    space_tiling_option Mode;

    tree_node *FocusedNode; // TODO replace with element/window pointers
    tree_node *PreviousNode; // TODO replace with element/window pointers
    tree_node *RootNode;
};

struct screen_info
{
    CFStringRef Identifier;
    unsigned int ID;

    bound_rect Boundary;
    container_offset Offset;

    int ActiveSpace;
    int OldWindowListCount;
    bool ForceContainerUpdate;
    std::map<int, space_info> Space;
};

struct kwm_mach
{
    void *WorkspaceWatcher;
    CFRunLoopSourceRef RunLoopSource;
    CFMachPortRef EventTap;
    CGEventMask EventMask;
};

struct kwm_border
{
    bool Enabled;
    FILE *Handle;

    color Color;
    double Radius;
    int Width;
};

struct kwm_prefix
{
    kwm_time_point Time;
    hotkey Key;

    double Timeout;
    bool Enabled;
    bool Active;
    bool Global;
};

struct kwm_hotkeys
{
    std::vector<hotkey> List;
    kwm_prefix Prefix;
    modifiers SpacesKey;
};

struct kwm_toggles
{
    bool UseMouseFollowsFocus;
    bool EnableTilingMode;
    bool UseBuiltinHotkeys;
    bool StandbyOnFloat;
};

struct kwm_path
{
    std::string EnvHome;
    std::string FilePath;
    std::string ConfigFolder;
    std::string ConfigFile;
    std::string BSPLayouts;
};

struct kwm_focus
{
    AXObserverRef Observer;
    AXUIElementRef Application;

    ProcessSerialNumber PSN;
    window_info *Window;
    window_info Cache;
    window_info NULLWindowInfo;
};

struct kwm_screen
{
    screen_info *Current;
    bool ForceRefreshFocus;
    bool Transitioning;
    double SplitRatio;

    int MarkedWindow;
    split_mode SplitMode;
    int PrevSpace;

    padding_offset DefaultPadding;
    container_offset DefaultOffset;
    CGDirectDisplayID *Displays;
    unsigned int MaxCount;
    unsigned int ActiveCount;
};

struct kwm_tiling
{
    bool SpawnAsLeftChild;
    bool FloatNonResizable;
    std::map<unsigned int, screen_info> DisplayMap;
    std::map<unsigned int, space_tiling_option> DisplayMode;

    std::map<std::string, std::vector<CFTypeRef> > AllowedWindowRoles;
    std::map<std::string, int> CapturedAppLst;
    std::vector<std::string> FloatingAppLst;

    std::vector<window_info> FocusLst;
    std::vector<window_info> WindowLst;
    std::vector<int> FloatingWindowLst;
};

struct kwm_mode
{
    space_tiling_option Space;
    cycle_focus_option Cycle;
    focus_option Focus;
};

struct kwm_thread
{
    pthread_t WindowMonitor;
    pthread_t SystemCommand;
    pthread_t Daemon;
    pthread_mutex_t Lock;
};

struct kwm_callback
{
    OnBSPWindowCreate *WindowCreate;
    OnBSPWindowDestroy *WindowDestroy;
};

#endif
