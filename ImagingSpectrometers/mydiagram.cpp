#include "mydiagram.h"

MyDiagram::MyDiagram(QWidget *parent)
	: QWidget(parent)
{
	x_initial = 0;
	x_offset = 0;
	y_offset = 0;
	deltaX = 0.0;
	deltaY = 0.0;
#ifdef __VISIBLE__
	x_count = 32;//此处可变
#else
	x_count = 45;
#endif

	m_path = new QPainterPath;
	isPaintEnable = false;
	isValueVisable = false;
	this->setMouseTracking(true);
	this->setFocusPolicy(Qt::StrongFocus);

}

MyDiagram::~MyDiagram()
{
	delete m_path;
}

void MyDiagram::setPaintEnable(bool flag)
{
	isPaintEnable = flag;//控制绘图进行
}

void MyDiagram::paintEvent(QPaintEvent *)
{
	///////////////////初始化坐标轴
	QPainter painter(this);

	QFontMetrics metrics = painter.fontMetrics();
	int textHeight = metrics.ascent() + metrics.descent();

	int leftWidth = metrics.width(tr("100")) + 5;//坐标轴到widget边框的左边距离，下面类似
	int rightWidth = 30;
	int topHeight = metrics.height() + 2;
	int bottomHeight = metrics.height() + 6;
	int width = this->size().width() - leftWidth - rightWidth;
	int height = this->size().height() - topHeight - bottomHeight;
	x_offset = leftWidth;//这两个偏置在坐标显示的时候要用
	y_offset = bottomHeight;//

	// 绘制外框
	painter.drawRect(0, 0, this->size().width() - 1, this->size().height() - 1);
	//　移动坐标系，把坐标原点从左上移动到坐标系原点
	painter.translate(leftWidth, height + topHeight);

	painter.setPen(QPen(Qt::black, 1)); //设置画笔颜色和大小
	QPoint beginPoint(0, 0);
	QPoint xEndPoint(width, 0);
	QPoint yEndPoint(0, -height);
	painter.drawLine(beginPoint, xEndPoint);
	painter.drawLine(beginPoint, yEndPoint);//*注意X轴偏移了1单位，尚未解决

	int count = 10;
	deltaX = width / (float)(x_count+1.0);         // x坐标上每分的宽度（46而不是45，是故意留出来一部分空间，下同）
	deltaY = (float)height / 10.5f; // y坐标上每分的宽度
	//X轴绘制
	painter.drawText(width, textHeight, tr("(nm)"));

	for (int i = 0; i <= x_count; ++i) {
		QString month = tr("%1").arg(x_initial + i * 10);
		int stringWidth = metrics.width(month);

		// 绘制坐标刻度
		painter.drawLine(deltaX * i, 0, deltaX * i, 4);

		// 绘制坐标文字
		int monthX = deltaX * i;
		painter.drawText(monthX, textHeight + 4, month);
	}
	//Y轴绘制
	painter.drawText(-metrics.width(tr("(%)")),
		-(deltaY * count + textHeight + metrics.descent()),
		tr("(%)"));

	for (int i = 0; i <= count; ++i) {
		QString value = QString("%1").arg(i * 100 / count);
		int stringWidth = metrics.width(value);

		// 绘制坐标刻度
		painter.drawLine(-4, -i * deltaY, 0, -i * deltaY);

		// 绘制坐标值
		painter.drawText(-stringWidth - 4, -(deltaY * i + textHeight / 2 - metrics.ascent()), value);
	}
	
	///////////////////数据绘制
	QPainter painter_curve(this);
	painter_curve.translate(leftWidth, height + topHeight);

	//painter_curve.drawLine(0, 0, deltaX/10, -deltaY / 10);
	for (int j = 0; j <m_wavelength.cols; j++)
	{
		QPoint m_Point;
		if (j == 0)
		{
			x_initial = points[j].x();
			m_Point.setX((points[j].x()-x_initial)*(deltaX / 10));//deltaX是X轴每10nm的轴长
			m_Point.setY(- points[j].y()*(deltaY / 10));//注意Y轴需要颠倒
			m_path->moveTo(m_Point);
			painter_curve.setPen(QPen(Qt::blue, 3)); //设置画笔颜色和大小
			painter_curve.drawPoint(m_Point);
			if (isValueVisable)
			{
				painter_curve.setPen(QPen(Qt::blue, 1));
				painter_curve.drawText(m_Point, QString::number(points[j].y()));
			}
			
		}
		else
		{
			m_Point.setX((points[j].x() - x_initial)*(deltaX / 10));
			m_Point.setY(-points[j].y()*(deltaY / 10));
			m_path->lineTo(m_Point);
			painter_curve.setPen(QPen(Qt::blue, 3));
			painter_curve.drawPoint(m_Point);
			if (isValueVisable)
			{
				painter_curve.setPen(QPen(Qt::blue, 1));
				painter_curve.drawText(m_Point, QString::number(points[j].y()));
			}
		}
		
	}

	painter_curve.setPen(QPen(Qt::red, 1));
	painter_curve.drawPath(*m_path);

}

void MyDiagram::refresh()
{
	if (!isPaintEnable)
	{
		return;
	}
	else if (m_data.empty())
	{
		return;
	}
	delete m_path;
	m_path = new QPainterPath;
	points.clear();//非常重要，清空上幅图像的点集
	x_count = m_data.cols - 1;

	QPoint temp_point;
	for (int i = 0; i < m_data.cols; i++)
	{
		temp_point.setX(m_wavelength.at<unsigned short>(i));
		temp_point.setY(m_data.at<float>(i) * 100);
		points.push_back(temp_point);
	}
	//根据波长值调整点的顺序
	std::sort(points.begin(), points.end(), [](QPoint& p1, QPoint& p2)->bool { return p1.x() < p2.x(); });

	update();
}

void MyDiagram::transMat(Mat data,Mat w)
{
	m_data = data;
	m_wavelength = w;
}

void MyDiagram::saveGraph(QString path)
{
	QPixmap pix = grab(this->rect());
	pix.save(path);
}

void MyDiagram::mouseMoveEvent(QMouseEvent *e)
{
	int x_current;
	int y_current;//坐标变换

	x_current = (e->x() - x_offset) / (deltaX / 10) + x_initial;
	if (x_current < x_initial)
	{
		x_current = x_initial;
	}
	y_current = (this->size().height() - e->y() - y_offset) / (deltaY / 10);
	if (y_current < 0)
	{
		y_current = 0;
	}
	else if (y_current>100)
	{
		y_current = 100;
	}
	emit mouseMove(x_current, y_current);
}

void MyDiagram::keyPressEvent(QKeyEvent  *event)
{
	if (event->key() == Qt::Key_Alt)
	{
		isValueVisable = !isValueVisable;
	}
	update();
}