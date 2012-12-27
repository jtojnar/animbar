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
	
	/* strip width in pixels, three seems to be a good value */
	stripWidth = 3;
	
	/* Initial zoom factor is 1, e.g. no zoom */
	zoomFactor = 1.;
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
	
	QString welcomeMsg = 
		tr("Welcome to ") + ANIMBAR_PROG_NAME + 
		" v" + QString("%1.%2").arg(ANIMBAR_VERSION_MAJOR).arg(ANIMBAR_VERSION_MINOR);
	
	imageLabel = new QLabel(rightSide);
	imageLabel->setWordWrap(true);
	imageLabel->setOpenExternalLinks(true);
	imageLabel->setText(
		"<p><font size=+3>" + welcomeMsg + "</font></p>" +
		"<p>This short step-by-step tutorial guides you through creating your first animation with " +
		ANIMBAR_PROG_NAME + ". For more documentation, please visit the project's webpage " +
		"<a href=\"http://animbar.mnim.org\" style=\"color: white;\">http://animbar.mnim.org</a>.</p>" +
		"<p><font size=+3>1.</font> Select <i>Open Images ...</i> from the <i>File</i> menu to open one or " +
		"many input images. The loaded images are displayed in the list view to the left. " + 
		"Three to six input images are a good number for a start.</p>" +
		"<p><font size=+3>2.</font> Select <i>Compute Animation</i> from the <i>Edit</i> menu to generate " +
		"the output files for the picket fence animation. You are asked to specify " +
		"the strip width, that is the pickets' width, in pixel. Three is a good value " +
		"for a first animation.</p>" +
		"<p><font size=+3>3.</font> Verify the generated output by moving the slider " +
		"at the bottom of the user interface. Use the <i>zoom</i> entries in the <i>View</i> " +
		"menu to further evaluate the animation.</p>" +
		"<p><font size=+3>4.</font> To test the generated output in a real world animation, " +
		"use <i>Save Base Image ...</i> and <i>Save Bar Mask ...</i> in the <i>File</i> menu " +
		"to save the output to image files. Print the base image on paper and the bar mask " +
		"to transparancy.</p>" +
		"<p>If you have any suggestion or need further assistance, check out " +
		"<a href=\"http://animbar.mnim.org\" style=\"color: white;\">http://animbar.mnim.org</a>.</p>"
	);
	
//This is a short step-by-step tutorial for creating your first
//picket fence animation.");

//<p><center><font size=+2><b>") +  + " v" + version + tr("</b></font></center></p>"
//"<p><center><a href=\"http://animbar.mnim.org\">http://animbar.mnim.org</a></center></p>"

	scrollArea->setWidget(imageLabel);
	
	slider = new QSlider(Qt::Horizontal, rightSide);
	slider->setTickInterval(1);
	slider->setTickPosition(QSlider::NoTicks);
	vLayoutR->addWidget(slider);
	
	centralWidget->setStretchFactor(1, 20);
	
	/**
	 * the status bar is already setup by default
	 **/
	 	
	statusBar()->showMessage(welcomeMsg, 5000);
	
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

    action = new QAction(tr("Save &Animation ..."), this);
