#include "mywindow.h"

MyWindow::MyWindow(QWidget *parent)
	: QMainWindow(parent), m_frame_count(0)
{
	ui.setupUi(this);

	//this->setWindowState(Qt::WindowMaximized);

	m_mydevice = new ImagingSpectrometers;
	m_config_dialog = new ConfigDialog(this);
	m_photocheck_window = new PhotoCheckWindow(this);
	m_specAnalysis_window = new SpectralAnalysis(m_photocheck_window);
	m_thread_curves = new MySpecThread(m_mydevice);

	m_mydevice->getMainWindow(this);//获取主窗口指针

	m_mydevice->moveToThread(&m_thread);

	//连接信号槽
	connect(ui.actionOpen, SIGNAL(triggered()), this, SLOT(openCamera()));
	connect(ui.actionClose, SIGNAL(triggered()), this, SLOT(closeCamera()));
	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));

	connect(ui.action_whiteboard, SIGNAL(triggered()), this, SLOT(whiteboard()));
	connect(ui.action_photo, SIGNAL(triggered()), this, SLOT(photo()));
	connect(ui.action_photoonce, SIGNAL(triggered()), this, SLOT(photoOnce()));

	connect(ui.pushButton_open, SIGNAL(clicked()), this, SLOT(openCamera()));
	connect(ui.pushButton_close, SIGNAL(clicked()), this, SLOT(closeCamera()));
	connect(ui.pushButton_pause, SIGNAL(clicked()), this, SLOT(pauseCamera()));

	connect(ui.pushButton_openLCTF, SIGNAL(clicked()), this, SLOT(openLCTF()));
	connect(ui.pushButton_regulate, SIGNAL(clicked()), this, SLOT(whiteboard()));
	connect(ui.pushButton_photo, SIGNAL(clicked()), this, SLOT(photo()));
	connect(ui.pushButton_PhotoOnce, SIGNAL(clicked()), this, SLOT(photoOnce()));
	connect(ui.pushButton_config, SIGNAL(clicked()), this, SLOT(showConfigDialog()));
	connect(ui.pushButton_checkphoto, SIGNAL(clicked()), this, SLOT(showPhotoCheckWindow()));

	connect(ui.lineEdit_WLGap, SIGNAL(editingFinished()), this, SLOT(setWavelengthGap()));
	connect(ui.lineEdit_ExposureTime, SIGNAL(editingFinished()), this, SLOT(setExposureTime_lineEdit()));
	connect(ui.lineEdit_Wavelength, SIGNAL(editingFinished()), this, SLOT(setWavelength_lineEdit()));
	connect(ui.lineEdit_DigitalGain, SIGNAL(editingFinished()), this, SLOT(setDigitalGain_lineEdit()));

	connect(ui.horizontalSlider_DigitalGain, SIGNAL(valueChanged(int)), this, SLOT(setDigitalGain()));
	connect(ui.horizontalSlider_ExposureTime, SIGNAL(valueChanged(int)), this, SLOT(setExposureTime()));
	connect(ui.horizontalSlider_Wavelength, SIGNAL(valueChanged(int)), this, SLOT(setWavelength()));

	connect(ui.comboBox_AnalogGain, SIGNAL(currentIndexChanged(int)), this, SLOT(setAnalogGain()));

	connect(&m_thread, SIGNAL(started()), m_mydevice, SLOT(doWork()));//子线程启动后执行doWork

	m_timer = new QTimer(this);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(showFPS()));

	//自定义的设置窗口相关信号
	connect(m_config_dialog->ui.pushButton_accept, SIGNAL(clicked()), this, SLOT(setConfig()));
	connect(m_config_dialog->ui.pushButton_path, SIGNAL(clicked()), this, SLOT(setConfigPath()));
	connect(m_photocheck_window->ui.pushButton_back, SIGNAL(clicked()), this, SLOT(closePhotoCheckWindow()));
	connect(m_photocheck_window->ui.label, SIGNAL(mousePress(int, int)), this, SLOT(setCurvesMat(int, int)));
	connect(m_thread_curves, SIGNAL(processUpdate(int)), this, SLOT(setPrepareProcess(int)));

	initializeUI();//此处设置控件最初状态
	initializePath();//创建相关路径
}

MyWindow::~MyWindow()
{
	if (m_thread.isRunning())
	{
		if (ui.pushButton_pause->text() == QStringLiteral("取消暂停"))
			m_mydevice->continueDevice();
		m_thread.quit();
		m_mydevice->setWorkFlag(false);
		m_mydevice->setCloseFlag(true);
		m_thread.wait();
	}
	if (m_thread_curves->isRunning())
	{
		m_thread_curves->quit();
		m_thread_curves->wait();
	}
	delete m_mydevice;
	delete m_config_dialog;
}

