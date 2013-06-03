#include <QDebug>

#include "joytabwidget.h"
#include "joyaxiswidget.h"
#include "joybuttonwidget.h"
#include "xmlconfigreader.h"
#include "xmlconfigwriter.h"
#include "buttoneditdialog.h"
#include "joycontrolstickeditdialog.h"
#include "joycontrolstickpushbutton.h"

JoyTabWidget::JoyTabWidget(Joystick *joystick, QWidget *parent) :
    QWidget(parent)
{
    this->joystick = joystick;

    verticalLayout = new QVBoxLayout (this);
    verticalLayout->setContentsMargins(4, 4, 4, 4);

    configHorizontalLayout = new QHBoxLayout();
    configBox = new QComboBox(this);
    configBox->addItem(tr("<New>"), "");

    configBox->setObjectName(QString::fromUtf8("configBox"));
    configBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    configHorizontalLayout->addWidget(configBox);
    spacer1 = new QSpacerItem(30, 20, QSizePolicy::Fixed, QSizePolicy::Fixed);
    configHorizontalLayout->addItem(spacer1);

    loadButton = new QPushButton(tr("Load"), this);
    loadButton->setObjectName(QString::fromUtf8("loadButton"));
    //loadButton->setFixedWidth(100);
    loadButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    loadButton->setIcon(QIcon::fromTheme("document-open"));
    configHorizontalLayout->addWidget(loadButton);

    saveButton = new QPushButton(tr("Save"), this);
    saveButton->setObjectName(QString::fromUtf8("saveButton"));
    //saveButton->setFixedWidth(100);
    saveButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    saveButton->setIcon(QIcon::fromTheme("document-save"));
    configHorizontalLayout->addWidget(saveButton);
    //configHorizontalLayout->setSpacing(-1);

    saveAsButton = new QPushButton(tr("Save As"), this);
    saveAsButton->setObjectName(QString::fromUtf8("saveAsButton"));
    //saveAsButton->setFixedWidth(100);
    saveAsButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    saveAsButton->setIcon(QIcon::fromTheme("document-save-as"));
    configHorizontalLayout->addWidget(saveAsButton);

    verticalLayout->addLayout(configHorizontalLayout);
    verticalLayout->setStretchFactor(configHorizontalLayout, 1);

    spacer2 = new QSpacerItem(20, 5, QSizePolicy::Fixed, QSizePolicy::Fixed);
    verticalLayout->addItem(spacer2);

    verticalSpacer_2 = new QSpacerItem(20, 5, QSizePolicy::Minimum, QSizePolicy::Fixed);

    verticalLayout->addItem(verticalSpacer_2);


    stackedWidget_2 = new QStackedWidget(this);
    stackedWidget_2->setObjectName(QString::fromUtf8("stackedWidget_2"));

    page = new QWidget();
    page->setObjectName(QString::fromUtf8("page"));

    QVBoxLayout *tempVBoxLayout = new QVBoxLayout(page);
    QScrollArea *scrollArea = new QScrollArea(page);
    scrollArea->setObjectName(QString::fromUtf8("scrollArea1"));
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //sizePolicy.setHorizontalStretch(0);
    //sizePolicy.setVerticalStretch(0);
    scrollArea->setSizePolicy(sizePolicy);
    scrollArea->setWidgetResizable(true);

    QWidget *scrollAreaWidgetContents1 = new QWidget();
    scrollAreaWidgetContents1->setObjectName(QString::fromUtf8("scrollAreaWidgetContents1"));

    gridLayout = new QGridLayout(scrollAreaWidgetContents1);
    gridLayout->setSpacing(4);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));

    scrollArea->setWidget(scrollAreaWidgetContents1);
    tempVBoxLayout->addWidget(scrollArea);
    stackedWidget_2->addWidget(page);

    page_2 = new QWidget();
    page_2->setObjectName(QString::fromUtf8("page_2"));

    tempVBoxLayout = new QVBoxLayout(page_2);
    QScrollArea *scrollArea2 = new QScrollArea(page_2);
    scrollArea2->setObjectName(QString::fromUtf8("scrollArea2"));
    scrollArea2->setSizePolicy(sizePolicy);
    scrollArea2->setWidgetResizable(true);

    QWidget *scrollAreaWidgetContents2 = new QWidget();
    scrollAreaWidgetContents2->setObjectName(QString::fromUtf8("scrollAreaWidgetContents2"));

    gridLayout2 = new QGridLayout(scrollAreaWidgetContents2);
    gridLayout2->setSpacing(4);
    gridLayout2->setObjectName(QString::fromUtf8("gridLayout2"));

    scrollArea2->setWidget(scrollAreaWidgetContents2);
    tempVBoxLayout->addWidget(scrollArea2);
    stackedWidget_2->addWidget(page_2);


    page_3 = new QWidget();
    page_3->setObjectName(QString::fromUtf8("page_3"));

    tempVBoxLayout = new QVBoxLayout(page_3);
    QScrollArea *scrollArea3 = new QScrollArea(page_3);
    scrollArea3->setObjectName(QString::fromUtf8("scrollArea3"));
    scrollArea3->setSizePolicy(sizePolicy);
    scrollArea3->setWidgetResizable(true);

    QWidget *scrollAreaWidgetContents3 = new QWidget();
    scrollAreaWidgetContents3->setObjectName(QString::fromUtf8("scrollAreaWidgetContents3"));

    gridLayout3 = new QGridLayout(scrollAreaWidgetContents3);
    gridLayout3->setSpacing(4);
    gridLayout3->setObjectName(QString::fromUtf8("gridLayout3"));

    scrollArea3->setWidget(scrollAreaWidgetContents3);
    tempVBoxLayout->addWidget(scrollArea3);
    stackedWidget_2->addWidget(page_3);


    page_4 = new QWidget();
    page_4->setObjectName(QString::fromUtf8("page_4"));

    tempVBoxLayout = new QVBoxLayout(page_4);
    QScrollArea *scrollArea4 = new QScrollArea(page_4);
    scrollArea4->setObjectName(QString::fromUtf8("scrollArea4"));
    scrollArea4->setSizePolicy(sizePolicy);
    scrollArea4->setWidgetResizable(true);

    QWidget *scrollAreaWidgetContents4 = new QWidget();
    scrollAreaWidgetContents4->setObjectName(QString::fromUtf8("scrollAreaWidgetContents4"));

    gridLayout4 = new QGridLayout(scrollAreaWidgetContents4);
    gridLayout4->setSpacing(4);
    gridLayout4->setObjectName(QString::fromUtf8("gridLayout4"));

    scrollArea4->setWidget(scrollAreaWidgetContents4);
    tempVBoxLayout->addWidget(scrollArea4);
    stackedWidget_2->addWidget(page_4);

    page_5 = new QWidget();
    page_5->setObjectName(QString::fromUtf8("page_5"));

    tempVBoxLayout = new QVBoxLayout(page_5);
    QScrollArea *scrollArea5 = new QScrollArea(page_5);
    scrollArea5->setObjectName(QString::fromUtf8("scrollArea5"));
    scrollArea5->setSizePolicy(sizePolicy);
    scrollArea5->setWidgetResizable(true);

    QWidget *scrollAreaWidgetContents5 = new QWidget();
    scrollAreaWidgetContents5->setObjectName(QString::fromUtf8("scrollAreaWidgetContents5"));

    gridLayout5 = new QGridLayout(scrollAreaWidgetContents5);
    gridLayout5->setSpacing(4);
    gridLayout5->setObjectName(QString::fromUtf8("gridLayout5"));

    scrollArea5->setWidget(scrollAreaWidgetContents5);
    tempVBoxLayout->addWidget(scrollArea5);
    stackedWidget_2->addWidget(page_5);

    page_6 = new QWidget();
    page_6->setObjectName(QString::fromUtf8("page_6"));

    tempVBoxLayout = new QVBoxLayout(page_6);
    QScrollArea *scrollArea6 = new QScrollArea(page_6);
    scrollArea6->setObjectName(QString::fromUtf8("scrollArea6"));
    scrollArea6->setSizePolicy(sizePolicy);
    scrollArea6->setWidgetResizable(true);

    QWidget *scrollAreaWidgetContents6 = new QWidget();
    scrollAreaWidgetContents6->setObjectName(QString::fromUtf8("scrollAreaWidgetContents6"));

    gridLayout6 = new QGridLayout(scrollAreaWidgetContents6);
    gridLayout6->setSpacing(4);
    gridLayout6->setObjectName(QString::fromUtf8("gridLayout6"));

    scrollArea6->setWidget(scrollAreaWidgetContents6);
    tempVBoxLayout->addWidget(scrollArea6);
    stackedWidget_2->addWidget(page_6);

    page_7 = new QWidget();
    page_7->setObjectName(QString::fromUtf8("page_7"));

    tempVBoxLayout = new QVBoxLayout(page_7);
    QScrollArea *scrollArea7 = new QScrollArea(page_7);
    scrollArea7->setObjectName(QString::fromUtf8("scrollArea7"));
    scrollArea7->setSizePolicy(sizePolicy);
    scrollArea7->setWidgetResizable(true);

    QWidget *scrollAreaWidgetContents7 = new QWidget();
    scrollAreaWidgetContents7->setObjectName(QString::fromUtf8("scrollAreaWidgetContents7"));

    gridLayout7 = new QGridLayout(scrollAreaWidgetContents7);
    gridLayout7->setSpacing(4);
    gridLayout7->setObjectName(QString::fromUtf8("gridLayout7"));

    scrollArea7->setWidget(scrollAreaWidgetContents7);
    tempVBoxLayout->addWidget(scrollArea7);
    stackedWidget_2->addWidget(page_7);

    page_8 = new QWidget();
    page_8->setObjectName(QString::fromUtf8("page_8"));

    tempVBoxLayout = new QVBoxLayout(page_8);
    QScrollArea *scrollArea8 = new QScrollArea(page_8);
    scrollArea8->setObjectName(QString::fromUtf8("scrollArea8"));
    scrollArea8->setSizePolicy(sizePolicy);
    scrollArea8->setWidgetResizable(true);

    QWidget *scrollAreaWidgetContents8 = new QWidget();
    scrollAreaWidgetContents8->setObjectName(QString::fromUtf8("scrollAreaWidgetContents8"));

    gridLayout8 = new QGridLayout(scrollAreaWidgetContents8);
    gridLayout8->setSpacing(4);
    gridLayout8->setObjectName(QString::fromUtf8("gridLayout8"));

    scrollArea8->setWidget(scrollAreaWidgetContents8);
    tempVBoxLayout->addWidget(scrollArea8);
    stackedWidget_2->addWidget(page_8);

    verticalLayout->addWidget(stackedWidget_2);

    verticalSpacer_3 = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Fixed);
    verticalLayout->addItem(verticalSpacer_3);


    horizontalLayout_2 = new QHBoxLayout();
    horizontalLayout_2->setSpacing(6);
    horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
    setPushButton1 = new QPushButton("1", this);
    setPushButton1->setObjectName(QString::fromUtf8("setPushButton1"));
    setPushButton1->setProperty("setActive", true);

    horizontalLayout_2->addWidget(setPushButton1);

    setPushButton2 = new QPushButton("2", this);
    setPushButton2->setObjectName(QString::fromUtf8("setPushButton2"));
    setPushButton2->setProperty("setActive", false);

    horizontalLayout_2->addWidget(setPushButton2);

    setPushButton3 = new QPushButton("3", this);
    setPushButton3->setObjectName(QString::fromUtf8("setPushButton3"));
    setPushButton3->setProperty("setActive", false);

    horizontalLayout_2->addWidget(setPushButton3);

    setPushButton4 = new QPushButton("4", this);
    setPushButton4->setObjectName(QString::fromUtf8("setPushButton4"));
    setPushButton4->setProperty("setActive", false);

    horizontalLayout_2->addWidget(setPushButton4);

    setPushButton5 = new QPushButton("5", this);
    setPushButton5->setObjectName(QString::fromUtf8("setPushButton5"));
    setPushButton5->setProperty("setActive", false);

    horizontalLayout_2->addWidget(setPushButton5);

    setPushButton6 = new QPushButton("6", this);
    setPushButton6->setObjectName(QString::fromUtf8("setPushButton6"));
    setPushButton6->setProperty("setActive", false);

    horizontalLayout_2->addWidget(setPushButton6);

    setPushButton7 = new QPushButton("7", this);
    setPushButton7->setObjectName(QString::fromUtf8("setPushButton7"));
    setPushButton7->setProperty("setActive", false);

    horizontalLayout_2->addWidget(setPushButton7);

    setPushButton8 = new QPushButton("8", this);
    setPushButton8->setObjectName(QString::fromUtf8("setPushButton8"));
    setPushButton8->setProperty("setActive", false);

    horizontalLayout_2->addWidget(setPushButton8);

    verticalLayout->addLayout(horizontalLayout_2);

    spacer3 = new QSpacerItem(20, 5, QSizePolicy::Fixed, QSizePolicy::Fixed);
    verticalLayout->addItem(spacer3);

    resetButton = new QPushButton(tr("Reset"), this);
    resetButton->setObjectName(QString::fromUtf8("resetButton"));
    resetButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    resetButton->setIcon(QIcon::fromTheme("document-revert"));
    verticalLayout->addWidget(resetButton, 0, Qt::AlignRight);

    fileDialog = new QFileDialog(this, tr("Open Config"), QDir::currentPath(), "Config Files (*.xml)");

    connect(loadButton, SIGNAL(clicked()), this, SLOT(openConfigFileDialog()));
    connect(saveButton, SIGNAL(clicked()), this, SLOT(saveConfigFile()));
    connect(resetButton, SIGNAL(clicked()), this, SLOT(resetJoystick()));
    connect(configBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeJoyConfig(int)));
    connect(saveAsButton, SIGNAL(clicked()), this, SLOT(saveAsConfig()));

    connect(setPushButton1, SIGNAL(clicked()), this, SLOT(changeSetOne()));
    connect(setPushButton2, SIGNAL(clicked()), this, SLOT(changeSetTwo()));
    connect(setPushButton3, SIGNAL(clicked()), this, SLOT(changeSetThree()));
    connect(setPushButton4, SIGNAL(clicked()), this, SLOT(changeSetFour()));
    connect(setPushButton5, SIGNAL(clicked()), this, SLOT(changeSetFive()));
    connect(setPushButton6, SIGNAL(clicked()), this, SLOT(changeSetSix()));
    connect(setPushButton7, SIGNAL(clicked()), this, SLOT(changeSetSeven()));
    connect(setPushButton8, SIGNAL(clicked()), this, SLOT(changeSetEight()));
}

