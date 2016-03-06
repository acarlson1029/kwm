#include "window.h"
#include "display.h" // GetDisplayOfWindow
#include "space.h" // IsSpaceTransitionInProgress, IsActiveSpaceManaged
#include "border.h" // ClearBorder, UpdateBorder
#include "application.h" // IsAppSpecificWindowRole, CaptureApplication, IsApplicationFloating
#include "windowref.h" // GetWindowFocusedByOSX, SetWindowFocus
#include "helpers.h"

#include <cmath>

extern kwm_screen KWMScreen;
extern kwm_focus KWMFocus;
extern kwm_toggles KWMToggles;
extern kwm_mode KWMMode;
extern kwm_tiling KWMTiling;
extern kwm_cache KWMCache;
extern kwm_path KWMPath;
extern kwm_border MarkedBorder;
extern kwm_border FocusedBorder;

bool WindowsAreEqual(window_info *Window, window_info *Match)
{
    if(!KWMScreen.ForceRefreshFocus && Window && Match)
    {
        if(Window->PID == Match->PID &&
           Window->WID == Match->WID &&
           Window->Layer == Match->Layer)
            return true;
    }

    return false;
}

std::vector<window_info> FilterWindowListAllDisplays()
{
    std::vector<window_info> FilteredWindowLst;
    std::vector<window_info>::iterator WinIt, end;
    for(WinIt = KWMTiling.FocusLst.begin(), end = KWMTiling.FocusLst.end(); WinIt != end; ++WinIt)
    {
        window_info *Window = &(*WinIt);

        if(Window->Layer == 0)
        {
            if((Window->Role && Window->SubRole) || GetWindowRole(Window))
            {
                bool RoleMatch = CFEqual(Window->Role, kAXWindowRole) && CFEqual(Window->SubRole, kAXStandardWindowSubrole);

                if(RoleMatch || IsAppSpecificWindowRole(Window, Window->Role, Window->SubRole))
                    FilteredWindowLst.push_back(*Window);
            }
        }
    }

    return FilteredWindowLst;
}

bool FilterWindowList(screen_info *Screen)
{
    std::vector<window_info> FilteredWindowLst;
    std::vector<window_info>::iterator WinIt, end;
    for(WinIt = KWMTiling.WindowLst.begin(), end = KWMTiling.WindowLst.end(); WinIt != end; ++WinIt)
    {
        window_info *Window = &(*WinIt);

        /* Note(koekeishiya):
         * Mission-Control mode is on and so we do not try to tile windows */
        if(Window->Owner == "Dock" && Window->Name == "")
        {
            ClearFocusedWindow();
            ClearMarkedWindow();
            return false;
        }

        CaptureApplication(Window);
        if((Window->Layer == 0) && (Screen == GetDisplayOfWindow(Window)))
        {
            if((Window->Role && Window->SubRole) || GetWindowRole(Window))
            {
                bool RoleMatch = CFEqual(Window->Role, kAXWindowRole) && CFEqual(Window->SubRole, kAXStandardWindowSubrole);

                if(RoleMatch || IsAppSpecificWindowRole(Window, Window->Role, Window->SubRole))
                    FilteredWindowLst.push_back(*Window);
            }
        }
    }

    KWMTiling.WindowLst = FilteredWindowLst;
    return true;
}

bool IsFocusedWindowFloating()
{
    return KWMFocus.Window && (IsWindowFloating(KWMFocus.Window->WID, NULL) || IsApplicationFloating(KWMFocus.Window));
}

bool IsWindowFloating(int WindowID, int *Index)
{
    for(std::size_t WindowIndex = 0; WindowIndex < KWMTiling.FloatingWindowLst.size(); ++WindowIndex)
    {
        if(WindowID == KWMTiling.FloatingWindowLst[WindowIndex])
        {
            if(Index)
                *Index = WindowIndex;

            return true;
        }
    }

    return false;
}

bool IsAnyWindowBelowCursor()
{
    CGPoint Cursor = GetCursorPos();
    for(std::size_t WindowIndex = 0; WindowIndex < KWMTiling.FocusLst.size(); ++WindowIndex)
    {
        window_info *Window = &KWMTiling.FocusLst[WindowIndex];
        if(Cursor.x >= Window->Boundary.X &&
           Cursor.x <= Window->Boundary.X + Window->Boundary.Width &&
           Cursor.y >= Window->Boundary.Y &&
           Cursor.y <= Window->Boundary.Y + Window->Boundary.Height)
            return true;
    }

    return false;
}