void MyWindow::initializeUI()
{
	ui.label_FPS->setText(QStringLiteral("就绪"));
	ui.label_LCTFState->setText(QStringLiteral("未开启"));
	ui.pushButton_close->setEnabled(false);
	ui.pushButton_pause->setEnabled(false);
	ui.pushButton_openLCTF->setEnabled(false);

	setCameraUiEnable(false);
	setLCTFUiEnable(false);
}

void MyWindow::initializePath()
{
	FILE *fp,*fc;
	/*TCHAR szPath[MAX_PATH];
	GetModuleFileName(NULL, szPath, MAX_PATH);
	int iLen = 2 * wcslen(szPath);    
	char* fromTCHAR = new char[iLen + 1];
	wcstombs(fromTCHAR, szPath, iLen + 1);
	String current_path(fromTCHAR);
	String current_path_sub = current_path.substr(0, current_path.find_last_of('\\'));
	sys_path = current_path_sub;*/
	String correctionPath = m_mydevice->getCorrectionPath();
	String photoPath = m_mydevice->getPhotoPath();
	String systemPath = m_mydevice->getSystemPath();
	String customPath = m_mydevice->getCustomPath();
	String customPathFull = photoPath + customPath + '/';
	const char* temp_c = correctionPath.c_str();
	const char* temp_p = photoPath.c_str();
	const char* temp = systemPath.c_str();
	const char* temp_cf = customPathFull.c_str();

	fp = fopen(temp, "r");
	if (fp == NULL)
	{
		mkdir(temp);
		mkdir(temp_c);
		mkdir(temp_p);
	}
	fc = fopen(temp_cf, "r");
	if (fc == NULL)
	{
		mkdir(temp_cf);
	}
	fcloseall();
}

void MyWindow::setLCTFUiEnable(bool flag)
{
	ui.horizontalSlider_Wavelength->setEnabled(flag);
	ui.lineEdit_Wavelength->setEnabled(flag);
	ui.lineEdit_WLGap->setEnabled(flag);
	ui.pushButton_regulate->setEnabled(flag);
	ui.pushButton_photo->setEnabled(flag);
	ui.pushButton_PhotoOnce->setEnabled(flag);
}

void MyWindow::setCameraUiEnable(bool flag)
{
	ui.horizontalSlider_DigitalGain->setEnabled(flag);
	ui.horizontalSlider_ExposureTime->setEnabled(flag);
	ui.lineEdit_ExposureTime->setEnabled(flag);
	ui.lineEdit_DigitalGain->setEnabled(flag);
	ui.comboBox_AnalogGain->setEnabled(flag);
}

void MyWindow::openCamera()
{
	if (m_mydevice->isDeviceConnected())
	{
		ui.pushButton_close->setEnabled(true);
		QMessageBox::about(this, QStringLiteral("提示"), QStringLiteral("设备已连接,请先关闭相机"));
	}
	else if (m_mydevice->openDevice())
	{
		m_mydevice->setWorkFlag(true);
		m_mydevice->setCloseFlag(false);
		m_thread.start();

		QValidator *validator_gap = new QIntValidator(1, 10, this);//输入数据的合法性，此处未根据实际情况限制，是否合法需要模型类进行进一步筛选
		ui.lineEdit_WLGap->setValidator(validator_gap);

		QValidator *validator_ext = new QIntValidator(1, 92, this);
		ui.lineEdit_ExposureTime->setValidator(validator_ext);

		QValidator *validator_dg = new QIntValidator(10, 80, this);
		ui.lineEdit_DigitalGain->setValidator(validator_dg);

		ui.horizontalSlider_DigitalGain->setMinimum(10);
		ui.horizontalSlider_DigitalGain->setMaximum(80);
		ui.horizontalSlider_DigitalGain->setValue(30);

		ui.horizontalSlider_ExposureTime->setMinimum(1);
		ui.horizontalSlider_ExposureTime->setMaximum(92);
		ui.horizontalSlider_ExposureTime->setValue(20);

		if (ui.comboBox_AnalogGain->count() == 0)
		{
			ui.comboBox_AnalogGain->addItem(QStringLiteral("一倍增益"));
			ui.comboBox_AnalogGain->addItem(QStringLiteral("二倍增益"));
			ui.comboBox_AnalogGain->addItem(QStringLiteral("四倍增益"));
			ui.comboBox_AnalogGain->addItem(QStringLiteral("八倍增益"));
			ui.comboBox_AnalogGain->addItem(QStringLiteral("十倍增益"));
		}

		ui.comboBox_AnalogGain->setCurrentIndex(0);
		ui.lineEdit_WLGap->setText(QString::number(10));

		setCameraUiEnable(true);

		ui.pushButton_openLCTF->setEnabled(true);
		ui.pushButton_close->setEnabled(true);
		ui.pushButton_pause->setEnabled(true);

		m_timer->start(1000);
	}
	else
	{
		QMessageBox::warning(this, QStringLiteral("警告"), QStringLiteral("设备打开失败"));
	}
}

