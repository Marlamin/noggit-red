#include "Changelog.hpp"
#include <qdir.h>
#include <qtextstream.h>
#include "ui_Changelog.h"

namespace Noggit
{
    namespace Ui
    {
        CChangelog::CChangelog(QWidget* parent) :
            QDialog(parent)
        {
            /*setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
            ui = new ::Ui::Changelog;
            ui->setupUi(this);

            QDir dir;
            if (!dir.exists(QDir::currentPath() + ChangelogFolder))
                dir.mkdir(QDir::currentPath() + ChangelogFolder);

            QDir folder(QDir::currentPath() + ChangelogFolder);
            QStringList files = folder.entryList(QStringList() << "*.changelog" << "*.CHANGELOG", QDir::Files);

            foreach(QString filename, files)
            {
                Changelog.push_back(filename);
            }

            std::sort(Changelog.begin(), Changelog.end(), [](QString& a, QString& b)
                {
                    return (a.toInt() < b.toInt());
                });

            for (int i = 0; i < Changelog.size(); ++i)
            {
                QListWidgetItem* item = new QListWidgetItem();
                item->setText(GetChangelogName(Changelog[Changelog.size() - 1 - i]));
                item->setData(1, Changelog[Changelog.size() - 1 - i]);
                item->setTextAlignment(Qt::AlignLeft);

                ui->listWidget->addItem(item);
            }

            SelectFirst();

            connect(ui->listWidget, &QListWidget::itemClicked, [&](QListWidgetItem *item)
                {
                    OpenChangelog(item->data(1).toString());
                });*/
        }

        void CChangelog::SelectFirst()
        {
            if (ui->listWidget->count() > 0)
            {
                ui->listWidget->setCurrentRow(0);
                OpenChangelog(ui->listWidget->item(0)->data(1).toString());
            }
        }

        QString CChangelog::GetChangelogName(QString name)
        {
            QFile changelog = QDir::currentPath() + ChangelogFolder + "/" + name;
            if (!changelog.exists())
                return name;

            if (changelog.open(QIODevice::ReadOnly))
            {
                QTextStream stream(&changelog);
                QString stored = stream.readLine();
                changelog.close();
                if (stored.contains("<!--") && stored.contains("-->"))
                {
                    stored.remove("<!--").remove("-->");
                    return stored;
                }
            }

            return name;
        }
        void CChangelog::OpenChangelog(QString name)
        {
            QFile changelog = QDir::currentPath() + ChangelogFolder + "/" + name;
            if (!changelog.exists())
                return;

            if (changelog.open(QIODevice::ReadOnly))
            {
                QTextStream stream(&changelog);
                ui->textEdit->setHtml(stream.readAll());
                changelog.close();
            }
        }
    }
}