void JoyTabWidget::openConfigFileDialog()
{
    QStringList filenames;
    QString filename;

    if (fileDialog->exec())
    {
        filenames = fileDialog->selectedFiles();
        filename = filenames.at(0);
    }

    if (!filename.isEmpty())
    {
        XMLConfigReader reader;
        //QFile *configFile = new QFile(filename);
        reader.setFileName(filename);
        reader.configJoystick(joystick);

        fillButtons();

        QFileInfo fileinfo(filename);
        int searchIndex = configBox->findData(fileinfo.absoluteFilePath());
        if (searchIndex == -1)
        {
            if (configBox->count() == 6)
            {
                configBox->removeItem(5);
            }

            configBox->insertItem(1, fileinfo.baseName(), fileinfo.absoluteFilePath());
            configBox->setCurrentIndex(1);
            emit joystickConfigChanged(joystick->getJoyNumber());
        }
        else
        {
            configBox->setCurrentIndex(searchIndex);
            emit joystickConfigChanged(joystick->getJoyNumber());
        }
    }
}

void JoyTabWidget::fillButtons()
{
    /*
    QWidget *child;
    //QList<QPushButton*> list = old_layout->findChildren<QPushButton*>();
    //qDeleteAll(list);
    while (gridLayout && gridLayout->count() > 0)
    {
        child = gridLayout->takeAt(0)->widget();
        gridLayout->removeWidget (child);
        //child->deleteLater();
        delete child;
        child = 0;
    }
    */

    for (int i=0; i < Joystick::NUMBER_JOYSETS; i++)
    {
        int row = 0;
        int column = 0;

        QWidget *child;
        QGridLayout *current_layout;
        switch (i)
        {
            case 0:
            {
                current_layout = gridLayout;
                break;
            }
            case 1:
            {
                current_layout = gridLayout2;
                break;
            }
            case 2:
            {
                current_layout = gridLayout3;
                break;
            }
            case 3:
            {
                current_layout = gridLayout4;
                break;
            }
            case 4:
            {
                current_layout = gridLayout5;
                break;
            }
            case 5:
            {
                current_layout = gridLayout6;
                break;
            }
            case 6:
            {
                current_layout = gridLayout7;
                break;
            }
            case 7:
            {
                current_layout = gridLayout8;
                break;
            }
            default:
                break;
        }

        while (current_layout && current_layout->count() > 0)
        {
            child = current_layout->takeAt(0)->widget();
            current_layout->removeWidget (child);
            delete child;
            child = 0;
        }

        connect (joystick, SIGNAL(setChangeActivated(int)), this, SLOT(changeCurrentSet(int)));

        for (int j=0; j < joystick->getNumberSticks(); j++)
        {
            JoyControlStick *stick = joystick->getSetJoystick(i)->getJoyStick(j);
            JoyControlStickPushButton *stickWidget = new JoyControlStickPushButton(stick, this);

            stickWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
            stickWidget->setMinimumSize(316, 30);

            connect(stickWidget, SIGNAL(clicked()), this, SLOT(showStickDialog()));

            if (column > 1)
            {
                column = 0;
                row++;
            }
            current_layout->addWidget(stickWidget, row, column);
            column++;
        }

        for (int j=0; j < joystick->getNumberAxes(); j++)
        {
            //JoyAxis *axis = joystick->getJoyAxis(i);
            JoyAxis *axis = joystick->getSetJoystick(i)->getJoyAxis(j);
            if (!axis->isPartControlStick())
            {
                JoyAxisWidget *axisWidget = new JoyAxisWidget(axis, this);
                axisWidget->setText(axis->getName());
                axisWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
                //axisWidget->setMinimumHeight(30);
                axisWidget->setMinimumSize(316, 30);

                connect(axisWidget, SIGNAL(clicked()), this, SLOT(showAxisDialog()));
                connect(axis, SIGNAL(throttleChanged()), axisWidget, SLOT(refreshLabel()));

                if (column > 1)
                {
                    column = 0;
                    row++;
                }
                current_layout->addWidget(axisWidget, row, column);
                column++;
            }
        }

        for (int j=0; j < joystick->getNumberHats(); j++)
        {
            JoyDPad *dpad = joystick->getSetJoystick(i)->getJoyDPad(j);
            QHash<int, JoyDPadButton*>::iterator iter;
            for (iter = dpad->getJoyButtons()->begin(); iter != dpad->getJoyButtons()->end(); iter++)
            {
                JoyDPadButton *button = (*iter);
                JoyButtonWidget *dude = new JoyButtonWidget (button, this);
                dude->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
                connect (dude, SIGNAL(clicked()), this, SLOT(showButtonDialog()));
                dude->setText(dude->text());
                //dude->setMinimumHeight(30);
                dude->setMinimumSize(316, 30);

                if (column > 1)
                {
                    column = 0;
                    row++;
                }

                current_layout->addWidget(dude, row, column);
                column++;
            }
        }

        for (int j=0; j < joystick->getNumberButtons(); j++)
        {
            JoyButton *button = joystick->getSetJoystick(i)->getJoyButton(j);
            JoyButtonWidget *dude = new JoyButtonWidget (button, this);
            dude->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
            connect (dude, SIGNAL(clicked()), this, SLOT(showButtonDialog()));
            dude->setText(dude->text());
            //dude->setMinimumHeight(30);
            dude->setMinimumSize(316, 30);

            if (column > 1)
            {
                column = 0;
                row++;
            }

            current_layout->addWidget(dude, row, column);
            column++;
        }
    }
}

