#include "configdialog.h"

ConfigDialog::ConfigDialog(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.pushButton_back, SIGNAL(clicked()), this, SLOT(close()));

	QValidator *validator_times = new QIntValidator(1, 10, this);//输入数据的合法性，此处未根据实际情况限制，是否合法需要模型类进行进一步筛选
	ui.lineEdit_times->setValidator(validator_times);

}

ConfigDialog::~ConfigDialog()
{

}
