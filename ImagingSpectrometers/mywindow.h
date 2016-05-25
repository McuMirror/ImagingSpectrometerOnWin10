#ifndef MYWINDOW_H
#define MYWINDOW_H

#include <QtWidgets/QMainWindow>
#include <direct.h>

#ifndef __IMAGINGSPEC__
#include "imagingspectrometers.h"
#define __IMAGINGSPEC__
#endif

#include "ui_mywindow.h"
#include "configdialog.h"
#include "photocheckwindow.h"
#include "spectralanalysis.h"
#include "myspecthread.h"

class MyWindow : public QMainWindow
{
	Q_OBJECT

public:
	MyWindow(QWidget *parent = 0);
	~MyWindow();
	void initializeUI();
	void initializePath();
	void setLCTFUiEnable(bool flag);
	void setCameraUiEnable(bool flag);

public slots:
	void openCamera();
	void closeCamera();
	void pauseCamera();
	void updateImage();
	void showFPS();

	//bool prepareLCTF();
	void openLCTF();
	void whiteboard();
	void photo();
	void photoOnce();

	void setWavelengthGap();
	void setExposureTime();
	void setWavelength();
	void setDigitalGain();
	void setExposureTime_lineEdit();
	void setWavelength_lineEdit();
	void setDigitalGain_lineEdit();
	void setAnalogGain();

	void showConfigDialog();
	void setConfig();

	void setCurvesMat(int x, int y);

	void showPhotoCheckWindow();
	void closePhotoCheckWindow();


private:
	ImagingSpectrometers *m_mydevice;//模型类，负责数据加工
	ConfigDialog *m_config_dialog;
	PhotoCheckWindow *m_photocheck_window;
	SpectralAnalysis *m_specAnalysis_window;
	QThread m_thread;//子线程，用于将模型类中的dowork依附于此子线程
	MySpecThread *m_thread_curves;//用于生成光谱曲线，在拍照后执行
	QTimer *m_timer;
	int m_frame_count;

private:
	Ui::MyWindowClass ui;

};

#endif // MYWINDOW_H
