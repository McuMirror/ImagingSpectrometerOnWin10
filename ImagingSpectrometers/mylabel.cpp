#include "mylabel.h"

MyLabel::MyLabel(QWidget *parent)
	: QLabel(parent)
{
	this->setMouseTracking(true);
}

MyLabel::~MyLabel()
{

}

void MyLabel::mouseMoveEvent(QMouseEvent *e)
{
	emit mouseMove(e->x(), e->y());
}

void MyLabel::mousePressEvent(QMouseEvent *e)
{
	if (e->button() == Qt::LeftButton){
		emit mousePress(e->x(), e->y());
	}
	else if (e->button() == Qt::RightButton){
		
	}
	else if (e->button() == Qt::MidButton){
		
	}
	
}