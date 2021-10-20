// Copyright 2020 (c) Elucid Bioimaging

#ifndef MANIFEST_H
#define MANIFEST_H

#include <QDateTime>
#include <QFile>
#include <QFileDevice>
#include <QHttpPart>
#include <QHttpMultiPart>
#include <QMap>
#include <QQueue>
#include <QString>
#include <QVariant>

#define MAX_UPLOAD_SIZE (20 * 1000 * 1000) - (4 * 1024)

struct fileinfo
{
    QString   key;
    QFile     *fileref;
    QString   fpath;
    QDateTime lastUpdate;
    QByteArray json;
    QString   header;
    QString   dispo;
    QString   dispo_filename;
    bool      uploaded;
    qint64    datasize;
    qint64    offset;
};

class manifestQueue
{
public:
    void addJson(QByteArray json, QString key, QString header, QString dispo)
    {
        fileinfo * fi = new fileinfo();

        fi->key = key;
        fi->fileref = NULL;
        fi->lastUpdate = QDateTime::currentDateTime();
        fi->json = json;
        fi->header = header;
        fi->dispo = dispo;
        fi->dispo_filename = "";
        fi->uploaded = false;
        fi->datasize = json.size();
        fi->offset = 0;
        mQueue.enqueue(fi);
    }

    void addFile(QString key, QString header, QString dispo, QString fdir, QString prefix, QString fname)
    {
        QString fpath = fname;
        if (!fdir.isEmpty())
            fpath.replace(prefix, fdir);

        fileinfo * fi = new fileinfo();
        fi->key = key;
        fi->fpath = fpath;
        // fi->lastUpdate = QT_FILETIME( *(fileref) );
        fi->json = QByteArray();
        fi->header = header;
        fi->dispo = dispo;
        fi->dispo_filename = fname;
        fi->uploaded = false;
        // fi->datasize = fileref->size();
        fi->offset = 0;
        mQueue.enqueue(fi);
    }

    void setUrl(QString urlstr)
    {
        url = urlstr;
    }

    QString getUrl()
    {
        return url;
    }

    void *getData()
    {
        return data;
    }

    void setData(void *cbdata)
    {
        data = cbdata;
    }
    bool getTransferReport()
    {
        return xferReport;
    }

    void setTransferReport(bool xfer)
    {
        xferReport = xfer;
    }

    bool isEmpty()
    {
        return mQueue.isEmpty();
    }

    fileinfo *getEntry()
    {
        fileinfo *entry = mQueue.head();
        mQueue.dequeue();
        return entry;
    }

private:
    QQueue<fileinfo *> mQueue; 
    QString url;
    void *data;
    bool xferReport;
};

class manifest 
{
public:
    manifest() { 
        uploadQueue = NULL;
    };

    void addJson(QHttpMultiPart *mp, QByteArray json, QString key, QString header, QString dispo)
    {
        fileinfo * fi;

        if (fileMap.contains(key)) {
            fi = fileMap.value(key);
            fi->json = json;
            fi->datasize = json.size();
        } else {
            fi = new fileinfo();
            fi->key = key;
            fi->fileref = NULL;
            fi->lastUpdate = QDateTime::currentDateTime();
            fi->json = json;
            fi->header = header;
            fi->dispo = dispo;
            fi->dispo_filename = "";
            fi->uploaded = false;
            fi->datasize = json.size();
            fi->offset = 0;
            fileMap[key] = fi;
        }

        if (!uploadFile(mp, fi))
            deferQueue.enqueue(fi);
    }

#define QT_DATETIME_SECS(dt) (dt.toMSecsSinceEpoch() / 1000.0)

#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
#define QT_FILETIME(fref) \
        QFileInfo(fref).lastModified()
#else
#define QT_FILETIME(fref) \
        (fref).fileTime(QFileDevice::FileModificationTime)
#endif