//关闭相机时应屏蔽一些控件
void MyWindow::closeCamera()
{
	if (m_mydevice->isDeviceConnected())
	{
		m_timer->stop();
		ui.label_FPS->setText(QStringLiteral("就绪"));

		//暂停按钮是否按下，进行相应操作，释放锁，防止死锁
		if (ui.pushButton_pause->text() == QStringLiteral("取消暂停"))
		{
			m_mydevice->continueDevice();
			ui.pushButton_pause->setText(QStringLiteral("暂停相机"));
		}

		m_thread.quit();
		m_mydevice->setWorkFlag(false);
		m_mydevice->setCloseFlag(true);
		m_thread.wait();
		ui.label_image->clear();
		m_mydevice->closeDevice();
		setCameraUiEnable(false);
		setLCTFUiEnable(false);
		ui.pushButton_openLCTF->setEnabled(false);
		ui.pushButton_close->setEnabled(false);
		ui.pushButton_pause->setEnabled(false);

	}
}

void MyWindow::pauseCamera()
{
	if (m_thread.isRunning() && (ui.pushButton_pause->text() == QStringLiteral("暂停相机")))
	{
		m_mydevice->pauseDevice();
		ui.pushButton_pause->setText(QStringLiteral("取消暂停"));
	}
	else if (m_thread.isRunning() && ui.pushButton_pause->text() == QStringLiteral("取消暂停"))
	{
		m_mydevice->continueDevice();
		ui.pushButton_pause->setText(QStringLiteral("暂停相机"));
	}
}

//更新图像
void MyWindow::updateImage()
{
	if (m_thread.isRunning())
	{
		//m_mydevice->getImage();
		ui.label_image->resize(m_mydevice->m_pixmap.size());
		ui.label_image->setPixmap(m_mydevice->m_pixmap);
		m_frame_count++;
	}
}

void MyWindow::showFPS()
{
	QString str = QString::number(m_frame_count) + "  fps";
	ui.label_FPS->setText(str);
	m_frame_count = 0;
}

//bool MyWindow::prepareLCTF()
//{
//	int num = 10;
//	bool flag = false;
//	QProgressDialog progress_getLCTFState(QStringLiteral("正在读取LCTF版本信息..."), QStringLiteral("取消"), 0, num, this);
//	for (int i = 0; i < num; i++)
//	{
//		progress_getLCTFState.setValue(i);
//		if (progress_getLCTFState.wasCanceled())
//		{
//			flag = false;
//			break;
//		}
//		if (m_mydevice->getLCTFState())
//		{
//			flag = true;
//			break;
//		}
//	}
//	progress_getLCTFState.setValue(num);
//	return flag;
//}

void MyWindow::openLCTF()
{
	m_thread.quit();
	m_mydevice->setWorkFlag(false);
	m_mydevice->setCloseFlag(true);
	m_thread.wait();
//	m_mydevice->pauseDevice();
	if (m_mydevice->m_usb_device == nullptr)
	{
		QMessageBox::about(this, QStringLiteral("提示"), QStringLiteral("设备未连接！"));
		return;
	}
	//if (!m_mydevice->isLCTFReady())//部分情况没有该功能
	//{
	//	setLCTFUiEnable(false);
	//	ui.label_LCTFState->setText(QStringLiteral("初始化中"));
	//	QMessageBox::about(this, QStringLiteral("提示"), QStringLiteral("LCTF正在进行初始化，请耐心等待！"));
	//}
	//else 
	if (m_mydevice->getLCTFState())
	{
		int min, max;
		min = m_mydevice->getWavelengthMin();
		max = m_mydevice->getWavelengthMax();

		QValidator *validator_wl = new QIntValidator(min, max, this);//直接设置合法范围为波长区间
		ui.lineEdit_Wavelength->setValidator(validator_wl);

		ui.horizontalSlider_Wavelength->setSingleStep(m_mydevice->getWavelengthGap());
		ui.horizontalSlider_Wavelength->setPageStep(m_mydevice->getWavelengthGap());
		ui.horizontalSlider_Wavelength->setMinimum(min);
		ui.horizontalSlider_Wavelength->setMaximum(max);
//		ui.horizontalSlider_Wavelength->setValue(min);

		setLCTFUiEnable(true);
		ui.pushButton_openLCTF->setEnabled(false);
		ui.label_LCTFState->setText(QStringLiteral("就绪"));
//		m_mydevice->continueDevice();
		m_mydevice->closeDevice();
		m_mydevice->openDevice();
//		Sleep(100);
		m_mydevice->setWorkFlag(true);
		m_mydevice->setCloseFlag(false);
		m_thread.start();
		//此处重置相机子线程相关信息
		return;
	}
	else
	{
		setLCTFUiEnable(false);
		ui.label_LCTFState->setText(QStringLiteral("未连接"));		
//		m_mydevice->continueDevice();
		m_mydevice->closeDevice();
		m_mydevice->openDevice();
		m_mydevice->setWorkFlag(true);
		m_mydevice->setCloseFlag(false);
		m_thread.start();
		QMessageBox::warning(this, QStringLiteral("警告"), QStringLiteral("LCTF信息读取失败"));
		return;
	}
}

