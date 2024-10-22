#pragma once

#include "QObject"

#include "../api/Types.hpp"

class ICoverageNoteProvider : public QObject
{
    Q_OBJECT

public:
    ICoverageNoteProvider(QObject *parent = nullptr): QObject(parent)
    {};
    virtual ~ICoverageNoteProvider() = default;

    // file level API
    virtual bool loadCoverageNoteFile(const QString& filePath) = 0;
    virtual bool saveCoverageNoteFile(const QString& filePath) = 0;
    virtual void clearCoverageNote() = 0;
    virtual bool exportCoverageNoteAsHTML(const QString& targetPath) = 0;

    // coverage note item methods
    virtual tCoverageNoteItemId addCoverageNoteItem() = 0;
    virtual void moveCoverageNoteItem(const tCoverageNoteItemId& from,
                                      const tCoverageNoteItemId& tos,
                                      bool after = true) = 0;

    virtual void setCoverageNoteItemRegex(const tCoverageNoteItemId& id, const QString& regex) = 0;
    virtual void setCoverageNoteMessage(const tCoverageNoteItemId& id,
                                        const QString& coverageNoteMessage) = 0;
    virtual void scrollToLastCoveageNoteItem() = 0;

    virtual tCoverageNoteItemPtr getCoverageNoteItem(const tCoverageNoteItemId& id) const = 0;

signals:
    void regexApplicationRequested(const QString& regex);
};

typedef std::shared_ptr<ICoverageNoteProvider> tCoverageNoteProviderPtr;
