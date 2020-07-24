set(DIR ${CMAKE_CURRENT_SOURCE_DIR})

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
    PARENT_SCOPE
)

set(UI_FILES
    ${DIR}/ui/DimensionSelectionWidget.ui
    PARENT_SCOPE
)