void MyWindow::whiteboard()
{
	if (m_mydevice->m_usb_device == nullptr)
	{
		QMessageBox::about(this, QStringLiteral("提示"), QStringLiteral("设备未连接！"));
		return;
	}
	QMessageBox::StandardButton reply;
	bool isCorrectionFinished = false;
	reply = QMessageBox::question(this, QStringLiteral("校正"),
		QStringLiteral("是否开始白板校正？"),
		QMessageBox::Yes | QMessageBox::Cancel);
	m_mydevice->pauseDevice();
	if (reply == QMessageBox::Yes)
	{
		//校正部分
		//拍大分辨率照片可以降速，防止图像出现分块
		//control(0x30, 0x2A, 0x00, 0x0C);
		int data_height = m_mydevice->getDataHeight();
		int data_width = m_mydevice->getDataWidth();
		int photo_height = m_mydevice->getPhotoHeight();
		int photo_width = m_mydevice->getPhotoWidth();
		int WavelengthMax = m_mydevice->getWavelengthMax();
		int WavelengthMin = m_mydevice->getWavelengthMin();
		int LCTF_Gap = m_mydevice->getWavelengthGap();
		int LCTF_range = WavelengthMax - WavelengthMin;
		int bandcount = LCTF_range / LCTF_Gap + 1;
		int photo_data_size = photo_width * photo_height;

		//进度条开启
		QProgressDialog process(this);
		process.setLabelText(QStringLiteral("正在校正..."));
		process.setRange(0, LCTF_range + 1);
		process.setModal(true);
		process.setCancelButtonText(QStringLiteral("取消"));

		//分辨率设置为大分辨率
		m_mydevice->setResolution(photo_width, photo_height);

		//此处为白板图片路径
		String correctionPath = m_mydevice->getCorrectionPath();

		int len = 0;
		int epMax = 91;
		int digitalGainMax = 70;
		int analogGainMax = 4;
		int exposureTime = 0;
		int digitalGain = 0;
		int analogGain = 0;
		int count;

#ifdef __IN8BITS__
		int thresholdLower = 240;//12位需要修改阈值
		int thresholdUpper = 252;
#else
		int thresholdLower = 0xF00;
		int thresholdUpper = 0xFC0;
#endif
		int pixelValueMax = 0;
		int sum;
		int step = 10;
		boolean flagAdjust;
		boolean isParameterAvailable;
		boolean isDirectionChanged;
		byte* cameraParW = new byte[3 + bandcount * 3];
		Mat data_photo_processing;
		int cameraParIndex = 2;

		cameraParW[0] = (byte)(LCTF_range / 10);
		cameraParW[1] = (byte)LCTF_Gap;

		while (len <= LCTF_range) {
			exposureTime = epMax;
			digitalGain = digitalGainMax;
			analogGain = analogGainMax;
			flagAdjust = true;
			isParameterAvailable = true;
			isDirectionChanged = false;

			m_mydevice->controlLCTF(WAVELENGTH, len + WavelengthMin);//更改波长

			while (flagAdjust) {
				m_mydevice->controlDevice(DIGITALGAIN, digitalGain);
				m_mydevice->controlDevice(ANALOGGAIN, analogGain);
				m_mydevice->controlDevice(EXPOSURETIME, exposureTime);
				for (int i = 0; i < 2; ++i)
					m_mydevice->takephoto();
				//去除最初的n张无效图片
				data_photo_processing = m_mydevice->takephoto();

				count = 0;
				while (count < photo_data_size - step){
					sum = 0;
					for (int i = 0; i < step; ++i) {
						sum += (data_photo_processing.data[count + i] );//12位需要修改
					}
					if (sum / step > thresholdUpper) {
						isParameterAvailable = false;
						break;
					}
					if (sum / step > pixelValueMax) {
						pixelValueMax = sum / step;
					}
					isParameterAvailable = true;
					count += step;
				}

				if (!isParameterAvailable)
				{
					isDirectionChanged = false;
					if (digitalGain < 8) {
						if (exposureTime < 6) {
							exposureTime = 0;
							analogGain--;
							digitalGain = digitalGainMax;
							exposureTime = epMax;
							if (analogGain < 0){
								analogGain = 0;
								//flagAdjust = false;
								break;
							}
						}
						else {
							exposureTime = exposureTime / 2;
						}
					}
					else {
						digitalGain = digitalGain / 2;
						exposureTime = epMax;
					}
				}
				else {
					if (pixelValueMax > thresholdLower || isDirectionChanged) {
						//flagAdjust = false;
						break;
					}
					//else if (sb_manualEp_value<1 && sb_global_value<1 && analogGain<=0){
					//flagAdjust = false;
					//break;
					//}
					else{
						isDirectionChanged = true;
						if (exposureTime >= epMax) {
							if (digitalGain >= digitalGainMax) {
								if (analogGain >= analogGainMax) {
									break;
								}
								else{
									analogGain++;
								}
								digitalGain = 0;
							}
							else{
								digitalGain += 5;
								exposureTime = 0;
							}
						}
						else{
							exposureTime += 5;
						}
					}
				}
			}
			//这里保存的图像
			int w = len + WavelengthMin;
			char c[5] = {0};
			itoa(w, c, 10);
			String t(c);
#ifdef __IN8BITS__
			String path = correctionPath + "/" + t + ".bmp";
#else
			String path = correctionPath + "/" + t + ".exr";
#endif
			cv::imwrite(path, data_photo_processing);

			cameraParW[cameraParIndex++] = (byte)exposureTime;
			cameraParW[cameraParIndex++] = (byte)digitalGain;
			cameraParW[cameraParIndex++] = (byte)analogGain;

			len += LCTF_Gap;
			if (len > LCTF_range)
			{
				isCorrectionFinished = true;
			}
			else{
				process.setValue(len);
			}
			if (process.wasCanceled())
			{
				QMessageBox::warning(this, QStringLiteral("正在取消"), QStringLiteral("已保存的图片将不会删除"));
				m_mydevice->setResolution(data_width, data_height);
				m_mydevice->continueDevice();
				return;
			}
		}
		
		////这里应该设置防止此时按下cancel按钮
		process.setCancelButton(0);
		// 写config文件
		cameraParW[3 + bandcount * 3] = '\0';
		if (!m_mydevice->writeConfigFile(correctionPath, cameraParW, 3 + bandcount * 3))
		{
			QMessageBox::warning(this, QStringLiteral("警告"), QStringLiteral("配置文件写入异常"));
		}

		//control(0x30, 0x2A, 0x00, 0x09);

		// 设置分辨率 640*480
//		delete[] cameraParW;不用销毁吗？
//		cameraParW = nullptr;
		m_mydevice->setResolution(data_width, data_height);
		process.setValue(LCTF_range+1);
		QMessageBox::about(this, QStringLiteral("提示"), QStringLiteral("白板校正完成"));
	}
	m_mydevice->continueDevice();
	
	
}

