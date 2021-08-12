#pragma once

#include "actions/Actions.h"

class QMenu;
class TsneSettingsAction;

/**
 * General TSNE setting action class
 *
 * Actions class for general TSNE settings
 *
 * @author Thomas Kroes
 */
class GeneralTsneSettingsAction : public hdps::gui::WidgetActionGroup
{
protected:

    /** Widget class for general TSNE settings action */
    class Widget : public hdps::gui::WidgetActionGroup::Widget {
    public:

        /**
         * Constructor
         * @param parent Pointer to parent widget
         * @param generalSettingsAction Pointer to general TSNE settings action
         * @param state State of the widget
         */
        Widget(QWidget* parent, GeneralTsneSettingsAction* generalSettingsAction, const hdps::gui::WidgetActionWidget::State& state);
    };

    /**
     * Get widget representation of the general TSNE settings action
     * @param parent Pointer to parent widget
     * @param state Widget state
     */
    QWidget* getWidget(QWidget* parent, const hdps::gui::WidgetActionWidget::State& state = hdps::gui::WidgetActionWidget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:

    /**
     * Constructor
     * @param tsneSettingsAction Reference to TSNE settings action
     */
    GeneralTsneSettingsAction(TsneSettingsAction& tsneSettingsAction);

public: // Action getters

    TsneSettingsAction& getTsneSettingsAction() { return _tsneSettingsAction; };
    hdps::gui::OptionAction& getKnnTypeAction() { return _knnTypeAction; };
    hdps::gui::OptionAction& getDistanceMetricAction() { return _distanceMetricAction; };
    hdps::gui::IntegralAction& getNumIterationsAction() { return _numIterationsAction; };
    hdps::gui::IntegralAction& getPerplexityAction() { return _perplexityAction; };
    hdps::gui::TriggerAction& getResetAction() { return _resetAction; };

protected:
    TsneSettingsAction&         _tsneSettingsAction;            /** Reference to parent tSNE settings action */
    hdps::gui::OptionAction     _knnTypeAction;                 /** KNN action */
    hdps::gui::OptionAction     _distanceMetricAction;          /** Distance metric action */
    hdps::gui::IntegralAction   _numIterationsAction;           /** Number of iterations action */
    hdps::gui::IntegralAction   _perplexityAction;              /** Perplexity action */
    hdps::gui::TriggerAction    _resetAction;                   /** Reset all input to defaults */

    friend class Widget;
};