    void addFile(QString key, QString header, QString dispo, QString fdir, QString prefix, QString fname)
    {
        fileinfo * fi;

        QString fpath = fname;
        if (!fdir.isEmpty())
            fpath.replace(prefix, fdir);

        //qInfo() << "addFile : " << fpath ;

        QFile *fileref = new QFile( fpath );
        fileref->open(QIODevice::ReadOnly);

        if (fileMap.contains(key)) {
            fi = fileMap.value(key);
            
            if (fileref->fileName() == fi->fpath && fi->uploaded && fi->lastUpdate >= QT_FILETIME((*fileref))) {
                delete(fileref);
                // file has not been updated since last upload
                return;
            }
            fi->fpath = fpath;
            fi->json = QByteArray();
            fi->dispo_filename = fname;
            fi->lastUpdate = QT_FILETIME( *(fileref) );
            fi->uploaded = false;
            fi->datasize = fileref->size();
        } else {
            fi = new fileinfo();
            fi->key = key;
            fi->fpath = fpath;
            fi->lastUpdate = QT_FILETIME( *(fileref) );
            fi->json = QByteArray();
            fi->header = header;
            fi->dispo = dispo;
            fi->dispo_filename = fname;
            fi->uploaded = false;
            fi->datasize = fileref->size();
            fi->offset = 0;
            fileMap[key] = fi;
        }

        if (!fileref->exists() || 0 == fileref->size()) {
            //qInfo() << "addfile: does not exist or zero length " << fpath;
        }
        delete(fileref);
        deferQueue.enqueue(fi);
    }

    void reconcile(QString key, int fsize, double fdate)
    {
        if (!fileMap.contains(key))
            return;

        fileinfo * fi = fileMap.value(key);
        if ( 0 > fdate || !(fsize != fi->datasize || QT_DATETIME_SECS(fi->lastUpdate) > fdate) ) {
            fi->uploaded = true;
        }
    }

    void prepareMetadata()
    {
        QString key("file-metadata");
        if (fileMap.contains(key)) {
            fileinfo *fi = fileMap.value(key);
            fi->uploaded = false;
            deferQueue.enqueue(fi);
        }
    }

    void updateFileMetadata()
    {
        QString meta("{\n");
        int first = 0;
        QList<QString> keys = fileMap.keys();
        for (int idx = 0; idx < keys.size(); idx++) {
            fileinfo *fi = fileMap.value(keys.at(idx));
            if ("application/dicom" == fi->header)
                continue;
            if (first)
                meta.append(", \n");
            meta.append("\"" + keys.at(idx) + "\" : ");
                meta.append(" { ");
                meta.append(" \"filename\" : \"" + fi->dispo_filename + "\" ");
                meta.append(", \"timestamp\" : \"" + QString::number(QT_DATETIME_SECS(fi->lastUpdate), 'f') + "\" ");
                meta.append(" }");
            qInfo() << "metadata: " << QString::number(QT_DATETIME_SECS(fi->lastUpdate), 'f')
                << " " << fi->dispo_filename;
            first = 1;
        }
        meta.append("\n}\n");
        QJsonDocument doc = QJsonDocument::fromJson( meta.toUtf8() );
        QString key("file-metadata");

        fileinfo *fi;
        if (fileMap.contains(key)) {
            fi = fileMap.value(key);
            fi->json = doc.toJson();
            fi->datasize = doc.object().size();
        } else {
            fi = new fileinfo();
            fi->key = key;
            fi->fileref = NULL;
            fi->lastUpdate = QDateTime::currentDateTime();
            fi->json = doc.toJson();
            fi->header = QString("application/json");
            fi->dispo = QString("form-data; name=\"file-metadata");
            fi->dispo_filename = "";
            fi->datasize = doc.object().size();
            fi->offset = 0;
            fileMap[fi->key] = fi;
        }
    }

    bool uploadFile(QHttpMultiPart *mp, fileinfo *fi) 
    {
        fi->uploaded = false;

        if (MAX_UPLOAD_SIZE < (uploadSize + fi->datasize)) {
            return false;
        }

        uploadSize += fi->datasize;

        QHttpPart part;

        if (NULL != fi->header)
            part.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(fi->header));