//    action->setShortcut(tr("Ctrl+A"));
    action->setStatusTip(tr("Save comptued animation to SVG file"));
    connect(action, SIGNAL(triggered()), this, SLOT(saveAnimation()));
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
	 * view menu
	 **/
	
	QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
	
	action = new QAction(tr("Zoom &In"), this);
	action->setShortcut(tr("Ctrl++"));
    action->setStatusTip(tr("Zoom in, e.g. enlarging the displayed view"));
    connect(action, SIGNAL(triggered()), this, SLOT(zoomIn()));
	viewMenu->addAction(action);
	
	action = new QAction(tr("Zoom &Out"), this);
	action->setShortcut(tr("Ctrl+-"));
    action->setStatusTip(tr("Zoom out, e.g. shrinking the displayed view"));
    connect(action, SIGNAL(triggered()), this, SLOT(zoomOut()));
	viewMenu->addAction(action);
	
	action = new QAction(tr("&Normal View"), this);
	action->setShortcut(tr("Ctrl+0"));
    action->setStatusTip(tr("Change view to normal, e.g. reset zoom level to default"));
    connect(action, SIGNAL(triggered()), this, SLOT(zoomReset()));
	viewMenu->addAction(action);
	
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
    /* store this to save the animation later */
    m_animationImages = imgs;

	int nrImgs = imgs.size();
	
	if (nrImgs <= 0) {
		QMessageBox::warning(
			this, 
			tr("Warning"), 
			tr("Open some input image files first (File -> Open) for having some input to compute the animation."));
		return false;
	}
	
	QSize size0 = imgs[0]->size();
	
	/* get strip width in pixels */
	
	bool ok;
	stripWidth = QInputDialog::getInt(
		this, 
		tr("Enter strip width in pixels"),
		tr("Strip width in pixels:"),
		stripWidth,
		1,
		size0.width(),
		1,
		&ok);
	
	if (!ok) return false;		
	
	/* reset results */
	baseImage = QImage();
	barMask = QImage();
	
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
	
	/* go from left to right through baseImage. Write k columns of each
	 * image again and again.
	 */
	for ( int col=0 ; col<size0.width() ; )
		for ( int i=0 ; i<nrImgs ; i++ )
			for ( int j=0 ; j<stripWidth && col<size0.width() ; j++, col++ ) {
				pbar->setValue(col+1);
				for ( int row=0 ; row<size0.height() ; row++ )
					baseImage.setPixel(col, row, imgs[i]->pixel(col, row));
			}
	
	/*
	 * then, compute barmask
	 */
	
	barMask = QImage(size0, QImage::Format_Mono);
	for ( int col=0 ; col < size0.width() ; )
		for ( int i=0 ; i<nrImgs ; i++ )
			for ( int j=0 ; j<stripWidth && col<size0.width() ; j++, col++ ) {
				pbar->setValue(size0.width()+col+1);
				for ( int row=0 ; row < size0.height() ; row++ )
					barMask.setPixel(col, row, (i == 0) ? 1 : 0 );
			}
			
	/* image is the one displayed on imageLabel */
	
	image = QImage(size0, QImage::Format_ARGB32_Premultiplied);
	
	/* remove progress bar again */
	statusBar()->removeWidget(pbar);
	delete pbar;
	
	/* reset zoomFactor to one before the slider signal is triggered */
	zoomFactor = 1.;
	
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
		currentPixmap = QPixmap::fromImage(baseImage);
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
			painter.fillRect(0,0,stripWidth*(idx-1),image.size().height(),Qt::SolidPattern);
		}
		
		/* set barMask with some offset to the right */
		painter.setCompositionMode(QPainter::CompositionMode_Source);
		painter.drawImage(stripWidth*(idx-1), 0, barMask);
		
		/* finally, multiply baseImage */
		painter.setCompositionMode(QPainter::CompositionMode_Multiply);
		painter.drawImage(0, 0, baseImage);

		painter.end();
		
		/* currentPixmap is 1:1 version from which any zoom is computed */
		currentPixmap = QPixmap::fromImage(image);
	}
	
	renderCurrentPixmap();
}

//----------------------------------------------------------------------

