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
        AXValueGetValue(Temp, kAXValueTypeCGSize, &WindowSize);
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
        AXValueGetValue(Temp, kAXValueTypeCGPoint, &WindowPos);
        CFRelease(Temp);
    }

    return WindowPos;
}

bool GetWindowRef(window_info *Window)
{
    if(Window->Owner == "Dock")
        return false;

    // Already have access to the Reference
    if(Window->Reference)
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
    CFIndex AppWindowCount = CFArrayGetCount(AppWindowLst);
    for(CFIndex WindowIndex = 0; WindowIndex < AppWindowCount; ++WindowIndex)
    {
        AXUIElementRef AppWindowRef = (AXUIElementRef)CFArrayGetValueAtIndex(AppWindowLst, WindowIndex);
        if(AppWindowRef)
        {
            if(!Found)
            {
                int AppWindowRefWID = -1;
                _AXUIElementGetWindow(AppWindowRef, &AppWindowRefWID);
                if(AppWindowRefWID == Window->WID)
                {
                    Window->Reference = AppWindowRef;
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

    DEBUG("SetWindowRefFocus() Focused Window: " << KWMFocus.Window->Name << " " << KWMFocus.Window->Boundary.X << "," << KWMFocus.Window->Boundary.Y)
    if(KWMMode.Focus != FocusModeDisabled &&
       KWMMode.Focus != FocusModeAutofocus &&
       KWMToggles.StandbyOnFloat)
        KWMMode.Focus = IsFocusedWindowFloating() ? FocusModeStandby : FocusModeAutoraise;
}

void SetWindowFocus(window_info *Window)
{
    if(GetWindowRef(Window))
        SetWindowRefFocus(Window->Reference, Window, false);
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
        CFTypeRef NewWindowPos = (CFTypeRef)AXValueCreate(kAXValueTypeCGPoint, (const void*)&WindowPos);

        CGSize WindowSize = CGSizeMake(Width, Height);
        CFTypeRef NewWindowSize = (CFTypeRef)AXValueCreate(kAXValueTypeCGSize, (void*)&WindowSize);

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
    CFTypeRef NewWindowPos = (CFTypeRef)AXValueCreate(kAXValueTypeCGPoint, (const void*)&WindowPos);

    CGSize WindowSize = CGSizeMake(Width, Height);
    CFTypeRef NewWindowSize = (CFTypeRef)AXValueCreate(kAXValueTypeCGSize, (void*)&WindowSize);

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
        Window->Boundary.X = X;
        Window->Boundary.Y = Y;
        Window->Boundary.Width = Width;
        Window->Boundary.Height = Height;
        UpdateBorder("focused");
    }

    CFRelease(NewWindowPos);
    CFRelease(NewWindowSize);
}

void ResizeWindowToContainerSize(window_info *Window, node_container *Container)
{
    if(Window)
    {
        if(GetWindowRef(Window))
        {
            SetWindowDimensions(Window->Reference, Window,
                        Container->Boundary.X, Container->Boundary.Y,
                        Container->Boundary.Width, Container->Boundary.Height);

            if(WindowsAreEqual(Window, KWMFocus.Window))
                KWMFocus.Cache = *Window;
        }
    }
}

void CenterWindow(screen_info *Screen, window_info *Window)
{
    if(GetWindowRef(Window))
    {
        int NewX = Screen->Boundary.X + Screen->Boundary.Width / 4;
        int NewY = Screen->Boundary.Y + Screen->Boundary.Height / 4;
        int NewWidth = Screen->Boundary.Width / 2;
        int NewHeight = Screen->Boundary.Height / 2;
        SetWindowDimensions(Window->Reference, Window, NewX, NewY, NewWidth, NewHeight);
    }
}

void MoveFloatingWindow(int X, int Y)
{
    if(!KWMFocus.Window ||
       (!IsWindowFloating(KWMFocus.Window->WID, NULL) &&
       !IsApplicationFloating(KWMFocus.Window)))
        return;

    if(GetWindowRef(KWMFocus.Window))
    {
        CGPoint WindowPos = GetWindowPos(KWMFocus.Window->Reference);
        WindowPos.x += X;
        WindowPos.y += Y;

        CFTypeRef NewWindowPos = (CFTypeRef)AXValueCreate(kAXValueTypeCGPoint, (const void*)&WindowPos);
        if(NewWindowPos)
        {
            AXUIElementSetAttributeValue(KWMFocus.Window->Reference, kAXPositionAttribute, NewWindowPos);
            CFRelease(NewWindowPos);
        }
    }
}

void MoveCursorToCenterOfWindow(AXUIElementRef WindowRef)
{
    CGPoint WindowPos = GetWindowPos(WindowRef);
    CGSize WindowSize = GetWindowSize(WindowRef);
    CGWarpMouseCursorPosition(CGPointMake(WindowPos.x + WindowSize.width / 2, WindowPos.y + WindowSize.height / 2));
}

void MoveCursorToCenterOfFocusedWindow()
{
    if(KWMToggles.UseMouseFollowsFocus && KWMFocus.Window)
    {
        if(GetWindowRef(KWMFocus.Window))
            MoveCursorToCenterOfWindow(KWMFocus.Window->Reference);
    }
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
