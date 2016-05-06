#include "photocheckwindow.h"


PhotoCheckWindow::PhotoCheckWindow(QWidget *parent)
	: QWidget(parent),
	m_files(NULL),
	m_current_file_index(0),
	m_max_file_index(0)
{
	ui.setupUi(this);

	this->setWindowFlags(Qt::Dialog);
	this->setWindowModality(Qt::WindowModal);//此处的模态设置无效，需要指定parent

	connect(ui.pushButton_backward, SIGNAL(clicked()), this, SLOT(checkBackward()));
	connect(ui.pushButton_forward, SIGNAL(clicked()), this, SLOT(checkForward()));
	connect(ui.pushButton_back, SIGNAL(clicked()), this, SLOT(close()));

}

PhotoCheckWindow::~PhotoCheckWindow()
{

}

void PhotoCheckWindow::checkBackward()
{
	if (m_current_file_index != 0)
		--m_current_file_index;

	QMetaObject::invokeMethod(this, "updateImage", Qt::QueuedConnection);
}

void PhotoCheckWindow::checkForward()
{
	if (m_current_file_index != m_max_file_index - 1)
		++m_current_file_index;

	QMetaObject::invokeMethod(this, "updateImage", Qt::QueuedConnection);
}

void PhotoCheckWindow::updateImage()
{
	Mat temp;
	temp = getMatFromFile(m_files[m_current_file_index]);
	Mat image_tmp(temp.rows, temp.cols, CV_8UC2, temp.data);
	//temp.convertTo(image_tmp, CV_8U/*, 0.062271*/);
	std::vector<Mat> mat_vector;
	split(image_tmp, mat_vector);
	QImage image_final = QImage(mat_vector[0].data, mat_vector[0].cols, mat_vector[0].rows, mat_vector[0].step, QImage::Format_Grayscale8);
	m_pixmap = QPixmap::fromImage(image_final);

//	ui.label->resize(m_pixmap.size());
	ui.label->setPixmap(m_pixmap);
	this->setWindowTitle(QString(QString::fromLocal8Bit(m_files[m_current_file_index].c_str())));
}

void PhotoCheckWindow::setPhotoSize(int h,int w)
{
	m_photo_height = h;
	m_photo_width = w;
}

Mat PhotoCheckWindow::getMatFromFile(cv::String path)
{
	Mat output = imread("error.bmp",CV_LOAD_IMAGE_GRAYSCALE);
	//Mat output(m_photo_height, m_photo_width, CV_16UC1, 0);
	output.convertTo(output, CV_16UC1/*, 16.058823*/);
	cv::String type = path.substr(path.find_last_of('.'));

	if (type == ".exr")
	{
		output = imread(path, CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
		output.convertTo(output, CV_16UC1);
	}
	else if (type == ".bin")
	{
		setPhotoSize(960,1280);
		std::ifstream ifs(path, std::ios::in | std::ios::binary);
		long len = m_photo_height * m_photo_width * 2;
		byte *temp;
		temp = new UCHAR[m_photo_height * m_photo_width * 2];
		ZeroMemory(temp, m_photo_height * m_photo_width * 2);
		ifs.read((char*)temp, len);
		ifs.close();
		Mat t = Mat(m_photo_height, m_photo_width, CV_16UC1, temp);
		t.copyTo(output);
		delete[] temp;
		temp = nullptr;
	}
	else if (type == ".hex")
	{
		setPhotoSize(640, 480);
		std::ifstream ifs(path, std::ios::in | std::ios::binary);
		long len = m_photo_height * m_photo_width * 2;
		byte *temp;
		temp = new UCHAR[m_photo_height * m_photo_width * 2];
		ZeroMemory(temp, m_photo_height * m_photo_width * 2);
		ifs.read((char*)temp, len);
		ifs.close();
		Mat t = Mat(m_photo_height, m_photo_width, CV_16UC1, temp);
		t.copyTo(output);
		delete[] temp;
		temp = nullptr;
	}

	return output;
}

void PhotoCheckWindow::setFileList(std::vector<std::string> &files)
{
	m_files = files;
	m_max_file_index = files.size();
}