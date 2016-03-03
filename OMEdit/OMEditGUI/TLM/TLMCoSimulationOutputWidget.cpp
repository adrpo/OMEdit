/*
 * This file is part of OpenModelica.
 *
 * Copyright (c) 1998-CurrentYear, Open Source Modelica Consortium (OSMC),
 * c/o Linköpings universitet, Department of Computer and Information Science,
 * SE-58183 Linköping, Sweden.
 *
 * All rights reserved.
 *
 * THIS PROGRAM IS PROVIDED UNDER THE TERMS OF GPL VERSION 3 LICENSE OR
 * THIS OSMC PUBLIC LICENSE (OSMC-PL) VERSION 1.2.
 * ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS PROGRAM CONSTITUTES RECIPIENT'S ACCEPTANCE
 * OF THE OSMC PUBLIC LICENSE OR THE GPL VERSION 3, ACCORDING TO RECIPIENTS CHOICE.
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
 *
 */

#include "TLMCoSimulationOutputWidget.h"

/*!
  \class TLMCoSimulationOutputWidget
  \brief Creates a widget that shows the current TLM co-simulation output.
  */

/*!
  \param pParent - pointer to MainWindow.
  */
TLMCoSimulationOutputWidget::TLMCoSimulationOutputWidget(MainWindow *pMainWindow)
  : mpMainWindow(pMainWindow)
{
  // progress label
  mpProgressLabel = new Label;
  mpProgressLabel->setTextFormat(Qt::RichText);
  mpProgressBar = new QProgressBar;
  mpProgressBar->setAlignment(Qt::AlignHCenter);
  // manager output label
  mpManagerOutputLabel = new Label(tr("Manager Output"));
  // stop manager button
  mpStopManagerButton = new QPushButton(tr("Stop Manager"));
  mpStopManagerButton->setEnabled(false);
  connect(mpStopManagerButton, SIGNAL(clicked()), SLOT(stopManager()));
  // open manager log button
  mpOpenManagerLogFileButton = new QPushButton(tr("Open Manager Log File"));
  mpOpenManagerLogFileButton->setEnabled(false);
  connect(mpOpenManagerLogFileButton, SIGNAL(clicked()), SLOT(openManagerLogFile()));
  // manager buttons layout
  QHBoxLayout *pManagerButtonsHorizontalLayout = new QHBoxLayout;
  pManagerButtonsHorizontalLayout->addWidget(mpStopManagerButton);
  pManagerButtonsHorizontalLayout->addWidget(mpOpenManagerLogFileButton);
  // manager Output TextBox
  mpManagerOutputTextBox = new QPlainTextEdit;
  mpManagerOutputTextBox->setFont(QFont(Helper::monospacedFontInfo.family()));
  // monitor output label
  mpMonitorOutputLabel = new Label(tr("Monitor Output"));
  // stop monitor button
  mpStopMonitorButton = new QPushButton(tr("Stop Monitor"));
  mpStopMonitorButton->setEnabled(false);
  connect(mpStopMonitorButton, SIGNAL(clicked()), SLOT(stopMonitor()));
  // open monitor log button
  mpOpenMonitorLogFileButton = new QPushButton(tr("Open Monitor Log File"));
  mpOpenMonitorLogFileButton->setEnabled(false);
  connect(mpOpenMonitorLogFileButton, SIGNAL(clicked()), SLOT(openMonitorLogFile()));
  // monitor buttons layout
  QHBoxLayout *pMonitorButtonsHorizontalLayout = new QHBoxLayout;
  pMonitorButtonsHorizontalLayout->addWidget(mpStopMonitorButton);
  pMonitorButtonsHorizontalLayout->addWidget(mpOpenMonitorLogFileButton);
  // monitor Output TextBox
  mpMonitorOutputTextBox = new QPlainTextEdit;
  mpMonitorOutputTextBox->setFont(QFont(Helper::monospacedFontInfo.family()));
  // layout
  QGridLayout *pMainLayout = new QGridLayout;
  pMainLayout->setContentsMargins(5, 5, 5, 5);
  pMainLayout->addWidget(mpProgressLabel, 0, 0, 1, 2);
  pMainLayout->addWidget(mpProgressBar, 1, 0, 1, 2);
  pMainLayout->addWidget(mpManagerOutputLabel, 2, 0);
  pMainLayout->addLayout(pManagerButtonsHorizontalLayout, 2, 1, Qt::AlignRight);
  pMainLayout->addWidget(mpManagerOutputTextBox, 3, 0, 1, 2);
  pMainLayout->addWidget(mpMonitorOutputLabel, 4, 0);
  pMainLayout->addLayout(pMonitorButtonsHorizontalLayout, 4, 1, Qt::AlignRight);
  pMainLayout->addWidget(mpMonitorOutputTextBox, 5, 0, 1, 2);
  setLayout(pMainLayout);
  // create the thread
  mpTLMCoSimulationProcessThread = new TLMCoSimulationThread(this);
  connect(mpTLMCoSimulationProcessThread, SIGNAL(sendManagerStarted()), SLOT(managerProcessStarted()));
  connect(mpTLMCoSimulationProcessThread, SIGNAL(sendManagerOutput(QString,StringHandler::SimulationMessageType)),
          SLOT(writeManagerOutput(QString,StringHandler::SimulationMessageType)));
  connect(mpTLMCoSimulationProcessThread, SIGNAL(sendManagerFinished(int,QProcess::ExitStatus)),
          SLOT(managerProcessFinished(int,QProcess::ExitStatus)));
  connect(mpTLMCoSimulationProcessThread, SIGNAL(sendMonitorStarted()), SLOT(monitorProcessStarted()));
  connect(mpTLMCoSimulationProcessThread, SIGNAL(sendMonitorOutput(QString,StringHandler::SimulationMessageType)),
          SLOT(writeMonitorOutput(QString,StringHandler::SimulationMessageType)));
  connect(mpTLMCoSimulationProcessThread, SIGNAL(sendMonitorFinished(int,QProcess::ExitStatus)),
          SLOT(monitorProcessFinished(int,QProcess::ExitStatus)));
  connect(mpTLMCoSimulationProcessThread, SIGNAL(sendManagerProgress(int)), mpProgressBar, SLOT(setValue(int)));
}

