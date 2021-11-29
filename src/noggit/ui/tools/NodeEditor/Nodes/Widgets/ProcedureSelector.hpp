// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_PROCEDURESELECTOR_HPP
#define NOGGIT_PROCEDURESELECTOR_HPP

#include <QWidget>

namespace noggit
{
    namespace ui::tools::NodeEditor::Nodes
    {
        class ProcedureSelector : public QWidget
        {
            Q_OBJECT

        public:
            explicit ProcedureSelector(QWidget* parent = nullptr);
            QString getPath();
            void setPath(QString const& path);

        signals:
              void entry_updated(QString const& entry);
        };
    }
}

#endif //NOGGIT_PROCEDURESELECTOR_HPP
