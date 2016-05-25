#ifndef MYLABEL_H
#define MYLABEL_H

#include <QLabel>
#include <QMouseEvent>

class MyLabel : public QLabel
{
	Q_OBJECT

public:
	MyLabel(QWidget *parent);
	~MyLabel();

signals:
	void mouseMove(int x_pos, int y_pos);
	void mousePress(int x_pos, int y_pos);

protected:
	void mouseMoveEvent(QMouseEvent *e);
	void mousePressEvent(QMouseEvent * e);
	
};

#endif // MYLABEL_H