void MyWindow::photo()
{
	if (m_mydevice->m_usb_device == nullptr)
	{
		QMessageBox::about(this, QStringLiteral("提示"), QStringLiteral("设备未连接！"));
		return;
	}
	QMessageBox::StandardButton reply;
	bool isPhotoFinished = false;
	reply = QMessageBox::question(this, QStringLiteral("拍照"),
		QStringLiteral("是否进行连续拍照？"),
		QMessageBox::Yes | QMessageBox::Cancel);
	m_mydevice->pauseDevice();
	if (reply == QMessageBox::Yes)
	{
		//拍照部分
		//拍大分辨率照片可以降速，防止图像出现分块
		//control(0x30, 0x2A, 0x00, 0x0C);
		String correctionPath = m_mydevice->getCorrectionPath();
		String photoPath_temp = m_mydevice->getPhotoPath();
		String customPath = m_mydevice->getCustomPath();
		String photoPath = photoPath_temp + customPath + '/';
		int PHOTO_TIMES = 1;
		char cameraParR[150] = {0};
		m_mydevice->readConfigFile(correctionPath, cameraParR);
		int cameraParIndex = 2;

		int data_height = m_mydevice->getDataHeight();
		int data_width = m_mydevice->getDataWidth();
		int photo_height = m_mydevice->getPhotoHeight();
		int photo_width = m_mydevice->getPhotoWidth();
		int WavelengthMax = m_mydevice->getWavelengthMax();
		int WavelengthMin = m_mydevice->getWavelengthMin();
		int LCTF_Gap = m_mydevice->getWavelengthGap();
		int LCTF_range = WavelengthMax - WavelengthMin;
		int bandcount = LCTF_range / LCTF_Gap + 1;
		int photo_data_size = photo_width * photo_height;
		Mat data_photo_processing;

		//进度条开启
		QProgressDialog process(this);
		process.setLabelText(QStringLiteral("正在拍照..."));
		process.setRange(0, LCTF_range + 1);
		process.setModal(true);
		process.setCancelButtonText(QStringLiteral("取消"));

		//分辨率设置为大分辨率
		m_mydevice->setResolution(photo_width, photo_height);
		if ((int)cameraParR[0] != (LCTF_range / 10) || (int)cameraParR[1] != LCTF_Gap)
		{
			process.setValue(LCTF_range + 1);
			QMessageBox::warning(this, QStringLiteral("警告"), QStringLiteral("所保存的配置不适用于当前情形，请重新校正！"));
			return;
		}
		else{

			int len = 0;
			int digitalGain;
			int analogGain;
			int exposureTime;

			while (len <= LCTF_range) {
				m_mydevice->controlLCTF(WAVELENGTH, len+WavelengthMin);
				exposureTime = cameraParR[cameraParIndex++];
				digitalGain = cameraParR[cameraParIndex++];
				analogGain = cameraParR[cameraParIndex++];

				m_mydevice->controlDevice(EXPOSURETIME, exposureTime);
				m_mydevice->controlDevice(DIGITALGAIN, digitalGain);
				m_mydevice->controlDevice(ANALOGGAIN, analogGain);

				//添加多次拍照
				//for (int i = 0; i < PHOTO_TIMES + 1; i++)
				//{
					//for (int j = 0; j < 2; ++j)
					//	m_mydevice->takephoto();
					////去除最初的n张无效图片
					data_photo_processing = m_mydevice->takephoto();
					int w = len + WavelengthMin;
					char c[5] = { 0 };
					itoa(w, c, 10);
					String t(c);
					//char ic[1] = { 0 };
					//itoa(i, ic, 10);
#ifdef __IN8BITS__
					cv::String name = photoPath + t + ".bmp";
#else
					cv::String name = photoPath + t + ".exr";
#endif
					cv::imwrite(name, data_photo_processing);
	/*			}*/
				len += LCTF_Gap;			
				if (len > LCTF_range)
				{
					isPhotoFinished = true;
				}
				else{
					process.setValue(len);
				}			
				if (process.wasCanceled())
				{
					QMessageBox::warning(this, QStringLiteral("正在取消"), QStringLiteral("已保存的图片将不会删除"));
					m_mydevice->setResolution(data_width, data_height);
					m_mydevice->continueDevice();
					return;
				}
			}
		}
		////这里应该设置防止此时按下cancel按钮
		process.setCancelButton(0);
		//control(0x30, 0x2A, 0x00, 0x09);
		// 设置分辨率 640*480
		m_mydevice->setResolution(data_width, data_height);
		process.setValue(LCTF_range + 1);
	}
	m_thread_curves->start();
	m_mydevice->continueDevice();
	QMessageBox::about(this, QStringLiteral("提示"), QStringLiteral("拍照完成"));
}

