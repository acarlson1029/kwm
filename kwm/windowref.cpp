#include "windowref.h"
#include "helpers.h"      // GetUTF8String()
#include "application.h"  // IsApplicationFloating
#include "window.h"       // IsFocusedWindowFloating, WindowsAreEqual, IsWindowFloating
#include "space.h"        // IsActiveSpaceFloating, GetActiveSpaceOfScreen, DoesSpaceExistInMapOfScreen
#include "notifications.h"// CreateApplicationNotifications
#include "border.h"       // UpdateBorder
#include "tree.h"         // GetNodeFromWindowID()
#include "windowtree.h"   // RemoveWindowFromBSPTree, RemoveWindowFromMonocleTree
#include "display.h"      // GetDisplayOfWindow()

extern kwm_focus KWMFocus;
extern kwm_cache KWMCache;
extern kwm_mode KWMMode;
extern kwm_screen KWMScreen;
extern kwm_toggles KWMToggles;
extern kwm_tiling KWMTiling;

std::string GetWindowTitle(AXUIElementRef WindowRef)
{
    CFStringRef Temp;
    std::string WindowTitle;
    AXUIElementCopyAttributeValue(WindowRef, kAXTitleAttribute, (CFTypeRef*)&Temp);

    if(Temp)
    {
        WindowTitle = GetUTF8String(Temp);
        if(WindowTitle.empty())
            WindowTitle = CFStringGetCStringPtr(Temp, kCFStringEncodingMacRoman);

        CFRelease(Temp);
    }

    return WindowTitle;
}

CGSize GetWindowSize(AXUIElementRef WindowRef)
{
    AXValueRef Temp;
    CGSize WindowSize;

    AXUIElementCopyAttributeValue(WindowRef, kAXSizeAttribute, (CFTypeRef*)&Temp);
    if(Temp)
    {
        AXValueGetValue(Temp, kAXValueCGSizeType, &WindowSize);
        CFRelease(Temp);
    }

    return WindowSize;
}

CGPoint GetWindowPos(AXUIElementRef WindowRef)
{
    AXValueRef Temp;
    CGPoint WindowPos;

    AXUIElementCopyAttributeValue(WindowRef, kAXPositionAttribute, (CFTypeRef*)&Temp);
    if(Temp)
    {
        AXValueGetValue(Temp, kAXValueCGPointType, &WindowPos);
        CFRelease(Temp);
    }

    return WindowPos;
}

bool GetWindowRefFromCache(window_info *Window, AXUIElementRef *WindowRef)
{
    std::vector<AXUIElementRef> Elements;
    bool IsCached = IsApplicationInCache(Window->PID, &Elements);

    if(IsCached)
    {
        for(std::size_t ElementIndex = 0; ElementIndex < Elements.size(); ++ElementIndex)
        {
            int AppWindowRefWID = -1;
            _AXUIElementGetWindow(Elements[ElementIndex], &AppWindowRefWID);
            if(AppWindowRefWID == Window->WID)
            {
                *WindowRef = Elements[ElementIndex];
                return true;
            }
        }
    }

    if(!IsCached)
        KWMCache.WindowRefs[Window->PID] = std::vector<AXUIElementRef>();

    return false;
}

bool GetWindowRef(window_info *Window, AXUIElementRef *WindowRef)
{
    if(Window->Owner == "Dock")
        return false;

    if(GetWindowRefFromCache(Window, WindowRef))
        return true;

    AXUIElementRef App = AXUIElementCreateApplication(Window->PID);
    if(!App)
    {
        DEBUG("GetWindowRef() Failed to get App for: " << Window->Name)
        return false;
    }

    CFArrayRef AppWindowLst;
    AXUIElementCopyAttributeValue(App, kAXWindowsAttribute, (CFTypeRef*)&AppWindowLst);
    if(!AppWindowLst)
    {
        DEBUG("GetWindowRef() Could not get AppWindowLst")
        return false;
    }

    bool Found = false;
    FreeWindowRefCache(Window->PID);
    CFIndex AppWindowCount = CFArrayGetCount(AppWindowLst);
    for(CFIndex WindowIndex = 0; WindowIndex < AppWindowCount; ++WindowIndex)
    {
        AXUIElementRef AppWindowRef = (AXUIElementRef)CFArrayGetValueAtIndex(AppWindowLst, WindowIndex);
        if(AppWindowRef)
        {
            KWMCache.WindowRefs[Window->PID].push_back(AppWindowRef);
            if(!Found)
            {
                int AppWindowRefWID = -1;
                _AXUIElementGetWindow(AppWindowRef, &AppWindowRefWID);
                if(AppWindowRefWID == Window->WID)
                {
                    *WindowRef = AppWindowRef;
                    Found = true;
                }
            }
        }
    }

    CFRelease(App);
    return Found;
}

