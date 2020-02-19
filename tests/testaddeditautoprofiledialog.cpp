#include "addeditautoprofiledialog.h"

#include <QtTest/QtTest>


/*

    AutoProfileInfo* getAutoProfile() const; // if not empty then
    1) profileLineEdit must be equal to getAutoProfile()->getProfileLocation()
    2) devicesComboBox must be equal to getAutoProfile()->getDeviceName()
    3) winClassLineEdit must be equal to getAutoProfile()->getWindowClass()
    4) if getAutoProfile()->isPartialState() then should be setPartialCheckBox->checked()
    5) if getAutoProfile()->isCurrentDefault() then should be asDefaultCheckBox->checked()
    6) if getAutoProfile()->getWindowName() is not empty then should be winNameLineEdit->text() == getAutoProfile()->getWindowName()
    7) if getAutoProfile()->getExe() is not empty then should be applicationLineEdit->text() == getAutoProfile()->getExe()

    //QString getOriginalGUID() const;
    QString getOriginalUniqueID() const;
    QString getOriginalExe() const;
    QString getOriginalWindowClass() const;
    QString getOriginalWindowName() const;

    QList<InputDevice*> *getDevices() const; // cannot be empty
    AntiMicroSettings *getSettings() const; // cannot be empty
    bool getEditForm() const;
    bool getDefaultInfo() const;
    // QList<QString> const& getReservedGUIDs();
    QList<QString> const& getReservedUniques(); // cannot be empty


    profileBrowsePushButton not disabled
    detectWinPropsSelectWindowPushButton not disabled
    devicesComboBox not disabled and not empty
    asDefaultCheckBox not disabled if devicesComboBox first element is not -1
    asDefaultCheckBox disabled if devicesComboBox first element is -1
    buttonBox Cancel not disabled
    applicationPushButton not disabled
    selectWindowPushButton not disabled
    profileLineEdit not disabled
    winClassLineEdit not disabled
    winNameLineEdit not disabled
    applicationLineEdit not disabled
    buttonBox Ok disabled if profileLineEdit is empty and doesn't exist and devicesComboBox first element is empty and winClassLineEdit is empty
    buttonBox Ok enabled if profileLineEdit isn't empty and exist and devicesComboBox first element is not empty and winClassLineEdit is not empty

    test keyClick on profileBrowsePushButton
    test keyClick on detectWinPropsSelectWindowPushButton
    test keyClick on buttonBox->cancel
    test keyClick on applicationPushButton
    test keyClick on selectWindowPushButton

    test mouseClick on profileBrowsePushButton
    test mouseClick on detectWinPropsSelectWindowPushButton
    test mouseClick on buttonBox->cancel
    test mouseClick on applicationPushButton
    test mouseClick on selectWindowPushButton
*/


class TestAddEditAutoProfileDialog: public QObject
{
    Q_OBJECT

public:
    TestAddEditAutoProfileDialog(QObject* parent = 0);
    void checkAutoProfile();
    void checkOriginalUniqueIDs();


private slots:

private:
    AddEditAutoProfileDialog addEditAutoProfileDialog;
};

TestAddEditAutoProfileDialog::TestAddEditAutoProfileDialog(QObject* parent) :
    QObject(parent),
    addEditAutoProfileDialog()
{
    QTestEventLoop::instance().enterLoop(1);
}

// QTEST_MAIN(TestAddEditAutoProfileDialog)
#include "testaddeditautoprofiledialog.moc"
