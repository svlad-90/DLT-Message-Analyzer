/**
 * @file    DMA_Plantuml.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the DMA_Plantuml concept
 */
#ifndef DMA_COMPONENTS_IBASICITEM_HPP
#define DMA_COMPONENTS_IBASICITEM_HPP

/**
 *
 * Module:   DMA_Plantuml
 * Version:  1.0.2
 * Author:   Vladyslav Goncharuk ( svlad1990@gmail.com )
 *
 * ////////////////////////////////////////////////////////////////////////////
 *
 * Brief
 *
 * Where am I?
 *
 * You are inside the header of the DMA_Plantuml concept - the poor man's C++
 * to platuml converter
 *
 * What it does? It allows you to declare a metadata for your classes, which is
 * then allows your application to produce plantuml class diagrams regarding
 * its own entities.
 *
 * Motivation:
 *
 * Firstly, as a developer I want documentation of my source code to be located
 * as near as possible to the code itself. Additionally, I do not want to
 * maintain multiple non-connected diagrams, but rather a one single model.
 *
 * Secondly, I do not want to use existing tools, which are generating the
 * diagrams during the build. Integration of such tools usually needs a lot of
 * efforts to configure them properly, and, at final point, the result diagram
 * is representing either the whole application ( which is overloaded with
 * classes ), or only a single class ( which is not informative ).
 *
 * Thirdly, when user states, that there is an issue in some version of the SW,
 * I want to be able to get class diagrams from the SW itself, without
 * referring to a specific base-line in git.
 *
 * Fourthly, from time to time I want to be able to review and adjust the
 * design of my application. When I'm doing this, I want to be able to
 * instantly see the result of code modification in a diagram, without the
 * need to refer to a thirdparty tools or files.
 *
 * Fifthly, currently existing plugins for IDE-s, which allow to visulize plantuml
 * comments in the source code are not sufficient, as I want to review not only
 * the diagram of a specific single class, but also connections between
 * multiple classes, interfaces, packages, etc. Current concept supports
 * filtering out the diagram based on certain package. You are able to request
 * creation of diagram for the specified package and its nearest connections.
 *
 * As of now, this concept is manual, but, for sure, later on it can be used as
 * a core module for a generator, which will produce the metadata macro-
 * definitions on the fly.
 *
 * General idea of this concept is to allow you to have UML data ( mainly class
 * diagrams ) of your app at your fingertips. From my previous experience, the
 * more overhead you have to create the documentation, the poorer it would be,
 * mainly due to lack of capacity to support it. With this small lib I try to
 * have a proper tool, which I will use at least for my own projects ☺
 *
 * //////////////////////////////////////////////////////
 *
 * How to use it?
 *
 * Step 1 ( optional ). Declare some class hierarchy, for which you will
 * declare the plantuml metadata afterwards.
 *
 * //////////////////////////////////////////////////////
 *
 * Step 2 ( mandatory ). In your cpp(s) add declarations of your classes, using
 * provided macro-based API.
 *
 * Example:
 *
 * >  PUML_PACKAGE_BEGIN(test_main)
 * >      PUML_INTERFACE_BEGIN(IInterface)
 * >          PUML_VIRTUAL_METHOD(+, ~IInterface())
 * >          PUML_VIRTUAL_METHOD(+, void virtFunc() )
 * >          PUML_PURE_VIRTUAL_METHOD(+, void pureVirtFunc(const int& val1, const int& val2) )
 * >      PUML_INTERFACE_END()
 * >
 * >      PUML_CLASS_BEGIN(CImpl_Test)
 * >          PUML_INHERITANCE(IInterface, implements)
 * >          PUML_INHERITANCE(IInterfaceHelper, implements)
 * >          PUML_INHERITANCE(UndeclaredClass, extends)
 * >          PUML_METHOD(+, CImpl_Test())
 * >          PUML_OVERRIDE_METHOD(+, void virtFunc() )
 * >          PUML_OVERRIDE_METHOD(+, void pureVirtFunc(const int& val1, const int& val2) )
 * >          PUML_STATIC_METHOD(+, void myStaticMethod() )
 * >          PUML_COMPOSITION_DEPENDENCY(CompositionDependency, 1, 1, contains)
 * >          PUML_AGGREGATION_DEPENDENCY(AggregationDependency, 1, 1, uses)
 * >      PUML_CLASS_END()
 * >  PUML_PACKAGE_END()
 * >
 * >  PUML_PACKAGE_BEGIN(test_helper)
 * >
 * >      PUML_INTERFACE_BEGIN(IInterfaceHelper)
 * >      PUML_INTERFACE_END()
 * >
 * >      PUML_INTERFACE_BEGIN(IDependency)
 * >      PUML_INTERFACE_END()
 * >
 * >      PUML_CLASS_BEGIN(CompositionDependency)
 * >          PUML_INHERITANCE(IDependency, implements)
 * >          PUML_AGGREGATION_DEPENDENCY(ExternalDependency, 1, 1, uses)
 * >      PUML_CLASS_END()
 * >
 * >      PUML_CLASS_BEGIN(AggregationDependency)
 * >          PUML_INHERITANCE(IDependency, implements)
 * >          PUML_AGGREGATION_DEPENDENCY(ExternalDependency, 1, 1, uses)
 * >      PUML_CLASS_END()
 * >
 * >      PUML_CLASS_BEGIN(CImpl_Helper)
 * >          PUML_INHERITANCE(IInterface, implements)
 * >          PUML_METHOD(+, CImpl_Helper())
 * >          PUML_OVERRIDE_METHOD(+, void virtFunc() )
 * >          PUML_OVERRIDE_METHOD(+, void pureVirtFunc(const int& val1, const int& val2) )
 * >          PUML_STATIC_METHOD(+, void myStaticMethod() )
 * >      PUML_CLASS_END()
 * >
 * >      PUML_CLASS_BEGIN(CImpl_HelperExtended)
 * >          PUML_INHERITANCE(CImpl_Helper, extends)
 * >          PUML_METHOD(+, CImpl_HelperExtended())
 * >          PUML_OVERRIDE_METHOD(+, void virtFunc() )
 * >      PUML_CLASS_END()
 * >
 * >  PUML_PACKAGE_END()
 * >
 * >  PUML_PACKAGE_BEGIN(test_external)
 * >      PUML_CLASS_BEGIN(ExternalDependency)
 * >      PUML_CLASS_END()
 * >  PUML_PACKAGE_END()
 *
 * Some rules and tips regarding the macro definitions:
 * - Package is mandatory. You can not declare anything outside the package.
 * You can try but you will get a compilcaiton error, as macro definitions are
 * created to work only in that way. Otherwise produced C++ code won't be valid.
 * That is my shoutout to the component architecture. Declare packages!
 * - You can declare the same package in different files. E.g. if you have a
 * package with dozens of files there is no need to have a seperate cpp in
 * order to store the whole metadata of the package there. No! Idea is to
 * declare metadata of the specific entities in their own cpp files. Even if
 * multiple classes are related to the same package.
 * - From the other side, you can't split the definition of metadata for the
 * classes and interfaces. There should be only one definition of entity with
 * its unique name within a package. In case of multiple definitions of an
 * element the "last win" strategy will be applied. So you can get partial
 * data ☺
 * - both class and interface do have: methods, inheritance and dependencies
 * - All parameters of the macro definitions should be placed without the
 * quotes. Stringification will be done inside the macro definitions
 * - Concept does contain a primitive type check mechanism. Refer to X_CHECKED
 * macro definitions. It's usage is optional, so, if needed, you can add to
 * your design even non-yet-existing classes.
 * - It is better to specify class names with namespaces in order to avoid
 * collisions between the entities. E.g. dependencies and inheritance lookup
 * is using names without considering name of the packages. Thus, if you will
 * have the package_1::class_1 and package_2::class_1, then dependency to
 * class_1 might lead to ambiguation, when random data will be chosen.
 * - All internal search functionality is CASE SENSITIVE. Thus, metadata
 * declarations should also consider this. Only Creator::findPackagesByName
 * method does a case-insensitive search of packages.
 * - Be aware, that you will need a graphwiz installed in order to get plantuml
 * class diagrams. It is better to install version 2.38 of it, as plantuml
 * documentation states, that currently it is the most compatible release.
 * - There is NO support of the nested packages, interfaces or classes. Only
 * package->class and package->interface folding.
 * - keywords virtual, override and "=0" should NOT be added together with
 * the method's definition. They will be added implicitly.
 * - Concept supports usage of template classes. But be aware, that X_CHECKED
 * macro definitions will not fully work with it. TClass<int> might work, while
 * TClass<T> ( where T is abstract template parameter ) definitely won't.
 * - Concept supports declaration of singletone and abstract classes
 *
 * //////////////////////////////////////////////////////
 *
 * Step 3 ( mandatory ). Somewhere in your code ( in non-global section ) call:
 *
 * DMA::PlantUML::Creator::getInstance().initialize();
 *
 * That will finish initialization of the model.
 *
 * //////////////////////////////////////////////////////
 *
 * Step 4. To obtain the full class diagram of your application, do the
 * following thing in your source code:
 *
 *  > auto diagramResult = DMA::PlantUML::Creator::getInstance().getClassDiagram();
 *  >
 *  > if(true == diagramResult.bIsSuccessful)
 *  > {
 *  >     // do something with diagramResult.diagramContent
 *  > }
 *  > else
 *  > {
 *  >     // dump diagramResult.error
 *  > }
 * //////////////////////////////////////////////////////
 *
 * Step 5. To obtain partial class diagram of the specific package, do the
 * following thing in your source code:
 *
 * >  auto diagramResult = DMA::PlantUML::Creator::getInstance().getPackageClassDiagram("test");
 * >
 * >  if(true == diagramResult.bIsSuccessful)
 * >  {
 * >      // do something with diagramResult.diagramContent
 * >  }
 * >  else
 * >  {
 * >      // dump diagramResult.error
 * >  }
 *
 * //////////////////////////////////////////////////////
 *
 * Step 6 ( mandatory ). Add the following define to your build:
 *
 *  > #define PUML_ENABLED
 *
 * Without it, all macro definitions which you've used will produce nothing.
 * Sometimes ( in release builds ) this define can be turned off by purpose in
 * order to avoid runtime overheads.
 *
 * Example of resulting diagram, which is based on the model from step 2:
 *
 * >  @startuml
 * >
 * >  package "test_external" #DDDDDD
 * >  {
 * >
 * >  class "ExternalDependency"
 * >  {
 * >  }
 * >
 * >  }
 * >
 * >  package "test_helper" #DDDDDD
 * >  {
 * >
 * >  class "AggregationDependency"
 * >  {
 * >  }
 * >
 * >  class "CImpl_Helper"
 * >  {
 * >      + CImpl_Helper()
 * >      + {static} void myStaticMethod()
 * >      + virtual void pureVirtFunc(const int& val1, const int& val2) override
 * >      + virtual void virtFunc() override
 * >  }
 * >
 * >  class "CImpl_HelperExtended"
 * >  {
 * >      + CImpl_HelperExtended()
 * >      + virtual void virtFunc() override
 * >  }
 * >
 * >  class "CompositionDependency"
 * >  {
 * >  }
 * >
 * >  interface "IDependency"
 * >  {
 * >  }
 * >
 * >  interface "IInterfaceHelper"
 * >  {
 * >  }
 * >
 * >  }
 * >
 * >  package "test_main" #DDDDDD
 * >  {
 * >
 * >  class "CImpl_Test"
 * >  {
 * >      + CImpl_Test()
 * >      + {static} void myStaticMethod()
 * >      + virtual void pureVirtFunc(const int& val1, const int& val2) override
 * >      + virtual void virtFunc() override
 * >  }
 * >
 * >  interface "IInterface"
 * >  {
 * >      + {abstract} virtual void pureVirtFunc(const int& val1, const int& val2) = 0
 * >      + virtual void virtFunc()
 * >      + virtual ~IInterface()
 * >  }
 * >
 * >  }
 * >
 * >  '====================Inheritance section====================
 * >  IDependency <|-- AggregationDependency : implements
 * >  IInterface <|-- CImpl_Helper : implements
 * >  CImpl_Helper <|-- CImpl_HelperExtended : extends
 * >  IDependency <|-- CompositionDependency : implements
 * >  IInterface <|-- CImpl_Test : implements
 * >  IInterfaceHelper <|-- CImpl_Test : implements
 * >  UndeclaredClass <|-- CImpl_Test : extends
 * >
 * >  '====================Dependencies section====================
 * >  AggregationDependency "1" o-- "1" ExternalDependency : uses
 * >  CompositionDependency "1" o-- "1" ExternalDependency : uses
 * >  CImpl_Test "1" o-- "1" AggregationDependency : uses
 * >  CImpl_Test "1" *-- "1" CompositionDependency : contains
 * >
 * >  @enduml
 *
 * For more complex examples, please, visit the following project:
 * https://github.com/svlad-90/DLT-Message-Analyzer
 * Its class-diagrams documentation is fully based on usage of this library.
 * Search for usage of this API within that repository, and you will get how to
 * deal with it.
 */

