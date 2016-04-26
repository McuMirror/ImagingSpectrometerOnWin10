#include "imagingspectrometers.h"


ImagingSpectrometers::ImagingSpectrometers(QObject *parent)
	:QObject(parent),
	//m_data_height(960),
	//m_data_width(1280),
	m_data_height(480),
	m_data_width(640),
	m_photo_height(960),
	m_photo_width(1280),
	m_image_data_size(m_data_height * m_data_width),
	m_photo_data_size(m_photo_height * m_photo_width),
	m_usb_device(nullptr),
	m_image_width(m_data_width),
	m_image_height(m_data_height),
	m_LCTF_gap(10),
	m_system_path("E:/ImagingSpectrometers/"),
	m_correction_path("E:/ImagingSpectrometers/correctionFiles/"),
	m_photo_path("E:/ImagingSpectrometers/photoFiles/"),
	m_custom_path("0001"),
	m_photo_times(2)
{
}


ImagingSpectrometers::~ImagingSpectrometers()
{
	if (isDeviceConnected())
	{
		closeDevice();
	}
}

//子线程包括的部分
void ImagingSpectrometers::doWork()
{
	while (1){
		if (m_is_working)
		{
			m_lock.lock();
			getData();
			m_lock.unlock();
		}
		if (m_is_closed)
			break;
	}
}

//获取主窗口指针
void ImagingSpectrometers::getMainWindow(QObject* obj)
{
	m_mainWindow = obj;
}

bool ImagingSpectrometers::isDeviceConnected()
{
	return m_usb_device != nullptr;
}

bool ImagingSpectrometers::openDevice()
{
	m_usb_device = new CCyUSBDevice((HANDLE)(((QWidget*)m_mainWindow)->winId()));
	if (!m_usb_device->Open(0))
	{
		delete m_usb_device;
		m_usb_device = nullptr;
		return false;
	}

	m_ept_control = m_usb_device->ControlEndPt;
	m_ept_control->Target = TGT_DEVICE;
	m_ept_control->ReqType = REQ_VENDOR;
	m_ept_control->Direction = DIR_TO_DEVICE;
	m_ept_control->Value = 0;
	m_ept_control->Index = 0;

	UCHAR buf[2];
	ZeroMemory(buf, 2);
	LONG len = 0;
	m_ept_control->ReqCode = 0xb2;
	m_ept_control->XferData(buf, len);

//	m_ept_control->ReqCode = 0xb8;
//	m_ept_control->XferData(buf, len);

	m_ept_control->ReqCode = 0xb4;
	m_ept_control->XferData(buf, len);

	//配置端点2
	UCHAR ept_addr = 0x82;
	m_ept_bulk_in = m_usb_device->EndPointOf(ept_addr);

	ULONG image_data_size_12bit = m_image_data_size * 2;
	ULONG image_data_size_large = 1280 * 960 * 2;
//	ULONG image_data_size_12bit = m_image_data_size;

	m_ept_bulk_in->SetXferSize(image_data_size_12bit);//非常重要，影响图像质量和帧率

//	m_ept_control->ReqCode = 0xA2;
//	m_ept_control->XferData(buf, len);

	m_image_data = new UCHAR[image_data_size_large];
	ZeroMemory(m_image_data, image_data_size_large);

	m_photo_data = new UCHAR[image_data_size_large];
	ZeroMemory(m_photo_data, image_data_size_large);

	setResolution(640, 480);

	return true;
}

void ImagingSpectrometers::closeDevice()
{
	delete[] m_image_data;
	delete[] m_photo_data;
	delete m_usb_device;
	m_image_data = nullptr;
	m_photo_data = nullptr;
	m_usb_device = nullptr;
	m_ept_bulk_in = nullptr;
	m_ept_control = nullptr;
}

//工作标示
void ImagingSpectrometers::setWorkFlag(bool flag)
{
	m_is_working = flag;
}

void ImagingSpectrometers::setCloseFlag(bool flag)
{
	m_is_closed = flag;
}

