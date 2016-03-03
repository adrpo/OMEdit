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

#include "TLMCoSimulationThread.h"

TLMCoSimulationThread::TLMCoSimulationThread(TLMCoSimulationOutputWidget *pTLMCoSimulationOutputWidget)
  : QThread(pTLMCoSimulationOutputWidget), mpTLMCoSimulationOutputWidget(pTLMCoSimulationOutputWidget)
{
  mpManagerProcess = 0;
  setIsManagerProcessRunning(false);
  mpMonitorProcess = 0;
  setIsMonitorProcessRunning(false);
  mpProgressFileTimer = 0;
}

void TLMCoSimulationThread::run()
{
  runManager();
  exec();
}

void TLMCoSimulationThread::runManager()
{
  mpManagerProcess = new QProcess;
  TLMCoSimulationOptions tlmCoSimulationOptions = mpTLMCoSimulationOutputWidget->getTLMCoSimulationOptions();
  QFileInfo fileInfo(tlmCoSimulationOptions.getFileName());
  mpManagerProcess->setWorkingDirectory(fileInfo.absoluteDir().absolutePath());
  qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");
  qRegisterMetaType<StringHandler::SimulationMessageType>("StringHandler::SimulationMessageType");
  connect(mpManagerProcess, SIGNAL(started()), SLOT(managerProcessStarted()));
  connect(mpManagerProcess, SIGNAL(readyReadStandardOutput()), SLOT(readManagerStandardOutput()));
  connect(mpManagerProcess, SIGNAL(readyReadStandardError()), SLOT(readManagerStandardError()));
  connect(mpManagerProcess, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(managerProcessFinished(int,QProcess::ExitStatus)));
  QStringList args;
  args << tlmCoSimulationOptions.getManagerArgs() << fileInfo.absoluteFilePath();
  QString fileName = tlmCoSimulationOptions.getManagerProcess();
  QProcessEnvironment environment;
#ifdef WIN32
  environment = StringHandler::simulationProcessEnvironment();
#else
  environment = QProcessEnvironment::systemEnvironment();
#endif
  environment.insert("PATH", tlmCoSimulationOptions.getTLMPluginPath() + ";" + environment.value("PATH"));
  environment.insert("TLMPluginPath", tlmCoSimulationOptions.getTLMPluginPath());
  mpManagerProcess->setProcessEnvironment(environment);
  // start the executable
  mpManagerProcess->start(fileName, args);
  emit sendManagerOutput(QString("%1 %2").arg(fileName).arg(args.join(" ")), StringHandler::OMEditInfo);
}

void TLMCoSimulationThread::runMonitor()
{
  mpMonitorProcess = new QProcess;
  TLMCoSimulationOptions tlmCoSimulationOptions = mpTLMCoSimulationOutputWidget->getTLMCoSimulationOptions();
  QFileInfo fileInfo(tlmCoSimulationOptions.getFileName());
  mpMonitorProcess->setWorkingDirectory(fileInfo.absoluteDir().absolutePath());
  connect(mpMonitorProcess, SIGNAL(started()), SLOT(monitorProcessStarted()));
  connect(mpMonitorProcess, SIGNAL(readyReadStandardOutput()), SLOT(readMonitorStandardOutput()));
  connect(mpMonitorProcess, SIGNAL(readyReadStandardError()), SLOT(readMonitorStandardError()));
  connect(mpMonitorProcess, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(monitorProcessFinished(int,QProcess::ExitStatus)));
  QStringList args;
  args << tlmCoSimulationOptions.getMonitorArgs() << fileInfo.absoluteFilePath();
  // start the executable
  QString fileName = tlmCoSimulationOptions.getMonitorProcess();
  // run the simulation executable to create the result file
  QProcessEnvironment environment;
#ifdef WIN32
  environment = StringHandler::simulationProcessEnvironment();
#else
  environment = QProcessEnvironment::systemEnvironment();
#endif
  environment.insert("PATH", tlmCoSimulationOptions.getTLMPluginPath() + ";" + environment.value("PATH"));
  environment.insert("TLMPluginPath", tlmCoSimulationOptions.getTLMPluginPath());
  mpMonitorProcess->setProcessEnvironment(environment);
  mpMonitorProcess->start(fileName, args);
  emit sendMonitorOutput(QString("%1 %2").arg(fileName).arg(args.join(" ")), StringHandler::OMEditInfo);
}

/*!
  Slot activated when mpManagerProcess started signal is raised.\n
  Notifies TLMCoSimulationOutputWidget about the start of the manager by emitting the sendManagerStarted SIGNAL.
  */
void TLMCoSimulationThread::managerProcessStarted()
{
  setIsManagerProcessRunning(true);
  emit sendManagerStarted();
  runMonitor();
}

/*!
  Slot activated when mpManagerProcess readyReadStandardOutput signal is raised.\n
  Notifies TLMCoSimulationOutputWidget about the standard output of the manager process by emitting the sendManagerOutput SIGNAL.
  */
void TLMCoSimulationThread::readManagerStandardOutput()
{
  emit sendManagerOutput(QString(mpManagerProcess->readAllStandardOutput()), StringHandler::Unknown);
}

