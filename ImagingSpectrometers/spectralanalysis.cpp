#include "spectralanalysis.h"

SpectralAnalysis::SpectralAnalysis(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	Qt::WindowFlags flag = 0;

	flag = Qt::Window;
	flag |= Qt::WindowCloseButtonHint;

	this->setWindowFlags(flag);
	this->setWindowModality(Qt::WindowModal);//此处的模态设置无效，需要指定parent
	setCursor(Qt::CrossCursor); //设置鼠标为十字星

	connect(ui.diagram, SIGNAL(mouseMove(int, int)), this, SLOT(updateMousePos(int, int)));
}

SpectralAnalysis::~SpectralAnalysis()
{

}

void SpectralAnalysis::updateMousePos(int x, int y)
{
	ui.lineEdit_x->setText(QString::number(x));
	ui.lineEdit_y->setText(QString::number(y));
}