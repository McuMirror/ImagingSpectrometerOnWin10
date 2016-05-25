#ifndef SPECTRALANALYSIS_H
#define SPECTRALANALYSIS_H

#include <QWidget>
#include "ui_spectralanalysis.h"

class SpectralAnalysis : public QWidget
{
	Q_OBJECT

public:
	SpectralAnalysis(QWidget *parent = 0);
	~SpectralAnalysis();
public:
	Ui::SpectralAnalysis ui;
public slots:
	void updateMousePos(int x, int y);

private:

};

#endif // SPECTRALANALYSIS_H
