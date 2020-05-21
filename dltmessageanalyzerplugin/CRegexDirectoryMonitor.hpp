#ifndef CREGEXDIRECTORYMONITOR_HPP
#define CREGEXDIRECTORYMONITOR_HPP

#include <set>

#include <QObject>
#include <QString>
#include <QFileSystemWatcher>

/**
 * @brief The CRegexDirectoryMonitor class - this class is used to monitor folder with regex configuration files.
 * As soon as folder is updated, the corresponding UI elements ( e.g. combo-boxes, views ) are also being updated.
 */
class CRegexDirectoryMonitor : public QObject
{
    Q_OBJECT

 public:
    typedef QString tFoundRegexItem;
    typedef std::set<tFoundRegexItem> tFoundRegexItemSet;

    /**
     * @brief CRegexDirectoryMonitor - default constructor
     */
    CRegexDirectoryMonitor();

    /**
     * @brief ~CRegexDirectoryMonitor - destructor
     */
    virtual ~CRegexDirectoryMonitor();

    /**
     * @brief setPath - sets path to be observed and activates monitoring
     * @param dirPath - path to be observed
     */
    void setPath( const QString& dirPath );

signals:
    /**
     * @brief regexItemSetChanged - signal, which is fired each time, when set of JSON files within the folder was updated.
     * @param regexItemSet - the set of found JSON files.
     */
    void regexItemSetChanged(const tFoundRegexItemSet& regexItemSet);

private: // methods

    /**
     * @brief analyzeFolder - manually triggers folder anlaysis
     */
    void analyzeFolder();

private:
    QFileSystemWatcher mFileSystemWatcher;
    QString mObservedPath;
    QStringList mCachedFileList;
};

#endif // CREGEXDIRECTORYMONITOR_HPP
