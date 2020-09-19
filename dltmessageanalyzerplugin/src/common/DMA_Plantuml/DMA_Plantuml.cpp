#include <algorithm>

#include "DMA_Plantuml.hpp"

namespace DMA
{
    namespace PlantUML
    {
        static const std::string sNewLine("\n");
        static const std::string sPackageColor("#DDDDDD");
        static const std::string sIndentation("    ");

        /////////////////////////////tStringPtrWrapper///////////////////////////
        tStringPtrWrapper::tStringPtrWrapper(): pString(nullptr)
        {}

        tStringPtrWrapper::tStringPtrWrapper(const tStringPtr& pString_): pString(pString_)
        {}

        bool tStringPtrWrapper::operator< ( const tStringPtrWrapper& rVal ) const
        {
            bool bResult = false;

            if(pString == nullptr && rVal.pString != nullptr)
                bResult = true;
            else if(pString != nullptr && rVal.pString == nullptr)
                bResult = false;
            else if(pString == nullptr && rVal.pString == nullptr)
                bResult = true;
            else
            {
                if( *pString < *rVal.pString )
                {
                    bResult = true;
                }
            }

            return bResult;
        }

        bool tStringPtrWrapper::operator== ( const tStringPtrWrapper& rVal ) const
        {
            if(pString == nullptr && rVal.pString != nullptr)
                return false;
            else if(pString != nullptr && rVal.pString == nullptr)
                return false;
            else if(pString == nullptr && rVal.pString == nullptr)
                return true;

            return ( *pString == *rVal.pString );
        }

        bool tStringPtrWrapper::operator!= ( const tStringPtrWrapper& rVal ) const
        {
            return !( *this == rVal );
        }
        //////////////////////////////////////////////////////////////////////////

        /////////////////////////////CCreator///////////////////////////
        Creator::Creator():
        mPackageMap(),
        mItemRegistry(),
        mbIsinitialized(false),
        mDataProtector()
        {}

        Creator& Creator::getInstance()
        {
            static Creator result;
            return result;
        }