        if (NULL != fi->dispo) {
            QString dispo = fi->dispo;
            dispo += "\"";              // close the dispo string
            if (NULL != fi->dispo_filename)
                dispo += "; filename=\"" + fi->dispo_filename + "\"";
            part.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(dispo));
        }

        part.setHeader(QNetworkRequest::ContentLengthHeader, fi->datasize);

        if (0 < fi->json.length()) {
            part.setBody(fi->json);
        } else {
            qInfo() << "uploadFile : " << fi->dispo << " : " << fi->dispo_filename << " : " << fi->fpath;
            fi->fileref = new QFile( fi->fpath );
            fi->fileref->open(QIODevice::ReadOnly);
            part.setBodyDevice(fi->fileref);
            fi->fileref->setParent(mp);
        }

        mp->append(part);
        uploadQueue->enqueue(fi);
        return true;
    }

    void uploadPartialFile(QHttpMultiPart *mp, fileinfo *fi)
    {
        QHttpPart part;
        qint64 size = fi->datasize - fi->offset;

        if (MAX_UPLOAD_SIZE < size || MAX_UPLOAD_SIZE < uploadSize + size)
            size = MAX_UPLOAD_SIZE - uploadSize;

        uploadSize += size;

        if (0 == fi->offset) {
            fi->fileref = new QFile( fi->fpath );
            fi->fileref->open(QIODevice::ReadOnly);
            partialFile = true;
        }

        QByteArray bodyPart = fi->fileref->read( size );
        QByteArray headerName, headerValue;
        
        headerName.append("Content-Offset");
        headerValue.append( QString::number( fi->offset ));
        part.setRawHeader(headerName, headerValue);

        if (NULL != fi->header)
            part.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(fi->header));
        if (NULL != fi->dispo) {
            QString dispo = fi->dispo;
            dispo += ":" + QString::number( fi->offset );
            dispo += "\"";              // close the dispo string
            if (NULL != fi->dispo_filename)
                dispo += "; filename=\"" + fi->dispo_filename + "\"";
            part.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(dispo));
        }

        qInfo() << "uploadPartialFile : " << fi->dispo << " : " << fi->dispo_filename << " offset: " << QString::number( fi->offset ) << " : " << fi->fileref->fileName();
        part.setBody(bodyPart);
        part.setHeader(QNetworkRequest::ContentLengthHeader, bodyPart.size());
        part.setRawHeader(QByteArray("Connection"), QByteArray("Close"));
        fi->offset += size;
        mp->append(part);
        uploadQueue->enqueue(fi);

        if (0 >= fi->datasize - fi->offset) {
            delete(fi->fileref);
            deferQueue.dequeue();
            partialFile = false;
            fi->offset = 0;
        }
    }

    void createUpload(QHttpMultiPart *mp)
    {
        uploadSize = 0L;
        uploadQueue = new QQueue<fileinfo*>();
        int uploadCount = 0;

        while (hasFilesToUpload() && (6 > uploadCount++)) {
            fileinfo *fi = deferQueue.head();

            if (0 < fi->offset) {
                // already has an offset set, send next part
                uploadPartialFile(mp, fi);
                return;
            } 

            if (MAX_UPLOAD_SIZE < (uploadSize + fi->datasize)) {
                // file is too big for this multipart packet
                if (MAX_UPLOAD_SIZE < fi->datasize) {
                    // file is too big for upload overall
                    uploadPartialFile(mp, fi);
                }
                return;
            }

            if (!uploadFile(mp, fi))
                return;
            deferQueue.dequeue();
        }
    }

    QQueue<fileinfo *> * getUploads() 
    {
        return uploadQueue;
    }

    bool hasFilesToUpload()
    {
        bool notempty;
        while ((notempty = !deferQueue.isEmpty())) {
            fileinfo *fi = deferQueue.head();
            if (!fi->uploaded) {
                break;
            }
            deferQueue.dequeue();
        }
        return notempty;
    }

    bool hasPartialFile()
    {
        return partialFile;
    }

private:
    QMap<QString, fileinfo *> fileMap; 
    QQueue<fileinfo *> deferQueue; 
    QQueue<fileinfo *>* uploadQueue; 
    qint64 uploadSize;
    bool partialFile = false;
};

extern void setFileTime(QFile &target, double ftime);
extern void setFileTime(QString fromPath, QString target);

#endif // MANIFEST_H