bool GetWindowFocusedByOSX(int *WindowWID)
{
    static AXUIElementRef SystemWideElement = AXUIElementCreateSystemWide();

    AXUIElementRef App;
    AXUIElementCopyAttributeValue(SystemWideElement, kAXFocusedApplicationAttribute, (CFTypeRef*)&App);
    if(App)
    {
        AXUIElementRef WindowRef;
        AXError Error = AXUIElementCopyAttributeValue(App, kAXFocusedWindowAttribute, (CFTypeRef*)&WindowRef);
        CFRelease(App);

        if (Error == kAXErrorSuccess)
        {
            _AXUIElementGetWindow(WindowRef, WindowWID);
            CFRelease(WindowRef);
            return true;
        }
    }

    return false;
}

void SetWindowRefFocus(AXUIElementRef WindowRef, window_info *Window, bool Notification)
{
    int OldProcessPID = KWMFocus.Window ? KWMFocus.Window->PID : -1;

    ProcessSerialNumber NewPSN;
    GetProcessForPID(Window->PID, &NewPSN);

    KWMFocus.PSN = NewPSN;
    KWMFocus.Cache = *Window;
    KWMFocus.Window = &KWMFocus.Cache;

    AXUIElementSetAttributeValue(WindowRef, kAXMainAttribute, kCFBooleanTrue);
    AXUIElementSetAttributeValue(WindowRef, kAXFocusedAttribute, kCFBooleanTrue);
    AXUIElementPerformAction(WindowRef, kAXRaiseAction);

    if(KWMMode.Focus != FocusModeAutofocus && KWMMode.Focus != FocusModeStandby)
        SetFrontProcessWithOptions(&KWMFocus.PSN, kSetFrontProcessFrontWindowOnly);

    if(!Notification &&
       KWMScreen.Current &&
       !IsActiveSpaceFloating())
    {
       if(OldProcessPID != KWMFocus.Window->PID ||
          !KWMFocus.Observer)
            CreateApplicationNotifications();

        if(Window->Layer == 0)
            UpdateBorder("focused");
    }

    if(KWMToggles.EnableTilingMode)
    {
        space_info *Space = GetActiveSpaceOfScreen(KWMScreen.Current);
        Space->FocusedNode = GetNodeFromWindowID(Space->RootNode, Window->WID, Space->Mode);
    }

    DEBUG("SetWindowRefFocus() Focused Window: " << KWMFocus.Window->Name << " " << KWMFocus.Window->X << "," << KWMFocus.Window->Y)
    if(KWMMode.Focus != FocusModeDisabled &&
       KWMMode.Focus != FocusModeAutofocus &&
       KWMToggles.StandbyOnFloat)
        KWMMode.Focus = IsFocusedWindowFloating() ? FocusModeStandby : FocusModeAutoraise;
}

void SetWindowFocus(window_info *Window)
{
    AXUIElementRef WindowRef;
    if(GetWindowRef(Window, &WindowRef))
        SetWindowRefFocus(WindowRef, Window, false);
}

