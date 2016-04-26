#include "configdialog.h"

ConfigDialog::ConfigDialog(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.pushButton_back, SIGNAL(clicked()), this, SLOT(close()));

	QValidator *validator_times = new QIntValidator(1, 10, this);//�������ݵĺϷ��ԣ��˴�δ����ʵ��������ƣ��Ƿ�Ϸ���Ҫģ������н�һ��ɸѡ
	ui.lineEdit_times->setValidator(validator_times);

}

ConfigDialog::~ConfigDialog()
{

}