TLMCoSimulationOutputWidget::~TLMCoSimulationOutputWidget()
{
  clear();
}

void TLMCoSimulationOutputWidget::showTLMCoSimulationOutputWidget(TLMCoSimulationOptions tlmCoSimulationOptions)
{
  mTLMCoSimulationOptions = tlmCoSimulationOptions;
  setWindowTitle(QString("%1 - %2 %3").arg(Helper::applicationName).arg(mTLMCoSimulationOptions.getClassName()).arg(Helper::tlmCoSimulation));
  mpProgressLabel->clear();
  mpOpenManagerLogFileButton->setEnabled(tlmCoSimulationOptions.getManagerArgs().contains("-d"));
  mpManagerOutputTextBox->clear();
  mpOpenMonitorLogFileButton->setEnabled(tlmCoSimulationOptions.getMonitorArgs().contains("-d"));
  mpMonitorOutputTextBox->clear();
  setVisible(true);
  mpTLMCoSimulationProcessThread->start();
}

void TLMCoSimulationOutputWidget::clear()
{
  stopMonitor();
  stopManager();
  mpTLMCoSimulationProcessThread->exit();
  mpTLMCoSimulationProcessThread->wait();
  mpMainWindow->getTLMCoSimulationDialog()->setIsTLMCoSimulationRunning(false);
}

/*!
  Slot activated when mpStopManagerButton clicked signal is raised.\n
  Kills the manager process.
  */
void TLMCoSimulationOutputWidget::stopManager()
{
  if (mpTLMCoSimulationProcessThread->isManagerProcessRunning()) {
    mpTLMCoSimulationProcessThread->getManagerProcess()->kill();
    mpProgressLabel->setText(tr("Co-Simulation using <b>%1</b> meta model is cancelled.").arg(mTLMCoSimulationOptions.getClassName()));
    mpStopManagerButton->setEnabled(false);
  }
}

/*!
  Slot activated when mpOpenManagerLogButton clicked signal is raised.\n
  Opens the manager log file.
  */
void TLMCoSimulationOutputWidget::openManagerLogFile()
{
  QFileInfo fileInfo(mTLMCoSimulationOptions.getFileName());
  QUrl logFilePath (QString("file:///").append(fileInfo.absoluteDir().absolutePath()).append("/TLMlogfile.log"));
  QDesktopServices::openUrl(logFilePath);
}

/*!
  Slot activated when mpStopMonitorButton clicked signal is raised.\n
  Kills the monitor process.
  */
void TLMCoSimulationOutputWidget::stopMonitor()
{
  if (mpTLMCoSimulationProcessThread->isMonitorProcessRunning()) {
    mpTLMCoSimulationProcessThread->getMonitorProcess()->kill();
    mpStopMonitorButton->setEnabled(false);
  }
}

/*!
  Slot activated when mpOpenMonitorLogButton clicked signal is raised.\n
  Opens the monitor log file.
  */