void CenterWindowInsideNodeContainer(AXUIElementRef WindowRef, int *Xptr, int *Yptr, int *Wptr, int *Hptr)
{
    CGPoint WindowOrigin = GetWindowPos(WindowRef);
    CGSize WindowOGSize = GetWindowSize(WindowRef);

    int &X = *Xptr, &Y = *Yptr, &Width = *Wptr, &Height = *Hptr;
    int XDiff = (X + Width) - (WindowOrigin.x + WindowOGSize.width);
    int YDiff = (Y + Height) - (WindowOrigin.y + WindowOGSize.height);

    if(XDiff > 0 || YDiff > 0)
    {
        double XOff = XDiff / 2.0f;
        X += XOff > 0 ? XOff : 0;
        Width -= XOff > 0 ? XOff : 0;

        double YOff = YDiff / 2.0f;
        Y += YOff > 0 ? YOff : 0;
        Height -= YOff > 0 ? YOff : 0;

        CGPoint WindowPos = CGPointMake(X, Y);
        CFTypeRef NewWindowPos = (CFTypeRef)AXValueCreate(kAXValueCGPointType, (const void*)&WindowPos);

        CGSize WindowSize = CGSizeMake(Width, Height);
        CFTypeRef NewWindowSize = (CFTypeRef)AXValueCreate(kAXValueCGSizeType, (void*)&WindowSize);

        if(NewWindowPos)
        {
            AXUIElementSetAttributeValue(WindowRef, kAXPositionAttribute, NewWindowPos);
            CFRelease(NewWindowPos);
        }

        if(NewWindowSize)
        {
            AXUIElementSetAttributeValue(WindowRef, kAXSizeAttribute, NewWindowSize);
            CFRelease(NewWindowSize);
        }
    }
}

void SetWindowDimensions(AXUIElementRef WindowRef, window_info *Window, int X, int Y, int Width, int Height)
{
    CGPoint WindowPos = CGPointMake(X, Y);
    CFTypeRef NewWindowPos = (CFTypeRef)AXValueCreate(kAXValueCGPointType, (const void*)&WindowPos);

    CGSize WindowSize = CGSizeMake(Width, Height);
    CFTypeRef NewWindowSize = (CFTypeRef)AXValueCreate(kAXValueCGSizeType, (void*)&WindowSize);

    if(!NewWindowPos || !NewWindowSize)
        return;

    Assert(WindowRef, "SetWindowDimensions() WindowRef")
    Assert(Window, "SetWindowDimensions() Window")

    DEBUG("SetWindowDimensions()")
    bool UpdateWindowInfo = true;
    if(KWMTiling.FloatNonResizable)
    {
        if(IsWindowNonResizable(WindowRef, Window, NewWindowPos, NewWindowSize))
            UpdateWindowInfo = false;
        else
            CenterWindowInsideNodeContainer(WindowRef, &X, &Y, &Width, &Height);

    }
    else
    {
        AXUIElementSetAttributeValue(WindowRef, kAXPositionAttribute, NewWindowPos);
        AXUIElementSetAttributeValue(WindowRef, kAXSizeAttribute, NewWindowSize);
        CenterWindowInsideNodeContainer(WindowRef, &X, &Y, &Width, &Height);
    }

    if(UpdateWindowInfo)
    {
        Window->X = X;
        Window->Y = Y;
        Window->Width = Width;
        Window->Height = Height;
        UpdateBorder("focused");
    }

    CFRelease(NewWindowPos);
    CFRelease(NewWindowSize);
}

void ResizeWindowToContainerSize(window_info *Window, node_container *Container)
{
    if(Window)
    {
        AXUIElementRef WindowRef;
        if(GetWindowRef(Window, &WindowRef))
        {
            SetWindowDimensions(WindowRef, Window,
                        Container->X, Container->Y,
                        Container->Width, Container->Height);

            if(WindowsAreEqual(Window, KWMFocus.Window))
                KWMFocus.Cache = *Window;
        }
    }
}

