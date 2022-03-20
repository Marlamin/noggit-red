// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QSlider>

#include <functional>
#include <string>

namespace Noggit
{
  namespace Ui
  {
    class zone_id_browser : public QWidget
    {
      Q_OBJECT

    public:
      zone_id_browser(QWidget* parent = nullptr);
      void setMapID(int id);
      void setZoneID(int id);
      void changeRadius(float change);
      void setRadius(float radius);

      float brushRadius() const { return _radius; }

    signals:
      void selected (int area_id);

    private:
      QTreeWidget* _area_tree;

      QSlider* _radius_slider;
      QDoubleSpinBox* _radius_spin;

      std::map<int, QTreeWidgetItem*> _items;
      int mapID;
      float _radius = 15.0f;

      void buildAreaList();
      QTreeWidgetItem* create_or_get_tree_widget_item(int area_id);
      QTreeWidgetItem* add_area(int area_id);
    };
  }
}
