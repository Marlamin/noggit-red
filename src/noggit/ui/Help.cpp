// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/Help.h>
#include <noggit/ui/font_noggit.hpp>

#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QTabWidget>

#include <initializer_list>

namespace noggit
{
  namespace ui
  {

    help::help(QWidget* parent)
      : widget (parent, Qt::Window)
    {
      setWindowTitle ("Help");
      setWindowIcon (QIcon (":/icon"));
      setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);

      QString header_style =
        "QLabel { \n "
        "  font-weight: bold; \n "
        "  margin-top: 8px; \n "
        "  margin-bottom: 4px; \n "
        "  margin-left: 150px; \n "
        "} \n ";


      auto layout (new QFormLayout (this));
      layout->setSizeConstraint(QLayout::SetFixedSize);

      auto tabs (new QTabWidget (this));

      auto base_widget (new QWidget (this));
      auto base_layout(new QGridLayout (base_widget));


      auto basic_controls_layout (new QFormLayout (this));
      base_layout->addLayout(basic_controls_layout, 0, 0);

      base_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      
      auto label = new QLabel("Basic controls:");
      label->setStyleSheet(header_style);
      basic_controls_layout->addRow(label);

      generate_hotkey_row({font_noggit::rmb_drag},                                            "\aRotate camera", basic_controls_layout);
      generate_hotkey_row({font_noggit::lmb},                                                 "\aSelect chunk or object", basic_controls_layout);
      generate_hotkey_row({font_noggit::i},                                                   "\aInvert mouse up and down", basic_controls_layout);
      generate_hotkey_row({font_noggit::q, font_noggit::e},                                   "\a,\aMove up and down", basic_controls_layout);
      generate_hotkey_row({font_noggit::w, font_noggit::a , font_noggit::s , font_noggit::d}, "\a\a\a\aMove left, right, forward, backwards", basic_controls_layout);
      generate_hotkey_row({font_noggit::home},                                                "\aMove position to the cursor", basic_controls_layout);
      generate_hotkey_row({font_noggit::m},                                                   "\aShow map", basic_controls_layout);
      generate_hotkey_row({font_noggit::u},                                                   "\a2D texture editor", basic_controls_layout);
      generate_hotkey_row({font_noggit::ctrl, font_noggit::f1},                               "\a+\aThis help", basic_controls_layout);
      generate_hotkey_row({font_noggit::shift, font_noggit::j},                               "\a+\aReload an adt under the camera", basic_controls_layout);
      generate_hotkey_row({font_noggit::shift, font_noggit::r},                               "\a+\aTurn camera 180 degrees", basic_controls_layout);
      generate_hotkey_row({font_noggit::shift},                                               "\a+     1, 2, 3 or 4        Set a predefined camera speed", basic_controls_layout);
      generate_hotkey_row({font_noggit::alt, font_noggit::f4},                                "\a+\aExit to main menu", basic_controls_layout);
      generate_hotkey_row({font_noggit::l},                                                   "\aToggle top view (hint: it's faster to use with graphic tablet stylus buttons)", basic_controls_layout);
      generate_hotkey_row({},                                                                 "", basic_controls_layout); // padding
       
      auto toggles_layout(new QFormLayout(this));
      base_layout->addLayout(toggles_layout, 0, 1);

      auto label_toggle = new QLabel("Toggles:");
      label_toggle->setStyleSheet(header_style);
      toggles_layout->addRow(label_toggle);

      generate_hotkey_row({font_noggit::f1},  "\aToggle M2s", toggles_layout);
      generate_hotkey_row({font_noggit::f2},  "\aToggle WMO doodads set", toggles_layout);
      generate_hotkey_row({font_noggit::f3},  "\aToggle ground", toggles_layout);
      generate_hotkey_row({font_noggit::f4},  "\aToggle water", toggles_layout);
      generate_hotkey_row({font_noggit::f6},  "\aToggle WMOs", toggles_layout);
      generate_hotkey_row({font_noggit::f7},  "\aToggle chunk (red) and ADT (green) lines", toggles_layout);
      generate_hotkey_row({font_noggit::f8},  "\aToggle detailed window", toggles_layout);
      generate_hotkey_row({font_noggit::f9},  "\aToggle map contour", toggles_layout);
      generate_hotkey_row({font_noggit::f10}, "\aToggle wireframe", toggles_layout);
      generate_hotkey_row({font_noggit::f11}, "\aToggle model animations", toggles_layout);
      generate_hotkey_row({font_noggit::f12}, "\aToggle fog", toggles_layout);
      generate_hotkey_row({},                 "1 - 9      Select the editing modes", toggles_layout);