void ImagingSpectrometers::pauseDevice()
{
	setWorkFlag(false);
	//m_lock.lock();
}

void ImagingSpectrometers::continueDevice()
{
	setWorkFlag(true);
	//m_lock.unlock();
}

//接收USB设备传输的数据
void ImagingSpectrometers::getData()
{

	LONG image_data_size_12bit = m_image_data_size * 2;
//	LONG image_data_size_12bit = m_image_data_size;
	UCHAR buf[2] = { 0, 0 };
	LONG len = 0;

	m_ept_control->ReqCode = 0xb5;
	m_ept_control->XferData(buf, len);

	OVERLAPPED in_overlap;
	in_overlap.hEvent = CreateEvent(NULL, false, false, L"CYUSB_IN");
	bool flag_data_received;

	UCHAR *in_context = m_ept_bulk_in->BeginDataXfer(m_image_data, image_data_size_12bit, &in_overlap);
	if (!m_ept_bulk_in->WaitForXfer(&in_overlap, 200))
	{
		m_ept_bulk_in->Abort();
		flag_data_received = false;
	}
	else if (!m_ept_bulk_in->FinishDataXfer(m_image_data, image_data_size_12bit, &in_overlap, in_context))
	{
		m_ept_bulk_in->Abort();
		flag_data_received = false;
	}
	else
	{
		flag_data_received = true;
	}

	CloseHandle(in_overlap.hEvent);

	if (flag_data_received)
	{
		getImage();
		QMetaObject::invokeMethod(m_mainWindow, "updateImage", Qt::QueuedConnection);
	}
}

//将数据变为显示用图片数据
void ImagingSpectrometers::getImage()
{
	//Mat image_8UC1_raw;
	//image_8UC1_raw = Mat(m_image_height, m_image_width, CV_8UC1, m_image_data);

	//Mat image_tmp;
	//cvtColor(image_8UC1_raw, image_tmp, CV_GRAY2RGB);//单通道变为3通道

	Mat image_8UC2_raw;
	image_8UC2_raw = Mat(m_image_height, m_image_width, CV_8UC2, m_image_data);

	std::vector<Mat> mat_vector;
	split(image_8UC2_raw, mat_vector);
	
	Mat image_tmp;
	cvtColor(mat_vector[0], image_tmp, CV_GRAY2RGB);//单通道变为3通道

	Mat image_show;
	image_show = image_tmp.clone();
	QImage image_final = QImage(image_show.data, image_show.cols, image_show.rows, image_show.step, QImage::Format_RGB888);
	m_pixmap = QPixmap::fromImage(image_final);

}

//LCTF忙碌状态检测
bool ImagingSpectrometers::isLCTFReady()
{
	LONG len = 2;
	byte commandbuf0[2] = { '!',' ' };
	m_ept_control->ReqCode = 0xb7;//自定义请求码
	m_ept_control->XferData(commandbuf0, len);

	Sleep(10);

	len = 1;
	unsigned char str[1] = { 0 };
	m_ept_control->ReqCode = 0xb8;//自定义请求码
	m_ept_control->Read(str, len);
	if (str[0] == '>')
	{
		return true;
	}
	else
	{
		return false;
	}

}

