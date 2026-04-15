cmake_minimum_required(VERSION 3.21)
project(SkillTreeApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Core5Compat)

qt_standard_project_setup()

qt_add_resources(APP_RESOURCES resources/resources.qrc)

qt_add_executable(SkillTreeApp
    src/main.cpp
    src/app/ApplicationController.cpp
    src/domain/SkillNode.cpp
    src/domain/SkillTree.cpp
    src/application/SkillTreeModel.cpp
    src/application/DiceEngine.cpp
    src/application/SkillTreeViewModel.cpp
    src/application/ThemeManager.cpp
    src/infrastructure/JsonRepository.cpp
    ${APP_RESOURCES}
)

target_include_directories(SkillTreeApp PRIVATE src)

target_link_libraries(SkillTreeApp PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Core5Compat
)
