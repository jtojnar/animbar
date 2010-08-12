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

#include <iostream>

#include "MainWindow.h"

//----------------------------------------------------------------------

/* stream operators for pointers to QImage, see comment in constructor
 * of MainWindow::MainWindow().
 */
QDataStream &operator<<( QDataStream &out, const QImage* & ) { return out; }
QDataStream &operator>>( QDataStream &in, QImage* & ) { return in; }

//----------------------------------------------------------------------

MainWindow::MainWindow() : QMainWindow()
{	
	_init();
	
	setupUI();
	setupMenus();
	
	loadSettings();
	
	/* register pointers to QImage as meta types and register functions
	 * to load and save. This is all a bit strange. If we do not give
	 * stream operators, my qt 4.6. complains about not being able to
	 * save user defined QVariant when drag and drop. I have no idea,
	 * what they need save when draggin and dropping (only in move 
	 * mode).
	 */
	qRegisterMetaType<QImage*>("QImage*");
	qRegisterMetaTypeStreamOperators<QImage*>("QImage*");
}

//----------------------------------------------------------------------

void MainWindow::_init()
{
	openDir = QDir::home();
	saveDir = openDir;
	setSaveToOpen = true;
}

//----------------------------------------------------------------------

MainWindow::~MainWindow()
{
	/* Iterate over all list items and delete the image pointer */
	for ( int i=0 ; i < imageList->count() ; i++ ) delete getImage(i);
}

//----------------------------------------------------------------------

bool MainWindow::setupUI()
{
	/**
	 * the menu bar is already setup by default
	 **/
	
	/**
	 * the central widget
	 **/
	
	QSplitter *centralWidget = new QSplitter(Qt::Horizontal, this);
	setCentralWidget(centralWidget);
	
	/* the left side */
	
	QWidget *leftSide = new QWidget(centralWidget);
	centralWidget->addWidget(leftSide);
	
	QVBoxLayout *vLayoutL = new QVBoxLayout(leftSide);
	leftSide->setLayout(vLayoutL);
	
	int left, top, right, bottom;
	vLayoutL->getContentsMargins(&left, &top, &right, &bottom);
	vLayoutL->setContentsMargins(left, top, right/2, bottom);
	
	/* see 
	 * 		http://doc.trolltech.com/4.4/model-view-dnd.html#using-convenience-views
	 * for drag & drop 
	 */
	imageList = new QListWidget(leftSide);
	vLayoutL->addWidget(imageList);
	imageList->setViewMode(QListView::ListMode);
	imageList->setIconSize(QSize(0,100));
	imageList->setMovement(QListView::Static);
	imageList->setFlow(QListView::TopToBottom);
	imageList->setSelectionMode(QAbstractItemView::ExtendedSelection);
	imageList->setDragEnabled(true);
	imageList->viewport()->setAcceptDrops(true);
	imageList->setDropIndicatorShown(true);
	imageList->setDragDropMode(QAbstractItemView::InternalMove);
	
	/* the right side */
	
	QWidget *rightSide = new QWidget(centralWidget);
	centralWidget->addWidget(rightSide);
	
	QVBoxLayout *vLayoutR = new QVBoxLayout(rightSide);
	rightSide->setLayout(vLayoutR);
	
	vLayoutR->getContentsMargins(&left, &top, &right, &bottom);
	vLayoutR->setContentsMargins(left/2, top, right, bottom);
	
	QScrollArea *scrollArea = new QScrollArea;
	scrollArea->setBackgroundRole(QPalette::Dark);
	scrollArea->setAlignment(Qt::AlignCenter);
	vLayoutR->addWidget(scrollArea);
	
	imageLabel = new QLabel(rightSide);
	scrollArea->setWidget(imageLabel);

	slider = new QSlider(Qt::Horizontal, rightSide);
	slider->setTickInterval(1);
	slider->setTickPosition(QSlider::NoTicks);
	vLayoutR->addWidget(slider);
	
	centralWidget->setStretchFactor(1, 20);
	
	/**
	 * the status bar is already setup by default
	 **/

	QString version = QString("%1.%2").arg(ANIMBAR_VERSION_MAJOR).arg(ANIMBAR_VERSION_MINOR);	
	statusBar()->showMessage(tr("Welcome to ") + ANIMBAR_PROG_NAME + " v" + version, 5000);
	
	return true;
}

