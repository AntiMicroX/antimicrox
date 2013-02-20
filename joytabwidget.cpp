#include <QDebug>

#include "joytabwidget.h"
#include "joyaxiswidget.h"
#include "joybuttonwidget.h"
#include "xmlconfigreader.h"
#include "xmlconfigwriter.h"

JoyTabWidget::JoyTabWidget(Joystick *joystick, QWidget *parent) :
    QWidget(parent)
{
    this->joystick = joystick;

    verticalLayout = new QVBoxLayout (this);
    verticalLayout->setContentsMargins(4, 4, 4, 4);

    configHorizontalLayout = new QHBoxLayout();
    configBox = new QComboBox(this);
    configBox->addItem("<New>", "");

    configBox->setObjectName(QString::fromUtf8("configBox"));
    configBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    configHorizontalLayout->addWidget(configBox);
    spacer1 = new QSpacerItem(30, 20, QSizePolicy::Fixed, QSizePolicy::Fixed);
    configHorizontalLayout->addItem(spacer1);

    loadButton = new QPushButton(tr("Load"), this);
    loadButton->setObjectName(QString::fromUtf8("loadButton"));
    loadButton->setFixedWidth(100);
    loadButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    configHorizontalLayout->addWidget(loadButton);

    saveButton = new QPushButton(tr("Save"), this);
    saveButton->setObjectName(QString::fromUtf8("saveButton"));
    saveButton->setFixedWidth(100);
    saveButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    configHorizontalLayout->addWidget(saveButton);
    //configHorizontalLayout->setSpacing(-1);

    saveAsButton = new QPushButton(tr("Save As"), this);
    saveAsButton->setObjectName(QString::fromUtf8("saveAsButton"));
    saveAsButton->setFixedWidth(100);
    saveAsButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    configHorizontalLayout->addWidget(saveAsButton);

    verticalLayout->addLayout(configHorizontalLayout);
    verticalLayout->setStretchFactor(configHorizontalLayout, 1);

    spacer2 = new QSpacerItem(20, 5, QSizePolicy::Fixed, QSizePolicy::Fixed);
    verticalLayout->addItem(spacer2);

    gridLayout = new QGridLayout();
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    //QPushButton *testButton = new QPushButton (tr("Push Button"), this);
    //testButton->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    //gridLayout->addWidget(testButton);
    verticalLayout->addLayout(gridLayout);

    spacer3 = new QSpacerItem(20, 5, QSizePolicy::Fixed, QSizePolicy::Fixed);
    verticalLayout->addItem(spacer3);

    resetButton = new QPushButton(tr("Reset"), this);
    resetButton->setObjectName(QString::fromUtf8("resetButton"));
    resetButton->setFixedWidth(75);
    resetButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    verticalLayout->addWidget(resetButton, 0, Qt::AlignRight);

    fileDialog = new QFileDialog(this, "Open Config", QDir::currentPath(), "Config Files (*.xml)");

    connect(loadButton, SIGNAL(clicked()), this, SLOT(openConfigFileDialog()));
    connect(saveButton, SIGNAL(clicked()), this, SLOT(saveConfigFile()));
    connect(resetButton, SIGNAL(clicked()), this, SLOT(resetJoystick()));
    connect(configBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeJoyConfig(int)));
    connect(saveAsButton, SIGNAL(clicked()), this, SLOT(saveAsConfig()));
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
            if (configBox->count() == 5)
            {
                configBox->removeItem(4);
            }

            configBox->insertItem(1, fileinfo.baseName(), fileinfo.absoluteFilePath());
            configBox->setCurrentIndex(1);
            emit joystickConfigChanged(1);
        }
        else
        {
            configBox->setCurrentIndex(searchIndex);
            emit joystickConfigChanged(searchIndex);
        }
    }
}

void JoyTabWidget::fillButtons()
{
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

    int row = 0;
    int column = 0;
    for (int i=0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getJoyAxis(i);
        JoyAxisWidget *axisWidget = new JoyAxisWidget(axis, this);
        axisWidget->setText(axis->getName());
        axisWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        connect(axisWidget, SIGNAL(clicked()), this, SLOT(showAxisDialog()));

        if (column > 1)
        {
            column = 0;
            row++;
        }
        gridLayout->addWidget(axisWidget, row, column);
        column++;
    }

    for (int i=0; i < joystick->getNumberHats(); i++)
    {
        JoyDPad *dpad = joystick->getJoyDPad(i);
        QHash<int, JoyDPadButton*>::iterator i;
        for (i = dpad->getJoyButtons()->begin(); i != dpad->getJoyButtons()->end(); i++)
        {
            JoyDPadButton *button = (*i);
            JoyButtonWidget *dude = new JoyButtonWidget (button, this);
            dude->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
            connect (dude, SIGNAL(clicked()), this, SLOT(showButtonDialog()));

            if (column > 1)
            {
                column = 0;
                row++;
            }

            gridLayout->addWidget(dude, row, column);
            column++;
        }
    }

    for (int i=0; i < joystick->getNumberButtons(); i++)
    {
        JoyButton *button = joystick->getJoyButton(i);
        JoyButtonWidget *dude = new JoyButtonWidget (button, this);
        dude->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        connect (dude, SIGNAL(clicked()), this, SLOT(showButtonDialog()));

        if (column > 1)
        {
            column = 0;
            row++;
        }

        gridLayout->addWidget(dude, row, column);
        column++;
    }
}

void JoyTabWidget::showButtonDialog()
{
    QObject *sender = QObject::sender();
    JoyButtonWidget *buttonWidget = (JoyButtonWidget*)sender;
    JoyButton *button = buttonWidget->getJoyButton();

    buttonDialog = new ButtonEditDialog(button, this);
    buttonDialog->show();
    connect(buttonDialog, SIGNAL(destroyed()), buttonWidget, SLOT(refreshLabel()));
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

void JoyTabWidget::saveConfigFile()
{
    int index = configBox->currentIndex();
    QString filename;
    if (index == 0)
    {
        QString tempfilename = QFileDialog::getSaveFileName(this, "Save Config", QDir::currentPath(), "Config File (*.xml)");
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

        if (index == 0)
        {
            if (configBox->count() == 5)
            {
                configBox->removeItem(4);
            }

            configBox->insertItem(1, fileinfo.baseName(), fileinfo.absoluteFilePath());
            configBox->setCurrentIndex(1);
            emit joystickConfigChanged(1);
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
    if (index != 0)
    {
        QString configPath = configBox->itemData(index).toString();
        QFileInfo temp(configPath);
        QString tempfilename = QFileDialog::getSaveFileName(this, "Save Config", temp.absoluteDir().absolutePath(), "Config File (*.xml)");
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

        if (configBox->count() == 5)
        {
            configBox->removeItem(4);
        }

        configBox->insertItem(1, fileinfo.baseName(), fileinfo.absoluteFilePath());
        configBox->setCurrentIndex(1);
        emit joystickConfigChanged(1);
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