//////////// dependencies to the standard library ////////////
#include <string>
#include <map>
#include <vector>
#include <set>
#include <memory>
#include <mutex>
#include <functional>
#include <type_traits>
//////////// dependencies to the standard library ( end ) ////

#define COMBINE1(X,Y) X##Y  // helper macro
#define COMBINE(X,Y) COMBINE1(X,Y)

/////////////////////////////////////////////////////////////////////
//////////////////////////////MACRO API//////////////////////////////
/////////////////////////////////////////////////////////////////////

/*
 * In case if PUML_ENABLED is not defined, all macro definitions will produce
 * nothing. That can be used to turn off plantuml functionality in the release
 * builds.
 */
#ifndef PUML_ENABLED

#define PUML_PACKAGE_BEGIN( PACKAGE_NAME )
#define PUML_PACKAGE_END()
#define PUML_INTERFACE_BEGIN( INTERFACE_NAME )
#define PUML_INTERFACE_BEGIN_CHECKED( INTERFACE_NAME )
#define PUML_INTERFACE_END()
#define PUML_CLASS_BEGIN( CLASS_NAME )
#define PUML_CLASS_BEGIN_CHECKED( CLASS_NAME )
#define PUML_CLASS_END()
#define PUML_ABSTRACT_CLASS_BEGIN( INTERFACE_NAME )
#define PUML_ABSTRACT_CLASS_BEGIN_CHECKED( INTERFACE_NAME )
#define PUML_ABSTRACT_CLASS_END()
#define PUML_SINGLETONE_BEGIN( CLASS_NAME )
#define PUML_SINGLETONE_BEGIN_CHECKED( CLASS_NAME )
#define PUML_SINGLETONE_END()
#define PUML_VIRTUAL_METHOD( ACCESS_MODIFIER, METHOD )
#define PUML_PURE_VIRTUAL_METHOD( ACCESS_MODIFIER, METHOD )
#define PUML_METHOD( ACCESS_MODIFIER, METHOD )
#define PUML_OVERRIDE_METHOD( ACCESS_MODIFIER, METHOD )
#define PUML_STATIC_METHOD( ACCESS_MODIFIER, METHOD )
#define PUML_INHERITANCE( BASE_CLASS, COMMENT )
#define PUML_INHERITANCE_CHECKED( BASE_CLASS, COMMENT )
#define PUML_COMPOSITION_DEPENDENCY( ITEM, USING_NUMBER, USED_NUMBER, COMMENT )
#define PUML_COMPOSITION_DEPENDENCY_CHECKED( ITEM, USING_NUMBER, USED_NUMBER, COMMENT )
#define PUML_AGGREGATION_DEPENDENCY( ITEM, USING_NUMBER, USED_NUMBER, COMMENT )
#define PUML_AGGREGATION_DEPENDENCY_CHECKED( ITEM, USING_NUMBER, USED_NUMBER, COMMENT )
#define PUML_USE_DEPENDENCY( ITEM, USING_NUMBER, USED_NUMBER, COMMENT )
#define PUML_USE_DEPENDENCY_CHECKED( ITEM, USING_NUMBER, USED_NUMBER, COMMENT )

