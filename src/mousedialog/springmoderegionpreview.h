#ifndef SPRINGMODEREGIONPREVIEW_H
#define SPRINGMODEREGIONPREVIEW_H

#include <QWidget>

class SpringModeRegionPreview : public QWidget
{
    Q_OBJECT
public:
    explicit SpringModeRegionPreview(int width = 0, int height = 0, QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *event);

    int adjustSpringSizeWidth(int width);
    int adjustSpringSizeHeight(int height);

signals:

public slots:
    void setSpringWidth(int width);
    void setSpringHeight(int height);
};

#endif // SPRINGMODEREGIONPREVIEW_H
