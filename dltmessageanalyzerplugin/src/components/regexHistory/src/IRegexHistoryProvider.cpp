#include "../api/IRegexHistoryProvider.hpp"

#include "DMA_Plantuml.hpp"

IRegexHistoryProvider::IRegexHistoryProvider()
{}

IRegexHistoryProvider::~IRegexHistoryProvider()
{}

PUML_PACKAGE_BEGIN(DMA_RegexHistory_API)
    PUML_CLASS_BEGIN(IRegexHistoryProvider)
        PUML_INHERITANCE_CHECKED(QObject, extends)
    PUML_CLASS_END()
PUML_PACKAGE_END()