//----------------------------------------------------------------------

bool MainWindow::setupMenus()
{
	/**
	 * file menu
	 **/
	
	QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

	/* at least on my system, QKeySequence::Quit is empty, which is 
	 * annoying.
	 */
	QAction *action = new QAction(tr("&Open Images ..."), this);
	if (QKeySequence(QKeySequence::Open).isEmpty()) action->setShortcut(tr("Ctrl+O"));
    else action->setShortcuts(QKeySequence::Open);
    action->setStatusTip(tr("Open a file"));
    connect(action, SIGNAL(triggered()), this, SLOT(openFile()));
	fileMenu->addAction(action);
	
	fileMenu->addSeparator();
	
	action = new QAction(tr("&Save Base Image ..."), this);
	if (QKeySequence(QKeySequence::Save).isEmpty()) action->setShortcut(tr("Ctrl+S"));
    else action->setShortcuts(QKeySequence::Save);
    action->setStatusTip(tr("Save computed image, that is the base of the animation"));
    connect(action, SIGNAL(triggered()), this, SLOT(saveBaseImage()));
	fileMenu->addAction(action);
	
	action = new QAction(tr("Save &Bar Mask ..."), this);
	action->setShortcut(tr("Ctrl+B"));
    action->setStatusTip(tr("Save bar mask image"));
    connect(action, SIGNAL(triggered()), this, SLOT(saveBarMask()));
	fileMenu->addAction(action);
	
	fileMenu->addSeparator();
	
	action = new QAction(tr("&Quit"), this);
    if (QKeySequence(QKeySequence::Quit).isEmpty()) action->setShortcut(tr("Ctrl+Q"));
    else action->setShortcuts(QKeySequence::Quit);
    action->setStatusTip(tr("Exit the application"));
    connect(action, SIGNAL(triggered()), this, SLOT(close()));
	fileMenu->addAction(action);
	
	/**
	 * edit menu
	 **/
	
	QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
	
	action = new QAction(tr("&Compute Animation"), this);
	action->setShortcut(tr("Ctrl+M"));
    action->setStatusTip(tr("Compute bar animation from input images"));
    connect(action, SIGNAL(triggered()), this, SLOT(compute()));
	editMenu->addAction(action);
	
	/**
	 * help menu
	 **/
	
	QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
	
	action = new QAction(tr("About &Qt"), this);
    action->setStatusTip(tr("Display information about qt"));
    connect(action, SIGNAL(triggered()), this, SLOT(aboutqt()));
	helpMenu->addAction(action);
		
	action = new QAction(tr("&About"), this);
    action->setStatusTip(tr("Display information about this program"));
    connect(action, SIGNAL(triggered()), this, SLOT(about()));
	helpMenu->addAction(action);
	
	return true;
}

//----------------------------------------------------------------------

bool MainWindow::loadSettings()
{
	QSettings settings("mnim.org", "animbar");
	
	QVariant winSizeV = settings.value("winSize");
	if (winSizeV.isValid()) resize(winSizeV.toSize());
	
	QVariant winPosV = settings.value("winPos");
	if (winPosV.isValid()) move(winPosV.toPoint());
	
	return true;
}

//----------------------------------------------------------------------

bool MainWindow::saveSettings()
{
	QSettings settings("mnim.org", "animbar");
	settings.setValue("winPos", pos());
	settings.setValue("winSize", size());
	
	return true;
}

//----------------------------------------------------------------------