bool IsWindowBelowCursor(window_info *Window)
{
    Assert(Window, "IsWindowBelowCursor()")

    CGPoint Cursor = GetCursorPos();
    if(Cursor.x >= Window->Boundary.X &&
       Cursor.x <= Window->Boundary.X + Window->Boundary.Width &&
       Cursor.y >= Window->Boundary.Y &&
       Cursor.y <= Window->Boundary.Y + Window->Boundary.Height)
        return true;

    return false;
}

bool IsWindowOnActiveSpace(int WindowID)
{
    for(std::size_t WindowIndex = 0; WindowIndex < KWMTiling.WindowLst.size(); ++WindowIndex)
    {
        if(WindowID == KWMTiling.WindowLst[WindowIndex].WID)
        {
            DEBUG("IsWindowOnActiveSpace() window found")
            return true;
        }
    }

    DEBUG("IsWindowOnActiveSpace() window was not found")
    return false;
}

int GetFocusedWindowID()
{
    return (KWMFocus.Window && KWMFocus.Window->Layer == 0) ? KWMFocus.Window->WID : -1;
}

void ClearFocusedWindow()
{
    ClearBorder(&FocusedBorder);
    KWMFocus.Window = NULL;
    KWMFocus.Cache = KWMFocus.NULLWindowInfo;
}

bool FocusWindowOfOSX()
{
    int WindowID;
    if(GetWindowFocusedByOSX(&WindowID))
    {
        if(IsSpaceTransitionInProgress() ||
           !IsActiveSpaceManaged())
            return false;

        for(std::size_t WindowIndex = 0; WindowIndex < KWMTiling.WindowLst.size(); ++WindowIndex)
        {
            if(KWMTiling.WindowLst[WindowIndex].WID == WindowID)
            {
                SetWindowFocus(&KWMTiling.WindowLst[WindowIndex]);
                return true;
            }
        }
    }

    return false;
}

bool ShouldWindowGainFocus(window_info *Window)
{
    return Window->Layer == CGWindowLevelForKey(kCGNormalWindowLevelKey) ||
           Window->Layer == CGWindowLevelForKey(kCGFloatingWindowLevelKey) ||
           Window->Layer == CGWindowLevelForKey(kCGTornOffMenuWindowLevelKey) ||
           Window->Layer == CGWindowLevelForKey(kCGDockWindowLevelKey) ||
           Window->Layer == CGWindowLevelForKey(kCGMainMenuWindowLevelKey) ||
           Window->Layer == CGWindowLevelForKey(kCGMaximumWindowLevelKey) ||
           Window->Layer == CGWindowLevelForKey(kCGModalPanelWindowLevelKey) ||
           Window->Layer == CGWindowLevelForKey(kCGUtilityWindowLevelKey) ||
           Window->Layer == CGWindowLevelForKey(kCGOverlayWindowLevelKey) ||
           Window->Layer == CGWindowLevelForKey(kCGHelpWindowLevelKey) ||
           Window->Layer == CGWindowLevelForKey(kCGPopUpMenuWindowLevelKey);
}

void FocusWindowBelowCursor()
{
    if(IsSpaceTransitionInProgress() ||
       !IsActiveSpaceManaged())
           return;

    for(std::size_t WindowIndex = 0; WindowIndex < KWMTiling.FocusLst.size(); ++WindowIndex)
    {
        if(KWMTiling.FocusLst[WindowIndex].Owner == "kwm-overlay")
            continue;

        if(KWMTiling.FocusLst[WindowIndex].Owner == "Dock" &&
           KWMTiling.FocusLst[WindowIndex].Boundary.X == 0 &&
           KWMTiling.FocusLst[WindowIndex].Boundary.Y == 0)
            continue;

        if(IsWindowBelowCursor(&KWMTiling.FocusLst[WindowIndex]) && ShouldWindowGainFocus(&KWMTiling.FocusLst[WindowIndex]))
        {
            if(WindowsAreEqual(KWMFocus.Window, &KWMTiling.FocusLst[WindowIndex]))
                KWMFocus.Cache = KWMTiling.FocusLst[WindowIndex];
            else
                SetWindowFocus(&KWMTiling.FocusLst[WindowIndex]);

            return;
        }
    }
}