void JoyTabWidget::showButtonDialog()
{
    QObject *sender = QObject::sender();
    JoyButtonWidget *buttonWidget = (JoyButtonWidget*)sender;
    JoyButton *button = buttonWidget->getJoyButton();

    ButtonEditDialog *dialog = new ButtonEditDialog(button, this);
    dialog->show();
    connect(dialog, SIGNAL(finished(int)), buttonWidget, SLOT(refreshLabel()));
}

void JoyTabWidget::showAxisDialog()
{
    QObject *sender = QObject::sender();
    JoyAxisWidget *axisWidget = (JoyAxisWidget*) sender;
    JoyAxis *axis = axisWidget->getAxis();

    axisDialog = new AxisEditDialog (axis, this);
    axisDialog->show();
    connect(axisDialog, SIGNAL(destroyed()), axisWidget, SLOT(refreshLabel()));
}

void JoyTabWidget::showStickDialog()
{
    QObject *sender = QObject::sender();
    JoyControlStickPushButton *stickWidget = (JoyControlStickPushButton*) sender;
    JoyControlStick *stick = stickWidget->getStick();

    JoyControlStickEditDialog *dialog = new JoyControlStickEditDialog (stick, this);
    dialog->show();
    connect(dialog, SIGNAL(destroyed()), stickWidget, SLOT(refreshLabel()));
}

