#ifndef TREECOMMANDS_H
#define TREECOMMANDS_H

/* Functions that map Spaces ~> Elements/Windows (through Trees) */
/* TODO - Replace WindowID references with functions for Elements/Windows wholesale */

/* TODO - define an option struct for arguments:
 * enum window_arg 
 * {
 *     id,
 *     first,
 *     last,
 *     prev,
 *     next,
 *     focused,
 *     marked,
 *     current,
 *     alternate,
 *     left,
 *     right,
 *     above,
 *     below,
 *     position,
 * }

/* Finding Windows */
window_info *GetWindowByIDOnSpace(const space_info &Space, const &WindowID);
window_info *GetFirstWindowOnSpace(const space_info &Space);
window_info *GetLastWindowOnSpace(const space_info &Space);
window_info *GetPreviousWindowOnSpace(const space_info &Space);
window_info *GetNextWindowOnSpace(const space_info &Space);
window_info *GetRelativeWindowOnSpace(const space_info &Space, const int &Degrees); // TODO replace Degrees with an enum
// The "GetWindowUnderCursor" will mux to find the right Display + current Space, then pass the coordinates to this function
window_info *GetWindowAtPositionOnSpace(const space_info &Space, const double &X, const double &Y); // TODO use CGPoint?
window_info *GetWindowOnSpaceFromArg(const space_info &Space, const window_mux &Arg);

// TODO
// GetElementByIDOnSpace()
// DrawElementIDsOnSpace()
// GetNextWindowInElement()
// GetPreviousWindowInElement()
// plus args
//

// TODO: Should probably refactor all functions to take a window_info* argument
// Any calling functions will need to get the window via the mux, or whatever other way preferred works too.

/* Set Window Focus */
void SetWindowFocus(space_info *Space, const window_info &Window);
void FocusWindowOnSpace(space_info *Space, const window_mux &Arg);
void FocusWindowOnSpace(space_info *Space, const int &WindowID);

/* Set Window Mark */
void SetWindowMark(space_info *Space, const window_info &Window);
void MarkWindowOnSpace(space_info *Space, const window_mux &Arg);
void MarkWindowOnSpace(space_info *Space, const int &WindowID);

/* Toggles Window Zoom */
void ToggleWindowZoom(space_info *Space, const window_mux &Arg, const zoom_type &ZoomType);
void ToggleWindowZoom(space_info *Space, const int &WindowID, const zoom_type &ZoomType);
void ToggleWindowZoom(space_info *Space, const window_mux &Arg, const bound_rect &Boundary);
void ToggleWindowZoom(space_info *Space, const int &WindowID, const bound_rect &Boundary);

/* Toggles Window Float */
void ToggleWindowFloat(space_info *Space, const window_mux &Arg);
void ToggleWindowFloat(space_info *Space, const int &WindowID);

#endif
