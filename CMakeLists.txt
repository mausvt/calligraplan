
project(calligraplan)

cmake_minimum_required(VERSION 3.0)
if (POLICY CMP0002)
    cmake_policy(SET CMP0002 NEW)
endif ()
if (POLICY CMP0017)
    cmake_policy(SET CMP0017 NEW)
endif ()
if (POLICY CMP0022)
    cmake_policy(SET CMP0022 NEW)
endif ()
if (POLICY CMP0026)
    cmake_policy(SET CMP0026 OLD)
endif()
if (POLICY CMP0046)
    cmake_policy(SET CMP0046 NEW)
endif ()
if (POLICY CMP0059)
    cmake_policy(SET CMP0059 OLD)
endif()
if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif()
if (POLICY CMP0071)
    cmake_policy(SET CMP0071 NEW)
endif()

list(APPEND CMAKE_AUTOMOC_MACRO_NAMES "PLAN_SCHEDULERPLUGIN_EXPORT")

# define the version of Plan
# update these version for every release:
set(PLAN_YEAR 2018) # update every year
set(PLAN_VERSION_STRING "3.1.89")
set(PLAN_STABLE_VERSION_MAJOR 3) # 3 for 3.x, 4 for 4.x, etc.
set(PLAN_STABLE_VERSION_MINOR 1) # 0 for 3.0, 1 for 3.1, etc.
set(PLAN_VERSION_RELEASE 89)     # 89 for Alpha, increase for next test releases, set 0 for first Stable, etc.
set(PLAN_ALPHA 1) # uncomment only for Alpha
#set(PLAN_BETA 2) # uncomment only for Beta
#set(PLAN_RC 1) # uncomment only for RC

# Define the generic version of the Calligra Plan libraries here
# This makes it easy to advance it when the next release comes.
# 14 was the last GENERIC_CALLIGRA_LIB_VERSION_MAJOR of the previous Calligra series
# (2.x) so we're starting with 15 in 3.x series.
if(PLAN_STABLE_VERSION_MAJOR EQUAL 3)
    math(EXPR GENERIC_PLAN_LIB_VERSION_MAJOR "${PLAN_STABLE_VERSION_MINOR} + 15")
else()
    # let's make sure we won't forget to update the "15"
    message(FATAL_ERROR "Reminder: please update offset == 15 used to compute GENERIC_PLAN_LIB_VERSION_MAJOR to something bigger")
endif()
set(GENERIC_PLAN_LIB_VERSION "${GENERIC_PLAN_LIB_VERSION_MAJOR}.0.0")
set(GENERIC_PLAN_LIB_SOVERSION "${GENERIC_PLAN_LIB_VERSION_MAJOR}")

message(STATUS "")
message(STATUS "Build Calligra Plan ${PLAN_VERSION_STRING} with cmake version: ${CMAKE_VERSION}, required version: ${CMAKE_MINIMUM_REQUIRED_VERSION}")
message(STATUS "")


# ensure out-of-source build
string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" isBuildInSource)
if(isBuildInSource)
    message(FATAL_ERROR "Compiling Plan inside the source folder is not possible.\nPlease refer to the build instruction: https://community.kde.org/Calligra/Building/3\nYou need to clean up the source folder from all build artifacts just created, otherwise further building attempts will fail again: With a git repo, you can use \"git clean -df\" in the toplevel source folder (attention! will remove also uncommited changes to the source code). With sources from a file bundle (like a zip file), delete the source folder and unbundle the sources again.")
endif()


############
#############
## Options ##
#############
############

option(PACKAGERS_BUILD "Build support of multiple CPU architectures in one binary. Should be used by packagers only." ON)

##########################
###########################
## Look for ECM, Qt, KF5 ##
###########################
##########################

