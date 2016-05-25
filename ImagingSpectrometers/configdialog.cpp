#include "configdialog.h"

ConfigDialog::ConfigDialog(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	//Qt::WindowFlags flag = 0;
	//flag = Qt::Window;
	//flag |= Qt::WindowCloseButtonHint;

	//this->setWindowFlags(flag);
	//this->setWindowModality(Qt::WindowModal);//此处的模态设置无效，需要指定parent

	connect(ui.pushButton_back, SIGNAL(clicked()), this, SLOT(close()));

	QValidator *validator_times = new QIntValidator(1, 10, this);//输入数据的合法性，此处未根据实际情况限制，是否合法需要模型类进行进一步筛选
	ui.lineEdit_times->setValidator(validator_times);

}

ConfigDialog::~ConfigDialog()
{

}
