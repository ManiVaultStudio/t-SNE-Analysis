#pragma once

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

class HsneRecomputeWarningDialog : public QDialog
{
public:
    HsneRecomputeWarningDialog() : QDialog()
    {
        setWindowTitle("Restarting HSNE");

        QLabel* warnText = new QLabel("The top level HSNE embedding has at least one refined scale.\n Recomputing the analysis might have unintended consequences.");
        QPushButton* stopButton = new QPushButton("Stop");
        QPushButton* continueButton = new QPushButton("Continue");

        QObject::connect(continueButton, &QPushButton::clicked, [&]() {
            accept();
        });

        QObject::connect(stopButton, &QPushButton::clicked, [&]() {
            reject();
        });

        QHBoxLayout* buttonLayout = new QHBoxLayout;
        buttonLayout->addWidget(stopButton);
        buttonLayout->addWidget(continueButton);

        QVBoxLayout* layout = new QVBoxLayout;
        layout->addWidget(warnText);
        layout->addLayout(buttonLayout);

        setLayout(layout);
    }
};