void MyWindow::photoOnce()
{
	if (m_mydevice->m_usb_device == nullptr)
	{
		QMessageBox::about(this, QStringLiteral("提示"), QStringLiteral("设备未连接！"));
		return;
	}
	int PHOTO_TIMES = m_mydevice->getPhotoTimes();
	Mat data_temp;
	int data_height = m_mydevice->getDataHeight();
	int data_width = m_mydevice->getDataWidth();
	int photo_height = m_mydevice->getPhotoHeight();
	int photo_width = m_mydevice->getPhotoWidth();

	m_mydevice->pauseDevice();
	//分辨率设置为大分辨率
	m_mydevice->setResolution(photo_width, photo_height);

	for (int i = 0; i < PHOTO_TIMES; i++)
	{
		data_temp = m_mydevice->takephoto();
		String photoPath = m_mydevice->getPhotoPath();
		QDateTime dt;
		QTime time;
		QDate date;
		dt.setTime(time.currentTime());
		dt.setDate(date.currentDate());
		QString currentDate = dt.toString("yyyyMMddhhmmss");
		String name = currentDate.toStdString();
		char a[1] = {0};
		itoa(i,a,10);
		String num = a;
#ifdef __IN8BITS__
		String nameFull = photoPath + '/' + name + '(' + num + ')' + ".bmp";
#else
		String nameFull = photoPath + '/' + name + '(' + num + ')' + ".exr";
#endif
		cv::imwrite(nameFull, data_temp);
	}

	m_mydevice->setResolution(data_width, data_height);
	m_mydevice->continueDevice();
}

void MyWindow::setWavelengthGap()
{
	QString str = ui.lineEdit_WLGap->text();
	if (str.isEmpty())
	{
		int gap = m_mydevice->getWavelengthGap();
		ui.lineEdit_WLGap->setText(QString::number(gap));
	}
	else
	{
		m_mydevice->setWavelengthGap(str.toInt());
		ui.horizontalSlider_Wavelength->setSingleStep(m_mydevice->getWavelengthGap());
		ui.horizontalSlider_Wavelength->setPageStep(m_mydevice->getWavelengthGap());
	}
}

