#include "DimensionSelectionAction.h"
#include "Application.h"
#include "ModelResetter.h"
#include "PointData.h"

#include <QLabel>
#include <QFileDialog>
#include <QTableView>
#include <QHeaderView>
#include <QTime>
#include <QAbstractEventDispatcher>

#include <deque>
#include <set>

#ifndef __APPLE__
	#include <execution>
#endif

using namespace hdps;
using namespace hdps::gui;

DimensionSelectionAction::DimensionSelectionAction(QObject* parent) :
	WidgetActionGroup(parent),
	_pointData(nullptr),
	_selectionHolder(),
	_selectionItemModel(new DimensionSelectionItemModel(_selectionHolder)),
	_selectionProxyModel(new DimensionSelectionProxyModel(_selectionHolder)),
	_nameFilterAction(this, "Name filter"),
	_showOnlySelectedDimensionsAction(this, "Show only selected dimensions"),
	_applyExclusionListAction(this, "Apply exclusion list"),
	_ignoreZeroValuesAction(this, "Ignore zero values"),
	_selectionThresholdAction(this, "Selection threshold"),
	_summaryAction(this, "Summary"),
	_computeStatisticsAction(this, "Compute statistics"),
	_selectVisibleAction(this, "Select visible"),
	_selectNonVisibleAction(this, "Select non-visible"),
	_loadSelectionAction(this, "Load selection"),
	_saveSelectionAction(this, "Save selection"),
	_loadExclusionAction(this, "Load exclusion"),
	_summaryUpdateAwakeConnection()
{
    setText("Dimension selection");

	connect(&_nameFilterAction, &StringAction::stringChanged, this, [this](const QString& name) {
		setNameFilter(name);
	});

	connect(&_showOnlySelectedDimensionsAction, &ToggleAction::triggered, this, [this](bool checked) {
		setShowOnlySelectedDimensions(checked);
	});

	connect(&_applyExclusionListAction, &ToggleAction::triggered, this, [this](bool checked) {
		setApplyExclusionList(checked);
	});

	connect(&_ignoreZeroValuesAction, &ToggleAction::triggered, this, [this](bool checked) {
		setIgnoreZeroValues(checked);
	});

	const auto selectionFileFilter = tr("Text files (*.txt);;All files (*.*)");

	connect(&_selectionThresholdAction, &IntegralAction::valueChanged, this, [this](const std::int32_t& value) {
		const auto sliderMaximum = _selectionThresholdAction.getMaximum();

		if (value >= 0 && value <= sliderMaximum)
		{
			const auto& distinctStandardDeviations = _selectionHolder.distinctStandardDeviationsWithAndWithoutZero[_selectionHolder._ignoreZeroValues ? 1 : 0];
			const auto numberOfDistinctStandardDeviations = distinctStandardDeviations.size();

			if (numberOfDistinctStandardDeviations > 0 && sliderMaximum > 0)
			{
				if ((sliderMaximum + 1) == numberOfDistinctStandardDeviations)
				{
					const ModelResetter modelResetter(_selectionProxyModel.get());

					_selectionProxyModel->SetMinimumStandardDeviation(distinctStandardDeviations[value]);
				}
				else
					assert(!"Slider maximum incorrect!");
			}
		}
		else
			assert(!"Slider value out of range!");
	});

	connect(&_computeStatisticsAction, &TriggerAction::triggered, this, [this]() {
		computeStatistics();
	});

	connect(&_selectVisibleAction, &TriggerAction::triggered, this, [this]() {
		selectDimensionsBasedOnVisibility<true>();
	});

	connect(&_selectNonVisibleAction, &TriggerAction::triggered, this, [this]() {
		selectDimensionsBasedOnVisibility<true>();
	});

	connect(&_loadSelectionAction, &TriggerAction::triggered, this, [this, selectionFileFilter]() {
		loadSelectionFromFile(QFileDialog::getOpenFileName(nullptr, tr("Dimension selection"), {}, selectionFileFilter));
	});

	connect(&_loadExclusionAction, &TriggerAction::triggered, this, [this, selectionFileFilter]() {
		loadSelectionFromFile(QFileDialog::getOpenFileName(nullptr, tr("Exclusion list"), {}, selectionFileFilter));
	});

	connect(&_saveSelectionAction, &TriggerAction::triggered, this, [this, selectionFileFilter]() {
		saveSelectionToFile(QFileDialog::getOpenFileName(nullptr, tr("Dimension selection"), {}, selectionFileFilter));
	});

	_summaryAction.setEnabled(false);

	updateSummary();

	_summaryUpdateAwakeConnection = connect(QAbstractEventDispatcher::instance(), &QAbstractEventDispatcher::awake,[this] {
		updateSummary();
	});

    const auto updateReadOnly = [this]() -> void {
        const auto enable = !isReadOnly();

        _nameFilterAction.setEnabled(enable);
        _showOnlySelectedDimensionsAction.setEnabled(enable);
        _applyExclusionListAction.setEnabled(enable);
        _ignoreZeroValuesAction.setEnabled(enable);
        _selectionThresholdAction.setEnabled(enable && !_selectionHolder._statistics.empty());
        _summaryAction.setEnabled(false);
        _computeStatisticsAction.setEnabled(enable);
        _selectVisibleAction.setEnabled(enable);
        _selectNonVisibleAction.setEnabled(enable);
        _loadSelectionAction.setEnabled(enable);
        _saveSelectionAction.setEnabled(enable);
        _loadExclusionAction.setEnabled(enable);
    };

    connect(this, &WidgetActionGroup::readOnlyChanged, this, [this, updateReadOnly](const bool& readOnly) {
        updateReadOnly();
    });

    updateReadOnly();

    _selectionThresholdAction.setEnabled(false);
}