void CenterWindow(screen_info *Screen, window_info *Window)
{
    AXUIElementRef WindowRef;
    if(GetWindowRef(Window, &WindowRef))
    {
        int NewX = Screen->X + Screen->Width / 4;
        int NewY = Screen->Y + Screen->Height / 4;
        int NewWidth = Screen->Width / 2;
        int NewHeight = Screen->Height / 2;
        SetWindowDimensions(WindowRef, Window, NewX, NewY, NewWidth, NewHeight);
    }
}

void MoveFloatingWindow(int X, int Y)
{
    if(!KWMFocus.Window ||
       (!IsWindowFloating(KWMFocus.Window->WID, NULL) &&
       !IsApplicationFloating(KWMFocus.Window)))
        return;

    AXUIElementRef WindowRef;
    if(GetWindowRef(KWMFocus.Window, &WindowRef))
    {
        CGPoint WindowPos = GetWindowPos(WindowRef);
        WindowPos.x += X;
        WindowPos.y += Y;

        CFTypeRef NewWindowPos = (CFTypeRef)AXValueCreate(kAXValueCGPointType, (const void*)&WindowPos);
        if(NewWindowPos)
        {
            AXUIElementSetAttributeValue(WindowRef, kAXPositionAttribute, NewWindowPos);
            CFRelease(NewWindowPos);
        }
    }
}

void MoveCursorToCenterOfWindow(window_info *Window)
{
    Assert(Window, "MoveCursorToCenterOfWindow()")
    AXUIElementRef WindowRef;
    if(GetWindowRef(Window, &WindowRef))
    {
        CGPoint WindowPos = GetWindowPos(WindowRef);
        CGSize WindowSize = GetWindowSize(WindowRef);
        CGWarpMouseCursorPosition(CGPointMake(WindowPos.x + WindowSize.width / 2, WindowPos.y + WindowSize.height / 2));
    }
}

void MoveCursorToCenterOfFocusedWindow()
{
    if(KWMToggles.UseMouseFollowsFocus && KWMFocus.Window)
        MoveCursorToCenterOfWindow(KWMFocus.Window);
}

bool IsWindowNonResizable(AXUIElementRef WindowRef, window_info *Window, CFTypeRef NewWindowPos, CFTypeRef NewWindowSize)
{
    Assert(WindowRef, "IsWindowNonResizable() WindowRef")
    Assert(Window, "IsWindowNonResizable() Window")

    AXError PosError = kAXErrorFailure;
    AXError SizeError = AXUIElementSetAttributeValue(WindowRef, kAXSizeAttribute, NewWindowSize);
    if(SizeError == kAXErrorSuccess)
    {
        PosError = AXUIElementSetAttributeValue(WindowRef, kAXPositionAttribute, NewWindowPos);
        // Why are we setting the size again?
        SizeError = AXUIElementSetAttributeValue(WindowRef, kAXSizeAttribute, NewWindowSize);
    }

    if(PosError != kAXErrorSuccess || SizeError != kAXErrorSuccess)
    {
        // Why are we doing any of this in the function IsWindowNonResizable ?
        KWMTiling.FloatingWindowLst.push_back(Window->WID);
        screen_info *Screen = GetDisplayOfWindow(Window);
        if(DoesSpaceExistInMapOfScreen(Screen))
        {
            // TODO can just have one RemoveWindowFromTree(..., Space->Mode)
            space_info *Space = GetActiveSpaceOfScreen(Screen);
            if(Space->Mode == SpaceModeBSP)
                RemoveWindowFromBSPTree(Screen, Window->WID, false);
            else if(Space->Mode == SpaceModeMonocle)
                RemoveWindowFromMonocleTree(Screen, Window->WID);
        }

        return true;
    }

    return false;
}

void FreeWindowRefCache(int PID)
{
    std::map<int, std::vector<AXUIElementRef> >::iterator It = KWMCache.WindowRefs.find(PID);

    if(It != KWMCache.WindowRefs.end())
    {
        int NumElements = KWMCache.WindowRefs[PID].size();
        for(int RefIndex = 0; RefIndex < NumElements; ++RefIndex)
            CFRelease(KWMCache.WindowRefs[PID][RefIndex]);

        KWMCache.WindowRefs[PID].clear();
    }
}