//用来与LCTF通信获得LCTF状态信息，初始化用
bool ImagingSpectrometers::getLCTFState()
{
	//LONG len_s = 36;
	//int times = 0;
	//unsigned char str[40] = { 0 };

	//LONG len = 5;
	//byte commandbuf0[5] = { 'v', ' ', '?', ' ', '\r' };

	//m_ept_control->ReqCode = 0xb7;//自定义请求码
	////m_ept_control->XferData(commandbuf0, len);
	//m_ept_control->XferData(commandbuf0, len);

	//do
	//{	
	//	Sleep(100);
	//	times++;
	//	m_ept_control->ReqCode = 0xb8;//自定义请求码
	//	m_ept_control->Read(str, len_s);
	//	m_ept_control->Direction = DIR_TO_DEVICE;
	//	if (times > 10)
	//	{
	//		return false;
	//	}
	//} while (str[0] == '\0' || str[0] != 'V');


	//len_s = strlen((char*)str);
	//int i = 0, j = 0;
	//int rangemin = 0, rangemax = 0;
	//while ((str[i] != '.') && (i<len_s))
	//{
	//	i++;
	//}
	//for (j = i - 1; j >= 0; j--)
	//{
	//	if (str[j] >= '0' && str[j] <= '9')
	//		rangemin += (str[j] - '0')*pow(10, i - 1 - j);
	//	else
	//		break;
	//}
	//i++;
	//while ((str[i] != '.') && (i<len_s))
	//{
	//	i++;
	//}
	//for (j = i - 1; j >= 0; j--)
	//{
	//	if (str[j] >= '0' && str[j] <= '9')
	//		rangemax += (str[j] - '0')*pow(10, i - 1 - j);
	//	else
	//		break;
	//}

	//LONG len_esc = 1;
	//byte commandbuf_esc[2] = { '\x1b','\0' };
	//m_ept_control->ReqCode = 0xb7;//自定义请求码
	//m_ept_control->XferData(commandbuf_esc, len_esc);

	//LONG len_w = 8;
	//byte commandbuf_w[8] = { 'W', ' ', '6', '5', '0', '0', '\r', '\0' };
	//m_ept_control->ReqCode = 0xb7;//自定义请求码
	//m_ept_control->XferData(commandbuf_w, len_w);

	//LONG len_re = 8;
	//byte commandbuf_re[8] = { 0 };
	//m_ept_control->ReqCode = 0xb7;
	//m_ept_control->XferData(commandbuf_re, len_re);

	//Sleep(10000);
	int rangemax = 1100;
	int rangemin = 650;
	if (rangemax < rangemin)
		return false;
	m_LCTF_max = rangemax;
	m_LCTF_min = rangemin;
	m_LCTF_wavelength = rangemin;
	m_LCTF_range = m_LCTF_max - m_LCTF_min;
	m_LCTF_bandcount = m_LCTF_range / m_LCTF_gap + 1;//注意此处的gap需要读取面板信息，默认为10
	return true;
}

void ImagingSpectrometers::setWavelengthGap(int gap)
{
	m_LCTF_gap = gap;
	m_LCTF_bandcount = m_LCTF_range / m_LCTF_gap + 1;
}

int ImagingSpectrometers::getWavelengthGap()
{
	return m_LCTF_gap;
}

int ImagingSpectrometers::getWavelengthMin()
{
	return m_LCTF_min;
}

int ImagingSpectrometers::getWavelengthMax()
{
	return m_LCTF_max;
}

int ImagingSpectrometers::getWavelengthNow()
{
	return m_LCTF_wavelength;
}

int ImagingSpectrometers::getPhotoWidth()
{
	return m_photo_width;
}

int ImagingSpectrometers::getPhotoHeight()
{
	return m_photo_height;
}

