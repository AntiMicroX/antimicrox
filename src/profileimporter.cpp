#include "profileimporter.h"
#include "ui_profileimporter.h"

ProfileImporter::ProfileImporter(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProfileImporter)
{
    ui->setupUi(this);
}

ProfileImporter::~ProfileImporter()
{
    delete ui;
}
