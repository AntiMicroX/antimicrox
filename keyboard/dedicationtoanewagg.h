#ifndef DEDICATIONTOANEWAGG_H
#define DEDICATIONTOANEWAGG_H

#include <QDialog>
#include <QPushButton>

#include "joybutton.h"
#include "event.h"

namespace Ui {
class DedicationToANewAgg;
}

class DedicationToANewAgg : public QDialog
{
    Q_OBJECT
    
public:
    explicit DedicationToANewAgg(JoyButton *button, QWidget *parent = 0);
    ~DedicationToANewAgg();
    
private:
    Ui::DedicationToANewAgg *ui;

protected:
    JoyButton *button;

    void setupKeyboardLayout();
    void setupMouseLayout();
    void createKey();


private slots:
    void shitfuck();
};

#endif // DEDICATIONTOANEWAGG_H
