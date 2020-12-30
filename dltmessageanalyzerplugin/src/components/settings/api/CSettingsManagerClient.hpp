#pragma once

#include "ISettingsManager.hpp"

/**
 * @brief The CSettingsManagerClient class
 * Small class, which purpose is to simplify injection of settings manager
 * instance to all classes, which are using it.
 */
class CSettingsManagerClient
{
public:

    CSettingsManagerClient();
    explicit CSettingsManagerClient(const tSettingsManagerPtr& pSettingsManager);

    const tSettingsManagerPtr& getSettingsManager() const;
    void setSettingsManager(const tSettingsManagerPtr& pSettingsManager);

protected:
    virtual void handleSettingsManagerChange(){}

private:
    tSettingsManagerPtr mpSettingsManager;
};