void UpdateActiveWindowList(screen_info *Screen)
{
    static CGWindowListOption OsxWindowListOption = kCGWindowListOptionOnScreenOnly |
                                                    kCGWindowListExcludeDesktopElements;

    Screen->OldWindowListCount = KWMTiling.WindowLst.size();
    KWMTiling.WindowLst.clear();

    CFArrayRef OsxWindowLst = CGWindowListCopyWindowInfo(OsxWindowListOption, kCGNullWindowID);
    if(!OsxWindowLst)
        return;

    CFIndex OsxWindowCount = CFArrayGetCount(OsxWindowLst);
    for(CFIndex WindowIndex = 0; WindowIndex < OsxWindowCount; ++WindowIndex)
    {
        CFDictionaryRef Elem = (CFDictionaryRef)CFArrayGetValueAtIndex(OsxWindowLst, WindowIndex);
        KWMTiling.WindowLst.push_back(window_info());
        CFDictionaryApplyFunction(Elem, GetWindowInfo, NULL);
    }
    CFRelease(OsxWindowLst);
    KWMTiling.FocusLst = KWMTiling.WindowLst;
}

bool WindowIsInDirection(window_info *A, window_info *B, int Degrees, bool Wrap)
{
    if(Wrap)
    {
        if(Degrees == 0 || Degrees == 180)
            return A->Boundary.Y != B->Boundary.Y && fmax(A->Boundary.X, B->Boundary.X) < fmin(B->Boundary.X + B->Boundary.Width, A->Boundary.X + A->Boundary.Width);
        else if(Degrees == 90 || Degrees == 270)
            return A->Boundary.X != B->Boundary.X && fmax(A->Boundary.Y, B->Boundary.Y) < fmin(B->Boundary.Y + B->Boundary.Height, A->Boundary.Y + A->Boundary.Height);
    }
    else
    {
        if(Degrees == 0)
            return B->Boundary.Y + B->Boundary.Height < A->Boundary.Y;
        else if(Degrees == 90)
            return B->Boundary.X > A->Boundary.X + A->Boundary.Width;
        else if(Degrees == 180)
            return B->Boundary.Y > A->Boundary.Y + A->Boundary.Height;
        else if(Degrees == 270)
            return B->Boundary.X + B->Boundary.Width < A->Boundary.X;
    }

    return false;
}

void GetCenterOfWindow(window_info *Window, int *X, int *Y)
{
    *X = Window->Boundary.X + Window->Boundary.Width / 2;
    *Y = Window->Boundary.Y + Window->Boundary.Height / 2;
}

double GetWindowDistance(window_info *A, window_info *B)
{
    double Dist = INT_MAX;

    if(A && B)
    {
        int X1, Y1, X2, Y2;
        GetCenterOfWindow(A, &X1, &Y1);
        GetCenterOfWindow(B, &X2, &Y2);

        int ScoreX = X1 >= X2 - 15 && X1 <= X2 + 15 ? 1 : 11;
        int ScoreY = Y1 >= Y2 - 10 && Y1 <= Y2 + 10 ? 1 : 22;
        int Weight = ScoreX * ScoreY;
        Dist = std::sqrt(std::pow(X2-X1, 2) + std::pow(Y2-Y1, 2)) + Weight;
    }

    return Dist;
}

bool FindClosestWindow(int Degrees, window_info *Target, bool Wrap)
{
    *Target = KWMFocus.Cache;
    window_info *Match = KWMFocus.Window;
    std::vector<window_info> Windows = KWMTiling.WindowLst;

    int MatchX, MatchY;
    GetCenterOfWindow(Match, &MatchX, &MatchY);

    double MinDist = INT_MAX;
    for(int Index = 0; Index < Windows.size(); ++Index)
    {
        if(!WindowsAreEqual(Match, &Windows[Index]) &&
           WindowIsInDirection(Match, &Windows[Index], Degrees, Wrap))
        {
            window_info FocusWindow = Windows[Index];

            if(Wrap)
            {
                int WindowX, WindowY;
                GetCenterOfWindow(&Windows[Index], &WindowX, &WindowY);

                window_info WrappedWindow = Windows[Index];
                if(Degrees == 0 && MatchY < WindowY)
                    WrappedWindow.Boundary.Y -= KWMScreen.Current->Boundary.Height;
                else if(Degrees == 180 && MatchY > WindowY)
                    WrappedWindow.Boundary.Y += KWMScreen.Current->Boundary.Height;
                else if(Degrees == 90 && MatchX > WindowX)
                    WrappedWindow.Boundary.X += KWMScreen.Current->Boundary.Width;
                else if(Degrees == 270 && MatchX < WindowX)
                    WrappedWindow.Boundary.X -= KWMScreen.Current->Boundary.Width;

                FocusWindow = WrappedWindow;
            }

            double Dist = GetWindowDistance(Match, &FocusWindow);
            if(Dist < MinDist)
            {
                MinDist = Dist;
                *Target = Windows[Index];
            }
        }
    }

    return MinDist != INT_MAX;
}

