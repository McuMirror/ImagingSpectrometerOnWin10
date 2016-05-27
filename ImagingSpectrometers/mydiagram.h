#ifndef MYDIAGRAM_H
#define MYDIAGRAM_H

#include "CONFIG.h"

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>

#ifndef __CV__
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#define __CV__
#endif

using namespace cv;

class MyDiagram : public QWidget
{
	Q_OBJECT

public:
	MyDiagram(QWidget *parent=0);
	~MyDiagram();

	void transMat(Mat data,Mat w);
	void saveGraph(QString path);
	void setPaintEnable(bool flag);
	void paintEvent(QPaintEvent *);
	void refresh();

signals:
	void mouseMove(int x_pos, int y_pos);

protected:
	void mouseMoveEvent(QMouseEvent *e);
	void keyPressEvent(QKeyEvent  *event);

private:
	Mat m_data;
	Mat m_wavelength;
	QPainterPath *m_path;
	bool isPaintEnable;
	bool isValueVisable;
	std::vector<QPoint> points;

	int x_count;
	int x_initial;
	int x_offset;
	int y_offset;
	float deltaX;
	float deltaY;


private:
	
};

#endif // MYDIAGRAM_H