//对相机的控制
bool ImagingSpectrometers::controlDevice(int TYPE, int value)
{
	LONG len = 4;
	int x, y;
	switch (TYPE)
	{
	case DIGITALGAIN:
	{
		x = value * 223 / 70;
		byte commandbuf[4] = { 0x30, 0x5E, 0x00, x };
		m_ept_control->ReqCode = 0xb3;//自定义请求码
		m_ept_control->XferData(commandbuf, len);
		break;
	}
	case EXPOSURETIME:
	{
		x = (value * 80 / 11) >> 8;
		y = (value * 80 / 11) & 0x00ff;
		byte commandbuf[4] = { 0x30, 0x12, x, y };
		m_ept_control->ReqCode = 0xb3;//自定义请求码
		m_ept_control->XferData(commandbuf, len);
		break;
	}
	case ANALOGGAIN:
	{
		switch (value)
		{
		case 2:
		{
			byte commandbuf0[4] = { 0x3E, 0xE4, 0xD2, 0x08 };
			m_ept_control->ReqCode = 0xb3;//自定义请求码
			m_ept_control->XferData(commandbuf0, len);
			byte commandbuf[4] = { 0x30, 0xB0, 0x00, 0x10 };
			m_ept_control->ReqCode = 0xb3;//自定义请求码
			m_ept_control->XferData(commandbuf, len);
			break;
		}
		case 4:
		{
			byte commandbuf0[4] = { 0x3E, 0xE4, 0xD2, 0x08 };
			m_ept_control->ReqCode = 0xb3;//自定义请求码
			m_ept_control->XferData(commandbuf0, len);
			byte commandbuf[4] = { 0x30, 0xB0, 0x00, 0x20 };
			m_ept_control->ReqCode = 0xb3;//自定义请求码
			m_ept_control->XferData(commandbuf, len);
			break;
		}
		case 8:
		{
			byte commandbuf0[4] = { 0x3E, 0xE4, 0xD2, 0x08 };
			m_ept_control->ReqCode = 0xb3;//自定义请求码
			m_ept_control->XferData(commandbuf0, len);
			byte commandbuf[4] = { 0x30, 0xB0, 0x00, 0x30 };
			m_ept_control->ReqCode = 0xb3;//自定义请求码
			m_ept_control->XferData(commandbuf, len);
			break;
		}
		case 10:
		{
			byte commandbuf0[4] = { 0x3E, 0xE4, 0xD3, 0x08 };
			m_ept_control->ReqCode = 0xb3;//自定义请求码
			m_ept_control->XferData(commandbuf0, len);
			byte commandbuf[4] = { 0x30, 0xB0, 0x00, 0x30 };
			m_ept_control->ReqCode = 0xb3;//自定义请求码
			m_ept_control->XferData(commandbuf, len);
			break;
		}
		case 1:
		default:
		{
			byte commandbuf0[4] = { 0x3E, 0xE4, 0xD2, 0x08 };
			m_ept_control->ReqCode = 0xb3;//自定义请求码
			m_ept_control->XferData(commandbuf0, len);
			byte commandbuf[4] = { 0x30, 0xB0, 0x00, 0x00 };
			m_ept_control->ReqCode = 0xb3;//自定义请求码
			m_ept_control->XferData(commandbuf, len);
			break;
		}
		}
		break;
	}
	default:
		return false;
	}
	return true;
}

//对LCTF的控制
bool ImagingSpectrometers::controlLCTF(int TYPE, int value)
{
	switch (TYPE)
	{
	case WAVELENGTH:
	{
		m_LCTF_wavelength = value;
		char cmd1 = 'w';
		char cmd2 = ' ';
		char cmd3 = value / 1000 % 10 + '0';
		char cmd4 = value / 100 % 10 + '0';
		char cmd5 = value / 10 % 10 + '0';
		char cmd6 = '0';
		char cmd7 = ' ';
		char cmd8 = '\r';

		if (cmd3 == '0') {
			LONG len = 8;
			byte commandbuf[8] = { cmd1, cmd2, cmd4, cmd5, cmd6, cmd7, cmd8, '\0' };
			m_ept_control->ReqCode = 0xb7;//自定义请求码
			m_ept_control->XferData(commandbuf, len);
		}
		else if (cmd3>'0'){
			LONG len = 8;
			byte commandbuf[8] = { cmd1, cmd2, cmd3, cmd4, cmd5, cmd6, cmd7, cmd8 };//需不需要\0待定
			m_ept_control->ReqCode = 0xb7;//自定义请求码
			m_ept_control->XferData(commandbuf, len);
		}

		//unsigned char str[40] = { 0 };

		//for (int i = 0; i < 10; i++)
		//{
		//	LONG len2 = 5;
		//	byte commandbuf2[5] = { 'r', ' ', '?', ' ', '\r' };
		//	m_ept_control->ReqCode = 0xb7;
		//	m_ept_control->Write(commandbuf2, len2);

		//	Sleep(100);

		//	LONG len_r = 36;
		//	m_ept_control->ReqCode = 0xb8;
		//	m_ept_control->Read(str, len_r);

		//}

		//m_ept_control->Direction = DIR_TO_DEVICE;

		break;
	}
	default:
		return false;
	}
	return true;
}

