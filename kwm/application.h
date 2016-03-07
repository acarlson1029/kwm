#ifndef APPLICATION_H
#define APPLICATION_H

#include "types.h"


void AllowRoleForApplication(std::string Application, std::string Role);
void CaptureApplication(window_info *Window);
bool IsAppSpecificWindowRole(window_info *Window, CFTypeRef Role, CFTypeRef SubRole);
bool IsApplicationCapturedByScreen(window_info *Window);
bool IsApplicationFloating(window_info *Window);

#endif

