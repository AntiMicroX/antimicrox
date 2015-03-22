#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QSpacerItem>

#include "mousesettingspage.h"

#include <joybutton.h>

MouseSettingsPage::MouseSettingsPage(AntiMicroSettings *settings, QWidget *parent) :
    QWizardPage(parent)
{
    this->settings = settings;

    setTitle(tr("Mouse Settings"));
    setSubTitle(tr("Customize settings used for mouse emulation"));
    QVBoxLayout *tempMainLayout = new QVBoxLayout;
    setLayout(tempMainLayout);

#ifdef Q_OS_WIN
    QCheckBox *disablePrecision = new QCheckBox(tr("Disable Enhance Pointer Precision"));
    disablePrecision->setToolTip(tr("Disable the \"Enhanced Pointer Precision\" Windows setting\n"
                                    "while antimicro is running. Disabling \"Enhanced Pointer Precision\"\n"
                                    "will allow mouse movement within antimicro to be more\n"
                                    "precise."));
    tempMainLayout->addWidget(disablePrecision);
    tempMainLayout->addSpacerItem(new QSpacerItem(10, 10));
    registerField("disableEnhancePrecision", disablePrecision);
#endif

    QGroupBox *smoothingGroupBox = new QGroupBox(tr("Smoothing"));
    smoothingGroupBox->setCheckable(true);
    smoothingGroupBox->setChecked(false);
    registerField("mouseSmoothing", smoothingGroupBox, "checked", SIGNAL(toggled(bool)));

    layout()->addWidget(smoothingGroupBox);

    QVBoxLayout *tempVLayout = new QVBoxLayout;

    QHBoxLayout *tempHLayout = new QHBoxLayout;
    QLabel *tempLabel = new QLabel(tr("History Buffer:"));
    QSpinBox *tempSpinBox = new QSpinBox;
    tempSpinBox->setMinimum(1);
    tempSpinBox->setMaximum(30);
    tempSpinBox->setValue(10);
    tempLabel->setBuddy(tempSpinBox);
    registerField("historyBuffer", tempSpinBox);

    tempHLayout->addWidget(tempLabel);
    tempHLayout->addWidget(tempSpinBox);
    tempVLayout->addLayout(tempHLayout);

    tempHLayout = new QHBoxLayout;
    tempLabel = new QLabel(tr("Weight Modifier:"));
    QDoubleSpinBox *tempDoubleSpinBox = new QDoubleSpinBox;
    tempDoubleSpinBox->setMinimum(0.0);
    tempDoubleSpinBox->setMaximum(1.0);
    tempDoubleSpinBox->setValue(0.20);
    tempDoubleSpinBox->setSingleStep(0.10);
    tempLabel->setBuddy(tempDoubleSpinBox);
    registerField("weightModifier", tempDoubleSpinBox);

    tempHLayout->addWidget(tempLabel);
    tempHLayout->addWidget(tempDoubleSpinBox);
    tempVLayout->addLayout(tempHLayout);

    smoothingGroupBox->setLayout(tempVLayout);

    tempHLayout = new QHBoxLayout;
    tempLabel = new QLabel(tr("Refresh Rate:"));
    QComboBox *tempComboBox = new QComboBox;

    for (int i = 1; i <= JoyButton::MAXIMUMMOUSEREFRESHRATE; i++)
    {
        tempComboBox->addItem(QString("%1 ms").arg(i), i);
    }

    int refreshIndex = tempComboBox->findData(JoyButton::getMouseRefreshRate());
    if (refreshIndex >= 0)
    {
        tempComboBox->setCurrentIndex(refreshIndex);
    }

    tempComboBox->setToolTip(tr("The refresh rate is the amount of time that will elapse\n"
                                "in between mouse events. Please be cautious when\n"
                                "editing this setting as it will cause the program to use\n"
                                "more CPU power. Setting this value too low can cause\n"
                                "system instability. Please test the setting before using\n"
                                "it unattended."));
    tempLabel->setBuddy(tempComboBox);
    registerField("mouseRefreshRate", tempComboBox);

    tempHLayout->addWidget(tempLabel);
    tempHLayout->addWidget(tempComboBox);
    tempMainLayout->addSpacerItem(new QSpacerItem(10, 10));
    tempMainLayout->addLayout(tempHLayout);
}

/**
 * @brief Determine whether the page should be initialized. Defaults to
 *   false.
 * @param AntiMicroSettings instance
 * @return If page instance should be created.
 */
bool MouseSettingsPage::shouldDisplay(AntiMicroSettings *settings)
{
    bool result = false;
    // Only show page if no saved settings exist.
    if (settings->allKeys().size() == 0)
    {
        result = true;
    }

    return result;
}
