#ifndef FLASHBUTTONWIDGET_H
#define FLASHBUTTONWIDGET_H

#include <QPushButton>
#include <QPaintEvent>

class FlashButtonWidget : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(bool isflashing READ isButtonFlashing)

public:
    explicit FlashButtonWidget(QWidget *parent = 0);
    explicit FlashButtonWidget(bool displayNames, QWidget *parent = 0);

    bool isButtonFlashing();
    void setDisplayNames(bool display);
    bool isDisplayingNames();

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual QString generateLabel() = 0;
    virtual void retranslateUi();

    bool isflashing;
    bool displayNames;
    bool leftAlignText;

signals:
    void flashed(bool flashing);

public slots:
    void refreshLabel();
    void toggleNameDisplay();
    virtual void disableFlashes() = 0;
    virtual void enableFlashes() = 0;

protected slots:
    void flash();
    void unflash();
};

#endif // FLASHBUTTONWIDGET_H
