#include "photocheckwindow.h"

PhotoCheckWindow::PhotoCheckWindow(QWidget *parent)
	: QWidget(parent),
	m_files(NULL),
	m_current_file_index(0),
	m_max_file_index(0)
{
	ui.setupUi(this);
	Qt::WindowFlags flag = 0;

	flag = Qt::Window;
	flag |= Qt::WindowCloseButtonHint;

	this->setWindowFlags(flag);
	this->setWindowModality(Qt::WindowModal);//此处的模态设置无效，需要指定parent

	connect(ui.pushButton_backward, SIGNAL(clicked()), this, SLOT(checkBackward()));
	connect(ui.pushButton_forward, SIGNAL(clicked()), this, SLOT(checkForward()));
	connect(ui.pushButton_back, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui.label, SIGNAL(mouseMove(int, int)), this, SLOT(updateMousePos(int, int)));
	connect(ui.label, SIGNAL(mousePress(int, int)), this, SLOT(updateCurves(int, int)));

	setCursor(Qt::CrossCursor); //设置鼠标为十字星

}

PhotoCheckWindow::~PhotoCheckWindow()
{

}

void PhotoCheckWindow::checkBackward()
{
	if (m_current_file_index != 0)
	{
		--m_current_file_index;
	}
	else
	{
		m_current_file_index = m_max_file_index;
	}


	QMetaObject::invokeMethod(this, "updateImage", Qt::QueuedConnection);
}

void PhotoCheckWindow::checkForward()
{
	if (m_current_file_index != m_max_file_index)
	{
		++m_current_file_index;
	}
	else
	{
		m_current_file_index = 0;
	}

	QMetaObject::invokeMethod(this, "updateImage", Qt::QueuedConnection);
}

#ifdef __IN8BITS__
void PhotoCheckWindow::updateImage()
{
	if (m_files.empty())
	{
		return;
	}
	Mat temp;
	temp = getMatFromFile(m_files[m_current_file_index]);

	//Mat hist_mat;
	//hist_mat = gethist(temp);
	Mat temp2;
	temp.convertTo(temp2, CV_8UC1/*, 16.00366*/);

	QImage image_final = QImage(temp2.data, temp2.cols, temp2.rows, temp2.step, QImage::Format_Grayscale8);
	//QImage image_final = QImage(hist_mat.data, hist_mat.cols, hist_mat.rows, hist_mat.step, QImage::Format_Grayscale8);
	m_pixmap = QPixmap::fromImage(image_final);

//	ui.label->resize(m_pixmap.size());
	ui.label->setPixmap(m_pixmap);
	this->setWindowTitle(QString(QString::fromLocal8Bit(m_files[m_current_file_index].c_str())));
}
#else
void PhotoCheckWindow::updateImage()
{
	Mat temp;
	temp = getMatFromFile(m_files[m_current_file_index]);

	//Mat hist_mat;
	//hist_mat = gethist(temp);
	Mat temp2;
	temp.convertTo(temp2, CV_16UC1/*, 16.00366*/);

	Mat image_tmp(temp2.rows, temp2.cols, CV_8UC2, temp2.data);
	//temp.convertTo(image_tmp, CV_8U/*, 0.062271*/);
	std::vector<Mat> mat_vector;
	split(image_tmp, mat_vector);
	Mat low8_mat = mat_vector[0];
	Mat high8_mat = mat_vector[1];

	QImage image_final = QImage(mat_vector[0].data, mat_vector[0].cols, mat_vector[0].rows, mat_vector[0].step, QImage::Format_Grayscale8);
	//QImage image_final = QImage(hist_mat.data, hist_mat.cols, hist_mat.rows, hist_mat.step, QImage::Format_Grayscale8);
	m_pixmap = QPixmap::fromImage(image_final);

	//	ui.label->resize(m_pixmap.size());
	ui.label->setPixmap(m_pixmap);
	this->setWindowTitle(QString(QString::fromLocal8Bit(m_files[m_current_file_index].c_str())));
}
#endif

Mat PhotoCheckWindow::gethist(Mat &temp)
{
	Mat image_16UC1_raw;
	image_16UC1_raw = Mat(temp.rows, temp.cols, CV_16UC1, temp.data);

	/// 设定bin数目
	int histSize = 0x0FFF;

	/// 设定取值范围 ( R,G,B) )
	float range[] = { 0, 0x0FFF };
	const float* histRange = { range };

	bool uniform = true; bool accumulate = false;
	Mat hist;
	calcHist(&image_16UC1_raw, 1, 0, Mat(), hist, 1, &histSize, &histRange, uniform, accumulate);

	Mat final;
	final.create(image_16UC1_raw.rows, image_16UC1_raw.cols, CV_8UC1);
	USHORT *ptr = image_16UC1_raw.ptr<USHORT>();
	UCHAR *ptr_dst = final.ptr<UCHAR>();

	for (int i = 0; i < image_16UC1_raw.total(); i++)
	{
		ptr_dst[i] = min(sqrt( ptr[i] * 65535.0 / 4095.0),255.0);
	}
	return final;
}

void PhotoCheckWindow::setPhotoSize(int h,int w)
{
	m_photo_height = h;
	m_photo_width = w;
}

#ifdef __IN8BITS__
Mat PhotoCheckWindow::getMatFromFile(cv::String path)
{
	Mat output = imread("error.bmp",CV_LOAD_IMAGE_GRAYSCALE);
	//Mat output(m_photo_height, m_photo_width, CV_16UC1, 0);
	output.convertTo(output, CV_8UC1/*, 16.058823*/);
	cv::String type = path.substr(path.find_last_of('.'));

	if (type == ".bmp")
	{
		output = imread(path, CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
		output.convertTo(output, CV_8UC1);
	}
	else if (type == ".bin")
	{
		setPhotoSize(960,1280);
		std::ifstream ifs(path, std::ios::in | std::ios::binary);
		long len = m_photo_height * m_photo_width;
		byte *temp;
		temp = new UCHAR[m_photo_height * m_photo_width];
		ZeroMemory(temp, m_photo_height * m_photo_width);
		ifs.read((char*)temp, len);
		ifs.close();
		Mat t = Mat(m_photo_height, m_photo_width, CV_8UC1, temp);
		t.copyTo(output);
		delete[] temp;
		temp = nullptr;
	}
	else if (type == ".hex")
	{
		setPhotoSize(640, 480);
		std::ifstream ifs(path, std::ios::in | std::ios::binary);
		long len = m_photo_height * m_photo_width;
		byte *temp;
		temp = new UCHAR[m_photo_height * m_photo_width];
		ZeroMemory(temp, m_photo_height * m_photo_width);
		ifs.read((char*)temp, len);
		ifs.close();
		Mat t = Mat(m_photo_height, m_photo_width, CV_8UC1, temp);
		t.copyTo(output);
		delete[] temp;
		temp = nullptr;
	}

	return output;
}
#else
Mat PhotoCheckWindow::getMatFromFile(cv::String path)
{
	Mat output = imread("error.bmp", CV_LOAD_IMAGE_GRAYSCALE);
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
		setPhotoSize(960, 1280);
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
#endif

void PhotoCheckWindow::setFileList(std::vector<std::string> &files)
{
	m_files = files;
	m_max_file_index = files.size()-1;
}

void PhotoCheckWindow::updateMousePos(int x,int y)
{
	ui.lineEdit_x->setText(QString::number(x));
	ui.lineEdit_y->setText (QString::number(y));
}

String PhotoCheckWindow::getCurrentFilePath()
{
	return m_files[m_current_file_index];
}