void JoyTabWidget::saveConfigFile()
{
    int index = configBox->currentIndex();
    QString filename;
    if (index == 0)
    {
        QString tempfilename = QFileDialog::getSaveFileName(this, tr("Save Config"), QDir::currentPath(), "Config File (*.xml)");
        if (!tempfilename.isEmpty())
        {
            filename = tempfilename;
        }
    }
    else
    {
        filename = configBox->itemData(index).value<QString> ();
    }

    if (!filename.isEmpty())
    {
        QFileInfo fileinfo(filename);
        if (fileinfo.suffix() != "xml")
        {
            filename = filename.append(".xml");
        }
        fileinfo.setFile(filename);

        XMLConfigWriter writer;
        writer.setFileName(fileinfo.absoluteFilePath());
        writer.write(joystick);

        int existingIndex = configBox->findData(fileinfo.absoluteFilePath());
        if (existingIndex == -1)
        {
            if (configBox->count() == 6)
            {
                configBox->removeItem(5);
            }

            configBox->insertItem(1, fileinfo.baseName(), fileinfo.absoluteFilePath());
            configBox->setCurrentIndex(1);
            emit joystickConfigChanged(joystick->getJoyNumber());
        }
        else
        {
            configBox->setCurrentIndex(existingIndex);
            emit joystickConfigChanged(joystick->getJoyNumber());
        }
    }
}

