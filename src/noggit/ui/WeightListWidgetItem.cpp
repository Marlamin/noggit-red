#include "WeightListWidgetItem.hpp"

#include <noggit/ui/FontAwesome.hpp>

namespace noggit 
{
    namespace Ui 
    {
        WeightListWidgetItem::WeightListWidgetItem(int doodad_value, QWidget* parent)
            : QWidget(parent), label_text_weight(new QLabel("W: ")),
            spinbox_weight(new QSpinBox), label_text_percentage(new QLabel("%: ")),
            label_percentage(new QLabel), weight_layout(new QHBoxLayout),
            percentage_layout(new QHBoxLayout), container_layout(new QVBoxLayout), line(new QFrame), 
            label_doodad_name(new QLabel), wrapper_layout(new QHBoxLayout), reset_button(new QPushButton),
            title_layout(new QHBoxLayout)
        {
            // Set up spinbox ranges
            spinbox_weight->setRange(0, 10);

            //
            QString doodad_name = "Doodad " + QString::number(doodad_value);
            label_doodad_name->setText(doodad_name);
            label_doodad_name->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
            label_doodad_name->setFixedWidth(60);
            label_percentage->setText("NaN");

            reset_button->setIcon(Noggit::Ui::FontAwesomeIcon(Noggit::Ui::FontAwesome::minus));
            reset_button->setFixedSize(15, 15);
            // Set fixed heights for labels and spin boxes
            //label_text_weight->setFixedHeight(20); // Adjust the height as needed
            //spinbox_weight->setFixedHeight(20);    // Adjust the height as needed
            //label_text_percentage->setFixedHeight(20); // Adjust the height as needed
            //label_percentage->setFixedHeight(20);    // Adjust the height as needed
            label_text_weight->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
            spinbox_weight->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

            // Setting the fixed width for the label and spin box
            label_text_weight->setFixedWidth(25);  // Adjust as needed
            spinbox_weight->setFixedWidth(50);  // Adjust as needed

            // Set fixed heights for labels and spin boxes
            //label_text_weight->setFixedWidth(10); 
            //spinbox_weight->setFixedWidth(20);    
            label_text_percentage->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
            label_percentage->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
            label_text_percentage->setFixedWidth(25);
            label_percentage->setFixedWidth(30);
            // Set up vertical line.
            //line->setFrameShape(QFrame::VLine);
            //line->setFrameShadow(QFrame::Sunken);

            // Set up layouts
            title_layout->setAlignment(Qt::AlignLeft);
            title_layout->addWidget(reset_button);
            title_layout->addWidget(label_doodad_name);

            weight_layout->setAlignment(Qt::AlignLeft);
            weight_layout->addWidget(label_text_weight);
            weight_layout->addWidget(spinbox_weight);

            percentage_layout->setAlignment(Qt::AlignLeft);
            percentage_layout->addWidget(label_text_percentage);
            percentage_layout->addWidget(label_percentage);

            container_layout->addLayout(title_layout);
            container_layout->addLayout(weight_layout);
            container_layout->addLayout(percentage_layout);

            wrapper_layout->addLayout(container_layout);
            wrapper_layout->addWidget(line);

            // Set layout for the item
            setLayout(wrapper_layout);
        }
    }
}