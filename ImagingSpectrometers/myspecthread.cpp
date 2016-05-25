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
	m_pImSpec->getCurvesMat();
	return;
}