DimensionSelectionAction::~DimensionSelectionAction()
{
	disconnect(_summaryUpdateAwakeConnection);
}

void DimensionSelectionAction::setDimensions(const std::uint32_t numDimensions, const std::vector<QString>& names)
{
	if (names.size() == numDimensions)
	{
		_selectionHolder = DimensionSelectionHolder(names.data(), numDimensions);
	}
	else
	{
		assert(names.empty());
		_selectionHolder = DimensionSelectionHolder(numDimensions);
	}

	auto dimensionSelectionItemModel = std::make_unique<DimensionSelectionItemModel>(_selectionHolder);
	auto proxyModel = std::make_unique<DimensionSelectionProxyModel>(_selectionHolder);

	proxyModel->setSourceModel(dimensionSelectionItemModel.get());
	
	_selectionProxyModel	= std::move(proxyModel);
	_selectionItemModel		= std::move(dimensionSelectionItemModel);

	updateSlider();
}

std::vector<bool> DimensionSelectionAction::getEnabledDimensions() const
{
	return _selectionHolder.getEnabledDimensions();
}

void DimensionSelectionAction::dataChanged(Points& points)
{
	_pointData = &points;

	setDimensions(points.getNumDimensions(), points.getDimensionNames());

	_computeStatisticsAction.setEnabled(true);
}

hdps::DimensionSelectionProxyModel* DimensionSelectionAction::getProxyModel()
{
	return _selectionProxyModel.get();
}

void DimensionSelectionAction::setNameFilter(const QString& nameFilter)
{
	if (_selectionProxyModel == nullptr)
		return;

	const ModelResetter modelResetter(_selectionProxyModel.get());
	_selectionProxyModel->setFilterRegExp(nameFilter);
}

void DimensionSelectionAction::setShowOnlySelectedDimensions(const bool& showOnlySelectedDimensions)
{
	if (_selectionProxyModel == nullptr)
		return;

	const ModelResetter modelResetter(_selectionProxyModel.get());
	_selectionProxyModel->SetFilterShouldAcceptOnlySelected(showOnlySelectedDimensions);
}

void DimensionSelectionAction::setApplyExclusionList(const bool& applyExclusionList)
{
	if (_selectionProxyModel == nullptr)
		return;

	const ModelResetter modelResetter(_selectionProxyModel.get());
	_selectionProxyModel->SetFilterShouldAppyExclusion(applyExclusionList);
}