QImage* MainWindow::getImage(QListWidgetItem* li)
{
#ifdef ANIMBAR_DEBUG
	if (!li) {
		std::cerr << "MainWindow::getImage - Invalid list item." << std::endl;
		return NULL;
	}
#endif
	
	return li->data(Qt::UserRole).value< QImage* >();
}

//----------------------------------------------------------------------

QImage* MainWindow::getImage(int idx)
{
#ifdef ANIMBAR_DEBUG
	if (idx < 0 || idx >= imageList->count()) {
		std::cerr << "MainWindow::getImage - Index out of bounds." << std::endl;
		return NULL;
	}
#endif

	return getImage(imageList->item(idx));
}

//----------------------------------------------------------------------

QString MainWindow::getSupportedImageFormats() const
{
	QString imageFilter(tr("Images ("));
	QList<QByteArray> supportedFormats = QImageReader::supportedImageFormats();
	for (int i=0; i<supportedFormats.size(); i++) {
		imageFilter += " *.";
		imageFilter += supportedFormats[i].data();
	}
	imageFilter += ")";
	
	return imageFilter;
}

//----------------------------------------------------------------------

void MainWindow::closeEvent(QCloseEvent *event)
{
	saveSettings();
	event->accept();
}

//----------------------------------------------------------------------

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
	if (event->matches(QKeySequence::Delete)) {
		/* if the delete key has been released, delete all selected 
		 * items. start in the back to not mess up with the indices.
		 */
		for ( int i=imageList->count()-1 ; i>=0 ; i-- ) {
			QListWidgetItem *li = imageList->item(i);
			if (li->isSelected()) {
				imageList->takeItem(i);
				delete getImage(li);
				delete li;
			}
		}
	}
	/* we do not accept in order to get the signal propagated further */
}

//----------------------------------------------------------------------

void MainWindow::openFile()
{
	/* get list of files to load */
	
	QStringList files = QFileDialog::getOpenFileNames(
		this, 
		tr("Open images to build animation from"), 
		openDir.absolutePath(),
		getSupportedImageFormats());
	
	if (files.size() > 0) openDir.setPath(files[0]);
	if (setSaveToOpen) {
		saveDir = openDir;
		setSaveToOpen = false;
	}
	
	QSize iconSize = imageList->iconSize();
	
	/* prepare progressbar */
	
	QProgressBar *pbar = new QProgressBar(statusBar());
	pbar->setMinimum(0);
	pbar->setMaximum(files.size());
	pbar->setOrientation(Qt::Horizontal);
	pbar->setFormat(tr("Loading image %v of %m"));
	statusBar()->addWidget(pbar, 1);
	
	/* open the images */
	
	for ( int i=0 ; i < files.size() ; i++ ) {
		pbar->setValue(i+1);
		
		/* open the image, we will keep this pointer until the image
		 * is removed from the list or program quits.
		 */
		QImage *img = new QImage(files[i]);
		
		/* check if open was succesful */
		if (!img || img->isNull()) {
			QMessageBox::warning(
				this, 
				tr("Warning"), 
				tr("Could not load image ") + files[i] + 
				tr(". Please verify that it is an image file of proper format."));
			if (img) delete img;
			continue;
		}
		
		/* check if image is of correct size */
		if ((imageList->count()) > 0 && (getImage(0)->size() != img->size())) {
			QMessageBox::warning(
				this,
				tr("Warning"),
				tr("All input images must be of same size. However, image ") + 
				files[i] + tr(" is not of reference pixel size ") +
				QString("%1").arg(getImage(0)->size().width()) + "x" + 
				QString("%1").arg(getImage(0)->size().height()) + 
				tr(". Hence, it will not be loaded."));
			delete img;
			continue;
		}
		
		/* create thumbnail. Do not use Qt::FastTransformation, it 
		 * displays resulting baseImages after scaling worong (e.g.
		 * only one of the input images.
		 */
		QImage thumbnail = img->scaledToHeight(iconSize.height(), Qt::SmoothTransformation);
		QIcon icon(QPixmap::fromImage(thumbnail));
		
		if (iconSize.width() <= 0) imageList->setIconSize(thumbnail.size());
		
		/* create new list entry and add it */
		QFileInfo fi(files[i]);
		QListWidgetItem *li = new QListWidgetItem(icon, fi.fileName(), imageList);
		li->setData(Qt::UserRole, qVariantFromValue(img));
		li->setFlags(li->flags() | Qt::ItemIsDragEnabled);
		imageList->addItem(li);	
	}
	
	/* remove progress bar again */
	statusBar()->removeWidget(pbar);
	delete pbar;
}

