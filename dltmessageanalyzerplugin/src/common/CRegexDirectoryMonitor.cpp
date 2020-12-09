// QT includes
#include <QDir>
#include <QFile>
#include <QDir>

// project includes
#include "Definitions.hpp"
#include "components/log/api/CLog.hpp"

// self-includes
#include "CRegexDirectoryMonitor.hpp"

#include "DMA_Plantuml.hpp"

CRegexDirectoryMonitor::CRegexDirectoryMonitor():
mFileSystemWatcher(),
mObservedPath()
{

}

CRegexDirectoryMonitor::~CRegexDirectoryMonitor()
{}

void CRegexDirectoryMonitor::setPath( const QString& dirPath )
{
    if(false == mObservedPath.isEmpty())
    {
        mFileSystemWatcher.removePath(mObservedPath);
        mObservedPath.clear();
    }

    bool bPathAdditionResult = mFileSystemWatcher.addPath(dirPath);

    if(false == bPathAdditionResult)
    {
        QString msg = QString("Error. Was not able to add path \"%1\" to watcher list").arg(mObservedPath);
        SEND_ERR(msg);
    }
    else
    {
        mObservedPath = dirPath;
        connect(&mFileSystemWatcher, &QFileSystemWatcher::directoryChanged,
                this, &CRegexDirectoryMonitor::analyzeFolder);
    }

    analyzeFolder();
}

void CRegexDirectoryMonitor::analyzeFolder()
{
    SEND_MSG(QString("[CRegexDirectoryMonitor]: Path \"%1\" analyzed")
             .arg(mObservedPath));

    tFoundRegexItemSet result;

    QString regexDir = mObservedPath;
    QDir directory(regexDir);

    if(true == directory.exists())
    {
        QStringList regexConfigs = directory.entryList(QStringList() << "*.json" << "*.JSON", QDir::Files);

        if (!regexConfigs.empty())
        {
            if(mCachedFileList != regexConfigs)
            {
                mCachedFileList = regexConfigs;

                for(const auto& regexConfigFile : regexConfigs)
                {
                    result.insert(regexConfigFile);

                    //SEND_MSG(QString("[CRegexDirectoryMonitor]: adding directory to result set - \"%1\"")
                    //.arg(regexConfigFile));
                }

                //SEND_MSG(QString("[CRegexDirectoryMonitor]: sending result of size - %1").arg(result.size()));

                regexItemSetChanged( result );
            }
        }
        else
        {
            {
                SEND_WRN(QString("[CRegexDirectoryMonitor]: 0 files found. Creating defult file").arg(mObservedPath));

                QString defaultFilePath = regexDir + QDir::separator() + sDefaultRegexFileName;
                QFile file(defaultFilePath);

                if(true == file.open(QIODevice::OpenModeFlag::ReadWrite))
                {
                    SEND_WRN(QString("[CRegexDirectoryMonitor]: Default file \"%1\" created").arg(defaultFilePath));
                    file.close();

                    result.insert(sDefaultRegexFileName);
                    regexItemSetChanged( result );
                }
            }
        }
    }
    else
    {
        SEND_WRN(QString("[CRegexDirectoryMonitor]: Path \"%1\" does not exist").arg(mObservedPath));
    }
}

PUML_PACKAGE_BEGIN(DMA_Common)
    PUML_CLASS_BEGIN_CHECKED(CRegexDirectoryMonitor)
        PUML_INHERITANCE_CHECKED(QObject, extends)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(QFileSystemWatcher, 1, 1, contains)
    PUML_CLASS_END()
PUML_PACKAGE_END()
