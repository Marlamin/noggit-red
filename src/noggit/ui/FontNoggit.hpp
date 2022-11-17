// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ui/FontAwesome.hpp>

#include <QtGui/QIcon>


namespace Noggit
{
  namespace Ui
  {
    struct FontNoggit
    {
      enum Icons
      {
        rmb = 0xf868,
        lmb = 0xf864,
        mmb = 0xf866,
        rmb_drag = 0xf869,
        lmb_drag = 0xf865,
        mmb_drag = 0xf867,
        drag = 0xf86A,
        alt = 0xf86B,
        ctrl = 0xf86C,
        shift = 0xf86D,
        a = 0xf86E,
        b = 0xf86F,
        c = 0xf870,
        d = 0xf871,
        e = 0xf872,
        f = 0xf873,
        g = 0xf874,
        h = 0xf875,
        i = 0xf876,
        j = 0xf877,
        k = 0xf878,
        l = 0xf879,
        m = 0xf87A,
        n = 0xf87B,
        o = 0xf87C,
        p = 0xf87D,
        q = 0xf87E,
        r = 0xf87F,
        s = 0xf880,
        t = 0xf881,
        u = 0xf882,
        v = 0xf883,
        w = 0xf884,
        x = 0xf885,
        y = 0xf886,
        z = 0xf887,
        space = 0xf888,
        page_up = 0xf889,
        page_down = 0xf88A,
        home = 0xf88B,
        num = 0xf88C,
        tab = 0xf88D,
        plus = 0xf88E,
        minus = 0xf88F,
        f1 = 0xf890,
        f2 = 0xf891,
        f3 = 0xf892,
        f4 = 0xf893,
        f5 = 0xf894,
        f6 = 0xf895,
        f7 = 0xf896,
        f8 = 0xf897,
        f9 = 0xf898,
        f10 = 0xf899,
        f11 = 0xf89A,
        f12 = 0xf89B,
        TOOL_RAISE_LOWER = 0xF89C,
        TOOL_FLATTEN_BLUR = 0xF89D,
        TOOL_TEXTURE_PAINT = 0xF89E,
        TOOL_HOLE_CUTTER = 0xF89F,
        TOOL_AREA_DESIGNATOR = 0xF8A0,
        TOOL_IMPASS_DESIGNATOR = 0xF8A1,
        TOOL_WATER_EDITOR = 0xF8A2,
        TOOL_VERTEX_PAINT = 0xF8A3,
        TOOL_OBJECT_EDITOR = 0xF8A4,
        TOOL_MINIMAP_EDITOR = 0xF8A5,
        TOOL_STAMP = 0xF8A6,
        SETTINGS = 0xf8a7,
        FAVORITE = 0xf8a8,
        VISIBILITY_WMO = 0xf8a9,
        VISIBILITY_WMO_DOODADS = 0xf8aa,
        VISIBILITY_DOODADS = 0xf8ab,
        VISIBILITY_WITH_BOX = 0xf8ac,
        VISIBILITY_UNUSED = 0xf8ad,
        VISIBILITY_TERRAIN = 0xf8ae,
        VISIBILITY_LINES = 0xf8af,
        VISIBILITY_WIREFRAME = 0xf8b0,
        VISIBILITY_CONTOURS = 0xf8b1,
        VISIBILITY_FOG = 0xf8b2,
        VISIBILITY_WATER = 0xf8b3,
        VISIBILITY_GROUNDEFFECTS= 0xf8b4,
        VISIBILITY_FLIGHT_BOUNDS = 0xf8b5,
        VISIBILITY_HIDDEN_MODELS = 0xf8b6,
        VISIBILITY_HOLE_LINES = 0xf8b7,
        VISIBILITY_ANIMATION = 0xf8b8,
        VISIBILITY_ANIMATION_2 = 0xf8b8,
        VISIBILITY_LIGHT = 0xf8ba,
        INFO = 0xf8bb,
        VISIBILITY_PARTICLES = 0xf8bc,
        TIME_NORMAL = 0xf8bd,
        TIME_PAUSE = 0xf8be,
        TIME_SPEED = 0xf8bf,
        CAMERA_TURN = 0xf8c0,
        CAMERA_SPEED_SLOWER = 0xf8c1,
        CAMERA_SPEED_FASTER = 0xf8c2,
        TEXTURE_PALETTE = 0xf8c3,
        TEXTURE_PALETTE_FAVORITE = 0xf8c4,
        MOUSE_INVERT = 0xf8c5,
        UI_TOGGLE = 0xf8c6,
        VIEW_MODE_2D = 0xf8c7,
        VIEW_AXIS = 0xf8c8,
        GIZMO_TRANSLATE = 0xf8c9,
        GIZMO_ROTATE = 0xf8ca,
        GIZMO_SCALE = 0xf8cb,
        GIZMO_VISIBILITY = 0xf8cc,
        GIZMO_GLOBAL = 0xf8cd,
        GIZMO_LOCAL = 0xf8ce,
        GIZMO_VISIBILITY_ALL = 0xf8cf,
        TOOL_LIGHT = 0xf8d0,
        VISIBILITY_VERTEX_PAINTER = 0xf8d1,
        VISIBILITY_CLIMB = 0xf8d2
      };
    };

    class FontNoggitButtonStyle : public QWidget
    {
    public:
        FontNoggitButtonStyle(QWidget* parent = nullptr) : QWidget(parent) { setAccessibleName("FontNoggitButtonStyle"); };
    };

    class FontNoggitIcon : public QIcon
    {
    public:
      FontNoggitIcon(FontNoggit::Icons const&);
    };

    class FontNoggitIconEngine : public FontAwesomeIconEngine    {
    public:
      FontNoggitIconEngine(const QString& text);

      [[nodiscard]]
      FontNoggitIconEngine* clone() const override;

      void paint(QPainter* painter, QRect const& rect, QIcon::Mode mode, QIcon::State state) override;


    private:
      const QString _text;

      static std::map<int, QFont> _fonts;

    };

  }
}