//----------------------------------------------------------------------

void MainWindow::compute()
{
	/* if more than one image is selected, we work only on the selected
	 * image, otherwise, on all. Here, we get a std::vector of pointers
	 * to the images, from which we compute the results.
	 */
	QList<QListWidgetItem *> selectedElements = imageList->selectedItems();
	
	std::vector< QImage* > imgs;
	if (selectedElements.size() > 1) {
		imgs.resize(selectedElements.size());
		for ( unsigned int i=0 ; i<imgs.size() ; i++ ) 
			imgs[i] = selectedElements[i]->data(Qt::UserRole).value< QImage* >();
	} else {
		imgs.resize(imageList->count());
		for ( unsigned int i=0 ; i<imgs.size() ; i++ ) 
			imgs[i] = getImage(i);
	}
	
	compute(imgs);
}

//----------------------------------------------------------------------

bool MainWindow::compute(const std::vector< QImage* > imgs)
{
	int nrImgs = imgs.size();
	
	if (nrImgs <= 0) {
		QMessageBox::warning(
			this, 
			tr("Warning"), 
			tr("Open some input image files first (File -> Open) for having some input to compute the animation."));
		return false;
	}
	
	/* reset results */
	baseImage = QImage();
	barMask = QImage();
	
	QSize size0 = imgs[0]->size();
	
	/* setup progress bar */
	
	QProgressBar *pbar = new QProgressBar(statusBar());
	pbar->setMinimum(0);
	pbar->setMaximum(2*size0.width());
	pbar->setOrientation(Qt::Horizontal);
	pbar->setFormat(tr("Processing %p%"));
	statusBar()->addWidget(pbar, 1);
	
	/*
	 * at first, compute baseImage
	 */
	
	baseImage = QImage(size0, QImage::Format_ARGB32_Premultiplied);
	
	/* go from left to right through baseImage. Write a column of each
	 * image again and again.
	 */
	for ( int col=0 ; col<size0.width() ; ) {
		pbar->setValue(col+1);
		for ( int i=0 ; i<nrImgs && col<size0.width() ; i++, col++ )
			for ( int row=0 ; row<size0.height() ; row++ )
				baseImage.setPixel(col, row, imgs[i]->pixel(col, row));
	}
	
	/*
	 * then, compute barmask
	 */
	
	barMask = QImage(size0, QImage::Format_Mono);
	for ( int col=0 ; col < size0.width() ; col++ ) {
		pbar->setValue(size0.width()+col+1);
		for ( int row=0 ; row < size0.height() ; row++ )
			barMask.setPixel(col, row, (col % nrImgs == 0) ? 1 : 0 );
	}
			
	/* image is the one displayed on imageLabel */
	
	image = QImage(size0, QImage::Format_ARGB32_Premultiplied);
	
	/* remove progress bar again */
	statusBar()->removeWidget(pbar);
	delete pbar;
	
	/* configure slider to current setup */
	slider->setRange(0, nrImgs);
	slider->setSingleStep(1);
	slider->setTracking(true);
	slider->setValue(0);
	slider->setTickPosition(QSlider::TicksBelow);
	connect(slider, SIGNAL(valueChanged(int)), this, SLOT(sliderChangedValue(int)));
	/* when slider's default value is zero, setValue will not trigger 
	 * the signal. hence we setValue before connect and then call the
	 * signal handler for 0.
	 */
	sliderChangedValue(0);

	return true;
}