void ImagingSpectrometers::setResolution(int width, int height)
{
	LONG len = 4;
	int x1, y1, x2, y2, x3, y3, x4, y4, x5, y5, x6, y6;
	int m, n, p1, p2;
	if (width == 1280 && height == 960)
	{
		x1 = 0x00; y1 = 0x09; x2 = 0x00; y2 = 0x08;//x1 y1 x2 y2设置结束点横坐标和纵坐标值
		x3 = 0x03; y3 = 0xC8; x4 = 0x05; y4 = 0x07;
		x5 = 0x04; y5 = 0x68; x6 = 0x07; y6 = 0x72;
		m = 0x20; n = 0x02; p1 = 0x03; p2 = 0x05;
	}
	else if (width == 1024 && height == 768)
	{
		x1 = 0x00; y1 = 0x69; x2 = 0x00; y2 = 0x88;
		x3 = 0x03; y3 = 0x68; x4 = 0x04; y4 = 0x87;
		x5 = 0x03; y5 = 0xDE; x6 = 0x06; y6 = 0x72;
		m = 0x20; n = 0x02; p1 = 0x03; p2 = 0x04;
	}
	else if (width == 640 && height == 480)
	{
		x1 = 0x00; y1 = 0xF9; x2 = 0x01; y2 = 0x48;
		x3 = 0x03; y3 = 0x78; x4 = 0x03; y4 = 0xC7;
		x5 = 700 / 255; y5 = 700 % 255; x6 = 1400 / 255; y6 = 1400 % 255;
		m = 0x22; n = 0x02; p1 = 0x03; p2 = 0x04;
	}
	else//防止之后有需要增加的分辨率
	{
		x1 = 0x00; y1 = 0xF9; x2 = 0x01; y2 = 0x48;
		x3 = 0x03; y3 = 0x78; x4 = 0x03; y4 = 0xC7;
		x5 = 700 / 255; y5 = 700 % 255; x6 = 1400 / 255; y6 = 1400 % 255;
		m = 0x22; n = 0x02; p1 = 0x03; p2 = 0x04;
	}
	byte commandbuf0[4] = { 0x30, 0x02, x1, y1 };
	m_ept_control->ReqCode = 0xb3;
	m_ept_control->XferData(commandbuf0, len);
	byte commandbuf1[4] = { 0x30, 0x04, x2, y2 };//起始点
	m_ept_control->ReqCode = 0xb3;
	m_ept_control->XferData(commandbuf1, len);
	byte commandbuf2[4] = { 0x30, 0x06, x3, y3 };
	m_ept_control->ReqCode = 0xb3;
	m_ept_control->XferData(commandbuf2, len);
	byte commandbuf3[4] = { 0x30, 0x08, x4, y4 };//结束点
	m_ept_control->ReqCode = 0xb3;
	m_ept_control->XferData(commandbuf3, len);
	byte commandbuf4[4] = { 0x30, 0x0A, x5, y5 };
	m_ept_control->ReqCode = 0xb3;//frame_length_lines
	m_ept_control->XferData(commandbuf4, len);
	byte commandbuf5[4] = { 0x30, 0x0C, x6, y6 };
	m_ept_control->ReqCode = 0xb3;//line_length_pck
	m_ept_control->XferData(commandbuf5, len);
	byte commandbuf6[4] = { 0x30, 0x30, 0x00, m };
	m_ept_control->ReqCode = 0xb3;//M
	m_ept_control->XferData(commandbuf6, len);
	byte commandbuf7[4] = { 0x30, 0x2E, 0x00, n };
	m_ept_control->ReqCode = 0xb3;//N
	m_ept_control->XferData(commandbuf7, len);
	byte commandbuf8[4] = { 0x30, 0x2C, 0x00, p1 };
	m_ept_control->ReqCode = 0xb3;//P1
	m_ept_control->XferData(commandbuf8, len);
	byte commandbuf9[4] = { 0x30, 0x2A, 0x00, p2 };
	m_ept_control->ReqCode = 0xb3;//P2
	m_ept_control->XferData(commandbuf9, len);

}

