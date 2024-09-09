#include "../api/ISettingsManager.hpp"

#include "dma/base/ForceLink.hpp"

#include "DMA_Plantuml.hpp"

ISettingsManager::ISettingsManager()
{

}

ISettingsManager::~ISettingsManager()
{

}

// tAliasItem
ISettingsManager::tAliasItem::tAliasItem():
    isDefault(false),
    alias(),
    regex()
{}

ISettingsManager::tAliasItem::tAliasItem(bool isDefault_, const QString& alias_, const QString& regex_):
    isDefault(isDefault_),
    alias(alias_),
    regex(regex_)
{}

bool ISettingsManager::tAliasItem::operator==(const tAliasItem& val) const
{
    return isDefault == val.isDefault &&
            alias == val.alias &&
            regex == val.regex;
}

ISettingsManager::tRegexUsageStatisticsItem::tRegexUsageStatisticsItem():
usageCounter(0),
updateDateTime()
{}

ISettingsManager::tRegexUsageStatisticsItem::tRegexUsageStatisticsItem(const uint32_t& usageCounter_,
                                                                       const QDateTime& updateDateTime_):
usageCounter(usageCounter_),
updateDateTime(updateDateTime_)
{}

bool ISettingsManager::tRegexUsageStatisticsItem::operator==
(const ISettingsManager::tRegexUsageStatisticsItem& val) const
{
    return usageCounter == val.usageCounter && updateDateTime == val.updateDateTime;
}

DMA_FORCE_LINK_ANCHOR_CPP(ISettingsManager)

PUML_PACKAGE_BEGIN(DMA_Settings_API)
    PUML_CLASS_BEGIN(ISettingsManager)
        PUML_INHERITANCE_CHECKED(QObject, extends)
    PUML_CLASS_END()
PUML_PACKAGE_END()
