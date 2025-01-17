#pragma once

#include <QDialog>

namespace Ui
{
  class Changelog;
}

namespace Noggit
{
    namespace Ui
    {
        class CChangelog : public QDialog
        {
            Q_OBJECT
            ::Ui::Changelog* ui;

        public:
            CChangelog(QWidget* parent = nullptr);
            void SelectFirst();

        private:
            const QString ChangelogFolder = "/changelog";
            const QString AttachmentFolder = "/changelog/attachment";

            QString GetChangelogName(QString name);
            void OpenChangelog(QString name);

            std::vector<QString> Changelog;
        };
    }
}

