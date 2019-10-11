
// Its own header file:
#include "DimensionSelectionWidget.h"

#include "DimensionSelectionHolder.h"
#include "DimensionSelectionItemModel.h"
#include "ModelResetter.h"

// Qt header files:
#include <QAbstractEventDispatcher>
#include <QDebug>
#include <QFileDialog>
#include <QString>

// Standard C++ header files:
#include <vector>

// Header file, generated by Qt User Interface Compiler:
#include <ui_DimensionSelectionWidget.h>


namespace hdps
{
    namespace
    {
        QString getSelectionFileFilter()
        {
            return QObject::tr("Text files (*.txt);;All files (*.*)");
        }


        template <typename T>
        void connectPushButton(QPushButton& pushButton, const T slot)
        {
            QObject::connect(&pushButton, &QPushButton::clicked, [slot, &pushButton]
            {
                try
                {
                    slot();
                }
                catch (const std::exception& stdException)
                {
                    qCritical()
                        << "Exception \""
                        << typeid(stdException).name()
                        << "\" on "
                        << pushButton.text()
                        << " button click: "
                        << stdException.what();
                }
            });
        }


        void readSelectionFromFile(QFile& file, DimensionSelectionHolder& holder)
        {
            holder.disableAllDimensions();

            while (!file.atEnd())
            {
                const auto timmedLine = file.readLine().trimmed();

                if (!timmedLine.isEmpty())
                {
                    const auto name = QString::fromUtf8(timmedLine);

                    if (!holder.tryToEnableDimensionByName(name))
                    {
                        qWarning() << "Failed to select dimension (name not found): " << name;
                    }
                }
            }
        }


        void readSelectionFromFile(const QString& fileName, QAbstractItemModel* const itemModel, DimensionSelectionHolder& holder)
        {
            if (!fileName.isEmpty())
            {
                QFile file(fileName);

                if (file.open(QIODevice::ReadOnly | QIODevice::Text))
                {
                    const ModelResetter modelResetter(itemModel);
                    readSelectionFromFile(file, holder);
                }
                else
                {
                    qCritical() << "Load failed to open file: " << fileName;
                }
            }
        }


        void writeSelectionToFile(const QString& fileName, const DimensionSelectionHolder& holder)
        {
            if (!fileName.isEmpty())
            {
                QFile file(fileName);

                if (file.open(QIODevice::WriteOnly | QIODevice::Text))
                {
                    const auto numberOfDimensions = holder.getNumberOfDimensions();

                    for (std::size_t i{}; i < numberOfDimensions; ++i)
                    {
                        if (holder.isDimensionEnabled(i))
                        {
                            file.write(holder.getName(i).toUtf8());
                            file.write("\n");
                        }
                    }
                }
                else
                {
                    qCritical() << "Save failed to open file: " << fileName;
                }
            }
        }

    }  // End of namespace.


    class DimensionSelectionWidget::Impl
    {
    public:
        const Ui_DimensionSelectionWidget _ui;

        std::unique_ptr<DimensionSelectionHolder> _holder = std::make_unique<DimensionSelectionHolder>();

        std::unique_ptr<DimensionSelectionItemModel> _itemModel;

        QMetaObject::Connection m_awakeConnection;


    private:
        void updateLabel()
        {
            const auto& holder = *_holder;

            _ui.label->setText(QObject::tr("%1 available, %2 visible, %3 selected").
                arg(holder.getNumberOfDimensions()).
                arg(holder.getNumberOfDimensions()).
                arg(holder.getNumberOfSelectedDimensions()) );
        }
    public:
        Impl(QWidget& widget)
            :
            _ui([&widget]
        {
            Ui_DimensionSelectionWidget ui;
            ui.setupUi(&widget);
            return ui;
        }())
        {
            updateLabel();

            connectPushButton(*_ui.loadPushButton, [this, &widget]
            {
                const auto fileName = QFileDialog::getOpenFileName(&widget,
                    QObject::tr("Dimension selection"), {}, getSelectionFileFilter());
                readSelectionFromFile(fileName, _itemModel.get(), *_holder);
            });

            connectPushButton(*_ui.savePushButton, [this, &widget]
            {
                const auto fileName = QFileDialog::getSaveFileName(&widget,
                    QObject::tr("Dimension selection"), {}, getSelectionFileFilter());
                writeSelectionToFile(fileName, *_holder);
            });

            // Reset the view "on idle".
            m_awakeConnection = connect(
                QAbstractEventDispatcher::instance(),
                &QAbstractEventDispatcher::awake,
                [this]
            {
                updateLabel();
            });
        }


        ~Impl()
        {
            disconnect(m_awakeConnection);
        }


        void setDimensions(
            const unsigned numberOfDimensions, const std::vector<QString>& names)
        {
            if (names.size() == numberOfDimensions)
            {
                _holder = std::make_unique<DimensionSelectionHolder>(
                    names.data(),
                    numberOfDimensions);

            }
            else
            {
                assert(names.empty());
                _holder = std::make_unique<DimensionSelectionHolder>(numberOfDimensions);
            }
            auto itemModel = std::make_unique<DimensionSelectionItemModel>(*_holder);
            _ui.treeView->setModel(&*itemModel);
            _itemModel = std::move(itemModel);
        }


        std::vector<bool> getEnabledDimensions() const
        {
            return _holder->getEnabledDimensions();
        }


    };


    DimensionSelectionWidget::DimensionSelectionWidget()
        :
        _pImpl(std::make_unique<Impl>(*this))
    {
    }

    DimensionSelectionWidget::~DimensionSelectionWidget() = default;


    void DimensionSelectionWidget::setDimensions(
        const unsigned numberOfDimensions, const std::vector<QString>& names)
    {
        _pImpl->setDimensions(numberOfDimensions, names);
    }

    std::vector<bool> DimensionSelectionWidget::getEnabledDimensions() const
    {
        return _pImpl->getEnabledDimensions();
    }

} // namespace hdps
