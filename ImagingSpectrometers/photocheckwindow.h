#ifndef PHOTOCHECKWINDOW_H
#define PHOTOCHECKWINDOW_H

#include <QWidget>
#include "ui_photocheckwindow.h"

#ifndef __WIN__
#include <Windows.h>
#include <fstream>
#include <io.h>
#define __WIN__
#endif

#ifndef __CV__
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#define __CV__
#endif

using namespace cv;

class PhotoCheckWindow : public QWidget
{
	Q_OBJECT

public:
	PhotoCheckWindow(QWidget *parent = 0);
	~PhotoCheckWindow();
	Ui::PhotoCheckWindow ui;
	void setFileList(std::vector<std::string> &files);
	void setPhotoSize(int h, int w);
	Mat getMatFromFile(cv::String path);
	Mat gethist(Mat &t);
	String getCurrentFilePath();

public slots:
	void checkBackward();
	void checkForward();
	void updateImage();
	void updateMousePos(int x,int y);
	
private:
	std::vector<std::string> m_files;
	int m_current_file_index;
	int m_max_file_index;
	int m_photo_height;
	int m_photo_width;
public:
	QPixmap m_pixmap;

public:
	
//	void mousePressEvent(QMouseEvent * e);
//	void mouseReleaseEvent(QMouseEvent * e);
//	void mouseDoubleClickEvent(QMouseEvent * e);
	
};

#endif // PHOTOCHECKWINDOW_H