void ClearMarkedWindow()
{
    KWMScreen.MarkedWindow = -1;
    ClearBorder(&MarkedBorder);
}

void MarkWindowContainer(window_info *Window)
{
    if(Window)
    {
        if(KWMScreen.MarkedWindow == Window->WID)
        {
            DEBUG("MarkWindowContainer() Unmarked " << Window->Name)
            ClearMarkedWindow();
        }
        else
        {
            DEBUG("MarkWindowContainer() Marked " << Window->Name)
            KWMScreen.MarkedWindow = Window->WID;
            UpdateBorder("marked");
        }
    }
}

void MarkFocusedWindowContainer()
{
    MarkWindowContainer(KWMFocus.Window);
}

CGPoint GetCursorPos()
{
    CGEventRef Event = CGEventCreate(NULL);
    CGPoint Cursor = CGEventGetLocation(Event);
    CFRelease(Event);

    return Cursor;
}

window_info *GetWindowByID(int WindowID)
{
    for(std::size_t WindowIndex = 0; WindowIndex < KWMTiling.FocusLst.size(); ++WindowIndex)
    {
        if(KWMTiling.FocusLst[WindowIndex].WID == WindowID)
            return &KWMTiling.FocusLst[WindowIndex];
    }

    return NULL;
}

void GetWindowInfo(const void *Key, const void *Value, void *Context)
{
    CFStringRef K = (CFStringRef)Key;
    std::string KeyStr = CFStringGetCStringPtr(K, kCFStringEncodingMacRoman);
    CFTypeID ID = CFGetTypeID(Value);
    if(ID == CFStringGetTypeID())
    {
        CFStringRef V = (CFStringRef)Value;
        std::string ValueStr = GetUTF8String(V);
        if(ValueStr.empty() && CFStringGetCStringPtr(V, kCFStringEncodingMacRoman))
            ValueStr = CFStringGetCStringPtr(V, kCFStringEncodingMacRoman);

        if(KeyStr == "kCGWindowName")
            KWMTiling.WindowLst[KWMTiling.WindowLst.size()-1].Name = ValueStr;
        else if(KeyStr == "kCGWindowOwnerName")
            KWMTiling.WindowLst[KWMTiling.WindowLst.size()-1].Owner = ValueStr;
    }
    else if(ID == CFNumberGetTypeID())
    {
        int MyInt;
        CFNumberRef V = (CFNumberRef)Value;
        CFNumberGetValue(V, kCFNumberSInt64Type, &MyInt);
        if(KeyStr == "kCGWindowNumber")
            KWMTiling.WindowLst[KWMTiling.WindowLst.size()-1].WID = MyInt;
        else if(KeyStr == "kCGWindowOwnerPID")
            KWMTiling.WindowLst[KWMTiling.WindowLst.size()-1].PID = MyInt;
        else if(KeyStr == "kCGWindowLayer")
            KWMTiling.WindowLst[KWMTiling.WindowLst.size()-1].Layer = MyInt;
        else if(KeyStr == "X")
            KWMTiling.WindowLst[KWMTiling.WindowLst.size()-1].Boundary.X = MyInt;
        else if(KeyStr == "Y")
            KWMTiling.WindowLst[KWMTiling.WindowLst.size()-1].Boundary.Y = MyInt;
        else if(KeyStr == "Width")
            KWMTiling.WindowLst[KWMTiling.WindowLst.size()-1].Boundary.Width = MyInt;
        else if(KeyStr == "Height")
            KWMTiling.WindowLst[KWMTiling.WindowLst.size()-1].Boundary.Height = MyInt;
    }
    else if(ID == CFDictionaryGetTypeID())
    {
        CFDictionaryRef Elem = (CFDictionaryRef)Value;
        CFDictionaryApplyFunction(Elem, GetWindowInfo, NULL);
    }
}

void GetWindowRole(window_info *Window)
{
    Result = false;
	AXUIElementRef WindowRef;

	if(GetWindowRef(Window, &WindowRef))
	{
		AXUIElementCopyAttributeValue(WindowRef, kAXRoleAttribute, (CFTypeRef *)&Window->Role);
		AXUIElementCopyAttributeValue(WindowRef, kAXSubroleAttribute, (CFTypeRef *)&Window->SubRole);
		Result = true;
	} 

	return Result;
}