find_package(ECM 5.19 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

# ECM KDE macros (include first, to have their policies and settings effect all other macros)
include(KDEInstallDirs)
include(KDECMakeSettings NO_POLICY_SCOPE)
include(KDECompilerSettings NO_POLICY_SCOPE)
# CMake macros
include(CMakePackageConfigHelpers)
include(WriteBasicConfigVersionFile)
include(CheckFunctionExists)
include(CheckTypeSize)
include(CheckIncludeFile)
include(GenerateExportHeader)
include(FeatureSummary)
# ECM macros
include(ECMOptionalAddSubdirectory)
include(ECMInstallIcons)
include(ECMAddAppIcon)
include(ECMSetupVersion)
include(ECMAddTests)
include(ECMMarkAsTest)
include(ECMMarkNonGuiExecutable)
include(ECMGenerateHeaders)
# Own macros
# This macro is a copy from calligra so only needs to be included when plan is build stand alone
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
include(MacroOptionalFindPackage)

set(REQUIRED_KF5_VERSION "5.53.0")

find_package(KF5 ${REQUIRED_KF5_VERSION} REQUIRED
    COMPONENTS
        Init
        Archive
#             Codecs
#             Completion
        Config
        ConfigWidgets
        CoreAddons
        DBusAddons
#             DocTools
        GuiAddons
        I18n
        IconThemes
        ItemViews
        JobWidgets
        KIO
        Notifications
#             NotifyConfig
        Parts
#             Sonnet
        TextWidgets
        WidgetsAddons
        WindowSystem
        XmlGui
#         Kross
)

find_package(KF5Activities)

set_package_properties(KF5Activities PROPERTIES
    DESCRIPTION "Library for Activities"
    PURPOSE "Required for activities support (optional)"
    TYPE OPTIONAL
)
if(KF5Activities_FOUND)
    set(HAVE_KACTIVITIES TRUE)
endif()

find_package(KF5KHtml)
set_package_properties(KF5KHtml PROPERTIES
    PURPOSE "Required for rich text support"
    TYPE REQUIRED
)

find_package(KF5Holidays)
set_package_properties(KF5Holidays PROPERTIES
    DESCRIPTION "Library for generation of public holidays"
    PURPOSE "Required for holidays support (optional)"
    TYPE RECOMMENDED
)


set(REQUIRED_QT_VERSION "5.9.0")

find_package(Qt5 ${REQUIRED_QT_VERSION} REQUIRED
    COMPONENTS
        Core
        Gui
        PrintSupport
        Test
        Widgets
        Xml
)
find_package(Qt5DBus ${REQUIRED_QT_VERSION} QUIET)
set_package_properties(Qt5DBus PROPERTIES
    DESCRIPTION "Library for dbus access"
    PURPOSE "Required for dbus support (optional)"
    TYPE OPTIONAL
)

find_package(X11)
if(X11_FOUND)
    find_package(Qt5 ${REQUIRED_QT_VERSION} REQUIRED
        COMPONENTS
            X11Extras
    )
    set(HAVE_X11 TRUE)
    add_definitions(-DHAVE_X11)
else()
    set(HAVE_X11 FALSE)
endif()

# use sane compile flags
add_definitions(
-DQT_USE_QSTRINGBUILDER
-DQT_STRICT_ITERATORS
-DQT_NO_SIGNALS_SLOTS_KEYWORDS
-DQT_NO_URL_CAST_FROM_STRING
-DQT_NO_CAST_TO_ASCII
)

# only with this definition will all the FOO_TEST_EXPORT macro do something
# TODO: check if this can be moved to only those places which make use of it,
# to reduce global compiler definitions that would trigger a recompile of
# everything on a change (like adding/removing tests to/from the build)
if(BUILD_TESTING)
    add_definitions(-DCOMPILING_TESTS)
endif()

# overcome some platform incompatibilities
if(WIN32)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/winquirks)
    add_definitions(-D_USE_MATH_DEFINES)
    add_definitions(-DNOMINMAX)
    set(WIN32_PLATFORM_NET_LIBS ws2_32.lib netapi32.lib)
endif()

###########################
############################
## Required dependencies  ##
############################
###########################

##
## Test for KGantt
##
macro_optional_find_package(KGantt 2.6.0 QUIET)
set_package_properties(KGantt PROPERTIES
    DESCRIPTION "Library for creating Gantt diagrams (part of KDiagram)"
    URL "https://www.kde.org/"
    TYPE REQUIRED
)

##
## Test for KChart
##
macro_optional_find_package(KChart 2.6.0 QUIET)
set_package_properties(KChart PROPERTIES
    DESCRIPTION "Library for creating business charts (part of KDiagram)"
    URL "https://www.kde.org/"
    TYPE REQUIRED
)

##
## Test for QCA2
##
macro_optional_find_package(Qca-qt5 2.1.0 QUIET)
set_package_properties(Qca-qt5 PROPERTIES
    DESCRIPTION "Qt Cryptographic Architecture"
    URL "https:/download.kde.org/stable/qca-qt5"
    PURPOSE "Required for encrypted OpenDocument files and encrypted xls files support"
    TYPE OPTIONAL
)

find_package(KF5Wallet ${REQUIRED_KF5_VERSION})

if(Qca-qt5_FOUND AND KF5Wallet_FOUND)
    add_definitions( -DQCA2 )
endif()

find_package(Perl REQUIRED)
find_package(ZLIB REQUIRED)

###########################
############################
## Optional dependencies  ##
############################
###########################

##
## Test for KF5CalendarCore
##
find_package(KF5CalendarCore CONFIG QUIET)
set_package_properties(KF5CalendarCore PROPERTIES
    DESCRIPTION "KDE Calendar Library"
    URL "https://www.kde.org/"
    PURPOSE "Required for Ical export"
    TYPE OPTIONAL
)

##
## Test for KF5AkonadiContact
##
find_package(KF5AkonadiContact CONFIG QUIET)
set_package_properties(KF5AkonadiContact PROPERTIES
    DESCRIPTION "Library for Accessing Contacts stored in Akonadi"
    URL "https://www.kde.org/"
    PURPOSE "Required for address book support"
    TYPE OPTIONAL
)

if(WIN32)
    set(LIB_INSTALL_DIR ${LIB_INSTALL_DIR}
                        RUNTIME DESTINATION ${BIN_INSTALL_DIR}
                        LIBRARY ${INSTALL_TARGETS_DEFAULT_ARGS}
                        ARCHIVE ${INSTALL_TARGETS_DEFAULT_ARGS} )
endif()

#### Disable kreport for now, reconsider when it is stable ####
if (TEST_FOR_KREPORT)
    #
    # Test for KReport
    #
    macro_optional_find_package(KReport 3.1 QUIET)
    set_package_properties(KReport PROPERTIES
        PURPOSE "Required for KReport support in Plan"
        TYPE OPTIONAL
    )

    ##
    ## Test for KPropertyWidgets
    ##
    macro_optional_find_package(KPropertyWidgets 3.1 QUIET)
    set_package_properties(KPropertyWidgets PROPERTIES
        PURPOSE "Required for KReports support in Plan"
        TYPE OPTIONAL
    )
    if (KReport_FOUND AND KPropertyWidgets_FOUND)
        set(PLAN_USE_KREPORT TRUE)
    endif()
endif (TEST_FOR_KREPORT)


add_subdirectory(src)

add_subdirectory(devtools)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif()

#add_custom_target(apidox doc/api/gendocs.pl WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

### Summarize findings ####
feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)


