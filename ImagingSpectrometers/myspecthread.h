#ifndef MYSPECTHREAD_H
#define MYSPECTHREAD_H

#ifndef __IMAGINGSPEC__
#include "imagingspectrometers.h"
#define __IMAGINGSPEC__
#endif

#include <QThread>

class MySpecThread : public QThread
{
	Q_OBJECT

public:
	MySpecThread(QObject *parent=0);
	MySpecThread(ImagingSpectrometers *p);
	~MySpecThread();

protected:
	void run();

private:
	ImagingSpectrometers *m_pImSpec;
};

#endif // MYSPECTHREAD_H
