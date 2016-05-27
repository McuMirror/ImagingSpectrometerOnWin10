#include "myspecthread.h"

MySpecThread::MySpecThread(QObject *parent)
	: QThread(parent)
{

}

MySpecThread::MySpecThread(ImagingSpectrometers *p)
{
	m_pImSpec = p;
}

MySpecThread::~MySpecThread()
{

}

void MySpecThread::run()
{
	//判断文件夹是否为空
	std::vector<std::string> correctionMatVector;
	std::vector<std::string> photoMatVector;
	cv::String correction_path = m_pImSpec->getCorrectionPath();
	cv::String photo_path = m_pImSpec->getPhotoPath();
	cv::String custom_path = m_pImSpec->getCustomPath();
	LONG photo_data_size = m_pImSpec->getPhotoHeight() * m_pImSpec->getPhotoWidth();

	m_pImSpec->getFileList(correction_path, correctionMatVector);
	m_pImSpec->getFileList(photo_path + custom_path + "/", photoMatVector);
	int size = correctionMatVector.size() - 1;
	Mat temp = Mat(2, size, CV_8UC1, 0);
	if (size != photoMatVector.size())
	{
		return;
	}

	Mat allWavelengthMat = Mat::zeros(1, size, CV_16UC1);
	Mat allCurvesMat = Mat::zeros(photo_data_size, size, CV_32FC1);
	Mat temp_c, temp_p, temp_curves, temp_oneCol;
	std::string str = "//";

	for (int i = 0; i < size; i++)
	{
		temp_c = m_pImSpec->getMatFromFile(correctionMatVector[i]);
		temp_c.convertTo(temp_c, CV_32F);
		temp_p = m_pImSpec->getMatFromFile(photoMatVector[i]);
		temp_p.convertTo(temp_p, CV_32F);
		divide(temp_p, temp_c, temp_curves);
		temp_oneCol = temp_curves.reshape(0, photo_data_size);
		temp_oneCol.copyTo(allCurvesMat.col(i));

		int n = correctionMatVector[i].find_last_of(str);
		std::string str2 = correctionMatVector[i].substr(n + 2);
		int t = str2.find(".");
		QString str3 = QString::fromStdString(str2.substr(0, t));
		allWavelengthMat.at<USHORT>(i) = str3.toInt();
		emit processUpdate(i * 99 / size);
	}
	m_pImSpec->m_curvesMat = allCurvesMat;
	m_pImSpec->m_wavelengthMat = allWavelengthMat;

	emit processUpdate(100);

	return;
}