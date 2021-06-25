set(DIR ${CMAKE_CURRENT_SOURCE_DIR}/Common)

set(TSNE_COMMON_SOURCES
    ${DIR}/TsneAnalysis.h
    ${DIR}/TsneAnalysis.cpp
    ${DIR}/TsneData.h
    ${DIR}/TsneParameters.h
)

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
