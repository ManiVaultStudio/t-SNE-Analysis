#pragma once
#include <QString>
#include <QWidget>

#include <memory> // For unique_ptr
#include <vector>

class PointData;

namespace hdps
{
    class DimensionSelectionWidget : public QWidget
    {
    private:
        class Impl;
        const std::unique_ptr<Impl> _pImpl;

    public:
        DimensionSelectionWidget();
        ~DimensionSelectionWidget();

        // Explicitly delete its copy and move member functions.
        DimensionSelectionWidget(const DimensionSelectionWidget&) = delete;
        DimensionSelectionWidget(DimensionSelectionWidget&&) = delete;
        DimensionSelectionWidget& operator=(const DimensionSelectionWidget&) = delete;
        DimensionSelectionWidget& operator=(DimensionSelectionWidget&&) = delete;

        void dataChanged(const PointData&);

        std::vector<bool> getEnabledDimensions() const;
    };

}

