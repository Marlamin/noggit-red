#pragma once

#include <qpushbutton.h>
#include "qprocess.h"
#include <qfile.h>
#include <qdir.h>
#include <qdiriterator.h>
#include <QtWidgets/QDialog>
#include <QtNetwork/qnetworkreply.h>
#include <QtNetwork/qnetworkrequest.h>
#include <QtNetwork/qnetworkaccessmanager.h>

#include "ui_Updater.h"

namespace Noggit
{
	namespace Ui
    {
        class CUpdater : public QDialog
        {
            Q_OBJECT
            ::Ui::Updater* ui;

        public:
            CUpdater(QWidget* parent = nullptr);

        private:
            QByteArray FileMD5(const QString& filename, QCryptographicHash::Algorithm algo);
            QString ToHashFile(const QString& name, const QString& hash);
            QUrl GenerateLink(const QString& name);

            void GenerateLocalMD5();
            void CompareMD5();
            void DownloadUpdate();
            void StartExternalUpdater();

        private slots:
            void GenerateOnlineMD5();
            void GetOnlineFile();

        signals:
            void OpenUpdater();

        private:
            const QString TemporaryFolder = "/temp";
            const QString StorageURL = "https://raw.githubusercontent.com/Intemporel/NoggitRedBinaries/main/%1";
            const QString FileURL = "%1%2/%3";
            const QString ExternalProcess = "/noggit-updater.exe";

            int FileNeededCount;
            int FileMissingCount;

            bool NeedUpdate;
            bool LocalCheck, OnlineCheck;

            QMap<QString, QString> LocalMD5;
            QMap<QString, QString> OnlineMD5;

            QVector<QString> FileNeeded;
        };
    }

}