//----------------------------------------------------------------------

void MainWindow::sliderChangedValue(int idx)
{
	if (idx == 0) {
		/* for idx=0, display without mask. */
		imageLabel->setPixmap(QPixmap::fromImage(baseImage));
		imageLabel->resize(imageLabel->pixmap()->size());
	} else {
		/* this will see some optimization one day. for now, it's a 
		 * proof of concept 
		 */
		
		QPainter painter(&image);
		
		/* when we move the mask to the right, there is a hole that 
		 * needs to be filled with black color to the left.
		 */
		if (idx > 1) {
			painter.setPen(QColor(0,0,0));
			painter.fillRect(0,0,idx-1,image.size().height(),Qt::SolidPattern);
		}
		
		/* set barMask with some offset to the right */
		painter.setCompositionMode(QPainter::CompositionMode_Source);
		painter.drawImage(idx-1, 0, barMask);
		
		/* finally, multiply baseImage */
		painter.setCompositionMode(QPainter::CompositionMode_Multiply);
		painter.drawImage(0, 0, baseImage);

		painter.end();
		
		imageLabel->setPixmap(QPixmap::fromImage(image));
		imageLabel->resize(imageLabel->pixmap()->size());
	}
}

//----------------------------------------------------------------------

void MainWindow::saveBaseImage()
{
	saveImage(baseImage, "Enter filename to save the animation's base image");
}

//----------------------------------------------------------------------

void MainWindow::saveBarMask()
{
	saveImage(barMask, "Enter filename to save bar mask image");
}

//----------------------------------------------------------------------

bool MainWindow::saveImage(const QImage& img, const QString& caption)
{
	if (img.isNull()) {
		QMessageBox::warning(
			this, 
			tr("Warning"), 
			tr("Compute the animation first (Edit -> Compute) for having any results to get saved."));
		return false;
	}
	
	do {
		QString filename = QFileDialog::getSaveFileName( 
			this, 
			caption, 
			saveDir.absolutePath(), 
			getSupportedImageFormats());
			
		if (!filename.isNull()) {
			saveDir.setPath(filename);
			if (!img.save(filename))
				QMessageBox::warning(
					this, 
					tr("Warning"), 
					tr("Failed to save to ") + filename + tr(". Please try again with another filename."));
			else break;
		} else break;
	} while (true);
	
	return true;
}

//----------------------------------------------------------------------

void MainWindow::aboutqt()
{
	QMessageBox::aboutQt(this, "About Qt");
}

//----------------------------------------------------------------------

void MainWindow::about()
{
	QString version = QString("%1.%2").arg(ANIMBAR_VERSION_MAJOR).arg(ANIMBAR_VERSION_MINOR);
	
	int year = QDate::currentDate().year();
	QString yearS = QString("%1").arg(year);
	if (year > 2010) yearS.prepend("2010 - ");
	
	QMessageBox::about(
		this, 
		tr("About ") + ANIMBAR_PROG_NAME, 
		tr("<p><center><font size=+2><b>") + ANIMBAR_PROG_NAME + " v" + version + tr("</b></font></center></p>"
			"<p><center><a href=\"http://animbar.mnim.org\">http://animbar.mnim.org</a></center></p>"
			"<p><center>&copy; ") + yearS + tr(" Simon Fl&ouml;ry</center></p>"
			"<p><b>") + ANIMBAR_PROG_NAME + tr("</b> is free software: you can redistribute it and/or modify "
			"it under the terms of the GNU General Public License as published by "
			"the Free Software Foundation, either version 3 of the License, or "
			"(at your option) any later version.</p>"
			"<p><b>") + ANIMBAR_PROG_NAME + tr("</b> is distributed in the hope that it will be useful, "
			"but WITHOUT ANY WARRANTY; without even the implied warranty of "
			"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the "
			"<a href=\"http://www.gnu.org/licenses/\">GNU General Public "
			"License</a> for more details.</p>"));
}