void MainWindow::renderCurrentPixmap()
{
	if (zoomFactor != 1.)
		/* no smoothing, as we want to see the pixels */
		imageLabel->setPixmap(
			currentPixmap.scaled(
				zoomFactor*currentPixmap.width(), 
				zoomFactor*currentPixmap.height(), 
				Qt::IgnoreAspectRatio, 
				Qt::FastTransformation)
		);	
	else imageLabel->setPixmap(currentPixmap);
	imageLabel->resize(imageLabel->pixmap()->size());
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
    /* we need an animation to save anything */
    if (!animationIsComputed()) return false;
	
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

/*! \brief Check if animation has already been computed
 *
 * Before we are able to save anything, the animation must have been computed.
 * This is checked by this method. If no animation has been computed, an error
 * message is displayed.
 *
 * \return True, if the animation has been computed and false otherwise.
 */
bool MainWindow::animationIsComputed()
{
    if (baseImage.isNull() || barMask.isNull()) {
        QMessageBox::warning(
            this,
            tr("Warning"),
            tr("Please compute the animation first (Edit -> Compute Animation). Then we are able to save any results."));
        return false;
    }

    return true;
}
//----------------------------------------------------------------------

/*! \brief Save computed animation to SVG file
 *
 * This method takes an already computed animation and saves it to an animated
 * SVG file. This is done in such a way, that the animation may be rendered
 * by viewing the SVG file in any modern browser. Moreover, the animation
 * shall be loadable to animbar for further edit in future versions.
 *
 * See:
 *  http://www.w3.org/TR/SVG/animate.html#CalcModeAttribute
 */
void MainWindow::saveAnimation()
{
    /* we need an animation to save anything */
    if (!animationIsComputed()) return;

    unsigned int nrFrames, stripWidth;
    getParameters(barMask, nrFrames, stripWidth);

    if (nrFrames != m_animationImages.size()) {
        QMessageBox::warning(
            this,
            tr("Warning"),
            tr("We are not able to store the animation."));
        return;
    }

    QFile xmlFile("/tmp/test.svg");
    if (!xmlFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(
            this,
            tr("Warning"),
            tr("Failed to open ") + xmlFile.fileName()  + tr(" for writing."));
        return;
    }

    QXmlStreamWriter xmlOutput(&xmlFile);
    xmlOutput.setAutoFormatting(true);

    xmlOutput.writeStartDocument("1.0", false);
    xmlOutput.writeDTD("<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">");

    xmlOutput.writeStartElement("svg");
    xmlOutput.writeAttribute("xmlns", "http://www.w3.org/2000/svg");
    xmlOutput.writeAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");
    xmlOutput.writeAttribute("version", "1.1");
    xmlOutput.writeAttribute("width", QString::number(barMask.width()));
    xmlOutput.writeAttribute("height", QString::number(barMask.height()));

    for ( unsigned int i=0 ; i<nrFrames ; i++ ) {
        xmlWriteImage(xmlOutput, *m_animationImages[i], i*(m_animationImages[i]->width() - stripWidth));
        xmlWriteAnimation(xmlOutput, *m_animationImages[i], nrFrames);
        xmlOutput.writeEndElement();
        xmlOutput.writeEndElement();
    }

    xmlWriteImage(xmlOutput, barMask, 0);
    xmlOutput.writeEndElement();

    xmlOutput.writeEndElement();        // svg
    xmlOutput.writeEndDocument();

    xmlFile.close();
}

//----------------------------------------------------------------------

/*! \brief
 *
 * We do not end the element, so the caller must call
 *      xmlOutput.writeEndElement();
 * sooner or later.
 *
 * \param xmlOutput
 * \param image
 * \param x0
 *
 * \return
 */
bool MainWindow::xmlWriteImage(
        QXmlStreamWriter& xmlOutput,
        const QImage& image,
        unsigned int x0) const
{
    xmlOutput.writeStartElement("image");
    xmlOutput.writeAttribute("id", "barMask");
    xmlOutput.writeAttribute("width", QString::number(image.width()));
    xmlOutput.writeAttribute("height", QString::number(image.height()));
    xmlOutput.writeAttribute("x", QString::number(x0));
    xmlOutput.writeAttribute("y", QString::number(0));

    /* This is copied from the QT docs */
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::ReadWrite);
    image.save(&buffer, "PNG");
    xmlOutput.writeAttribute("xlink:href", QString("data:image/png;base64,") + QString(buffer.buffer().toBase64().data()));
    buffer.close();

    return true;
}

//----------------------------------------------------------------------

/*! \brief
 *
 * We do not end the element, so the caller must call
 *      xmlOutput.writeEndElement();
 * sooner or later.
 *
 * \param xmlOutput
 * \param image
 * \param nrFrames
 *
 * \return
 */
bool MainWindow::xmlWriteAnimation(
        QXmlStreamWriter& xmlOutput,
        const QImage& image,
        unsigned int nrFrames) const
{
    xmlOutput.writeStartElement("animateMotion");
    xmlOutput.writeAttribute("dur", QString("%1s").arg(nrFrames));
    xmlOutput.writeAttribute("calcMode", "discrete");
    xmlOutput.writeAttribute("repeatCount", "indefinite");
    QString values, keyTimes;
    for (unsigned int i=0; i<nrFrames; i++) {
        values += QString("%1,0;").arg((-1) * ((int) i) * image.width());
        keyTimes += QString("%1;").arg(((float) i) / (nrFrames));
    }
    xmlOutput.writeAttribute("values", values);
    xmlOutput.writeAttribute("keyTimes", keyTimes);

    return true;
}

//----------------------------------------------------------------------

/*! \brief Get parameters from bar mask image
 *
 * This method reconstruct number of frames and pixel width per frame from
 * a bar mask image.
 *
 * \param img Bar mask image
 * \param nrFrames (out) Number of frame images
 * \param stripWidth (out) Strip width in pixels
 *
 * \return True if reconstruction of parameters was successful and false
 *  otherwise. The latter case indicates that the provided image is not a valid
 *  bar mask image.
 */
bool MainWindow::getParameters(const QImage& img, unsigned int& nrFrames, unsigned int& stripWidth)
{
    nrFrames = 0;
    stripWidth = 0;

    /* check format */
    if (img.format() != QImage::Format_Mono) {
        QMessageBox::warning(
            this,
            tr("Warning"),
            tr("The bar mask image is of invalid format."));
        return false;
    }

    /* we start with a white strip, followed by the black strips */
    if (img.pixelIndex(0,0) != 1) {
        QMessageBox::warning(
            this,
            tr("Warning"),
            tr("The bar mask image is of unexpected contents."));
        return false;
    }

    /* We examine the first row always. The width of the white strip is the
     * stripwidth
     */
    while ((img.pixelIndex(stripWidth,0) == 1) && (stripWidth < (unsigned int) img.width())) stripWidth++;

    if (stripWidth >= (unsigned int) img.width()) {
        QMessageBox::warning(
            this,
            tr("Warning"),
            tr("The bar mask image is of unexpected size (stripWidth)."));
        return false;
    }

    if (stripWidth == 0) {
        QMessageBox::warning(
            this,
            tr("Warning"),
            tr("The bar mask image is of unsupported contents (stripWidth)."));
        return false;
    }

    /* The number of frame images is the width of the black strip, divided by
     * the strip width, plus one.
     */
    nrFrames = stripWidth;
    while ((img.pixelIndex(nrFrames,0) == 0) && (nrFrames < (unsigned int) img.width())) nrFrames++;

    if (nrFrames >= (unsigned int) img.width()) {
        QMessageBox::warning(
            this,
            tr("Warning"),
            tr("The bar mask image is of unexpected size (nrFrames)."));
        return false;
    }

    /* check */

    if (nrFrames == 0) {
        QMessageBox::warning(
            this,
            tr("Warning"),
            tr("The bar mask image is of unsupported contents (nrFrames)."));
        return false;
    }

    div_t res = div(nrFrames, stripWidth);
    if (res.rem != 0) {
        QMessageBox::warning(
            this,
            tr("Warning"),
            tr("The bar mask image is of invalid contents (nrFrames)."));
        return false;
    }

    nrFrames /= stripWidth;

    return true;
}

//----------------------------------------------------------------------

void MainWindow::zoomIn()
{
	zoomFactor *= 1.25;
	
	renderCurrentPixmap();
}

//----------------------------------------------------------------------

void MainWindow::zoomOut()
{
	zoomFactor *= 0.75;
		
	renderCurrentPixmap();
}

//----------------------------------------------------------------------

void MainWindow::zoomReset()
{
	zoomFactor = 1.;
	
	renderCurrentPixmap();
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
