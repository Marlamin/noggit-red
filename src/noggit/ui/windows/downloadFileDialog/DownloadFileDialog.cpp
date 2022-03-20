// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/windows/downloadFileDialog/DownloadFileDialog.h>
#include "revision.h"
#include <noggit/Log.h>
#include <sstream>
#include <map>

namespace Noggit::Ui
{
    DownloadFileDialog::DownloadFileDialog(QString url, QString fileDownloadPath, QWidget* parent) : QDialog(parent), ui(new ::Ui::DownloadFileDialog)
    {
        ui->setupUi(this);

        auto urlPath = QUrl(url);
        auto request = QNetworkRequest(urlPath);

        ui->file_download_name->setText(urlPath.toDisplayString());
        ui->progressBar->setMinimum(0);
        ui->progressBar->setValue(0);

        localFile = new QFile(fileDownloadPath);
        if (!localFile->open(QIODevice::WriteOnly))
            return;

        _response = manager.get(request); 

        QObject::connect(&manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(downloadFinished(QNetworkReply*)));
        QObject::connect(_response, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(updateProgress(qint64, qint64)));
        QObject::connect(_response, SIGNAL(readyRead()), this, SLOT(readingReadyBytes()));
    }

    void DownloadFileDialog::downloadFinished(QNetworkReply* data) 
    {  
        data->deleteLater();    
        localFile->close();

        //this->close();
    }

    void DownloadFileDialog::readingReadyBytes()
    {
         localFile->write(_response->read(_response->bytesAvailable()));        
    }

    void DownloadFileDialog::updateProgress(qint64 recieved, qint64 total)
    {
        auto headerMappedvalues = std::map<std::string, std::string>();

        //Parse content-disposition Header
        QVariant contentDispositionHeader = _response->header(QNetworkRequest::ContentDispositionHeader);
        if (contentDispositionHeader.isValid()) {
            auto contentDispositionHeaderValue = contentDispositionHeader.toString();
            auto values = contentDispositionHeaderValue.split(";");
            for (auto const& part : values)
            {
                if (part.contains("="))
                {
                    auto actualSize = part.split("=");
                    headerMappedvalues.insert({ actualSize[0].toLower().toStdString(), actualSize[1].toStdString() });
                }
                else
                {
                    headerMappedvalues.insert({ part.toLower().toStdString(), ""});
                }
            }     
        }

        //Parse content-length Header
        QVariant contentLengthHeader = _response->header(QNetworkRequest::ContentLengthHeader);
        if (contentLengthHeader.isValid()) {
            auto contentLengthHeaderValue = contentLengthHeader.toInt();
            headerMappedvalues.insert({ "size", std::to_string(contentLengthHeaderValue) });
        }

        auto fileSize = std::atoi(headerMappedvalues["size"].c_str());

        ui->progressBar->setMaximum(fileSize);
        ui->progressBar->setValue(recieved);
     
        auto sstream = std::stringstream();
        sstream << "Downloaded :  " << (recieved / 1024) << "kB";

        ui->file_download_progress->setText(QString::fromStdString(sstream.str()));
    }
}