      auto files_layout(new QFormLayout(this));
      base_layout->addLayout(files_layout, 1, 0);

      auto label_files = new QLabel("Files:");
      label_files->setStyleSheet(header_style);
      files_layout->addRow(label_files);

      generate_hotkey_row({font_noggit::f5},                                       "\aSave bookmark", files_layout);
      generate_hotkey_row({font_noggit::ctrl, font_noggit::s},                     "\a+\a Save all changed ADT tiles", files_layout);
      generate_hotkey_row({font_noggit::ctrl, font_noggit::shift, font_noggit::s}, "\a+\a+\aSave ADT tile at camera position", files_layout);
      generate_hotkey_row({font_noggit::ctrl, font_noggit::shift, font_noggit::a}, "\a+\a+\aSave all loaded ADT tiles", files_layout);
      generate_hotkey_row({font_noggit::g},                                        "\aSave port commands to ports.txt", files_layout);

      auto adjust_layout(new QFormLayout(this));
      base_layout->addLayout(adjust_layout, 1, 1);

      auto label_adjust = new QLabel("Adjust:");
      label_adjust->setStyleSheet(header_style);
      adjust_layout->addRow(label_adjust);

      generate_hotkey_row({font_noggit::o, font_noggit::p},                            "\aor\aSlower /  Faster movement", adjust_layout);
      generate_hotkey_row({font_noggit::b, font_noggit::n},                            "\aor\aSlower /  Faster time", adjust_layout);
      generate_hotkey_row({font_noggit::j},                                            "\aPause time", adjust_layout);
      generate_hotkey_row({font_noggit::shift, font_noggit::plus, font_noggit::minus}, "\a+\aor\aFog distance when no model is selected", adjust_layout);

      auto flag_widget (new QWidget (this));
      auto flag_layout (new QFormLayout (flag_widget));

      auto holes_label = new QLabel("Holes:");
      holes_label->setStyleSheet(header_style);
      flag_layout->addRow(holes_label);

      generate_hotkey_row({font_noggit::shift, font_noggit::lmb}, "\a+\aFog distance when no model is selected", flag_layout);
      generate_hotkey_row({font_noggit::ctrl, font_noggit::lmb},  "\a+\aAdd hole", flag_layout);
      generate_hotkey_row({font_noggit::t},                       "\aRemove all holes on ADT", flag_layout);
      generate_hotkey_row({font_noggit::alt, font_noggit::t},     "\a+\aRemove all ground on ADT", flag_layout);

      auto impass_flags_label = new QLabel("Impassible Flags:");
      impass_flags_label->setStyleSheet(header_style);
      flag_layout->addRow(impass_flags_label);

      generate_hotkey_row({ font_noggit::shift, font_noggit::lmb }, "\a+\aPaint flag", flag_layout);
      generate_hotkey_row({ font_noggit::ctrl, font_noggit::lmb },  "\a+\aClear flag", flag_layout);

      auto areaid_label = new QLabel("AreaID Flags:");
      areaid_label->setStyleSheet(header_style);
      flag_layout->addRow(areaid_label);

      generate_hotkey_row({ font_noggit::ctrl, font_noggit::lmb },  "\a+\aPick existing AreaID", flag_layout);
      generate_hotkey_row({ font_noggit::shift, font_noggit::lmb }, "\a+\aPaint selected AreaID", flag_layout);


      auto ground_widget (new QWidget (this));
      auto ground_layout (new QGridLayout (ground_widget));

      auto ground_column1_layout(new QFormLayout(this));
      ground_layout->addLayout(ground_column1_layout, 0, 0);

      auto ground_label = new QLabel("Edit ground:");
      ground_label->setStyleSheet(header_style);
      ground_column1_layout->addRow(ground_label);

      generate_hotkey_row({ font_noggit::shift, font_noggit::f1 },       "\a+\aToggle ground edit mode", ground_column1_layout);
      generate_hotkey_row({ font_noggit::alt, font_noggit::lmb_drag },   "\a+\aChange brush size", ground_column1_layout);
      generate_hotkey_row({ font_noggit::space, font_noggit::lmb_drag }, "\a+\aChange speed", ground_column1_layout);

      auto raise_label = new QLabel("Raise / Lower tool:");
      raise_label->setStyleSheet(header_style);
      ground_column1_layout->addRow(raise_label);