void JoyTabWidget::resetJoystick()
{
    int currentIndex = configBox->currentIndex();
    if (currentIndex != 0)
    {
        XMLConfigReader reader;

        QString filename = configBox->itemData(currentIndex).toString();

        reader.setFileName(filename);
        reader.configJoystick(joystick);
        fillButtons();
    }
    else
    {
        joystick->reset();
        fillButtons();
    }
    /*else
    {
        emit joystickRefreshRequested(joystick);
    }*/
}

void JoyTabWidget::saveAsConfig()
{
    int index = configBox->currentIndex();
    QString filename;
    if (index == 0)
    {
        QString tempfilename = QFileDialog::getSaveFileName(this, tr("Save Config"), QDir::currentPath(), "Config File (*.xml)");
        if (!tempfilename.isEmpty())
        {
            filename = tempfilename;
        }
    }
    else
    {
        QString configPath = configBox->itemData(index).toString();
        QFileInfo temp(configPath);
        QString tempfilename = QFileDialog::getSaveFileName(this, tr("Save Config"), temp.absoluteDir().absolutePath(), "Config File (*.xml)");
        if (!tempfilename.isEmpty())
        {
            filename = tempfilename;
        }
    }

    if (!filename.isEmpty())
    {
        QFileInfo fileinfo(filename);
        if (fileinfo.suffix() != "xml")
        {
            filename = filename.append(".xml");
        }
        fileinfo.setFile(filename);

        XMLConfigWriter writer;
        writer.setFileName(fileinfo.absoluteFilePath());
        writer.write(joystick);

        int existingIndex = configBox->findData(fileinfo.absoluteFilePath());
        if (existingIndex == -1)
        {
            if (configBox->count() == 6)
            {
                configBox->removeItem(5);
            }

            configBox->insertItem(1, fileinfo.baseName(), fileinfo.absoluteFilePath());
            configBox->setCurrentIndex(1);
            emit joystickConfigChanged(joystick->getJoyNumber());
        }
        else
        {
            configBox->setCurrentIndex(existingIndex);
            emit joystickConfigChanged(joystick->getJoyNumber());
        }
    }
}

