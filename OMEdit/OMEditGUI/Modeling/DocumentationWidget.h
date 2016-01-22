/*
 * This file is part of OpenModelica.
 *
 * Copyright (c) 1998-2014, Open Source Modelica Consortium (OSMC),
 * c/o Linköpings universitet, Department of Computer and Information Science,
 * SE-58183 Linköping, Sweden.
 *
 * All rights reserved.
 *
 * THIS PROGRAM IS PROVIDED UNDER THE TERMS OF GPL VERSION 3 LICENSE OR
 * THIS OSMC PUBLIC LICENSE (OSMC-PL) VERSION 1.2.
 * ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS PROGRAM CONSTITUTES
 * RECIPIENT'S ACCEPTANCE OF THE OSMC PUBLIC LICENSE OR THE GPL VERSION 3,
 * ACCORDING TO RECIPIENTS CHOICE.
 *
 * The OpenModelica software and the Open Source Modelica
 * Consortium (OSMC) Public License (OSMC-PL) are obtained
 * from OSMC, either from the above address,
 * from the URLs: http://www.ida.liu.se/projects/OpenModelica or
 * http://www.openmodelica.org, and in the OpenModelica distribution.
 * GNU version 3 is obtained from: http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without
 * even the implied warranty of  MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE, EXCEPT AS EXPRESSLY SET FORTH
 * IN THE BY RECIPIENT SELECTED SUBSIDIARY LICENSE CONDITIONS OF OSMC-PL.
 *
 * See the full OSMC Public License conditions for more details.
 *
 */
/*
 *
 * @author Adeel Asghar <adeel.asghar@liu.se>
 *
 * RCS: $Id$
 *
 */

#ifndef DOCUMENTATIONWIDGET_H
#define DOCUMENTATIONWIDGET_H

#include "MainWindow.h"

class DocumentationHistory
{
public:
  QString mUrl;
  DocumentationHistory(QString url) {mUrl = url;}
};

class ModelWidget;
class DocumentationViewer;
class DocumentationWidget : public QWidget
{
  Q_OBJECT
public:
  DocumentationWidget(MainWindow *pParent);
  ~DocumentationWidget();
  MainWindow* getMainWindow();
  QToolButton* getPreviousToolButton();
  QToolButton* getNextToolButton();
  DocumentationViewer* getDocumentationViewer();
  void showDocumentation(QString className);
private:
  MainWindow *mpMainWindow;
  QFile mDocumentationFile;
  QToolButton *mpPreviousToolButton;
  QToolButton *mpNextToolButton;
  DocumentationViewer *mpDocumentationViewer;
  QList<DocumentationHistory> *mpDocumentationHistoryList;
  int mDocumentationHistoryPos;
public slots:
  void previousDocumentation();
  void nextDocumentation();
};

class DocumentationViewer : public QWebView
{
  Q_OBJECT
private:
  DocumentationWidget *mpDocumentationWidget;
  qreal zoomFact;
public:
  DocumentationViewer(DocumentationWidget *pParent);
public slots:
  void processLinkClick(QUrl url);
  void requestFinished();
  void processLinkHover(QString link, QString title, QString textContent);
  void showContextMenu(QPoint point);
protected:
  virtual QWebView* createWindow(QWebPage::WebWindowType type);
  virtual void keyPressEvent(QKeyEvent *event);
  virtual void wheelEvent(QWheelEvent *event);
  virtual void mouseDoubleClickEvent(QMouseEvent *event);
};

#endif // DOCUMENTATIONWIDGET_H
