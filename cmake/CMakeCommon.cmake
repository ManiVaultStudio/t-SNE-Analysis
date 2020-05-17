set(DIR ${CMAKE_SOURCE_DIR}/Common)

set(DIMENSION_SELECTION_SOURCES
    ${DIR}/ModelResetter.h
    ${DIR}/ModelResetter.cpp
    ${DIR}/DimensionSelectionHolder.h
    ${DIR}/DimensionSelectionHolder.cpp
    ${DIR}/DimensionSelectionItemModel.h
    ${DIR}/DimensionSelectionItemModel.cpp
    ${DIR}/DimensionSelectionProxyModel.h
    ${DIR}/DimensionSelectionProxyModel.cpp
    ${DIR}/DimensionSelectionWidget.h
    ${DIR}/DimensionSelectionWidget.cpp
    #${DIR}/GeneralSettingsWidget.h
    #${DIR}/GeneralSettingsWidget.cpp
)

set(UI_FILES
    ${DIR}/ui/DimensionSelectionWidget.ui
)
