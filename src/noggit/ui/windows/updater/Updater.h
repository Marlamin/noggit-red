#pragma once

#include <QByteArray>
#include <QCryptographicHash>
#include <QDialog>
#include <QMap>
#include <QString>
#include <QUrl>
#include <QVector>

namespace Ui
{
  class Updater;
}

namespace Noggit
{
	namespace Ui
    {
        class CUpdater : public QDialog
        {
            Q_OBJECT
            ::Ui::Updater* ui = nullptr;

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

            int FileNeededCount = 0;
            int FileMissingCount = 0;

            bool NeedUpdate = false;
            bool LocalCheck = false, OnlineCheck = false;

            QMap<QString, QString> LocalMD5;
            QMap<QString, QString> OnlineMD5;

            QVector<QString> FileNeeded;
        };
    }
}
