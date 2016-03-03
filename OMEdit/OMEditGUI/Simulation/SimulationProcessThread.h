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

#ifndef SIMULATIONPROCESSTHREAD_H
#define SIMULATIONPROCESSTHREAD_H

#include "SimulationOutputWidget.h"

class SimulationOutputWidget;

class SimulationProcessThread : public QThread
{
  Q_OBJECT
public:
  SimulationProcessThread(SimulationOutputWidget *pSimulationOutputWidget);
  QProcess* getCompilationProcess() {return mpCompilationProcess;}
  bool isCompilationProcessRunning() {return mIsCompilationProcessRunning;}
  QProcess* getSimulationProcess() {return mpSimulationProcess;}
  bool isSimulationProcessRunning() {return mIsSimulationProcessRunning;}
protected:
  virtual void run();
private:
  SimulationOutputWidget *mpSimulationOutputWidget;
  QProcess *mpCompilationProcess;
  bool mIsCompilationProcessRunning;
  QProcess *mpSimulationProcess;
  bool mIsSimulationProcessRunning;

  void compileModel();
  void runSimulationExecutable();
private slots:
  void compilationProcessStarted();
  void readCompilationStandardOutput();
  void readCompilationStandardError();
  void compilationProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
  void simulationProcessStarted();
  void readSimulationStandardOutput();
  void readSimulationStandardError();
  void simulationProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
  void createSimulationProgressSocket();
  void readSimulationProgress();
signals:
  void sendCompilationStarted();
  void sendCompilationOutput(QString, QColor);
  void sendCompilationFinished(int, QProcess::ExitStatus);
  void sendSimulationStarted();
  void sendSimulationOutput(QString, StringHandler::SimulationMessageType type, bool);
  void sendSimulationFinished(int, QProcess::ExitStatus);
  void sendSimulationProgress(int);
};

#endif // SIMULATIONPROCESSTHREAD_H
