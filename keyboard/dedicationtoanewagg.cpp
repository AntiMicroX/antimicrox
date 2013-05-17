#include <QDebug>

#include "dedicationtoanewagg.h"
#include "ui_dedicationtoanewagg.h"

#include "event.h"

DedicationToANewAgg::DedicationToANewAgg(JoyButton *button, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DedicationToANewAgg)
{
    ui->setupUi(this);

    this->button = button;
    qDebug() << keyToKeycode("Up") << endl;
    qDebug() << "Question: " << keyToKeycode("numbersign") << endl;
    qDebug() << "Question: " << keycodeToKey(11) << endl;

    connect(ui->pushButton_20, SIGNAL(clicked()), this, SLOT(shitfuck()));
}

DedicationToANewAgg::~DedicationToANewAgg()
{
    delete ui;
}

void DedicationToANewAgg::shitfuck()
{
    qDebug() << "I Hate myself and want to die" << endl;
    QPushButton *button = static_cast<QPushButton*> (sender());
    qDebug() << button->text() << endl;
    qDebug() << "Question: " << keyToKeycode(button->text()) << endl;
    this->button->setAssignedSlot(keyToKeycode(button->text()), JoyButtonSlot::JoyKeyboard);
}
