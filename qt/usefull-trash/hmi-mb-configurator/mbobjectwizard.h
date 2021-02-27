#ifndef MBOBJECTWIZARD_H
#define MBOBJECTWIZARD_H


#include "mbobject.h"
#include <QWizard>
#include <QWizardPage>



class MbObjectWizard : public QWizard
{
    Q_OBJECT

public:
    MbObjectWizard(QWidget *parent = nullptr);
    void accept() override;

signals:
    void accepted(const MbObject &object);
};



/*
    PAGES
*/



class UiChooserPage;
class QObjectChooserPage;
class MbSettingsPage;



class UiChooserPage : public QWizardPage
{
    Q_OBJECT

public:
    UiChooserPage(QWidget *parent = nullptr);
    ~UiChooserPage() = default;
private:

};



class QObjectChooserPage : public QWizardPage
{
    Q_OBJECT

public:
    QObjectChooserPage(QWidget *parent = nullptr);
    ~QObjectChooserPage() = default;
private:

};



class MbSettingsPage : public QWizardPage
{
    Q_OBJECT

public:
    MbSettingsPage(QWidget *parent = nullptr);
    ~MbSettingsPage() = default;
private:

};


#endif // MBOBJECTWIZARD_H
