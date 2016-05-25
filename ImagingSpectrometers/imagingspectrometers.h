#pragma once
#include <QtWidgets>

#ifndef __WIN__
#include <Windows.h>
#include <fstream>
#include <io.h>
#define __WIN__
#endif

#ifndef __CYAPI__
#include "CyAPI.h"
#define __CYAPI__
#endif

#ifndef __CV__
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#define __CV__
#endif

#define WAVELENGTH 100
#define DIGITALGAIN 200
#define EXPOSURETIME 201
#define ANALOGGAIN 202

using namespace cv;

class ImagingSpectrometers:public QObject
{
	Q_OBJECT

public:
	ImagingSpectrometers(QObject *parent = 0);
	~ImagingSpectrometers();

	bool isDeviceConnected();
	bool openDevice();//打开相机的部分
	void closeDevice();
	bool controlDevice(int TYPE,int value);

	bool isLCTFReady();
	//读取LCTF的部分
	//LCTF控制不需要在相机关闭后关闭，只有接收图像的线程是受closeDevice影响的
	bool getLCTFState();
	bool controlLCTF(int TYPE, int value);

	void getMainWindow(QObject* obj);//获取主窗口指针
	void setWorkFlag(bool flag);
	void setCloseFlag(bool flag);

	void setWavelengthGap(int gap);
	int getWavelengthGap();
	int getWavelengthMin();
	int getWavelengthMax();
	int getWavelengthNow();
	int getPhotoWidth();
	int getPhotoHeight();
	void setResolution(int width, int height);

	void pauseDevice();
	void continueDevice();

	void getData();//获取原始数据
	void getImage();//变为图像显示
	void getImage_hist();
	Mat takephoto();
	Mat getMatFromFile(String path);

	void getCurvesMat();
	//bool writeCurvesMat();
	//bool readCurvesMat();

	String getCorrectionPath();
	String getPhotoPath();
	String getSystemPath();
	void setCustomPath(String);
	String getCustomPath();
	void setPhotoTimes(int);
	int getPhotoTimes();

	bool writeConfigFile(String, byte *,int);
	bool readConfigFile(String, char *);
	void getFileList(std::string path, std::vector<std::string>& files);

public:
	CCyUSBDevice *m_usb_device;
	CCyUSBEndPoint *m_ept_bulk_in;    //块接收端点
	CCyControlEndPoint *m_ept_control;  //控制端点

	QPixmap m_pixmap;//显示用图片
	Mat m_curvesMat;//光谱曲线数据
	Mat m_wavelengthMat;//与光谱曲线对应波长值

private:
	const int m_data_height;
	const int m_data_width;

	const int m_image_height;
	const int m_image_width;

	const int m_photo_height;
	const int m_photo_width;

	bool m_is_LCTF_ready;
	int m_LCTF_min;
	int m_LCTF_max;
	int m_LCTF_range;
	int m_LCTF_gap;
	int m_LCTF_bandcount;
	int m_LCTF_wavelength;

	const LONG m_image_data_size;
	const LONG m_photo_data_size;
	QObject *m_mainWindow;//主线程指针

	String m_system_path;
	String m_correction_path;
	String m_photo_path;
	String m_custom_path;
	int m_photo_times;

	bool m_is_working;
	bool m_is_closed;
	QMutex m_lock;//线程锁

	UCHAR *m_image_data;
	UCHAR *m_photo_data;
	Mat m_image_16bit;//处理前16位数据
	Mat m_final_image;//处理后数据

private slots:
	void doWork();

};

