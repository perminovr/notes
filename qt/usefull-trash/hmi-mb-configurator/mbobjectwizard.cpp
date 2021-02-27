#include "mbobjectwizard.h"

#include <QDebug>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>



MbObjectWizard::MbObjectWizard(QWidget *parent)
    : QWizard(parent)
{
    addPage(new UiChooserPage);
    addPage(new QObjectChooserPage);
    addPage(new MbSettingsPage);

    setWindowTitle(tr("ModBus Object Wizard"));
}



void MbObjectWizard::accept()
{
    QByteArray className = field("className").toByteArray();

    qInfo() << "accepted: " << className << endl;

    QDialog::accept();

    QPushButton btn;
    btn.
}



/*
    PAGES
*/
