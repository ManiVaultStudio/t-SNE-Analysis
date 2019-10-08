#pragma once
#include <algorithm> // For fill_n.
#include <cassert>
#include <memory> // For unique_ptr.
#include <vector>

#include <QString>

namespace hdps
{

    class DimensionSelectionHolder
    {
        std::unique_ptr<QString[]> _names;
        std::unique_ptr<bool[]> _enabledDimensions;
        unsigned _numberOfDimensions{};

    public:

        explicit DimensionSelectionHolder(const unsigned numberOfDimensions);

        explicit DimensionSelectionHolder(
            const QString* const names,
            const unsigned numberOfDimensions);

        unsigned getNumberOfDimensions() const noexcept;

        QString getName(const std::size_t) const;

        void disableAllDimensions();

        bool tryToEnableDimensionByName(const QString& name);

        bool isDimensionEnabled(std::size_t) const;

        void toggleDimensionEnabled(std::size_t);

        std::vector<bool> getEnabledDimensions() const;
    };

} // namespace hdps