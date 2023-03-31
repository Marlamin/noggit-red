#include "Updater.h"
#include "ui_Updater.h"

namespace Noggit
{
	namespace Ui
    {
        CUpdater::CUpdater(QWidget* parent) :
            QDialog(parent)
        {
            /*setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
            ui = new ::Ui::Updater;
            ui->setupUi(this);

            // hide progress
            ui->ProgressFile->hide();
            ui->ProgressDownload->hide();
            hide();

            FileNeeded.clear();
            LocalMD5.clear();
            OnlineMD5.clear();

            FileNeededCount = 0;
            FileMissingCount = 0;

            LocalCheck = false;
            OnlineCheck = false;
            NeedUpdate = false;

            // connect ui button to update or close
            connect(ui->Close, &QPushButton::clicked, this, [=]() { close(); });
            connect(ui->Update, &QPushButton::clicked, this, [=]() { DownloadUpdate(); });

            QNetworkRequest request((QUrl(StorageURL.arg("MD5"))));
            QNetworkReply* reply = (new QNetworkAccessManager)->get(request);
            connect(reply, SIGNAL(finished()), this, SLOT(GenerateOnlineMD5()));

            GenerateLocalMD5();*/
        }

        QByteArray CUpdater::FileMD5(const QString& filename, QCryptographicHash::Algorithm algo)
        {
            QFile file(filename);
            if (file.open(QFile::ReadOnly))
            {
                QCryptographicHash hash(algo);

                if (hash.addData(&file))
                    return hash.result();
            }

            return QByteArray();
        }

        QString CUpdater::ToHashFile(const QString& name, const QString& hash)
        {
            return QString("%1 %2").arg(name, hash);
        }

        QUrl CUpdater::GenerateLink(const QString& name)
        {
            return QUrl(StorageURL.arg(name));
        }

        void CUpdater::GenerateLocalMD5()
        {
            QVector<QString> ignore = {
                "md5",
                "listfile.csv",
                "log.txt"
            };

            QDirIterator it(QDir::currentPath(), QStringList() << "*", QDir::Files, QDirIterator::Subdirectories);
            QFile MD5 = QDir::currentPath() + "/MD5";
            if (MD5.open(QIODevice::WriteOnly))
            {
                QTextStream stream(&MD5);

                while (it.hasNext())
                {
                    const QString file = it.next();

                    if (ignore.contains(QFileInfo(file).fileName().toLower()))
                        continue;

                    QString name = file.mid(QDir::currentPath().size() + 1);
                    QString hash = FileMD5(file, QCryptographicHash::Md5).toHex();

                    LocalMD5[name] = hash;
                    stream << ToHashFile(name, hash) << Qt::endl;
                }

                MD5.flush();
                MD5.close();
            }

            LocalCheck = true;
            CompareMD5();
        }

        void CUpdater::GenerateOnlineMD5()
        {
            QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
            if (!reply || reply->size() == 0xE)
                return;

            QFile temp_md5(QDir::currentPath() + "online_MD5.txt");
            if (temp_md5.open(QIODevice::WriteOnly))
            {
                QTextStream stream(&temp_md5);
                stream << reply->readAll();
                temp_md5.flush();
                temp_md5.close();
            }

            if (temp_md5.open(QIODevice::ReadOnly))
            {
                QTextStream stream(&temp_md5);

                while (!stream.atEnd())
                {
                    QString line = stream.readLine();
                    QStringList split = line.split(R"( )");
                    OnlineMD5[split[0]] = split[1];
                }

                temp_md5.close();
            }

            QDir dir;
            dir.remove(temp_md5.fileName());

            OnlineCheck = true;
            CompareMD5();
        }

        void CUpdater::CompareMD5()
        {
            if (!LocalCheck || !OnlineCheck)
                return;

            for (const auto& e : OnlineMD5.toStdMap())
            {
                if (LocalMD5[e.first] != e.second)
                {
                    FileNeeded.push_back(e.first);

                    if (ui->FileList->toPlainText().isEmpty())
                    {
                        ui->FileList->append(QString(tr("This is all files needed to update.")));
                    }

                    ui->FileList->append(QString(" - %1 [%2 => %3]").arg(FileNeeded.last()).arg(LocalMD5[e.first]).arg(e.second));
                }
            }

            if (FileNeeded.size() == 0)
            {
                NeedUpdate = false;
                return;
            }

            FileNeededCount = FileNeeded.size();
            NeedUpdate = true;

            if (NeedUpdate)
            {
                ui->ProgressFile->setMaximum(FileNeededCount);
                ui->ProgressFile->setFormat(QString(tr("File %v/%1")).arg(FileNeededCount));
                ui->ProgressFile->show();

                emit OpenUpdater();
            }
        }

        void CUpdater::DownloadUpdate()
        {
            if (FileNeeded.size() == 0)
                return;

            ui->ProgressDownload->show();

            QNetworkRequest request(GenerateLink(FileNeeded[0]));
            QNetworkReply* reply = (new QNetworkAccessManager)->get(request);
            connect(reply, &QNetworkReply::downloadProgress, reply, [this](qint64 received, qint64 total)
                {
                    ui->ProgressDownload->setMaximum(total);
                    ui->ProgressDownload->setValue(received);
                });

            connect(reply, SIGNAL(finished()), this, SLOT(GetOnlineFile()));

            ui->FileList->append(QString(tr("Downloading : %1")).arg(FileNeeded[0]));
        }

        void CUpdater::GetOnlineFile()
        {
            QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());

            ui->ProgressFile->setValue(ui->ProgressFile->value() + 1);

            if (!reply || reply->size() == 0xE)
            {
                FileMissingCount += 1;
                FileNeeded.removeAt(0);
                if (FileNeeded.size() > 0)
                {
                    DownloadUpdate();
                    return;
                }

                StartExternalUpdater();
                return;
            }

            QDir dir;
            if (!dir.exists(QDir::currentPath() + TemporaryFolder))
                dir.mkpath(QDir::currentPath() + TemporaryFolder);

            auto index = FileNeeded[0].lastIndexOf(R"(/)");
            if (index >= 0)
            {
                QDir dir;
                if (!dir.exists(FileURL.arg(QDir::currentPath(), TemporaryFolder, FileNeeded[0].mid(0, index))))
                    dir.mkpath(FileURL.arg(QDir::currentPath(), TemporaryFolder, FileNeeded[0].mid(0, index)));
            }

            QFile file = FileURL.arg(QDir::currentPath(), TemporaryFolder, FileNeeded[0]);
            if (file.open(QIODevice::WriteOnly))
            {
                file.write(reply->readAll());
                file.flush();
                file.close();
            }

            FileNeeded.removeAt(0);
            if (FileNeeded.size() > 0)
            {
                DownloadUpdate();
                return;
            }

            StartExternalUpdater();
        }

        void CUpdater::StartExternalUpdater()
        {
            if (!NeedUpdate)
                return;

            if (FileNeededCount == FileMissingCount)
                return;

            {
                QString exec(QDir::currentPath() + ExternalProcess);
                QProcess process;
                process.setProgram(exec);
                process.startDetached();
                QCoreApplication::quit();
            }
        }
    }
}
