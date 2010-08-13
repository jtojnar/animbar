/* 
 * (c) 2010 Simon Flöry (simon.floery@gmx.at)
 * 
 * This file is part of animbar.
 * 
 * animbar is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * animbar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with animbar.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _MAINWINDOW_H
#define _MAINWINDOW_H

#include <QtGui>

#include "animbar.h"

/* we want to use pointers to QImages as user defined data type in 
 * QVariants. See also constructor MainWindow::MainWindow().
 */
Q_DECLARE_METATYPE(QImage*)

class MainWindow : public QMainWindow
{
	Q_OBJECT
	
public:
	MainWindow();
	~MainWindow();
	
protected:
	void closeEvent(QCloseEvent*);
	void keyReleaseEvent (QKeyEvent*);
	
private slots:
	/* the menu slots */
	void openFile();
	void saveBaseImage();
	void saveBarMask();

	void compute();
	
	void aboutqt();
	void about();
	
	/* the other slots */
	void sliderChangedValue(int);
	

private:
	/* private member functions */
	void _init();
	
	bool setupUI();
	bool setupMenus();
	
	bool loadSettings();
	bool saveSettings();
	
	QImage* getImage(QListWidgetItem*);
	QImage* getImage(int);
	
	QString getSupportedImageFormats() const;
	
	bool saveImage(const QImage&, const QString&);
	
	bool compute(const std::vector< QImage* >);
	
	/* private member variables */
	QListWidget *imageList;
	QLabel *imageLabel;
	QSlider *slider;
	
	QDir openDir;
	QDir saveDir;
	bool setSaveToOpen;
	
	QImage baseImage;
	QImage barMask;
	QImage image;
	
	int stripWidth;
};

#endif // _MAINWINDOWFORM_H
