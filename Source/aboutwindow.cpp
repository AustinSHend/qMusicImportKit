#include "aboutwindow.h"
#include "ui_aboutwindow.h"

AboutWindow::AboutWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutWindow)
{
    ui->setupUi(this);

    // Disable resizing hints
    setWindowFlags(Qt::Widget | Qt::MSWindowsFixedSizeDialogHint);
}

AboutWindow::~AboutWindow()
{
    delete ui;
}
