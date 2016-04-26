#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QWidget>
#include "ui_configdialog.h"

class ConfigDialog : public QWidget
{
	Q_OBJECT

public:
	ConfigDialog(QWidget *parent = 0);
	~ConfigDialog();
	Ui::ConfigDialog ui;//…Ë÷√≥…public
private:

};

#endif // CONFIGDIALOG_H