/*!
  Slot activated when mpManagerProcess readyReadStandardError signal is raised.\n
  Notifies TLMCoSimulationOutputWidget about the standard error of the manager process by emitting the sendManagerOutput SIGNAL.
  */
void TLMCoSimulationThread::readManagerStandardError()
{
  emit sendManagerOutput(QString(mpManagerProcess->readAllStandardError()), StringHandler::Error);
}

/*!
  Slot activated when mpManagerProcess finished signal is raised.\n
  Notifies TLMCoSimulationOutputWidget about the exit status by emitting the sendManagerFinished SIGNAL.
  */
void TLMCoSimulationThread::managerProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  setIsManagerProcessRunning(false);
  QString exitCodeStr = tr("TLMManager process failed. Exited with code %1.").arg(QString::number(exitCode));
  if (exitStatus == QProcess::NormalExit && exitCode == 0) {
    emit sendManagerOutput(tr("TLMManager process finished successfully."), StringHandler::OMEditInfo);
  } else if (mpManagerProcess->error() == QProcess::UnknownError) {
    emit sendManagerOutput(exitCodeStr, StringHandler::Error);
  } else {
    emit sendManagerOutput(mpManagerProcess->errorString() + "\n" + exitCodeStr, StringHandler::Error);
  }
  emit sendManagerFinished(exitCode, exitStatus);
}

/*!
  Slot activated when mpMonitorProcess started signal is raised.\n
  Notifies TLMCoSimulationOutputWidget about the start of the monitor by emitting the sendMonitorStarted SIGNAL.
  */
void TLMCoSimulationThread::monitorProcessStarted()
{
  setIsMonitorProcessRunning(true);
  emit sendMonitorStarted();
  QFileInfo fileInfo(mpTLMCoSimulationOutputWidget->getTLMCoSimulationOptions().getFileName());
  // give 5 secs to tlmmonitor to create .run file
  mProgressFile.setFileName(fileInfo.absoluteDir().absolutePath() + "/" + fileInfo.completeBaseName() + ".run");
  int ticks = 0;
  while (ticks < 5) {
    if (mProgressFile.exists()) break;
    Sleep::sleep(1);
    ticks++;
  }
  mpProgressFileTimer = new QTimer(this);
  connect(mpProgressFileTimer, SIGNAL(timeout()), SLOT(progressFileChanged()));
  mpProgressFileTimer->start(2000);
}

/*!
  Slot activated when mpMonitorProcess readyReadStandardOutput signal is raised.\n
  Notifies TLMCoSimulationOutputWidget about the standard output of the monitor process by emitting the sendMonitorOutput SIGNAL.
  */
void TLMCoSimulationThread::readMonitorStandardOutput()
{
  emit sendMonitorOutput(QString(mpMonitorProcess->readAllStandardOutput()), StringHandler::Unknown);
}

/*!
  Slot activated when mpMonitorProcess readyReadStandardError signal is raised.\n
  Notifies TLMCoSimulationOutputWidget about the standard error of the monitor process by emitting the sendMonitorOutput SIGNAL.
  */
void TLMCoSimulationThread::readMonitorStandardError()
{
  emit sendMonitorOutput(QString(mpMonitorProcess->readAllStandardError()), StringHandler::Error);
}

/*!
  Slot activated when mpMonitorProcess finished signal is raised.\n
  Notifies TLMCoSimulationOutputWidget about the exit status by emitting the sendMonitorFinished SIGNAL.
  */
void TLMCoSimulationThread::monitorProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  setIsMonitorProcessRunning(false);
  // stop the timer that reads the progres file i.e <model>.run
  if (mpProgressFileTimer) {
    mpProgressFileTimer->stop();
  }
  QString exitCodeStr = tr("TLMMonitor process failed. Exited with code %1.").arg(QString::number(exitCode));
  if (exitStatus == QProcess::NormalExit && exitCode == 0) {
    emit sendMonitorOutput(tr("TLMMonitor process finished successfully."), StringHandler::OMEditInfo);
  } else if (mpMonitorProcess->error() == QProcess::UnknownError) {
    emit sendMonitorOutput(exitCodeStr, StringHandler::Error);
  } else {
    emit sendMonitorOutput(mpMonitorProcess->errorString() + "\n" + exitCodeStr, StringHandler::Error);
  }
  emit sendMonitorFinished(exitCode, exitStatus);
}

void TLMCoSimulationThread::progressFileChanged()
{
  if (mProgressFile.open(QIODevice::ReadOnly)) {
    QTextStream textStream(&mProgressFile);
    while (!textStream.atEnd()) {
      QString currentLine = textStream.readLine();
      if (currentLine.startsWith("Progress")) {
        QString progress = currentLine.mid(QString("Progress  : ").length());
        progress = progress.remove("%").trimmed();
        emit sendManagerProgress(progress.toDouble());
      }
    }
    mProgressFile.close();
  } else {
    MessagesWidget *pMessagesWidget = mpTLMCoSimulationOutputWidget->getMainWindow()->getMessagesWidget();
    pMessagesWidget->addGUIMessage(MessageItem(MessageItem::Modelica, "", false, 0, 0, 0, 0,
                                               GUIMessages::getMessage(GUIMessages::UNABLE_TO_OPEN_FILE).arg(mProgressFile.fileName()),
                                               Helper::scriptingKind, Helper::errorLevel));
  }
}