void JoyTabWidget::changeJoyConfig(int index)
{
    QString filename;

    if (index != 0)
    {
        filename = configBox->itemData(index).toString();
    }

    if (!filename.isEmpty())
    {
        XMLConfigReader reader;
        //QFile *configFile = new QFile(filename);

        reader.setFileName(filename);
        reader.configJoystick(joystick);

        fillButtons();

        //QFileInfo fileinfo(filename);
        //int searchIndex = configBox->findData(fileinfo.absoluteFilePath());
        //configBox->setCurrentIndex(searchIndex);
    }
    else
    {
        emit joystickRefreshRequested(joystick);
        //QMetaObject::invokeMethod(joystick, "reset");
        //joystick->reset();
        //fillButtons();
    }

    //fillButtons(joystick);
}

void JoyTabWidget::saveSettings(QSettings *settings)
{
    QString filename = "";
    QString lastfile = "";

    int joyindex = joystick->getRealJoyNumber();
    int index = configBox->currentIndex();
    int currentjoy = 1;

    QString controlString = QString("Controller%1ConfigFile%2").arg(QString::number(joyindex));
    QString controlLastSelected = QString("Controller%1LastSelected").arg(QString::number(joyindex));

    if (index != 0)
    {
        filename = lastfile = configBox->itemData(index).toString();
        settings->setValue(controlString.arg(currentjoy), filename);
        currentjoy++;
    }
    else
    {
        lastfile = "";
    }

    for (int i=1; i < configBox->count(); i++)
    {
        if (i != index)
        {
           filename = configBox->itemData(i).toString();
           settings->setValue(controlString.arg(currentjoy), filename);
           currentjoy++;
        }
    }

    settings->setValue(controlLastSelected, lastfile);
}

