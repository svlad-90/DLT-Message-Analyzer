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
    virtual tCoverageNoteItemId addCoverageNoteItem() = 0;
    virtual void setCoverageNoteItemRegex(const tCoverageNoteItemId& id, const QString& regex) = 0;
    virtual void setCoverageNoteItemMessage(const tCoverageNoteItemId& id,
                                        const QString& comment) = 0;
    virtual void scrollToLastCoveageNoteItem() = 0;

signals:
    void regexApplicationRequested(const QString& regex);
};

typedef std::shared_ptr<ICoverageNoteProvider> tCoverageNoteProviderPtr;