      generate_hotkey_row({ font_noggit::shift, font_noggit::lmb },    "\a+\aRaise terrain", ground_column1_layout);
      generate_hotkey_row({ font_noggit::ctrl, font_noggit::lmb },     "\a+\aLower terrain", ground_column1_layout);
      generate_hotkey_row({ font_noggit::y },                          "\aSwitch to next type", ground_column1_layout);
      generate_hotkey_row({ font_noggit::alt, font_noggit::rmb_drag }, "\a+\aChange inner radius", ground_column1_layout);

      auto raise_label_vm = new QLabel("Raise / Lower tool (vertex mode):");
      raise_label_vm->setStyleSheet(header_style);
      ground_column1_layout->addRow(raise_label_vm);

      generate_hotkey_row({ font_noggit::shift, font_noggit::lmb },      "\a+\aSelect vertices", ground_column1_layout);
      generate_hotkey_row({ font_noggit::ctrl, font_noggit::lmb },       "\a+\aDeselect vertices", ground_column1_layout);
      generate_hotkey_row({ font_noggit::c },                            "\aClear selection", ground_column1_layout);
      generate_hotkey_row({ font_noggit::space, font_noggit::f },        "\a+\aFlatten vertices", ground_column1_layout);
      generate_hotkey_row({ font_noggit::space, font_noggit::rmb_drag }, "\a+\aOrient vertices toward the mouse cursor", ground_column1_layout);
      generate_hotkey_row({ font_noggit::shift, font_noggit::rmb_drag }, "\a+\aChange vertices height", ground_column1_layout);
      generate_hotkey_row({ font_noggit::shift, font_noggit::mmb },      "\a+\aChange angle", ground_column1_layout);
      generate_hotkey_row({ font_noggit::alt, font_noggit::mmb },        "\a+\aChange orientation", ground_column1_layout);

      auto ground_column2_layout(new QFormLayout(this));
      ground_layout->addLayout(ground_column2_layout, 0, 1);

      auto flatten_label = new QLabel("Flatten / Blur tool:");
      flatten_label->setStyleSheet(header_style);
      ground_column2_layout->addRow(flatten_label);

      generate_hotkey_row({ font_noggit::shift, font_noggit::lmb }, "\a+\aFlatten terrain", ground_column2_layout);
      generate_hotkey_row({ font_noggit::ctrl, font_noggit::lmb },  "\a+\aBlur terrain", ground_column2_layout);
      generate_hotkey_row({ font_noggit::t },                       "\aToggle flatten angle", ground_column2_layout);
      generate_hotkey_row({ font_noggit::space, font_noggit::t },   "\a+\aToggle flatten type", ground_column2_layout);
      generate_hotkey_row({ font_noggit::shift, font_noggit::mmb }, "\a+\aChange angle", ground_column2_layout);
      generate_hotkey_row({ font_noggit::alt, font_noggit::mmb },   "\a+\aChange orientation", ground_column2_layout);
      generate_hotkey_row({ font_noggit::y },                       "\aSwitch to next type", ground_column2_layout);
      generate_hotkey_row({ font_noggit::f },                       "\aSet relative point", ground_column2_layout);
      generate_hotkey_row({ font_noggit::space, font_noggit::f },   "\a+\aToggle flatten relative mode", ground_column2_layout);
      generate_hotkey_row({ font_noggit::space, font_noggit::mmb }, "\a+\aChange height", ground_column2_layout);


      auto texture_widget (new QWidget (this));
      auto texture_layout (new QFormLayout (texture_widget));

      auto common_controls_label = new QLabel("Common controls:");
      common_controls_label->setStyleSheet(header_style);
      texture_layout->addRow(common_controls_label);

      generate_hotkey_row({ font_noggit::ctrl, font_noggit::lmb }, "\a+\aOpen texture picker for the chunk", texture_layout);

      auto paint_label = new QLabel("Paint:");
      paint_label->setStyleSheet(header_style);
      texture_layout->addRow(paint_label);