void JoyTabWidget::loadSettings(QSettings *settings)
{
    int joyindex = joystick->getRealJoyNumber();
    QString controlString = QString("Controllers/Controller%1ConfigFile%2").arg(QString::number(joyindex));
    QString controlLastSelected = QString("Controllers/Controller%1LastSelected").arg(QString::number(joyindex));

    QString file1 = settings->value(controlString.arg(QString::number(1)), "").toString();

    if (!file1.isEmpty())
    {
        QFileInfo fileInfo(file1);
        if (configBox->findData(fileInfo.absoluteFilePath()) == -1)
        {
            configBox->addItem(fileInfo.baseName(), fileInfo.absoluteFilePath());
        }
    }
    QString file2 = settings->value(controlString.arg(QString::number(2)), "").toString();
    if (!file2.isEmpty())
    {
        QFileInfo fileInfo(file2);
        if (configBox->findData(fileInfo.absoluteFilePath()) == -1)
        {
            configBox->addItem(fileInfo.baseName(), fileInfo.absoluteFilePath());
        }
    }
    QString file3 = settings->value(controlString.arg(QString::number(3)), "").toString();
    if (!file3.isEmpty())
    {
        QFileInfo fileInfo(file3);
        if (configBox->findData(fileInfo.absoluteFilePath()) == -1)
        {
            configBox->addItem(fileInfo.baseName(), fileInfo.absoluteFilePath());
        }
    }
    QString file4 = settings->value(controlString.arg(QString::number(4)), "").toString();
    if (!file4.isEmpty())
    {
        QFileInfo fileInfo(file4);
        if (configBox->findData(fileInfo.absoluteFilePath()) == -1)
        {
            configBox->addItem(fileInfo.baseName(), fileInfo.absoluteFilePath());
        }
    }
    QString file5 = settings->value(controlString.arg(QString::number(5)), "").toString();
    if (!file5.isEmpty())
    {
        QFileInfo fileInfo(file5);
        if (configBox->findData(fileInfo.absoluteFilePath()) == -1)
        {
            configBox->addItem(fileInfo.baseName(), fileInfo.absoluteFilePath());
        }
    }

    QString lastfile = settings->value(controlLastSelected).toString();
    if (!lastfile.isEmpty())
    {
        int lastindex = configBox->findData(lastfile);
        if (lastindex > 0)
        {
            configBox->setCurrentIndex(lastindex);
        }
    }

}