void MyWindow::setWavelength()
{
	int value = ui.horizontalSlider_Wavelength->value();
	ui.lineEdit_Wavelength->setText(QString::number(value));

	if (!m_mydevice->controlLCTF(WAVELENGTH, value))
	{
		//ERROR：波长设置失败
		QMessageBox::StandardButton reply;
		reply = QMessageBox::critical(this, QStringLiteral("错误"),
			QStringLiteral("波长设置时出现错误！"),
			QMessageBox::Retry | QMessageBox::Cancel);
		if (reply == QMessageBox::Retry)
			setWavelength();
	}
	
}

void MyWindow::setDigitalGain()
{
	int value = ui.horizontalSlider_DigitalGain->value();
	ui.lineEdit_DigitalGain->setText(QString::number(value));
	if (!m_mydevice->controlDevice(DIGITALGAIN, value))
	{
		/////ERROR:数字增益设置失败
		QMessageBox::StandardButton reply;
		reply = QMessageBox::critical(this, QStringLiteral("错误"),
			QStringLiteral("数字增益设置时出现错误！"),
			QMessageBox::Retry | QMessageBox::Cancel);
		if (reply == QMessageBox::Retry)
			setDigitalGain();
	}
}

void MyWindow::setExposureTime()
{
	int value = ui.horizontalSlider_ExposureTime->value();
	ui.lineEdit_ExposureTime->setText(QString::number(value));
	if (!m_mydevice->controlDevice(EXPOSURETIME, value))
	{
		/////ERROR:曝光时间设置失败
		QMessageBox::StandardButton reply;
		reply = QMessageBox::critical(this, QStringLiteral("错误"),
			QStringLiteral("曝光时间设置时出现错误！"),
			QMessageBox::Retry | QMessageBox::Cancel);
		if (reply == QMessageBox::Retry)
			setExposureTime();
	}
}

void MyWindow::setExposureTime_lineEdit()
{
	QString str = ui.lineEdit_ExposureTime->text();
	int value = ui.horizontalSlider_ExposureTime->value();
	if (str.isEmpty())
	{
		ui.lineEdit_ExposureTime->setText(QString::number(value));
	}
	else if (value == str.toInt())
	{
		return;
	}
	else
	{
		ui.horizontalSlider_ExposureTime->setValue(str.toInt());
	}
}

void MyWindow::setWavelength_lineEdit()
{
	QString str = ui.lineEdit_Wavelength->text();
	int value = ui.horizontalSlider_Wavelength->value();
	if (str.isEmpty())
	{
		ui.lineEdit_Wavelength->setText(QString::number(value));
	}
	else if (value == str.toInt())
	{
		return;
	}
	else
	{
		ui.horizontalSlider_Wavelength->setValue(str.toInt());
	}
}

void MyWindow::setDigitalGain_lineEdit()
{
	QString str = ui.lineEdit_DigitalGain->text();
	int value = ui.horizontalSlider_DigitalGain->value();
	if (str.isEmpty())
	{
		ui.lineEdit_DigitalGain->setText(QString::number(value));
	}
	else if (value == str.toInt())
	{
		return;
	}
	else
	{
		ui.horizontalSlider_DigitalGain->setValue(str.toInt());
	}
}

void MyWindow::setAnalogGain()
{
	int num = ui.comboBox_AnalogGain->currentIndex();
	switch (num)
	{
		case 1://二倍增益
			if (!m_mydevice->controlDevice(ANALOGGAIN, 2))
			{
				/////ERROR:模拟增益错误
				QMessageBox::StandardButton reply;
				reply = QMessageBox::critical(this, QStringLiteral("错误"),
					QStringLiteral("模拟增益设置时出现错误！"),
					QMessageBox::Retry | QMessageBox::Cancel);
				if (reply == QMessageBox::Retry)
					setAnalogGain();
			}
			break;
		case 2://四倍增益
			if (!m_mydevice->controlDevice(ANALOGGAIN, 4))
			{
				/////ERROR:模拟增益错误
				QMessageBox::StandardButton reply;
				reply = QMessageBox::critical(this, QStringLiteral("错误"),
					QStringLiteral("模拟增益设置时出现错误！"),
					QMessageBox::Retry | QMessageBox::Cancel);
				if (reply == QMessageBox::Retry)
					setAnalogGain();
			}
			break;
		case 3://八倍增益
			if (!m_mydevice->controlDevice(ANALOGGAIN, 8))
			{
				/////ERROR:模拟增益错误
				QMessageBox::StandardButton reply;
				reply = QMessageBox::critical(this, QStringLiteral("错误"),
					QStringLiteral("模拟增益设置时出现错误！"),
					QMessageBox::Retry | QMessageBox::Cancel);
				if (reply == QMessageBox::Retry)
					setAnalogGain();
			}
			break;
		case 4://十倍增益
			if (!m_mydevice->controlDevice(ANALOGGAIN, 10))
			{
				/////ERROR:模拟增益错误
				QMessageBox::StandardButton reply;
				reply = QMessageBox::critical(this, QStringLiteral("错误"),
					QStringLiteral("模拟增益设置时出现错误！"),
					QMessageBox::Retry | QMessageBox::Cancel);
				if (reply == QMessageBox::Retry)
					setAnalogGain();
			}
			break;
		case 0:
		default:
			if (!m_mydevice->controlDevice(ANALOGGAIN, 1))
			{
				/////ERROR:模拟增益错误
				QMessageBox::StandardButton reply;
				reply = QMessageBox::critical(this, QStringLiteral("错误"),
					QStringLiteral("模拟增益设置时出现错误！"),
					QMessageBox::Retry | QMessageBox::Cancel);
				if (reply == QMessageBox::Retry)
					setAnalogGain();
			}
			break;
	}
}

