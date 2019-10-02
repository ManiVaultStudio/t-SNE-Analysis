
// Its own header file:
#include "DimensionSelectionItemModel.h"

#include "DimensionSelectionHolder.h"

// Qt header files:
#include <QString>

// Standard C++ header files:
#include <vector>


namespace hdps
{

    DimensionSelectionItemModel::DimensionSelectionItemModel(
        DimensionSelectionHolder& holder)
        :
        _holder(holder)
    {
    }


    int DimensionSelectionItemModel::rowCount(const QModelIndex &) const
    {
        const auto rowCount = _holder.getNumberOfDimensions();
        assert(rowCount <= INT_MAX);
        return static_cast<int>(rowCount);
    }


    int DimensionSelectionItemModel::columnCount(const QModelIndex &) const noexcept
    {
        return 1;
    }


    QModelIndex DimensionSelectionItemModel::index(const int row, const int column, const QModelIndex &) const
    {
        return createIndex(row, column);

    }


    QModelIndex DimensionSelectionItemModel::parent(const QModelIndex &) const noexcept
    {
        return{};
    }


    QVariant DimensionSelectionItemModel::data(const QModelIndex & modelIndex, const int role) const
    {
        try
        {
            if (modelIndex.isValid())
            {
                const auto row = static_cast<unsigned>(modelIndex.row());
                const auto column = static_cast<unsigned>(modelIndex.column());

                if ((role == Qt::DisplayRole) && (column == 0))
                {
                    if (row < _holder.getNumberOfDimensions())
                    {
                        if (column == 0)
                        {
                            return _holder.getName(row);
                        }
                    }
                }
                if ((role == Qt::CheckStateRole) && (column == 0))
                {
                    return _holder.isDimensionEnabled(row) ? Qt::Checked : Qt::Unchecked;
                }
            }
        }
        catch (const std::exception&)
        {
            // Out of luck.
        }
        return {};
    }


    Qt::ItemFlags DimensionSelectionItemModel::flags(const QModelIndex& modelIndex) const noexcept
    {
        // https://doc.qt.io/qt-5/qabstractitemmodel.html#flags
        // The base class implementation returns a combination of flags that enables the
        // item (ItemIsEnabled) and allows it to be selected ().
        const auto flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

        if (modelIndex.isValid() && (modelIndex.column() == 0))
        {
            return flags | Qt::ItemIsUserCheckable;
        }
        else
        {
            return flags;
        }
    }


    bool DimensionSelectionItemModel::setData(const QModelIndex &modelIndex, const QVariant &value, int role)
    {
        if ((role == Qt::CheckStateRole) && modelIndex.isValid() && (modelIndex.column() == 0))
        {
            _holder.toggleDimensionEnabled(static_cast<unsigned>(modelIndex.row()));

            // https://doc.qt.io/qt-5/qabstractitemmodel.html#setData
            // "The dataChanged() signal should be emitted if the data was successfully set."
            Q_EMIT dataChanged(modelIndex, modelIndex, QVector<int>(1, Qt::CheckStateRole));


            return true;
        }
        else
        {
            // https://doc.qt.io/qt-5/qabstractitemmodel.html#setData
            // "The base class implementation returns false."
            return false;
        }
        return QAbstractItemModel::setData(modelIndex, value, role);
    }

} // namespace hdps
