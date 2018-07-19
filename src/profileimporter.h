#ifndef PROFILEIMPORTER_H
#define PROFILEIMPORTER_H

#include <QWidget>

namespace Ui {
class ProfileImporter;
}

class ProfileImporter : public QWidget
{
    Q_OBJECT

public:
    explicit ProfileImporter(QWidget *parent = 0);
    ~ProfileImporter();

private:
    Ui::ProfileImporter *ui;
};

#endif // PROFILEIMPORTER_H