      generate_hotkey_row({ font_noggit::ctrl, font_noggit::shift, font_noggit::alt, font_noggit::lmb }, "\a+\a+\a+\aOpen texture picker for the chunk", texture_layout);
      generate_hotkey_row({ font_noggit::shift, font_noggit::lmb },                                      "\a+\aDraw texture or fills if chunk is empty", texture_layout);
      generate_hotkey_row({ font_noggit::alt, font_noggit::lmb_drag },                                   "\a+\aChange radius", texture_layout);
      generate_hotkey_row({ font_noggit::alt, font_noggit::rmb_drag },                                   "\a+\aChange hardness", texture_layout);
      generate_hotkey_row({ font_noggit::space, font_noggit::lmb_drag },                                 "\a+\aChange pressure", texture_layout);
      generate_hotkey_row({ font_noggit::space, font_noggit::mmb },                                      "\a+\aChange strength (gradient)", texture_layout);
      generate_hotkey_row({ font_noggit::space, font_noggit::r },                                        "\a+\aToggle min and max strength (gradient)", texture_layout);
      generate_hotkey_row({ font_noggit::t },                                                            "\aToggle spray brush", texture_layout);
      generate_hotkey_row({ font_noggit::alt, font_noggit::mmb },                                        "\a+\aChange spray radius", texture_layout);
      generate_hotkey_row({ font_noggit::shift, font_noggit::mmb },                                      "\a+\aChange spray pressure", texture_layout);

      auto swapper_label = new QLabel("Swap:");
      swapper_label->setStyleSheet(header_style);
      texture_layout->addRow(swapper_label);

      generate_hotkey_row({ font_noggit::shift, font_noggit::lmb },    "\a+\aSwap texture", texture_layout);
      generate_hotkey_row({ font_noggit::alt, font_noggit::lmb_drag }, "\a+\aChange radius", texture_layout);
      generate_hotkey_row({ font_noggit::t },                          "\aToggle brush swapper", texture_layout);

      auto anim_label = new QLabel("Anim:");
      anim_label->setStyleSheet(header_style);
      texture_layout->addRow(anim_label);

      generate_hotkey_row({ font_noggit::shift, font_noggit::lmb }, "\a+\aUpdate animation", texture_layout);
      generate_hotkey_row({ font_noggit::t },                       "\aSwitch between add/remove animation mode", texture_layout);


      auto water_widget (new QWidget (this));
      auto water_layout (new QFormLayout (water_widget));

      auto water_label = new QLabel("Water:");
      water_label->setStyleSheet(header_style);
      water_layout->addRow(water_label);

      generate_hotkey_row({ font_noggit::shift, font_noggit::lmb },    "\a+\aAdd liquid", water_layout);
      generate_hotkey_row({ font_noggit::ctrl, font_noggit::lmb },     "\a+\aRemove liquid", water_layout);
      generate_hotkey_row({ font_noggit::alt, font_noggit::lmb_drag }, "\a+\aChange brush size", water_layout);
      generate_hotkey_row({ font_noggit::t },                          "\aToggle angled mode", water_layout);
      generate_hotkey_row({ font_noggit::alt, font_noggit::mmb },      "\a+\aChange orientation", water_layout);
      generate_hotkey_row({ font_noggit::shift, font_noggit::mmb },    "\a+\aChange angle", water_layout);
      generate_hotkey_row({ font_noggit::f },                          "\aSet lock position to cursor position", water_layout);
      generate_hotkey_row({ font_noggit::space, font_noggit::f },      "\a+\aToggle lock mode", water_layout);
      generate_hotkey_row({ font_noggit::space, font_noggit::mmb },    "\a+\aChange height", water_layout);


      auto object_widget (new QWidget (this));
      auto object_layout (new QFormLayout (object_widget));

      auto object_label = new QLabel("Edit objects if a model is selected with left click (in object editor):");
      object_label->setStyleSheet(header_style);
      object_layout->addRow(object_label);