#else

#define PUML_PACKAGE_BEGIN( PACKAGE_NAME )\
static DMA::PlantUML::tCallOnCreate COMBINE(PACKAGE_NAME##_##PackageRegistration_, __LINE__)\
([](){\
\
DMA::PlantUML::tStringPtrWrapper pPackageName = std::make_shared<std::string>(#PACKAGE_NAME);\

#define PUML_PACKAGE_END()\
});

#define PUML_INTERFACE_BEGIN( INTERFACE_NAME )\
{\
    int interfaceCheck = 0;\
    DMA::PlantUML::tInterfaceDataPtr pItem = std::make_shared<DMA::PlantUML::tInterfaceData>();\
    pItem->name.setItemName( DMA::PlantUML::tStringPtrWrapper( std::make_shared<std::string>(#INTERFACE_NAME) ) );

#define PUML_INTERFACE_BEGIN_CHECKED( INTERFACE_NAME )\
{\
    int interfaceCheck = 0;\
    static_assert (std::is_class<INTERFACE_NAME>::value, "[PUML_INTERFACE_BEGIN_CHECKED] Provided type should be an existing class or structure");\
    DMA::PlantUML::tInterfaceDataPtr pItem = std::make_shared<DMA::PlantUML::tInterfaceData>();\
    pItem->name.setItemName( DMA::PlantUML::tStringPtrWrapper( std::make_shared<std::string>(#INTERFACE_NAME) ) );

#define PUML_INTERFACE_END()\
    DMA::PlantUML::Creator::getInstance().addItem(pPackageName, pItem);\
    static_cast<void>(interfaceCheck);\
}

#define PUML_CLASS_BEGIN( CLASS_NAME )\
{\
    int classCheck = 0;\
    DMA::PlantUML::tClassDataPtr pItem = std::make_shared<DMA::PlantUML::tClassData>();\
    pItem->name.setItemName( DMA::PlantUML::tStringPtrWrapper( std::make_shared<std::string>(#CLASS_NAME) ) );

#define PUML_CLASS_BEGIN_CHECKED( CLASS_NAME )\
{\
    int classCheck = 0;\
    static_assert (std::is_class<CLASS_NAME>::value, "[PUML_CLASS_BEGIN_CHECKED] Provided type should be an existing class or structure");\
    DMA::PlantUML::tClassDataPtr pItem = std::make_shared<DMA::PlantUML::tClassData>();\
    pItem->name.setItemName( DMA::PlantUML::tStringPtrWrapper( std::make_shared<std::string>(#CLASS_NAME) ) );

#define PUML_CLASS_END()\
    DMA::PlantUML::Creator::getInstance().addItem(pPackageName, pItem);\
    static_cast<void>(classCheck);\
}

#define PUML_SINGLETONE_BEGIN( CLASS_NAME )\
{\
    int singletoneCheck = 0;\
    DMA::PlantUML::tSingletoneDataPtr pItem = std::make_shared<DMA::PlantUML::tSingletoneData>();\
    pItem->name.setItemName( DMA::PlantUML::tStringPtrWrapper( std::make_shared<std::string>(#CLASS_NAME) ) );

#define PUML_SINGLETONE_BEGIN_CHECKED( CLASS_NAME )\
{\
    int singletoneCheck = 0;\
    static_assert (std::is_class<CLASS_NAME>::value, "[PUML_SINGLETONE_BEGIN_CHECKED] Provided type should be an existing class or structure");\
    DMA::PlantUML::tSingletoneDataPtr pItem = std::make_shared<DMA::PlantUML::tSingletoneData>();\
    pItem->name.setItemName( DMA::PlantUML::tStringPtrWrapper( std::make_shared<std::string>(#CLASS_NAME) ) );

#define PUML_SINGLETONE_END()\
    DMA::PlantUML::Creator::getInstance().addItem(pPackageName, pItem);\
    static_cast<void>(singletoneCheck);\
}

#define PUML_ABSTRACT_CLASS_BEGIN( INTERFACE_NAME )\
{\
    int abstractClassCheck = 0;\
    DMA::PlantUML::tAbstractClassDataPtr pItem = std::make_shared<DMA::PlantUML::tAbstractClassData>();\
    pItem->name.setItemName( DMA::PlantUML::tStringPtrWrapper( std::make_shared<std::string>(#INTERFACE_NAME) ) );

#define PUML_ABSTRACT_CLASS_BEGIN_CHECKED( INTERFACE_NAME )\
{\
    int abstractClassCheck = 0;\
    static_assert (std::is_class<INTERFACE_NAME>::value, "[PUML_ABSTRACT_CLASS_BEGIN_CHECKED] Provided type should be an existing class or structure");\
    DMA::PlantUML::tAbstractClassDataPtr pItem = std::make_shared<DMA::PlantUML::tAbstractClassData>();\
    pItem->name.setItemName( DMA::PlantUML::tStringPtrWrapper( std::make_shared<std::string>(#INTERFACE_NAME) ) );

#define PUML_ABSTRACT_CLASS_END()\
    DMA::PlantUML::Creator::getInstance().addItem(pPackageName, pItem);\
    static_cast<void>(abstractClassCheck);\
}

#define PUML_VIRTUAL_METHOD( ACCESS_MODIFIER, METHOD )\
{\
    DMA::PlantUML::tMethodData methodData( #ACCESS_MODIFIER, #METHOD, DMA::PlantUML::eMethodType::eVirtual );\
    pItem->methodMap[methodData.method] = methodData;\
}

#define PUML_PURE_VIRTUAL_METHOD( ACCESS_MODIFIER, METHOD )\
{\
    DMA::PlantUML::tMethodData methodData( #ACCESS_MODIFIER, #METHOD, DMA::PlantUML::eMethodType::ePureVirtual );\
    pItem->methodMap[methodData.method] = methodData;\
}

#define PUML_METHOD( ACCESS_MODIFIER, METHOD )\
{\
    DMA::PlantUML::tMethodData methodData( #ACCESS_MODIFIER, #METHOD, DMA::PlantUML::eMethodType::eUsual );\
    pItem->methodMap[methodData.method] = methodData;\
}

#define PUML_OVERRIDE_METHOD( ACCESS_MODIFIER, METHOD )\
{\
    DMA::PlantUML::tMethodData methodData( #ACCESS_MODIFIER, #METHOD, DMA::PlantUML::eMethodType::eOverride );\
    pItem->methodMap[methodData.method] = methodData;\
}

#define PUML_STATIC_METHOD( ACCESS_MODIFIER, METHOD )\
{\
    DMA::PlantUML::tMethodData methodData( #ACCESS_MODIFIER, #METHOD, DMA::PlantUML::eMethodType::eStatic );\
    pItem->methodMap[methodData.method] = methodData;\
}

#define PUML_INHERITANCE( BASE_CLASS, COMMENT )\
{\
    DMA::PlantUML::tInheritanceDataPtr pInheritanceData = std::make_shared<DMA::PlantUML::tInheritanceData>();\
    pInheritanceData->comment = #COMMENT;\
    pInheritanceData->baseClass.setItemName( DMA::PlantUML::tStringPtrWrapper( std::make_shared<std::string>(#BASE_CLASS) ) );\
    pItem->inheritanceMap[pInheritanceData->baseClass.getItemName()] = pInheritanceData;\
}

#define PUML_INHERITANCE_CHECKED( BASE_CLASS, COMMENT )\
{\
    static_assert (std::is_class<BASE_CLASS>::value, "[PUML_INHERITANCE_CHECKED] Provided type should be an existing class or structure");\
    DMA::PlantUML::tInheritanceDataPtr pInheritanceData = std::make_shared<DMA::PlantUML::tInheritanceData>();\
    pInheritanceData->comment = #COMMENT;\
    pInheritanceData->baseClass.setItemName( DMA::PlantUML::tStringPtrWrapper( std::make_shared<std::string>(#BASE_CLASS) ) );\
    pItem->inheritanceMap[pInheritanceData->baseClass.getItemName()] = pInheritanceData;\
}

#define PUML_COMPOSITION_DEPENDENCY( ITEM, USING_NUMBER, USED_NUMBER, COMMENT )\
{\
    DMA::PlantUML::tDependencyDataPtr pDependencyData =\
    std::make_shared<DMA::PlantUML::tDependencyData>(DMA::PlantUML::eDependencyType::eComposition,\
    #ITEM,\
    nullptr,\
    #USING_NUMBER,\
    #USED_NUMBER,\
    #COMMENT);\
\
    pItem->dependencyMap[pDependencyData->name.getItemName()] = pDependencyData;\
}

#define PUML_COMPOSITION_DEPENDENCY_CHECKED( ITEM, USING_NUMBER, USED_NUMBER, COMMENT )\
{\
    static_assert (std::is_class<ITEM>::value, "[PUML_COMPOSITION_DEPENDENCY_CHECKED] Provided type should be an existing class or structure");\
    DMA::PlantUML::tDependencyDataPtr pDependencyData =\
    std::make_shared<DMA::PlantUML::tDependencyData>(DMA::PlantUML::eDependencyType::eComposition,\
    #ITEM,\
    nullptr,\
    #USING_NUMBER,\
    #USED_NUMBER,\
    #COMMENT);\
\
    pItem->dependencyMap[pDependencyData->name.getItemName()] = pDependencyData;\
}

#define PUML_AGGREGATION_DEPENDENCY( ITEM, USING_NUMBER, USED_NUMBER, COMMENT )\
{\
    DMA::PlantUML::tDependencyDataPtr pDependencyData =\
    std::make_shared<DMA::PlantUML::tDependencyData>(DMA::PlantUML::eDependencyType::eAggregation,\
    #ITEM,\
    nullptr,\
    #USING_NUMBER,\
    #USED_NUMBER,\
    #COMMENT);\
\
    pItem->dependencyMap[pDependencyData->name.getItemName()] = pDependencyData;\
}

#define PUML_AGGREGATION_DEPENDENCY_CHECKED( ITEM, USING_NUMBER, USED_NUMBER, COMMENT )\
{\
    static_assert (std::is_class<ITEM>::value, "[PUML_AGGREGATION_DEPENDENCY_CHECKED] Provided type should be an existing class or structure");\
    DMA::PlantUML::tDependencyDataPtr pDependencyData =\
    std::make_shared<DMA::PlantUML::tDependencyData>(DMA::PlantUML::eDependencyType::eAggregation,\
    #ITEM,\
    nullptr,\
    #USING_NUMBER,\
    #USED_NUMBER,\
    #COMMENT);\
\
    pItem->dependencyMap[pDependencyData->name.getItemName()] = pDependencyData;\
}

#define PUML_USE_DEPENDENCY( ITEM, USING_NUMBER, USED_NUMBER, COMMENT )\
{\
    DMA::PlantUML::tDependencyDataPtr pDependencyData =\
    std::make_shared<DMA::PlantUML::tDependencyData>(DMA::PlantUML::eDependencyType::eUse,\
    #ITEM,\
    nullptr,\
    #USING_NUMBER,\
    #USED_NUMBER,\
    #COMMENT);\
\
    pItem->dependencyMap[pDependencyData->getItemName()] = pDependencyData;\
}

#define PUML_USE_DEPENDENCY_CHECKED( ITEM, USING_NUMBER, USED_NUMBER, COMMENT )\
{\
    static_assert (std::is_class<ITEM>::value, "[PUML_USE_DEPENDENCY_CHECKED] Provided type should be an existing class or structure");\
    DMA::PlantUML::tDependencyDataPtr pDependencyData =\
    std::make_shared<DMA::PlantUML::tDependencyData>(DMA::PlantUML::eDependencyType::eUse,\
    #ITEM,\
    nullptr,\
    #USING_NUMBER,\
    #USED_NUMBER,\
    #COMMENT);\
\
    pItem->dependencyMap[pDependencyData->name.getItemName()] = pDependencyData;\
}

#endif

namespace DMA
{
    namespace PlantUML
    {
        typedef std::shared_ptr<std::string> tStringPtr;
        typedef std::set<tStringPtr> tStringPtrSet;

        struct tStringPtrWrapper
        {
            tStringPtrWrapper();
            tStringPtrWrapper(const tStringPtr& pString_);
            bool operator== ( const tStringPtrWrapper& rVal ) const;
            bool operator!= ( const tStringPtrWrapper& rVal ) const;
            bool operator< ( const tStringPtrWrapper& rVal ) const;
            tStringPtr pString = nullptr;
        };

        typedef tStringPtrWrapper tItemName;

        struct tName
        {
            const tItemName& getItemName() const;
            const tItemName& getTemplateAlias() const;
            void setItemName(const tItemName& val);

        private:
            tItemName itemName;
            tItemName templateAlias;
        };

        typedef std::string tComment;

        // forward declarations
        class IItem;
        typedef std::shared_ptr<IItem> tIItemPtr;
        typedef std::weak_ptr<IItem> tIItemWeakPtr;
        typedef std::map<tItemName, tIItemPtr> tItemMap;
        struct tPackageData;
        typedef std::shared_ptr<tPackageData> tPackageDataPtr;
        typedef std::weak_ptr<tPackageData> tPackageDataWeakPtr;
        // forward declarations ( end )

        enum class eDependencyType
        {
            eComposition = 0,
            eAggregation,
            eUse
        };

        struct tDependencyData
        {
            tDependencyData();
            tDependencyData(const eDependencyType& dependencyType_,
                            const std::string& itemName_,
                            const tIItemPtr& pToItem_,
                            const std::string& fromNumber_,
                            const std::string& toNumber_,
                            const std::string& comment_);
            eDependencyType dependencyType;
            tName name;
            tIItemWeakPtr pToItem;
            std::string fromNumber;
            std::string toNumber;
            std::string comment;
        };

        typedef std::shared_ptr<tDependencyData> tDependencyDataPtr;
        typedef std::weak_ptr<tDependencyData> tDependencyDataWeakPtr;

        typedef std::map<tItemName, tDependencyDataPtr> tDependencyMap;

        struct tInheritanceData
        {
            tName baseClass;
            tIItemWeakPtr pFromItem;
            tComment comment;
        };

        typedef std::shared_ptr<tInheritanceData> tInheritanceDataPtr;

        typedef std::map<tItemName, tInheritanceDataPtr> tInheritanceMap;

        typedef tStringPtrWrapper tMethod;

        typedef std::string tAccessModifier;

        enum class eMethodType
        {
            eUsual = 0,
            eVirtual,
            ePureVirtual,
            eOverride,
            eStatic
        };

        struct tMethodData
        {
            tMethodData();
            tMethodData(const tAccessModifier& accessModifier_,
                        const std::string& method_,
                        const eMethodType& methodType_);
            tAccessModifier accessModifier;
            tMethod method;
            eMethodType methodType;
        };

        typedef std::map<tMethod, tMethodData> tMethodMap;

        enum class eItemType
        {
            eInterface = 0,
            eClass,
            eAbstractClass,
            eSingletone
        };

        class IItem
        {
        public:
            virtual ~IItem();
            virtual const eItemType& getType() const = 0;
            virtual const tItemName& getItemName() const = 0;
            virtual void setItemName(const tItemName& val) = 0;
            virtual const tItemName& getTemplateAlias() const = 0;
            virtual tDependencyMap& getDependencyMap() = 0;
            virtual tDependencyMap& getDependentMap() = 0;
            virtual tInheritanceMap& getInheritanceMap() = 0;
            virtual tInheritanceMap& getInheritanceFromMeMap() = 0;
            virtual tPackageDataWeakPtr& getParent() = 0;
            virtual tMethodMap& getMethodMap() = 0;
        };

        struct tBaseData : public IItem
        {
            virtual const tItemName& getItemName() const override;
            virtual void setItemName(const tItemName& val) override;
            virtual const tItemName& getTemplateAlias() const override;
            virtual tDependencyMap& getDependencyMap() override;
            virtual tInheritanceMap& getInheritanceMap() override;
            virtual tInheritanceMap& getInheritanceFromMeMap() override;
            virtual tDependencyMap& getDependentMap() override;
            virtual tPackageDataWeakPtr& getParent() override;
            virtual tMethodMap& getMethodMap() override;

            tName name;
            tMethodMap methodMap;
            tDependencyMap dependencyMap;
            tDependencyMap dependentMap;
            tPackageDataWeakPtr pParent;
            tInheritanceMap inheritanceMap;
            tInheritanceMap inheritanceFromMeMap;
        };

        struct tClassData : public tBaseData
        {
            virtual const eItemType& getType() const override;            
        };

        typedef std::shared_ptr<tClassData> tClassDataPtr;

        struct tInterfaceData : public tBaseData
        {
            virtual const eItemType& getType() const override;
        };

        typedef std::shared_ptr<tInterfaceData> tInterfaceDataPtr;

        struct tAbstractClassData : public tBaseData
        {
            virtual const eItemType& getType() const override;
        };

        typedef std::shared_ptr<tAbstractClassData> tAbstractClassDataPtr;

        struct tSingletoneData : public tBaseData
        {
            virtual const eItemType& getType() const override;
        };

        typedef std::shared_ptr<tSingletoneData> tSingletoneDataPtr;

        struct tPackageData
        {
            tItemName packageName;
            tItemMap itemMap;
        };

        typedef std::map<tItemName, tPackageDataPtr> tPackageMap;

        /**
         * @brief The Creator class - the class, which is used to store all UML-related data and
         * dump the plantuml class diagram as a string.
         * Note! This class IS thread-safe.
         */
        class Creator
        {
            public:

                /**
                 * @brief getInstance - gets instance of single-tone
                 * @return - instance of single-tone
                 */
                static Creator& getInstance();

                /**
                 * @brief initialize - should be called once AFTER all global variables were
                 * already initialized. In other words - call it from context of any user thread.
                 * E.g. inside main function, or anywhere else, but outside the global section.
                 * Call from global section will lead to undefined behavior of this concept!
                 */
                void initialize();

                /**
                 * @brief isInitislized - tells whether UML data is initialized
                 * @return
                 */
                bool isInitislized() const;

                struct tClassDiagramResult
                {
                    bool bIsSuccessful = false;
                    std::string error;
                    std::string diagramContent;
                };

                /**
                 * @brief getClassDiagram - gets full class diagram of all registered elements
                 * @return - instance of tClassDiagramResult struct, which contains content of
                 * diagram, in case if tClassDiagramResult::bIsSuccessful == true.
                 * Otherwise returns an error message.
                 */
                tClassDiagramResult getClassDiagram() const;

                /**
                 * @brief getPackageClassDiagram - gets filtered class diagram of sub-set of the
                 * registered elements
                 * @param packageName - name of target package, starting from which diagram should
                 * be created. Diagram will show target package and first level of its dependencies.
                 * @param excludeDependencies - whether we should represent package WITHOUT its
                 * first level external dependencies
                 * @return - instance of tClassDiagramResult struct, which contains content of
                 * diagram, in case if tClassDiagramResult::bIsSuccessful == true.
                 * Otherwise returns an error message.
                 */
                tClassDiagramResult getPackageClassDiagram(const std::string& packageName, bool excludeDependencies = false) const;

                /**
                 * @brief findPackagesByName - tries to find package by name.
                 * @param packageName - keyword to search for. If empty - names of the all
                 * available packages will be provided
                 * @return - found elements, for which creation of diagram can be requested.
                 * Note! Search logic is using the "start with" policy.
                 * Search is case-insensitive.
                 */
                tStringPtrSet findPackagesByName( const std::string& packageName ) const;

                /**
                 * @brief addItem - adds item ( class or interface ) to the model
                 * @param packageName - name of the package.
                 * @param pItemData - pointer to item data
                 * Note! This method should be called before initialization of the model.
                 * After initialization it will do nothing.
                 */
                void addItem( const tItemName& packageName, const tIItemPtr& pItemData );

            private:

                /**
                 * @brief Creator - private constructor of single-tone
                 */
                Creator();

            private:

                tPackageMap mPackageMap;
                tItemMap mItemRegistry;
                bool mbIsinitialized;
                std::mutex mDataProtector;
        };

        struct tCallOnCreate
        {
            tCallOnCreate( const std::function<void(void)>& callable );
        };
    }
}

#endif // DMA_COMPONENTS_IBASICITEM_HPP
