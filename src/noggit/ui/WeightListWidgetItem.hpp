#pragma once

#include <QListWidget>
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QFrame>
#include <QPushButton>
#include <QWidget>

namespace noggit
{
	namespace Ui
	{
		class WeightListWidgetItem : public QWidget
		{
			Q_OBJECT

		public:
			WeightListWidgetItem(int doodad_value, QWidget* parent = nullptr);

		private:
			QLabel* label_text_weight;
			QLabel* label_doodad_name;
			QSpinBox* spinbox_weight;
			QLabel* label_text_percentage;
			QLabel* label_percentage;
			QHBoxLayout* title_layout;
			QHBoxLayout* weight_layout;
			QHBoxLayout* percentage_layout;
			QVBoxLayout* container_layout;
			QHBoxLayout* wrapper_layout;
			QPushButton* reset_button;
			QFrame* line;
		};
	}
}