void DimensionSelectionAction::setIgnoreZeroValues(const bool& ignoreZeroValues)
{
	const ModelResetter modelResetter(_selectionProxyModel.get());
	_selectionHolder._ignoreZeroValues = ignoreZeroValues;
	
	updateSlider();
}

void DimensionSelectionAction::loadSelectionFromFile(const QString& fileName)
{
	QFile file(fileName);

	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		const ModelResetter modelResetter(_selectionItemModel.get());

		_selectionHolder.disableAllDimensions();

		while (!file.atEnd())
		{
			const auto trimmedLine = file.readLine().trimmed();

			if (!trimmedLine.isEmpty())
			{
				const auto name = QString::fromUtf8(trimmedLine);

				if (!_selectionHolder.tryToEnableDimensionByName(name))
					qWarning() << "Failed to select dimension (name not found): " << name;
			}
		}
	}
	else
	{
		qCritical() << "Load failed to open file: " << fileName;
	}
}

void DimensionSelectionAction::loadExclusionFromFile(const QString& fileName)
{
	if (fileName.isEmpty())
		return;

	QFile file(fileName);

	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		const ModelResetter modelResetter(_selectionProxyModel.get());

		std::deque<QString> strings;

		while (!file.atEnd())
		{
			const auto trimmedLine = file.readLine().trimmed();

			if (!trimmedLine.isEmpty())
				strings.push_back(QString::fromUtf8(trimmedLine));
		}

		_selectionProxyModel->SetExclusion(std::vector<QString>(strings.cbegin(), strings.cend()));
	}
	else
	{
		qCritical() << "Load exclusion failed to open file: " << fileName;
	}
}

void DimensionSelectionAction::saveSelectionToFile(const QString& fileName)
{
	if (fileName.isEmpty())
		return;

	QFile file(fileName);

	if (file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		const auto numberOfDimensions = _selectionHolder.getNumberOfDimensions();

		for (std::size_t i{}; i < numberOfDimensions; ++i)
		{
			if (_selectionHolder.isDimensionEnabled(i))
			{
				file.write(_selectionHolder.getName(i).toUtf8());
				file.write("\n");
			}
		}
	}
	else
	{
		qCritical() << "Save failed to open file: " << fileName;
	}
}