void MyWindow::showConfigDialog()
{
	String temp = m_mydevice->getSystemPath();
	String temp2 = m_mydevice->getCustomPath();
	int temp_int = m_mydevice->getPhotoTimes();
	m_config_dialog->ui.lineEdit_customPath->setText(temp2.c_str());
	m_config_dialog->ui.lineEdit_path->setText(temp.c_str());
	m_config_dialog->ui.lineEdit_times->setText(QString::number(temp_int));
	m_config_dialog->show();
}

void MyWindow::setConfig()
{
	QString customPath = m_config_dialog->ui.lineEdit_customPath->text();
	if (!customPath.isEmpty())
	{
		m_mydevice->setCustomPath(customPath.toStdString());
		initializePath();
	}
	QString photoPath = m_config_dialog->ui.lineEdit_path->text();
	if (!photoPath.isEmpty())
	{
		m_mydevice->setSystemPath(photoPath.toStdString());
		initializePath();
	}
	QString photoTimes = m_config_dialog->ui.lineEdit_times->text();
	if (!photoTimes.isEmpty())
	{
		m_mydevice->setPhotoTimes(photoTimes.toInt());
	}
	m_config_dialog->close();
}

void MyWindow::showPhotoCheckWindow()
{
	std::string path;
	std::vector<std::string> files;

	path = path = m_mydevice->getPhotoPath().c_str();
	int photo_height = m_mydevice->getPhotoHeight();
	int photo_width = m_mydevice->getPhotoWidth();
	////获取该路径下的所有文件  

	m_mydevice->pauseDevice();
	m_mydevice->getFileList(path, files);
	m_photocheck_window->setFileList(files);
	m_photocheck_window->setPhotoSize(photo_height, photo_width);

	m_photocheck_window->show();

}

void MyWindow::closePhotoCheckWindow()
{
	m_mydevice->continueDevice();
}

void MyWindow::setCurvesMat(int x, int y)
{
	if (m_thread_curves->isRunning())
	{
		QMessageBox::about(this, QStringLiteral("提示"), QStringLiteral("正在进行数据处理！"));
		return;
	}
	else if (/*!m_mydevice->readCurvesMat()&&*/m_mydevice->m_curvesMat.empty())
	{
		m_thread_curves->start();
		QMessageBox::about(this, QStringLiteral("提示"), QStringLiteral("正在生成数据！"));
		return;
	}
	//检测点击的是否是连拍图像
	String str = m_photocheck_window->getCurrentFilePath();
	const char* sub_str = m_mydevice->getCustomPath().c_str();
	if (strstr(str.c_str(), sub_str) == NULL)
	{
		return;
	}

	int row_length = m_mydevice->getPhotoWidth();
	Mat temp_row = m_mydevice->m_curvesMat.row(y*row_length+x);

	m_specAnalysis_window->ui.diagram->transMat(temp_row,m_mydevice->m_wavelengthMat);
	m_specAnalysis_window->ui.diagram->setPaintEnable(true);
	m_specAnalysis_window->ui.diagram->refresh();

	m_specAnalysis_window->show();

	return;

	////FOR DEBUG
	//Mat temp1, temp2;

	//m_specAnalysis_window->ui.diagram->transMat(temp1, temp2);
	//m_specAnalysis_window->ui.diagram->setPaintEnable(true);
	//m_specAnalysis_window->ui.diagram->refresh();

	//m_specAnalysis_window->show();
	//return;
}

void MyWindow::setPrepareProcess(int process)
{
	QString num = QString::number(process);
	QString full = QStringLiteral("当前进度：") + num + QStringLiteral("%");
	if (process == 100)
	{
		m_photocheck_window->ui.label_state->setText(QStringLiteral("就绪"));
	}
	else
	{
		m_photocheck_window->ui.label_state->setText(full);
	}

}

void MyWindow::setConfigPath()
{
	QString filename = QFileDialog::getExistingDirectory(this,
		QStringLiteral("选择路径"), ""); //选择路径  
	if (filename.isEmpty())
	{
		return;
	}
	else
	{
		m_config_dialog->ui.lineEdit_path->setText(filename);
		return;
	}
}