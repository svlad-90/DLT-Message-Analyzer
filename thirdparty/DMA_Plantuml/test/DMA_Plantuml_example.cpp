#include <iostream>

#include "../src/DMA_Plantuml.hpp"

PUML_PACKAGE_BEGIN(test_main)
    PUML_INTERFACE_BEGIN(IInterface)
        PUML_VIRTUAL_METHOD(+, ~IInterface())
        PUML_VIRTUAL_METHOD(+, void virtFunc() )
        PUML_PURE_VIRTUAL_METHOD(+, void pureVirtFunc(const int& val1, const int& val2) )
    PUML_INTERFACE_END()

    PUML_CLASS_BEGIN(CImpl_Test)
        PUML_INHERITANCE(IInterface, implements)
        PUML_INHERITANCE(IInterfaceHelper, implements)
        PUML_INHERITANCE(UndeclaredClass, extends)
        PUML_METHOD(+, CImpl_Test())
        PUML_OVERRIDE_METHOD(+, void virtFunc() )
        PUML_OVERRIDE_METHOD(+, void pureVirtFunc(const int& val1, const int& val2) )
        PUML_STATIC_METHOD(+, void myStaticMethod() )
        PUML_COMPOSITION_DEPENDENCY(CompositionDependency, 1, 1, contains)
        PUML_AGGREGATION_DEPENDENCY(AggregationDependency, 1, 1, uses)
    PUML_CLASS_END()
PUML_PACKAGE_END()

PUML_PACKAGE_BEGIN(test_helper)

    PUML_INTERFACE_BEGIN(IInterfaceHelper)
    PUML_INTERFACE_END()

    PUML_INTERFACE_BEGIN(IDependency)
    PUML_INTERFACE_END()

    PUML_CLASS_BEGIN(CompositionDependency)
        PUML_INHERITANCE(IDependency, implements)
        PUML_AGGREGATION_DEPENDENCY(ExternalDependency, 1, 1, uses)
    PUML_CLASS_END()

    PUML_CLASS_BEGIN(AggregationDependency)
        PUML_INHERITANCE(IDependency, implements)
        PUML_AGGREGATION_DEPENDENCY(ExternalDependency, 1, 1, uses)
    PUML_CLASS_END()

    PUML_CLASS_BEGIN(CImpl_Helper)
        PUML_INHERITANCE(IInterface, implements)
        PUML_METHOD(+, CImpl_Helper())
        PUML_OVERRIDE_METHOD(+, void virtFunc() )
        PUML_OVERRIDE_METHOD(+, void pureVirtFunc(const int& val1, const int& val2) )
        PUML_STATIC_METHOD(+, void myStaticMethod() )
    PUML_CLASS_END()

    PUML_CLASS_BEGIN(CImpl_HelperExtended)
        PUML_INHERITANCE(CImpl_Helper, extends)
        PUML_METHOD(+, CImpl_HelperExtended())
        PUML_OVERRIDE_METHOD(+, void virtFunc() )
    PUML_CLASS_END()

PUML_PACKAGE_END()

PUML_PACKAGE_BEGIN(test_external)
    PUML_CLASS_BEGIN(ExternalDependency)
    PUML_CLASS_END()
PUML_PACKAGE_END()

int main()
{
    DMA::PlantUML::Creator::getInstance().initialize();

    {
        auto diagramResult = DMA::PlantUML::Creator::getInstance().getClassDiagram();

        if(true == diagramResult.bIsSuccessful)
        {
            std::cout << "Diagram 1:" << std::endl;
            std::cout << diagramResult.diagramContent << std::endl << std::endl;
        }
        else
        {
            std::cout << diagramResult.error << std::endl;
        }
    }

    {
        auto diagramResult = DMA::PlantUML::Creator::getInstance().getPackageClassDiagram("test_main");

        if(true == diagramResult.bIsSuccessful)
        {
            std::cout << "Diagram 2:" << std::endl;
            std::cout << diagramResult.diagramContent << std::endl << std::endl;
        }
        else
        {
            std::cout << diagramResult.error << std::endl;
        }
    }

    {
        auto diagramResult = DMA::PlantUML::Creator::getInstance().getPackageClassDiagram("test_main", true);

        if(true == diagramResult.bIsSuccessful)
        {
            std::cout << "Diagram 3:" << std::endl;
            std::cout << diagramResult.diagramContent << std::endl;
        }
        else
        {
            std::cout << diagramResult.error << std::endl;
        }
    }

    return 0;
}