void DimensionSelectionAction::computeStatistics()
{
	const ModelResetter modelResetter(_selectionProxyModel.get());

	auto& statistics = _selectionHolder._statistics;
	statistics.clear();

	if (_pointData != nullptr)
	{
		QTime time;

		time.start();
		const auto& pointData = *_pointData;

		pointData.visitFromBeginToEnd([&statistics, &pointData](auto beginOfData, auto endOfData)
		{
			const auto numberOfDimensions = pointData.getNumDimensions();
			const auto numberOfPoints = pointData.getNumPoints();

			constexpr static auto quiet_NaN = std::numeric_limits<double>::quiet_NaN();

			if (numberOfPoints == 0)
			{
				statistics.resize(numberOfDimensions, { quiet_NaN, quiet_NaN, quiet_NaN, quiet_NaN });
			}
			else
			{
				statistics.resize(numberOfDimensions);
				const auto* const statisticsData = statistics.data();

				if (numberOfPoints == 1)
				{
#ifndef __APPLE__
					(void)std::for_each_n(std::execution::par_unseq, statistics.begin(), numberOfDimensions,
#else
					(void)std::for_each_n(statistics.begin(), numberOfDimensions,
#endif
						[statisticsData, beginOfData](auto& statisticsPerDimension)
					{
						const auto i = &statisticsPerDimension - statisticsData;

						const double dataValue = beginOfData[i];
						statisticsPerDimension = { {dataValue, dataValue}, {quiet_NaN, quiet_NaN} };
					});
				}
				else
				{
#ifndef __APPLE__
					(void)std::for_each_n(std::execution::par_unseq, statistics.begin(), numberOfDimensions,
#else
					(void)std::for_each_n(statistics.begin(), numberOfDimensions,
#endif
						[statisticsData, numberOfDimensions, numberOfPoints, beginOfData](auto& statisticsPerDimension)
					{
						const std::unique_ptr<double[]> data(new double[numberOfPoints]);
						{
							const auto i = &statisticsPerDimension - statisticsData;

							for (unsigned j{}; j < numberOfPoints; ++j)
							{
								data[j] = beginOfData[j * numberOfDimensions + i];
							}
						}

						double sum{};
						unsigned numberOfNonZeroValues{};

						for (unsigned j{}; j < numberOfPoints; ++j)
						{
							const auto value = data[j];

							if (value != 0.0)
							{
								sum += value;
								++numberOfNonZeroValues;
							}
						}
						const auto mean = sum / numberOfPoints;

						double sumOfSquares{};

						for (unsigned j{}; j < numberOfPoints; ++j)
						{
							const auto value = data[j] - mean;
							sumOfSquares += value * value;
						}

						static_assert(quiet_NaN != quiet_NaN);

						statisticsPerDimension = StatisticsPerDimension
						{
							{
								mean,
								(numberOfNonZeroValues == 0) ? quiet_NaN : (sum / numberOfNonZeroValues)
							},
							{
								std::sqrt(sumOfSquares / (numberOfPoints - 1)),
								(numberOfNonZeroValues == 0) ? quiet_NaN : std::sqrt(sumOfSquares / numberOfNonZeroValues)
							}
						};
					});
				}
			}
		});
		qDebug()
			<< " Duration: " << time.elapsed() << " microsecond(s)";

		for (unsigned i{}; i <= 1; ++i)
		{
			std::set<double> distinctStandardDeviations;

			for (const auto& statisticsPerDimension : _selectionHolder._statistics)
			{
				if (!std::isnan(statisticsPerDimension.standardDeviation[i]))
				{
					distinctStandardDeviations.insert(statisticsPerDimension.standardDeviation[i]);
				}
			}

			_selectionHolder.distinctStandardDeviationsWithAndWithoutZero[i].assign(distinctStandardDeviations.cbegin(), distinctStandardDeviations.cend());
		}

		assert(_selectionThresholdAction.getMinimum() == 0);
		updateSlider();

		_computeStatisticsAction.setEnabled(false);
	}
}

void DimensionSelectionAction::updateSlider()
{
	const auto numberOfDistinctStandardDeviations	= _selectionHolder.distinctStandardDeviationsWithAndWithoutZero[_selectionHolder._ignoreZeroValues ? 1 : 0].size();
	const auto isSliderValueAtMaximum				= (_selectionThresholdAction.isAtMaximum());

	if (numberOfDistinctStandardDeviations > 0 && numberOfDistinctStandardDeviations <= std::numeric_limits<int>::max())
	{
		const auto newMaximum = static_cast<int>(numberOfDistinctStandardDeviations) - 1;

		if (newMaximum != _selectionThresholdAction.getMaximum())
			_selectionThresholdAction.setMaximum(newMaximum);
		
		_selectionThresholdAction.setValue(isSliderValueAtMaximum ? newMaximum : 0);
	}
	else
	{
		_selectionThresholdAction.setValue(isSliderValueAtMaximum ? 1 : 0);
		_selectionThresholdAction.setMaximum(1);
	}

	_selectionThresholdAction.setEnabled(!isReadOnly() && !_selectionHolder._statistics.empty());
}

void DimensionSelectionAction::updateSummary()
{
	const auto& holder = _selectionHolder;

	const auto numberOfDimensions			= holder.getNumberOfDimensions();
	const auto numberOfVisibleDimensions	= (_selectionProxyModel == nullptr) ? 0 : _selectionProxyModel->rowCount();

	_summaryAction.setString(tr("%1 available, %2 visible, %3 selected").arg(numberOfDimensions).arg(numberOfVisibleDimensions).arg(holder.getNumberOfSelectedDimensions()));
}

DimensionSelectionAction::Widget::Widget(QWidget* parent, DimensionSelectionAction* dimensionsSettingsAction, const Widget::State& state) :
    WidgetAction::Widget(parent, dimensionsSettingsAction, state)
{
    auto layout = new QVBoxLayout();

	auto nameMatchesLayout = new QHBoxLayout();

	nameMatchesLayout->addWidget(dimensionsSettingsAction->_nameFilterAction.createLabelWidget(this));
	nameMatchesLayout->addWidget(dimensionsSettingsAction->_nameFilterAction.createWidget(this));

	layout->addLayout(nameMatchesLayout);
	
	layout->addWidget(dimensionsSettingsAction->_showOnlySelectedDimensionsAction.createCheckBoxWidget(this));
	layout->addWidget(dimensionsSettingsAction->_applyExclusionListAction.createCheckBoxWidget(this));
	layout->addWidget(dimensionsSettingsAction->_ignoreZeroValuesAction.createCheckBoxWidget(this));

	auto selectionThresholdLayout = new QHBoxLayout();

	auto moreLabel = new QLabel("More");
	auto lessLabel = new QLabel("Less");

	selectionThresholdLayout->addWidget(moreLabel);
	selectionThresholdLayout->addWidget(dimensionsSettingsAction->_selectionThresholdAction.createSliderWidget(this));
	selectionThresholdLayout->addWidget(lessLabel);

    const auto updateSelectionThresholdLabels = [this, dimensionsSettingsAction, moreLabel, lessLabel]() -> void {
        const auto isEnabled = dimensionsSettingsAction->_selectionThresholdAction.isEnabled();

        moreLabel->setEnabled(isEnabled);
        lessLabel->setEnabled(isEnabled);
    };

	connect(&dimensionsSettingsAction->_selectionThresholdAction, &IntegralAction::changed, this, [this, updateSelectionThresholdLabels]() {
        updateSelectionThresholdLabels();
	});

    updateSelectionThresholdLabels();

	layout->addLayout(selectionThresholdLayout);
	
	auto tableView = new QTableView();

	tableView->setSortingEnabled(true);
	tableView->setModel(dimensionsSettingsAction->getProxyModel());
	
	auto horizontalHeader = tableView->horizontalHeader();

	horizontalHeader->setSectionResizeMode(QHeaderView::Stretch);
	horizontalHeader->setStretchLastSection(true);
	horizontalHeader->setDefaultAlignment(Qt::AlignLeft);
	horizontalHeader->setSortIndicator(2, Qt::DescendingOrder);

	tableView->verticalHeader()->hide();

	layout->addWidget(tableView);

	connect(&dimensionsSettingsAction->_nameFilterAction, &IntegralAction::changed, this, [this, dimensionsSettingsAction, tableView]() {
		const auto isEnabled = dimensionsSettingsAction->_nameFilterAction.isEnabled();

		tableView->setEnabled(isEnabled);
	});

	layout->addWidget(dimensionsSettingsAction->_summaryAction.createWidget(this));
	layout->addWidget(dimensionsSettingsAction->_computeStatisticsAction.createWidget(this));

	auto selectLayout = new QHBoxLayout();

	selectLayout->addWidget(dimensionsSettingsAction->_selectVisibleAction.createWidget(this));
	selectLayout->addWidget(dimensionsSettingsAction->_selectNonVisibleAction.createWidget(this));

	layout->addLayout(selectLayout);

	auto fileLayout = new QHBoxLayout();

	fileLayout->addWidget(dimensionsSettingsAction->_loadSelectionAction.createWidget(this));
	fileLayout->addWidget(dimensionsSettingsAction->_loadExclusionAction.createWidget(this));
	fileLayout->addWidget(dimensionsSettingsAction->_saveSelectionAction.createWidget(this));

	layout->addLayout(fileLayout);

    switch (state)
    {
        case Widget::State::Standard:
            //layout->setMargin(0);
            setLayout(layout);
            break;

        case Widget::State::Popup:
            setPopupLayout(layout);
            break;

        default:
            break;
    }
}