        void Creator::initialize()
        {
            std::lock_guard<std::mutex> guard(mDataProtector);

            if(false == mbIsinitialized)
            {
                // at this point the model is formed, as this method is called somewhere in non-global area.
                // The only thing we need to do is to assign dependency & inheritance pointers

                for(auto& packagePair : mPackageMap)
                {
                    if(nullptr != packagePair.second)
                    {
                        for(auto& itemPair : packagePair.second->itemMap)
                        {
                            if(nullptr != itemPair.second)
                            {
                                auto& pItem = itemPair.second;

                                auto& dependencyMap = pItem->getDependencyMap();

                                for(auto& dependencyPair : dependencyMap)
                                {
                                    auto pDependency = dependencyPair.second;

                                    if(nullptr != pDependency)
                                    {
                                        auto foundItem = mItemRegistry.find(pDependency->itemName);

                                        if(foundItem != mItemRegistry.end() && nullptr != foundItem->second)
                                        {
                                            auto& pFoundItem = foundItem->second;

                                            pDependency->pToItem = pFoundItem;

                                            auto& dependentMap = pFoundItem->getDependentMap();
                                            tDependencyDataPtr pDependent = std::make_shared<tDependencyData>();
                                            pDependent->comment = pDependency->comment;
                                            pDependent->pToItem = pItem;
                                            pDependent->itemName = pItem->getItemName();
                                            pDependent->toNumber = pDependency->fromNumber;
                                            pDependent->fromNumber = pDependency->toNumber;
                                            pDependent->dependencyType = pDependency->dependencyType;
                                            dependentMap[pItem->getItemName()] = pDependent;
                                        }
                                    }
                                }

                                if(eItemType::eClass == pItem->getType())
                                {
                                    auto& inheritanceMap = pItem->getInheritanceMap();

                                    for(auto& inheritancePair : inheritanceMap)
                                    {
                                        auto pInheritance = inheritancePair.second;

                                        if(nullptr != pInheritance)
                                        {
                                            auto foundItem = mItemRegistry.find(pInheritance->baseClass);

                                            if(foundItem != mItemRegistry.end() && nullptr != foundItem->second)
                                            {
                                                auto& pFoundItem = foundItem->second;

                                                pInheritance->pFromItem = pFoundItem;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                mbIsinitialized = true;
            }
        }

        bool Creator::isInitislized() const
        {
            bool bResult = false;

            {
                std::lock_guard<std::mutex> guard(*const_cast<std::mutex*>(&mDataProtector));
                bResult = mbIsinitialized;
            }

            return bResult;
        }

        static std::string getAccessModifier( const std::string& candidate )
        {
            std::string result;

            if(candidate != "+" && candidate != "#" && candidate != "~" && candidate != "-") // if user has provided garbage
            {
                result = "+"; // let's fallback too public method
            }
            else // otherwise
            {
                result = candidate; // let's return user's input
            }

            return result;
        }

        static std::string getClassDiagramInternal(const tPackageMap& packageMap)
        {
            std::string diagram;

            if(false == packageMap.empty())
            {
                int packageCounter = 0;
                int packageSize = static_cast<int>(packageMap.size());

                diagram.append(sNewLine);

                diagram.append("@startuml").append(sNewLine);
                diagram.append(sNewLine);

                for(const auto& packagePair : packageMap)
                {
                    if(nullptr != packagePair.second)
                    {
                        const auto& pPackage = packagePair.second;

                        diagram.append("package \"").append(*pPackage->packageName.pString).append("\" ").append(sPackageColor).append(sNewLine);
                        diagram.append("{").append(sNewLine);
                        diagram.append(sNewLine);

                        auto appendMethods = [&diagram](const tMethodMap& methodMap)
                        {
                            for(const auto& methodPair : methodMap)
                            {
                                const auto& method = methodPair.second;

                                diagram.append(sIndentation).append( getAccessModifier(method.accessModifier) ).append(" ");

                                switch(method.methodType)
                                {
                                    case eMethodType::eUsual:
                                    {
                                        diagram.append(*method.method.pString).append(sNewLine);
                                    }
                                        break;
                                    case eMethodType::eStatic:
                                    {
                                        diagram.append("{static} ").append(*method.method.pString).append(sNewLine);
                                    }
                                        break;
                                    case eMethodType::eVirtual:
                                    {
                                        diagram.append("virtual ").append(*method.method.pString).append(sNewLine);
                                    }
                                        break;
                                    case eMethodType::eOverride:
                                    {
                                        diagram.append("virtual ").append(*method.method.pString).append(" override").append(sNewLine);
                                    }
                                        break;
                                    case eMethodType::ePureVirtual:
                                    {
                                        diagram.append("{abstract} virtual ").append(*method.method.pString).append(" = 0").append(sNewLine);
                                    }
                                        break;
                                }
                            }
                        };

                        for(const auto& itemPair : pPackage->itemMap)
                        {
                            if(nullptr != itemPair.second)
                            {
                                auto pItem = itemPair.second;

                                switch(pItem->getType())
                                {
                                    case eItemType::eClass:
                                    {
                                        diagram.append("class \"");
                                    }
                                        break;
                                    case eItemType::eInterface:
                                    {
                                        diagram.append("interface \"");
                                    }
                                        break;
                                }

                                diagram.append(*pItem->getItemName().pString).append("\"").append(sNewLine);
                                diagram.append("{").append(sNewLine);

                                appendMethods(pItem->getMethodMap());

                                diagram.append("}").append(sNewLine);
                                diagram.append(sNewLine);
                            }
                        }

                        diagram.append("}").append(sNewLine);
                    }

                    if(packageCounter != packageSize - 1)
                    {
                        diagram.append(sNewLine);
                    }

                    ++packageCounter;
                }

                bool bShouldPlaceHeader = true;

                for(const auto& packagePair : packageMap)
                {
                    if(nullptr != packagePair.second)
                    {
                        const auto& pPackage = packagePair.second;

                        auto appendInheritance = [&diagram](const tInheritanceMap& inheritanceMap, const std::string& className)
                        {
                            for(const auto& inheritancePair : inheritanceMap)
                            {
                                auto& pInheritance = inheritancePair.second;

                                if(nullptr != pInheritance)
                                {
                                    diagram.append(*pInheritance->baseClass.pString).append(" <|-- ").append(className);

                                    if(false == pInheritance->comment.empty())
                                    {
                                        diagram.append(" : ").append(pInheritance->comment);
                                    }

                                    diagram.append(sNewLine);
                                }
                            }
                        };

                        for(const auto& itemPair : pPackage->itemMap)
                        {
                            if(nullptr != itemPair.second)
                            {
                                auto pItem = itemPair.second;
                                if(eItemType::eClass == pItem->getType())
                                {
                                    if(false == pItem->getInheritanceMap().empty() && true == bShouldPlaceHeader)
                                    {
                                        diagram.append(sNewLine);
                                        diagram.append("\'====================Inheritance section====================");
                                        diagram.append(sNewLine);
                                        bShouldPlaceHeader = false;
                                    }

                                    appendInheritance(pItem->getInheritanceMap(), *pItem->getItemName().pString);
                                }
                            }
                        }
                    }
                }

                bShouldPlaceHeader = true;

                for(const auto& packagePair : packageMap)
                {
                    if(nullptr != packagePair.second)
                    {
                        const auto& pPackage = packagePair.second;

                        auto appendDependencies = [&diagram](const tDependencyMap& dependencyMap, const std::string& className)
                        {
                            for(const auto& dependencyPair : dependencyMap)
                            {
                                auto pDependency = dependencyPair.second;

                                diagram.append(className);

                                if(false == pDependency->fromNumber.empty())
                                {
                                    diagram.append(" \"").append( pDependency->fromNumber ).append("\"");
                                }

                                switch(pDependency->dependencyType)
                                {
                                    case eDependencyType::eComposition:
                                    {
                                        diagram.append(" *-- ");
                                    }
                                        break;
                                    case eDependencyType::eAggregation:
                                    {
                                        diagram.append(" o-- ");
                                    }
                                        break;
                                }

                                if(false == pDependency->toNumber.empty())
                                {
                                    diagram.append("\"").append( pDependency->toNumber ).append("\" ");
                                }

                                diagram.append(*pDependency->itemName.pString);

                                if(false == pDependency->comment.empty())
                                {
                                    diagram.append(" : ").append(pDependency->comment);
                                }

                                diagram.append(sNewLine);
                            }
                        };

                        for(const auto& itemPair : pPackage->itemMap)
                        {
                            if(nullptr != itemPair.second)
                            {
                                auto pItem = itemPair.second;

                                if(false == pItem->getDependencyMap().empty() && true == bShouldPlaceHeader)
                                {
                                    diagram.append(sNewLine);
                                    diagram.append("\'====================Dependencies section====================");
                                    diagram.append(sNewLine);
                                    bShouldPlaceHeader = false;
                                }
                                appendDependencies(pItem->getDependencyMap(), *pItem->getItemName().pString);
                            }
                        }
                    }
                }

                diagram.append(sNewLine).append("@enduml");
            }

            return diagram;
        }

        Creator::tClassDiagramResult Creator::getClassDiagram() const
        {
            tClassDiagramResult result;

            std::lock_guard<std::mutex> guard(*const_cast<std::mutex*>(&mDataProtector));

            result.diagramContent = getClassDiagramInternal(mPackageMap);
            result.bIsSuccessful = true;

            return result;
        }

        Creator::tClassDiagramResult Creator::getPackageClassDiagram(const std::string& packageName,
                                                                     bool excludeDependencies) const
        {
            tClassDiagramResult result;
            auto packageItemName = tStringPtrWrapper( std::make_shared<std::string>(packageName) );

            std::lock_guard<std::mutex> guard(*const_cast<std::mutex*>(&mDataProtector));

            auto foundPackage = mPackageMap.find(packageItemName);

            if(foundPackage != mPackageMap.end()) // package found
            {
                const auto& pRequestedPackage = foundPackage->second;

                // we should simply trace requested package, but in addition, add dependency
                // and dependent items of all of its elements from the other packages.

                tPackageMap dumpMap;

                tPackageDataPtr pPackageFiltered = std::make_shared<tPackageData>();
                pPackageFiltered->packageName = pRequestedPackage->packageName;

                for(const auto& itemPair : pRequestedPackage->itemMap)
                {
                    const auto& pItem = itemPair.second;

                    if(nullptr != pItem)
                    {
                        tIItemPtr pItemCopied = nullptr;

                        switch(pItem->getType())
                        {
                            case eItemType::eClass:
                                pItemCopied = std::make_shared<tClassData>();
                            break;
                            case eItemType::eInterface:
                                pItemCopied = std::make_shared<tInterfaceData>();
                            break;
                        }

                        pItemCopied->getParent() = pPackageFiltered;
                        pItemCopied->getItemName() = pItem->getItemName();
                        pItemCopied->getMethodMap() = pItem->getMethodMap();
                        pItemCopied->getDependentMap() = pItem->getDependentMap();
                        pItemCopied->getDependencyMap() = pItem->getDependencyMap();

                        if(eItemType::eClass == pItem->getType())
                        {
                            pItemCopied->getInheritanceMap() = pItem->getInheritanceMap();
                        }

                        pPackageFiltered->itemMap[pItemCopied->getItemName()] = pItemCopied;
                    }
                }

                // we add add it to map at the beginning, as this value in map might
                // be used within the loop.
                dumpMap[pPackageFiltered->packageName] = pPackageFiltered;

                if(true == excludeDependencies)
                {
                    // in this case let's exclude external dependencies from the package
                    for(const auto& itemPair : pPackageFiltered->itemMap)
                    {
                        const auto& pItem = itemPair.second;

                        if(nullptr != pItem)
                        {
                            auto& dependencyMap = pItem->getDependencyMap();

                            for(auto it = dependencyMap.begin(); it != dependencyMap.end(); )
                            {
                                auto& pDependency = it->second;

                                if(nullptr != pDependency)
                                {
                                    if(false == pDependency->pToItem.expired()
                                       && false == pDependency->pToItem.lock()->getParent().expired())
                                    {
                                        if(pDependency->pToItem.lock()->getParent().lock()->packageName !=
                                           pPackageFiltered->packageName)
                                        {
                                            it = dependencyMap.erase(it);
                                        }
                                        else
                                        {
                                            ++it;
                                        }
                                    }
                                    else
                                    {
                                        ++it;
                                    }
                                }
                                else
                                {
                                    ++it;
                                }
                            }

                            auto& dependentMap = pItem->getDependentMap();

                            for(auto it = dependentMap.begin(); it != dependentMap.end(); )
                            {
                                auto& pDependent = it->second;

                                if(false == pDependent.expired())
                                {
                                    if(false == pDependent.lock()->pToItem.expired()
                                       && false == pDependent.lock()->pToItem.lock()->getParent().expired())
                                    {
                                        if(pDependent.lock()->pToItem.lock()->getParent().lock()->packageName !=
                                           pPackageFiltered->packageName)
                                        {
                                            it = dependentMap.erase(it);
                                        }
                                        else
                                        {
                                            ++it;
                                        }
                                    }
                                    else
                                    {
                                        ++it;
                                    }
                                }
                                else
                                {
                                    ++it;
                                }
                            }
                        }
                    }
                }

                for(const auto& itemPair : pPackageFiltered->itemMap)
                {
                    const auto& pItem = itemPair.second;

                    if(nullptr != pItem)
                    {
                        if(false == excludeDependencies)
                        {
                            auto fillInDependency = [&dumpMap](const tDependencyMap& dependencyMap)
                            {
                                for(const auto& dependencyPair : dependencyMap)
                                {
                                    auto& pDependency = dependencyPair.second;

                                    if(nullptr != pDependency)
                                    {
                                        if(false == pDependency->pToItem.expired())
                                        {
                                            auto pToItem = pDependency->pToItem.lock();

                                            if(nullptr != pToItem && false == pToItem->getParent().expired())
                                            {
                                                auto pParentPackage = pToItem->getParent().lock();

                                                if(nullptr != pParentPackage)
                                                {
                                                    auto addFilteredItem = [&pToItem](const tPackageDataPtr& pDependencyPackage)
                                                    {
                                                        if(nullptr != pDependencyPackage)
                                                        {
                                                            auto foundItem = pDependencyPackage->itemMap.find(pToItem->getItemName());

                                                            if(foundItem == pDependencyPackage->itemMap.end())
                                                            {
                                                                tIItemPtr pDependencyItemFiltered_ = nullptr;

                                                                switch(pToItem->getType())
                                                                {
                                                                    case eItemType::eClass:
                                                                        pDependencyItemFiltered_ = std::make_shared<tClassData>();
                                                                    break;
                                                                    case eItemType::eInterface:
                                                                        pDependencyItemFiltered_ = std::make_shared<tInterfaceData>();
                                                                    break;
                                                                }

                                                                pDependencyItemFiltered_->getParent() = pDependencyPackage;
                                                                pDependencyItemFiltered_->getItemName() = pToItem->getItemName();
                                                                pDependencyItemFiltered_->getMethodMap() = pToItem->getMethodMap();

                                                                pDependencyPackage->itemMap[pDependencyItemFiltered_->getItemName()] = pDependencyItemFiltered_;
                                                            }
                                                        }
                                                    };

                                                    auto foundPackage_ = dumpMap.find(pParentPackage->packageName);

                                                    if(foundPackage_ != dumpMap.end())
                                                    {
                                                        auto pFoundPackage_ = foundPackage_->second;
                                                        auto& pDependencyPackage = foundPackage_->second;
                                                        addFilteredItem(pDependencyPackage);
                                                    }
                                                    else
                                                    {
                                                        tPackageDataPtr pDependencyPackage = std::make_shared<tPackageData>();
                                                        pDependencyPackage->packageName = pParentPackage->packageName;

                                                        addFilteredItem(pDependencyPackage);

                                                        dumpMap[pDependencyPackage->packageName] = pDependencyPackage;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            };

                            fillInDependency(pItem->getDependencyMap());

                            tDependencyMap dependentLockedMap;

                            for(const auto& dependentPair : pItem->getDependentMap())
                            {
                                auto& pDependentWeak = dependentPair.second;

                                if(false == pDependentWeak.expired())
                                {
                                    auto pDependent = pDependentWeak.lock();
                                    dependentLockedMap[pDependent->itemName] = pDependent;
                                }
                            }

                            fillInDependency(dependentLockedMap);
                        }

                        auto fillInInheritance = [&dumpMap](const tInheritanceMap& inheritanceMap)
                        {
                            for(const auto& inharitancePair : inheritanceMap)
                            {
                                auto& pInheritance = inharitancePair.second;

                                if(nullptr != pInheritance)
                                {
                                    if(false == pInheritance->pFromItem.expired())
                                    {
                                        auto pFromItem = pInheritance->pFromItem.lock();

                                        if(nullptr != pFromItem && false == pFromItem->getParent().expired())
                                        {
                                            auto pParentPackage = pFromItem->getParent().lock();

                                            if(nullptr != pParentPackage)
                                            {
                                                auto addFilteredItem = [&pFromItem](const tPackageDataPtr& pInheritancePackage)
                                                {
                                                    if(nullptr != pInheritancePackage)
                                                    {
                                                        auto foundItem = pInheritancePackage->itemMap.find(pFromItem->getItemName());

                                                        if(foundItem == pInheritancePackage->itemMap.end())
                                                        {
                                                            tIItemPtr pInheritanceItemFiltered_ = nullptr;

                                                            switch(pFromItem->getType())
                                                            {
                                                                case eItemType::eClass:
                                                                    pInheritanceItemFiltered_ = std::make_shared<tClassData>();
                                                                break;
                                                                case eItemType::eInterface:
                                                                    pInheritanceItemFiltered_ = std::make_shared<tInterfaceData>();
                                                                break;
                                                            }

                                                            pInheritanceItemFiltered_->getParent() = pInheritancePackage;
                                                            pInheritanceItemFiltered_->getItemName() = pFromItem->getItemName();
                                                            pInheritanceItemFiltered_->getMethodMap() = pFromItem->getMethodMap();

                                                            pInheritancePackage->itemMap[pInheritanceItemFiltered_->getItemName()] = pInheritanceItemFiltered_;
                                                        }
                                                    }
                                                };

                                                auto foundPackage_ = dumpMap.find(pParentPackage->packageName);

                                                if(foundPackage_ != dumpMap.end())
                                                {
                                                    auto pFoundPackage_ = foundPackage_->second;
                                                    auto& pInheritancePackage = foundPackage_->second;
                                                    addFilteredItem(pInheritancePackage);
                                                }
                                                else
                                                {
                                                    tPackageDataPtr pInheritancePackage = std::make_shared<tPackageData>();
                                                    pInheritancePackage->packageName = pParentPackage->packageName;

                                                    addFilteredItem(pInheritancePackage);

                                                    dumpMap[pInheritancePackage->packageName] = pInheritancePackage;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        };

                        fillInInheritance(pItem->getInheritanceMap());
                    }
                }

                result.diagramContent = getClassDiagramInternal(dumpMap);
                result.bIsSuccessful = true;
            }
            else
            {
                result.error.append("Error: package with name \"").append(packageName).append("\" was not found!");
            }

            return result;
        }

        tStringPtrSet Creator::findPackagesByName( const std::string& packageName ) const
        {
            tStringPtrSet result;

            {
                auto toLowerCandidate = packageName;
                std::transform(toLowerCandidate.begin(), toLowerCandidate.end(), toLowerCandidate.begin(),
                               [](unsigned char c){ return std::tolower(c); });

                std::lock_guard<std::mutex> guard(*const_cast<std::mutex*>(&mDataProtector));

                for(const auto& packagePair : mPackageMap)
                {
                    if(nullptr != packagePair.first.pString)
                    {
                        auto toLowerPackageName = *packagePair.second->packageName.pString;
                        std::transform(toLowerPackageName.begin(), toLowerPackageName.end(), toLowerPackageName.begin(),
                                       [](unsigned char c){ return std::tolower(c); });

                        if(toLowerPackageName.rfind(toLowerCandidate, 0) == 0)
                        {
                            result.insert(packagePair.first.pString);
                        }
                    }
                }
            }

            return result;
        }

        void Creator::addItem( const tItemName& packageName, const tIItemPtr& pItemData )
        {
            if(nullptr != pItemData)
            {
                std::lock_guard<std::mutex> guard(mDataProtector);

                auto foundPackage = mPackageMap.find(packageName);

                if(foundPackage != mPackageMap.end())
                {
                    pItemData->getParent() = foundPackage->second;
                    foundPackage->second->itemMap[pItemData->getItemName()] = pItemData;
                }
                else
                {
                    tPackageDataPtr pPackageData = std::make_shared<tPackageData>();
                    pItemData->getParent() = pPackageData;
                    pPackageData->itemMap[pItemData->getItemName()] = pItemData;
                    pPackageData->packageName = packageName;
                    mPackageMap[pPackageData->packageName] = pPackageData;
                }

                mItemRegistry[pItemData->getItemName()] = pItemData;
            }
        }
        ////////////////////////////////////////////////////////////////

        /////////////////////////////Item///////////////////////////////
        IItem::~IItem(){}
        /////////////////////////////Item ( end )///////////////////////

        /////////////////////////////tBaseData//////////////////////////
        tItemName& tBaseData::getItemName()
        {
            return itemName;
        }

        tDependencyMap& tBaseData::getDependencyMap()
        {
            return dependencyMap;
        }

        tInheritanceMap& tBaseData::getInheritanceMap()
        {
            static tInheritanceMap inheritanceMap;
            return inheritanceMap;
        }

        tDependencyWeakMap& tBaseData::getDependentMap()
        {
            return dependentMap;
        }

        tPackageDataWeakPtr& tBaseData::getParent()
        {
            return pParent;
        }

        tMethodMap& tBaseData::getMethodMap()
        {
            return methodMap;
        }
        /////////////////////////////tBaseData ( end )//////////////////

        /////////////////////////////tClassData/////////////////////////
        const eItemType& tClassData::getType() const
        {
            static const eItemType res = eItemType::eClass;
            return res;
        }

        tInheritanceMap& tClassData::getInheritanceMap()
        {
            return inheritanceMap;
        }
        /////////////////////////////tClassData ( end )/////////////////

        /////////////////////////////tInterfaceData/////////////////////
        const eItemType& tInterfaceData::getType() const
        {
            static const eItemType res = eItemType::eInterface;
            return res;
        }
        /////////////////////////////tInterfaceData ( end )/////////////

        /////////////////////////////Other//////////////////////////////
        tCallOnCreate::tCallOnCreate( const std::function<void(void)>& callable )
        {
            if(callable)
            {
                callable();
            }
        }

        tMethodData::tMethodData():
        accessModifier("+"),
        method(),
        methodType(eMethodType::eUsual)
        {}

        tMethodData::tMethodData(const tAccessModifier& accessModifier_,
                    const std::string& method_,
                    const eMethodType& methodType_):
        accessModifier(accessModifier_),
        method(tStringPtrWrapper(std::make_shared<std::string>(method_))),
        methodType(methodType_)
        {}

        tDependencyData::tDependencyData():
        dependencyType(eDependencyType::eComposition),
        itemName(),
        pToItem(),
        fromNumber(),
        toNumber(),
        comment()
        {}

        tDependencyData::tDependencyData(const eDependencyType& dependencyType_,
                        const std::string& itemName_,
                        const tIItemPtr& pToItem_,
                        const std::string& fromNumber_,
                        const std::string& toNumber_,
                        const std::string& comment_):
        dependencyType(dependencyType_),
        itemName(tStringPtrWrapper(std::make_shared<std::string>(itemName_))),
        pToItem(pToItem_),
        fromNumber(fromNumber_),
        toNumber(toNumber_),
        comment(comment_)
        {}
        /////////////////////////////Other ( end )//////////////////////
    }
}