Mat ImagingSpectrometers::takephoto()
{
	LONG photo_data_size_12bit = m_photo_data_size * 2;
	UCHAR buf[2] = { 0, 0 };
	LONG len = 0;

	m_ept_control->ReqCode = 0xb5;
	m_ept_control->XferData(buf, len);

	OVERLAPPED in_overlap;
	in_overlap.hEvent = CreateEvent(NULL, false, false, L"CYUSB_IN");
	bool flag_data_received;

	UCHAR *in_context = m_ept_bulk_in->BeginDataXfer(m_photo_data, photo_data_size_12bit, &in_overlap);
	if (!m_ept_bulk_in->WaitForXfer(&in_overlap, 10000))
	{
		m_ept_bulk_in->Abort();
		flag_data_received = false;
	}
	else if (!m_ept_bulk_in->FinishDataXfer(m_photo_data, photo_data_size_12bit, &in_overlap, in_context))
	{
		m_ept_bulk_in->Abort();
		flag_data_received = false;
	}
	else
	{
		flag_data_received = true;
	}
	CloseHandle(in_overlap.hEvent);
	if (flag_data_received)
	{
		Mat image_8UC2_raw;
		image_8UC2_raw = Mat(m_photo_height, m_photo_width, CV_8UC2, m_photo_data);

		std::vector<Mat> mat_vector;
		split(image_8UC2_raw, mat_vector);

		Mat temp = Mat(mat_vector[0]);
		return temp;
	}
	else
	{
		Mat temp = Mat(m_photo_height, m_photo_width, CV_8UC1, Scalar::all(0));
		return temp;
	}
}

bool ImagingSpectrometers::writeConfigFile(String path, byte* config, int len)
{
	std::string str = path + "/" +"config.dat";
	//FILE *fp = fopen(str.c_str(), "w");
	//if (fp == nullptr)
	//	return false;
	//for (int i = 0; i<len ; i++)
	//{
	//	fprintf(fp, "%d", config + i);
	//}
	//fclose(fp);

	std::ofstream ofs(str, std::ios::out | std::ios::binary);
	const char* temp = (char*)config;
	ofs.write(temp, len);
	ofs.close();

	return true;
}

bool ImagingSpectrometers::readConfigFile(String path, char* config)
{
	std::string str = path + "/" + "config.dat";
	//FILE *fp = fopen(str.c_str(), "r");
	//if (fp == nullptr)
	//	return false;
	//int i_temp[150] = { 0 };
	//for (int i = 0; fscanf(fp, "%d", i_temp + i) != EOF; i++);
	//config = (byte*)i_temp;
	//fclose(fp);

	std::ifstream ifs(str, std::ios::in | std::ios::binary);
	char temp[150] = { 0 };
	ifs.read(config, 150);
	ifs.close();

	return true;
}

String ImagingSpectrometers::getCorrectionPath()
{
	return m_correction_path;
}

String ImagingSpectrometers::getPhotoPath()
{
	return m_photo_path;
}

String ImagingSpectrometers::getSystemPath()
{
	return m_system_path;
}

String ImagingSpectrometers::getCustomPath()
{
	return m_custom_path;
}

void ImagingSpectrometers::setCustomPath(String path)
{
	m_custom_path = path;
}

void ImagingSpectrometers::setPhotoTimes(int times)
{
	m_photo_times = times;
}

int ImagingSpectrometers::getPhotoTimes()
{
	return m_photo_times;
}
