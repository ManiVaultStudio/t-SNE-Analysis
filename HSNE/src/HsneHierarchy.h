#pragma once

#include "CoreInterface.h"
#include "TsneAnalysis.h"
#include "hdi/dimensionality_reduction/hierarchical_sne.h"

#include <QString>
#include <QDebug>

#include <vector>
#include <memory>

using HsneMatrix = std::vector<hdi::data::MapMemEff<uint32_t, float>>;
using Hsne = hdi::dr::HierarchicalSNE<float, HsneMatrix>;

class Points;
class HsneParameters;

class HsneHierarchy : public QObject
{
    Q_OBJECT
public:
    /**
     * Initialize the HSNE hierarchy with a data-level scale.
     *
     * @param  data        The high-dimensional data
     * @param  parameters  Parameters with which to run the HSNE algorithm
     */
    void initialize(hdps::CoreInterface* core, const Points& inputData, const std::vector<bool>& enabledDimensions, const HsneParameters& parameters);
    HsneMatrix getTransitionMatrixAtScale(int scale) { return _hsne->scale(scale)._transition_matrix; }
    void getInfluencedLandmarksInPreviousScale(std::vector<unsigned int> indices, std::map<uint32_t, float>& neighbors)
    {
        _hsne->getInfluencedLandmarksInPreviousScale(_currentScale, indices, neighbors);
    }
    int getCurrentScale() { return _currentScale; }
    QString getInputDataName() { return _inputDataName; }
    int getNumPoints() { return _numPoints; }
    int getNumDimensions() { return _numDimensions; }

public slots:
    void newScale();

private:
    hdps::CoreInterface* _core;

    std::unique_ptr<Hsne> _hsne;

    QString _inputDataName;
    QString _embeddingName;

    int _currentScale = 0;

    unsigned int _numPoints;
    unsigned int _numDimensions;
};