QHash<int, QString>* JoyTabWidget::recentConfigs()
{
    QHash<int, QString> *temp = new QHash<int, QString> ();
    for (int i=1; i < configBox->count(); i++)
    {
        QString current = configBox->itemText(i);
        temp->insert(i, current);
    }

    return temp;
}

void JoyTabWidget::setCurrentConfig(int index)
{
    if (index >= 1 && index < configBox->count())
    {
        configBox->setCurrentIndex(index);
    }
}

int JoyTabWidget::getCurrentConfigIndex()
{
    return configBox->currentIndex();
}

QString JoyTabWidget::getCurrentConfigName()
{
    return configBox->currentText();
}

void JoyTabWidget::changeCurrentSet(int index)
{
    int currentPage = stackedWidget_2->currentIndex();
    QPushButton *oldSetButton;
    QPushButton *activeSetButton;

    switch (currentPage)
    {
        case 0: oldSetButton = setPushButton1; break;
        case 1: oldSetButton = setPushButton2; break;
        case 2: oldSetButton = setPushButton3; break;
        case 3: oldSetButton = setPushButton4; break;
        case 4: oldSetButton = setPushButton5; break;
        case 5: oldSetButton = setPushButton6; break;
        case 6: oldSetButton = setPushButton7; break;
        case 7: oldSetButton = setPushButton8; break;
        default: break;
    }

    oldSetButton->setProperty("setActive", false);
    oldSetButton->style()->unpolish(oldSetButton);
    oldSetButton->style()->polish(oldSetButton);

    joystick->setActiveSetNumber(index);
    stackedWidget_2->setCurrentIndex(index);

    switch (index)
    {
        case 0: activeSetButton = setPushButton1; break;
        case 1: activeSetButton = setPushButton2; break;
        case 2: activeSetButton = setPushButton3; break;
        case 3: activeSetButton = setPushButton4; break;
        case 4: activeSetButton = setPushButton5; break;
        case 5: activeSetButton = setPushButton6; break;
        case 6: activeSetButton = setPushButton7; break;
        case 7: activeSetButton = setPushButton8; break;
        default: break;
    }

    activeSetButton->setProperty("setActive", true);
    activeSetButton->style()->unpolish(activeSetButton);
    activeSetButton->style()->polish(activeSetButton);
}

void JoyTabWidget::changeSetOne()
{
    changeCurrentSet(0);
}

void JoyTabWidget::changeSetTwo()
{
    changeCurrentSet(1);
}

void JoyTabWidget::changeSetThree()
{
    changeCurrentSet(2);
}

void JoyTabWidget::changeSetFour()
{
    changeCurrentSet(3);
}

void JoyTabWidget::changeSetFive()
{
    changeCurrentSet(4);
}

void JoyTabWidget::changeSetSix()
{
    changeCurrentSet(5);
}

void JoyTabWidget::changeSetSeven()
{
    changeCurrentSet(6);
}

void JoyTabWidget::changeSetEight()
{
    changeCurrentSet(7);
}