void TLMCoSimulationOutputWidget::openMonitorLogFile()
{
  QFileInfo fileInfo(mTLMCoSimulationOptions.getFileName());
  QUrl logFilePath (QString("file:///").append(fileInfo.absoluteDir().absolutePath()).append("/monitor.log"));
  QDesktopServices::openUrl(logFilePath);
}

/*!
  Slot activated when TLMCoSimulationProcessThread sendManagerStarted signal is raised.\n
  Updates the progress label, bar and stop manager button controls.
  */
void TLMCoSimulationOutputWidget::managerProcessStarted()
{
  mpProgressLabel->setText(tr("Running co-simulation using the <b>%1</b> meta model. Please wait for a while.").arg(mTLMCoSimulationOptions.getClassName()));
  mpProgressBar->setRange(0, 100);
  mpProgressBar->setTextVisible(true);
  mpProgressBar->setValue(0);
  mpStopManagerButton->setEnabled(true);
  // save the last modified datetime of result file.
  QFileInfo fileInfo(mTLMCoSimulationOptions.getFileName());
  QFileInfo resultFileInfo(fileInfo.absoluteDir().absolutePath() + "/" + fileInfo.completeBaseName() + ".csv");
  if (resultFileInfo.exists()) {
    mResultFileLastModifiedDateTime = resultFileInfo.lastModified();
  } else {
    mResultFileLastModifiedDateTime = QDateTime::currentDateTime();
  }
}

/*!
  Slot activated when TLMCoSimulationProcessThread sendManagerOutput signal is raised.\n
  Writes the manager standard output/error to the manager output text box.
  */
void TLMCoSimulationOutputWidget::writeManagerOutput(QString output, StringHandler::SimulationMessageType type)
{
  /* move the cursor down before adding to the logger. */
  QTextCursor textCursor = mpManagerOutputTextBox->textCursor();
  textCursor.movePosition(QTextCursor::End);
  mpManagerOutputTextBox->setTextCursor(textCursor);
  /* set the text color */
  QTextCharFormat charFormat = mpManagerOutputTextBox->currentCharFormat();
  charFormat.setForeground(StringHandler::getSimulationMessageTypeColor(type));
  mpManagerOutputTextBox->setCurrentCharFormat(charFormat);
  /* append the output */
  mpManagerOutputTextBox->insertPlainText(output + "\n");
  /* move the cursor */
  textCursor.movePosition(QTextCursor::End);
  mpManagerOutputTextBox->setTextCursor(textCursor);
}

/*!
  Slot activated when TLMCoSimulationProcessThread sendManagerFinished signal is raised.
  */
void TLMCoSimulationOutputWidget::managerProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  Q_UNUSED(exitCode);
  Q_UNUSED(exitStatus);
  mpProgressLabel->setText(tr("Co-simulation using the <b>%1</b> meta model is finished.").arg(mTLMCoSimulationOptions.getClassName()));
  mpProgressBar->setValue(mpProgressBar->maximum());
  mpStopManagerButton->setEnabled(false);
  mpMainWindow->getTLMCoSimulationDialog()->simulationProcessFinished(mTLMCoSimulationOptions, mResultFileLastModifiedDateTime);
}

/*!
  Slot activated when TLMCoSimulationProcessThread sendMonitorStarted signal is raised.\n
  Updates the stop monitor button control.
  */
void TLMCoSimulationOutputWidget::monitorProcessStarted()
{
  mpStopMonitorButton->setEnabled(true);
}

/*!
  Slot activated when TLMCoSimulationProcessThread sendMonitorOutput signal is raised.\n
  Writes the monitor standard output/error to the monitor output text box.
  */
void TLMCoSimulationOutputWidget::writeMonitorOutput(QString output, StringHandler::SimulationMessageType type)
{
  /* move the cursor down before adding to the logger. */
  QTextCursor textCursor = mpMonitorOutputTextBox->textCursor();
  textCursor.movePosition(QTextCursor::End);
  mpMonitorOutputTextBox->setTextCursor(textCursor);
  /* set the text color */
  QTextCharFormat charFormat = mpMonitorOutputTextBox->currentCharFormat();
  charFormat.setForeground(StringHandler::getSimulationMessageTypeColor(type));
  mpMonitorOutputTextBox->setCurrentCharFormat(charFormat);
  /* append the output */
  mpMonitorOutputTextBox->insertPlainText(output + "\n");
  /* move the cursor */
  textCursor.movePosition(QTextCursor::End);
  mpMonitorOutputTextBox->setTextCursor(textCursor);
}

/*!
  Slot activated when TLMCoSimulationProcessThread sendMonitorFinished signal is raised.
  */
void TLMCoSimulationOutputWidget::monitorProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  Q_UNUSED(exitCode);
  Q_UNUSED(exitStatus);
  mpStopMonitorButton->setEnabled(false);
}
