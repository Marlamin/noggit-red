#pragma once

#include <imgui.h>

class QWidget;
class QWindow;

namespace QtImGui {

#ifdef QT_WIDGETS_LIB
ImGuiContext* initialize(QWidget *window);
#endif

ImGuiContext* initialize(QWindow *window);
void newFrame();

}
