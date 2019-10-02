#pragma once
#include <QString>
#include <QWidget>

#include <memory> // For unique_ptr
#include <vector>

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

        void setDimensions(unsigned numberOfDimensions, const std::vector<QString>& names);

        std::vector<bool> getEnabledDimensions() const;
    };

}

