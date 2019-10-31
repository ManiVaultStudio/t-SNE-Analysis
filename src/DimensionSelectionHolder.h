#pragma once
#include <algorithm> // For fill_n.
#include <cassert>
#include <memory> // For unique_ptr.
#include <vector>

#include <QString>

namespace hdps
{
    struct StatisticsPerDimension
    {
        double mean;
        double meanOfNonZero;
        double standardDeviation;
    };

    class DimensionSelectionHolder
    {
        std::unique_ptr<QString[]> _names;
        std::unique_ptr<bool[]> _enabledDimensions;
        unsigned _numberOfDimensions{};

    public:
        std::vector<StatisticsPerDimension> _statistics;

        
        // C++ "Rule of Zero": implicitly defaulted move-constructor and move assignment are just fine.

        DimensionSelectionHolder() = default;

        explicit DimensionSelectionHolder(const unsigned numberOfDimensions);

        explicit DimensionSelectionHolder(
            const QString* const names,
            const unsigned numberOfDimensions);

        unsigned getNumberOfDimensions() const noexcept;

        unsigned getNumberOfSelectedDimensions() const noexcept;

        QString getName(const std::size_t) const;

        bool lessThanName(std::size_t, std::size_t) const noexcept;

        void disableAllDimensions();

        bool tryToEnableDimensionByName(const QString& name);

        bool isDimensionEnabled(std::size_t) const;

        void toggleDimensionEnabled(std::size_t);

        std::vector<bool> getEnabledDimensions() const;
    };

} // namespace hdps
