#ifndef PCREHELPER_HPP
#define PCREHELPER_HPP

#include <string>

#include "../Definitions.hpp"

// Note! This header should not expose any antlr types
// And it should not use any Qt types.
// Reason - collision in antlr4 & Qt regarding the "emit" symbol, which is used in both technologies
// In other words - proper encapsulation is our best friend.

/**
 * @brief parseRegexFiltersView - parses the provided regex
 * @param pFiltersViewTree - filters view tree, which should be modified as result of this method.
 * @param regex - input regex string
 */
void parseRegexFiltersView( const tTreeItemSharedPtr& pFiltersViewTree, const QString& regex );

#endif // PCREHELPER_HPP