      generate_hotkey_row({ font_noggit::mmb },                                                          "\aMove object", object_layout);
      generate_hotkey_row({ font_noggit::alt, font_noggit::mmb },                                        "\a+\aScale M2", object_layout);
      generate_hotkey_row({ font_noggit::shift, font_noggit::ctrl, font_noggit::alt, font_noggit::lmb }, "\aor\aor\a+\aRotate object", object_layout);
      generate_hotkey_row({ font_noggit::ctrl },                                                         "\a+   0 - 9        Change doodadset of selected WMO", object_layout);
      generate_hotkey_row({ font_noggit::ctrl, font_noggit::r },                                         "\a+\aReset rotation", object_layout);
      generate_hotkey_row({ font_noggit::h },                                                            "\aToggle selected model/wmo visibility", object_layout);
      generate_hotkey_row({ font_noggit::space, font_noggit::h },                                        "\a+\a - Hide/Show hidden model/wmo", object_layout);
      generate_hotkey_row({ font_noggit::shift, font_noggit::h },                                        "\a+\a - Clear hidden model/wmo list", object_layout);
      generate_hotkey_row({ font_noggit::page_down },                                                    "\aSet object to ground level", object_layout);
      generate_hotkey_row({ font_noggit::ctrl, font_noggit::c },                                         "\a+\aCopy object to clipboard", object_layout);
      generate_hotkey_row({ font_noggit::ctrl, font_noggit::v },                                         "\a+\aPaste object on mouse position", object_layout);
      generate_hotkey_row({ font_noggit::ctrl, font_noggit::b },                                         "\a+\aDuplicate selected object to mouse position", object_layout);
      generate_hotkey_row({ font_noggit::shift, font_noggit::v },                                        "\a+\aImport last M2 from WMV", object_layout);
      generate_hotkey_row({ font_noggit::alt, font_noggit::v },                                          "\a+\aImport last WMO from WMV", object_layout);
      generate_hotkey_row({ font_noggit::t },                                                            "\aSwitch between paste modes", object_layout);
      generate_hotkey_row({ font_noggit::f },                                                            "\aMove selection to cursor position", object_layout);
      generate_hotkey_row({ font_noggit::minus, font_noggit::plus },                                     "\aor\aScale M2", object_layout);
      generate_hotkey_row({ font_noggit::num },                                                          "\a 7  or  9        Rotate object", object_layout);
      generate_hotkey_row({ font_noggit::num },                                                          "\a 4  or  8  or  6  or  2        Vertical position", object_layout);
      generate_hotkey_row({ font_noggit::num },                                                          "\a 1  or  3               Move up/down", object_layout);
      generate_hotkey_row({ font_noggit::shift },                                                        "Holding \a 1 /  3         Double speed", object_layout);
      generate_hotkey_row({ font_noggit::ctrl },                                                         "Holding \a 1 /  3         Triple speed", object_layout);
      generate_hotkey_row({ font_noggit::shift, font_noggit::ctrl },                                     "Holding \a and \a         Half speed", object_layout);

      auto shader_widget (new QWidget (this));
      auto shader_layout (new QFormLayout (shader_widget));

      generate_hotkey_row({ font_noggit::shift, font_noggit::lmb },      "\a+\aAdd shader", shader_layout);
      generate_hotkey_row({ font_noggit::ctrl, font_noggit::lmb },       "\a+\aRemove shader", shader_layout);
      generate_hotkey_row({ font_noggit::alt, font_noggit::lmb_drag },   "\a+\aChange brush size", shader_layout);
      generate_hotkey_row({ font_noggit::space, font_noggit::lmb_drag }, "\a+\aChange speed", shader_layout);
      generate_hotkey_row({ font_noggit::mmb },                          "\aPick shader color from the ground", shader_layout);
      generate_hotkey_row({ font_noggit::plus },                         "\aAdd current color to palette", shader_layout);

      layout->addWidget(tabs);
      tabs->addTab(base_widget, "Basic");
      tabs->addTab(ground_widget, "Terrain Editors");
      tabs->addTab(texture_widget, "Texture Painter");
      tabs->addTab(water_widget, "Water Editor");
      tabs->addTab(object_widget, "Object Editor");
      tabs->addTab(shader_widget, "Vertex Painter");
      tabs->addTab(flag_widget, "Impass Flag / Hole Cutter / Area ID");
    }


    void help::generate_hotkey_row(std::initializer_list<font_noggit::icons>&& hotkeys, const char* description, QFormLayout* layout)
    {
      auto row_layout = new QHBoxLayout(this);

      const char* from = nullptr;
      auto icon = hotkeys.begin();

      while (*description)
      {
        if (*description == '\a')
        {
          if (from)
          {
            auto label = new QLabel(::std::string(from, description - from).c_str());
            row_layout->addWidget(label);
          }
            
          auto label = new QLabel(this);
          QIcon hotkey_icon = font_noggit_icon(*icon++);
          label->setPixmap(hotkey_icon.pixmap(22, 22));
          row_layout->addWidget(label);

          from = ++description;
        }
        else
        {
          if (!from)
          {
            from = description;
          }     
          ++description;
        }
      }

      if (from && *from)
      {
        auto label = new QLabel(from);
        row_layout->addWidget(label);
      }

      row_layout->setAlignment(Qt::AlignLeft);
      layout->addRow(row_layout);

    }

  }
}
