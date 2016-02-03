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

#include <QTcpSocket>
#include <QTcpServer>
#include <QDebug>
#include <limits>

#include "SimulationDialog.h"
#include "SimulationOutputWidget.h"
#include "VariablesWidget.h"
#include "Commands.h"

/*!
  \class SimulationDialog
  \brief Displays a dialog with simulation options.
  */

/*!
  \param pParent - pointer to MainWindow.
  */
SimulationDialog::SimulationDialog(MainWindow *pParent)
  : QDialog(pParent, Qt::WindowTitleHint)
{
  mpMainWindow = pParent;
  resize(550, 550);
  setUpForm();
}

SimulationDialog::~SimulationDialog()
{
  foreach (SimulationOutputWidget *pSimulationOutputWidget, mSimulationOutputWidgetsList) {
    SimulationProcessThread *pSimulationProcessThread = pSimulationOutputWidget->getSimulationProcessThread();
    /* If the SimulationProcessThread is running then we need to stop it i.e exit its event loop.
       Kill the compilation and simulation processes if they are running before exiting the SimulationProcessThread.
      */
    if (pSimulationProcessThread->isRunning()) {
      if (pSimulationProcessThread->isCompilationProcessRunning() && pSimulationProcessThread->getCompilationProcess()) {
        pSimulationProcessThread->getCompilationProcess()->kill();
      }
      if (pSimulationProcessThread->isSimulationProcessRunning() && pSimulationProcessThread->getSimulationProcess()) {
        pSimulationProcessThread->getSimulationProcess()->kill();
      }
      pSimulationProcessThread->exit();
      pSimulationProcessThread->wait();
      delete pSimulationOutputWidget;
    }
  }
  mSimulationOutputWidgetsList.clear();
}

/*!
  Reimplementation of QDialog::show method.
  \param pLibraryTreeItem - pointer to LibraryTreeItem
  */
void SimulationDialog::show(LibraryTreeItem *pLibraryTreeItem, bool isReSimulate, SimulationOptions simulationOptions)
{
  mpLibraryTreeItem = pLibraryTreeItem;
  initializeFields(isReSimulate, simulationOptions);
  setVisible(true);
}

/*!
 * \brief SimulationDialog::directSimulate
 * Directly simulates the model without showing the simulation dialog.
 * \param pLibraryTreeItem
 * \param launchTransformationalDebugger
 * \param launchAlgorithmicDebugger
 */
void SimulationDialog::directSimulate(LibraryTreeItem *pLibraryTreeItem, bool launchTransformationalDebugger, bool launchAlgorithmicDebugger)
{
  mpLibraryTreeItem = pLibraryTreeItem;
  initializeFields(false, SimulationOptions());
  mpBuildOnlyCheckBox->setChecked(false);
  mpLaunchTransformationalDebuggerCheckBox->setChecked(launchTransformationalDebugger);
  mpLaunchAlgorithmicDebuggerCheckBox->setChecked(launchAlgorithmicDebugger);
  simulate();
}

/*!
  Creates all the controls and set their layout.
  */
void SimulationDialog::setUpForm()
{
  // simulation widget heading
  mpSimulationHeading = new Label;
  mpSimulationHeading->setElideMode(Qt::ElideMiddle);
  mpSimulationHeading->setFont(QFont(Helper::systemFontInfo.family(), Helper::headingFontSize));
  // Horizontal separator
  mpHorizontalLine = new QFrame();
  mpHorizontalLine->setFrameShape(QFrame::HLine);
  mpHorizontalLine->setFrameShadow(QFrame::Sunken);
  // simulation tab widget
  mpSimulationTabWidget = new QTabWidget;
  // General Tab
  mpGeneralTab = new QWidget;
  // Simulation Interval
  mpSimulationIntervalGroupBox = new QGroupBox(tr("Simulation Interval"));
  mpStartTimeLabel = new Label(tr("Start Time:"));
  mpStartTimeTextBox = new QLineEdit("0");
  mpStopTimeLabel = new Label(tr("Stop Time:"));
  mpStopTimeTextBox = new QLineEdit("1");
  // set the layout for simulation interval groupbox
  QGridLayout *pSimulationIntervalGridLayout = new QGridLayout;
  pSimulationIntervalGridLayout->setColumnStretch(1, 1);
  pSimulationIntervalGridLayout->addWidget(mpStartTimeLabel, 0, 0);
  pSimulationIntervalGridLayout->addWidget(mpStartTimeTextBox, 0, 1);
  pSimulationIntervalGridLayout->addWidget(mpStopTimeLabel, 1, 0);
  pSimulationIntervalGridLayout->addWidget(mpStopTimeTextBox, 1, 1);
  mpSimulationIntervalGroupBox->setLayout(pSimulationIntervalGridLayout);
  // Integration
  mpIntegrationGroupBox = new QGroupBox(tr("Integration"));
  mpMethodLabel = new Label(tr("Method:"));
  // get the solver methods
  QStringList solverMethods, solverMethodsDesc;
  mpMainWindow->getOMCProxy()->getSolverMethods(&solverMethods, &solverMethodsDesc);
  mpMethodComboBox = new QComboBox;
  mpMethodComboBox->addItems(solverMethods);
  for (int i = 0 ; i < solverMethodsDesc.size() ; i++) {
    mpMethodComboBox->setItemData(i, solverMethodsDesc.at(i), Qt::ToolTipRole);
  }
  connect(mpMethodComboBox, SIGNAL(currentIndexChanged(int)), SLOT(updateMethodToolTip(int)));
  // make dassl default solver method.
  int currentIndex = mpMethodComboBox->findText("dassl", Qt::MatchExactly);
  if (currentIndex > -1) {
    mpMethodComboBox->setCurrentIndex(currentIndex);
  }
  connect(mpMethodComboBox, SIGNAL(currentIndexChanged(QString)), SLOT(enableDasslOptions(QString)));
  mpMehtodHelpButton = new QToolButton;
  mpMehtodHelpButton->setIcon(QIcon(":/Resources/icons/link-external.svg"));
  mpMehtodHelpButton->setToolTip(tr("Integration help"));
  connect(mpMehtodHelpButton, SIGNAL(clicked()), SLOT(showIntegrationHelp()));
  // Tolerance
  mpToleranceLabel = new Label(tr("Tolerance:"));
  mpToleranceTextBox = new QLineEdit("1e-6");
  // dassl options
  mpDasslOptionsGroupBox = new QGroupBox(tr("DASSL Options"));
  // dassl jacobian
  mpDasslJacobianLabel = new Label(tr("Jacobian:"));
  mpDasslJacobianComboBox = new QComboBox;
  mpDasslJacobianComboBox->addItem(tr("Colored Numerical"), "coloredNumerical");
  mpDasslJacobianComboBox->setItemData(0, "colored numerical jacobian", Qt::ToolTipRole);
  mpDasslJacobianComboBox->addItem(tr("Colored Symbolical"), "coloredSymbolical");
  mpDasslJacobianComboBox->setItemData(1, "colored symbolic jacobian - needs omc compiler flags +generateSymbolicJacobian or +generateSymbolicLinearization", Qt::ToolTipRole);
  mpDasslJacobianComboBox->addItem(tr("Internal Numerical"), "internalNumerical");
  mpDasslJacobianComboBox->setItemData(2, "internal numerical jacobian", Qt::ToolTipRole);
  mpDasslJacobianComboBox->addItem(tr("Symbolical"), "symbolical");
  mpDasslJacobianComboBox->setItemData(3, "symbolic jacobian - needs omc compiler flags +generateSymbolicJacobian or +generateSymbolicLinearization", Qt::ToolTipRole);
  mpDasslJacobianComboBox->addItem(tr("Numerical"), "numerical");
  mpDasslJacobianComboBox->setItemData(4, "numerical jacobian", Qt::ToolTipRole);
  // no root finding
  mpDasslRootFindingCheckBox = new QCheckBox(tr("Root Finding"));
  mpDasslRootFindingCheckBox->setToolTip(tr("Activates the internal root finding procedure of dassl"));
  mpDasslRootFindingCheckBox->setChecked(true);
  // no restart
  mpDasslRestartCheckBox = new QCheckBox(tr("Restart After Event"));
  mpDasslRestartCheckBox->setToolTip(tr("Activates the restart of dassl after an event is performed"));
  mpDasslRestartCheckBox->setChecked(true);
  // initial step size
  mpDasslInitialStepSizeLabel = new Label(tr("Initial Step Size:"));
  mpDasslInitialStepSizeTextBox = new QLineEdit;
  // max step size
  mpDasslMaxStepSizeLabel = new Label(tr("Maximum Step Size:"));
  mpDasslMaxStepSizeTextBox = new QLineEdit;
  // max integration order
  mpDasslMaxIntegrationOrderLabel = new Label(tr("Maximum Integration Order:"));
  mpDasslMaxIntegrationOrderSpinBox = new QSpinBox;
  mpDasslMaxIntegrationOrderSpinBox->setValue(5);
  // set the layout for DASSL options groupbox
  QGridLayout *pDasslOptionsGridLayout = new QGridLayout;
  pDasslOptionsGridLayout->setColumnStretch(1, 1);
  pDasslOptionsGridLayout->addWidget(mpDasslJacobianLabel, 0, 0);
  pDasslOptionsGridLayout->addWidget(mpDasslJacobianComboBox, 0, 1);
  pDasslOptionsGridLayout->addWidget(mpDasslRootFindingCheckBox, 1, 0, 1, 2);
  pDasslOptionsGridLayout->addWidget(mpDasslRestartCheckBox, 2, 0, 1, 2);
  pDasslOptionsGridLayout->addWidget(mpDasslInitialStepSizeLabel, 3, 0);
  pDasslOptionsGridLayout->addWidget(mpDasslInitialStepSizeTextBox, 3, 1);
  pDasslOptionsGridLayout->addWidget(mpDasslMaxStepSizeLabel, 4, 0);
  pDasslOptionsGridLayout->addWidget(mpDasslMaxStepSizeTextBox, 4, 1);
  pDasslOptionsGridLayout->addWidget(mpDasslMaxIntegrationOrderLabel, 5, 0);
  pDasslOptionsGridLayout->addWidget(mpDasslMaxIntegrationOrderSpinBox, 5, 1);
  mpDasslOptionsGroupBox->setLayout(pDasslOptionsGridLayout);
  // set the layout for integration groupbox
  QGridLayout *pIntegrationGridLayout = new QGridLayout;
  pIntegrationGridLayout->setColumnStretch(1, 1);
  pIntegrationGridLayout->addWidget(mpMethodLabel, 0, 0);
  pIntegrationGridLayout->addWidget(mpMethodComboBox, 0, 1);
  pIntegrationGridLayout->addWidget(mpMehtodHelpButton, 0, 2);
  pIntegrationGridLayout->addWidget(mpToleranceLabel, 1, 0);
  pIntegrationGridLayout->addWidget(mpToleranceTextBox, 1, 1, 1, 2);
  pIntegrationGridLayout->addWidget(mpDasslOptionsGroupBox, 2, 0, 1, 3);
  mpIntegrationGroupBox->setLayout(pIntegrationGridLayout);
  // Compiler Flags
  mpCflagsLabel = new Label(tr("Compiler Flags (Optional):"));
  mpCflagsTextBox = new QLineEdit;
  // Number of Processors
  mpNumberOfProcessorsLabel = new Label(tr("Number of Processors:"));
  mpNumberOfProcessorsSpinBox = new QSpinBox;
  mpNumberOfProcessorsSpinBox->setMinimum(1);
  mpNumberOfProcessorsSpinBox->setValue(mpMainWindow->getOMCProxy()->numProcessors().toInt());
  mpNumberOfProcessorsNoteLabel = new Label(tr("Use 1 processor if you encounter problems during compilation."));
  // build only
  mpBuildOnlyCheckBox = new QCheckBox(tr("Build Only"));
  connect(mpBuildOnlyCheckBox, SIGNAL(toggled(bool)), SLOT(buildOnly(bool)));
  // Launch Transformational Debugger checkbox
  mpLaunchTransformationalDebuggerCheckBox = new QCheckBox(tr("Launch Transformational Debugger"));
  // Launch Algorithmic Debugger checkbox
  mpLaunchAlgorithmicDebuggerCheckBox = new QCheckBox(tr("Launch Algorithmic Debugger"));
  // set General Tab Layout
  QGridLayout *pGeneralTabLayout = new QGridLayout;
  pGeneralTabLayout->setAlignment(Qt::AlignTop);
  pGeneralTabLayout->addWidget(mpSimulationIntervalGroupBox, 0, 0, 1, 3);
  pGeneralTabLayout->addWidget(mpIntegrationGroupBox, 1, 0, 1, 3);
  pGeneralTabLayout->addWidget(mpCflagsLabel, 2, 0);
  pGeneralTabLayout->addWidget(mpCflagsTextBox, 2, 1, 1, 2);
  pGeneralTabLayout->addWidget(mpNumberOfProcessorsLabel, 3, 0);
  pGeneralTabLayout->addWidget(mpNumberOfProcessorsSpinBox, 3, 1);
  pGeneralTabLayout->addWidget(mpNumberOfProcessorsNoteLabel, 3, 2);
  pGeneralTabLayout->addWidget(mpBuildOnlyCheckBox, 4, 0, 1, 3);
  pGeneralTabLayout->addWidget(mpLaunchTransformationalDebuggerCheckBox, 5, 0, 1, 3);
  pGeneralTabLayout->addWidget(mpLaunchAlgorithmicDebuggerCheckBox, 6, 0, 1, 3);
  mpGeneralTab->setLayout(pGeneralTabLayout);
  // add General Tab to Simulation TabWidget
  mpSimulationTabWidget->addTab(mpGeneralTab, Helper::general);
  // Output Tab
  mpOutputTab = new QWidget;
  // Output Interval
  mpNumberofIntervalLabel = new Label(tr("Number of Intervals:"));
  mpNumberofIntervalsSpinBox = new QSpinBox;
  mpNumberofIntervalsSpinBox->setRange(0, std::numeric_limits<int>::max());
  mpNumberofIntervalsSpinBox->setSingleStep(100);
  mpNumberofIntervalsSpinBox->setValue(500);
  // Output Format
  mpOutputFormatLabel = new Label(tr("Output Format:"));
  mpOutputFormatComboBox = new QComboBox;
  mpOutputFormatComboBox->addItems(Helper::ModelicaSimulationOutputFormats.toLower().split(","));
  // Output filename
  mpFileNameLabel = new Label(tr("File Name Prefix (Optional):"));
  mpFileNameTextBox = new QLineEdit;
  mpFileNameTextBox->setToolTip(tr("The name is used as a prefix for the output files. This is just a name not the path.\n"
                                   "If you want to change the output path then update the working directory in Options/Preferences."));
  mpResultFileNameLabel = new Label(tr("Result File (Optional):"));
  mpResultFileNameTextBox = new QLineEdit;
  mpResultFileName = new Label;
  connect(mpResultFileNameTextBox, SIGNAL(textEdited(QString)), SLOT(resultFileNameChanged(QString)));
  connect(mpOutputFormatComboBox, SIGNAL(currentIndexChanged(QString)), SLOT(resultFileNameChanged(QString)));
  // Variable filter
  mpVariableFilterLabel = new Label(tr("Variable Filter (Optional):"));
  mpVariableFilterTextBox = new QLineEdit(".*");
  // Protected Variabels
  mpProtectedVariablesCheckBox = new QCheckBox(tr("Protected Variables"));
  // Equidistant time grid
  mpEquidistantTimeGridCheckBox = new QCheckBox(tr("Equidistant Time Grid"));
  mpEquidistantTimeGridCheckBox->setChecked(true);
  // store variables at events
  mpStoreVariablesAtEventsCheckBox = new QCheckBox(tr("Store Variables at Events"));
  mpStoreVariablesAtEventsCheckBox->setChecked(true);
  // show generated files checkbox
  mpShowGeneratedFilesCheckBox = new QCheckBox(tr("Show Generated Files"));
  // set Output Tab Layout
  QGridLayout *pOutputTabLayout = new QGridLayout;
  pOutputTabLayout->setAlignment(Qt::AlignTop);
  pOutputTabLayout->addWidget(mpNumberofIntervalLabel, 0, 0);
  pOutputTabLayout->addWidget(mpNumberofIntervalsSpinBox, 0, 1, 1, 2);
  pOutputTabLayout->addWidget(mpOutputFormatLabel, 1, 0);
  pOutputTabLayout->addWidget(mpOutputFormatComboBox, 1, 1, 1, 2);
  pOutputTabLayout->addWidget(mpFileNameLabel, 2, 0);
  pOutputTabLayout->addWidget(mpFileNameTextBox, 2, 1, 1, 2);
  pOutputTabLayout->addWidget(mpResultFileNameLabel, 3, 0);
  pOutputTabLayout->addWidget(mpResultFileNameTextBox, 3, 1);
  pOutputTabLayout->addWidget(mpResultFileName, 3, 2);
  pOutputTabLayout->addWidget(mpVariableFilterLabel, 4, 0);
  pOutputTabLayout->addWidget(mpVariableFilterTextBox, 4, 1, 1, 2);
  pOutputTabLayout->addWidget(mpProtectedVariablesCheckBox, 5, 0, 1, 3);
  pOutputTabLayout->addWidget(mpEquidistantTimeGridCheckBox, 6, 0, 1, 3);
  pOutputTabLayout->addWidget(mpStoreVariablesAtEventsCheckBox, 7, 0, 1, 3);
  pOutputTabLayout->addWidget(mpShowGeneratedFilesCheckBox, 8, 0, 1, 3);
  mpOutputTab->setLayout(pOutputTabLayout);
  // add Output Tab to Simulation TabWidget
  mpSimulationTabWidget->addTab(mpOutputTab, Helper::output);
  // Simulation Flags Tab
  mpSimulationFlagsTab = new QWidget;
  // Simulation Flags Tab scroll area
  mpSimulationFlagsTabScrollArea = new QScrollArea;
  mpSimulationFlagsTabScrollArea->setFrameShape(QFrame::NoFrame);
  mpSimulationFlagsTabScrollArea->setBackgroundRole(QPalette::Base);
  mpSimulationFlagsTabScrollArea->setWidgetResizable(true);
  mpSimulationFlagsTabScrollArea->setWidget(mpSimulationFlagsTab);
  // Model Setup File
  mpModelSetupFileLabel = new Label(tr("Model Setup File (Optional):"));
  mpModelSetupFileLabel->setToolTip(tr("Specifies a new setup XML file to the generated simulation code."));
  mpModelSetupFileTextBox = new QLineEdit;
  mpModelSetupFileBrowseButton = new QPushButton(Helper::browse);
  connect(mpModelSetupFileBrowseButton, SIGNAL(clicked()), SLOT(browseModelSetupFile()));
  mpModelSetupFileBrowseButton->setAutoDefault(false);
  // Initialization Methods
  mpInitializationMethodLabel = new Label(tr("Initialization Method (Optional):"));
  mpInitializationMethodLabel->setToolTip(tr("Specifies the initialization method."));
  // get the initialization methods
  QStringList initializationMethods, initializationMethodsDesc;
  mpMainWindow->getOMCProxy()->getInitializationMethods(&initializationMethods, &initializationMethodsDesc);
  initializationMethods.prepend("");
  initializationMethodsDesc.prepend("");
  mpInitializationMethodComboBox = new QComboBox;
  mpInitializationMethodComboBox->addItems(initializationMethods);
  for (int i = 0 ; i < initializationMethodsDesc.size() ; i++) {
    mpInitializationMethodComboBox->setItemData(i, initializationMethodsDesc.at(i), Qt::ToolTipRole);
  }
  // Equation System Initialization File
  mpEquationSystemInitializationFileLabel = new Label(tr("Equation System Initialization File (Optional):"));
  mpEquationSystemInitializationFileLabel->setToolTip(tr("Specifies an external file for the initialization of the model."));
  mpEquationSystemInitializationFileTextBox = new QLineEdit;
  mpEquationSystemInitializationFileBrowseButton = new QPushButton(Helper::browse);
  connect(mpEquationSystemInitializationFileBrowseButton, SIGNAL(clicked()), SLOT(browseEquationSystemInitializationFile()));
  mpEquationSystemInitializationFileBrowseButton->setAutoDefault(false);
  // Equation System time
  mpEquationSystemInitializationTimeLabel = new Label(tr("Equation System Initialization Time (Optional):"));
  mpEquationSystemInitializationTimeLabel->setToolTip(tr("Specifies a time for the initialization of the model."));
  mpEquationSystemInitializationTimeTextBox = new QLineEdit;
  // clock
  mpClockLabel = new Label(tr("Clock (Optional):"));
  mpClockComboBox = new QComboBox;
  mpClockComboBox->addItems(Helper::clockOptions.split(","));
  // Linear Solvers
  mpLinearSolverLabel = new Label(tr("Linear Solver (Optional):"));
  // get the linear solvers
  QStringList linearSolverMethods, linearSolverMethodsDesc;
  mpMainWindow->getOMCProxy()->getLinearSolvers(&linearSolverMethods, &linearSolverMethodsDesc);
  linearSolverMethods.prepend("");
  linearSolverMethodsDesc.prepend("");
  mpLinearSolverComboBox = new QComboBox;
  mpLinearSolverComboBox->addItems(linearSolverMethods);
  for (int i = 0 ; i < linearSolverMethodsDesc.size() ; i++) {
    mpLinearSolverComboBox->setItemData(i, linearSolverMethodsDesc.at(i), Qt::ToolTipRole);
  }
  // Non Linear Solvers
  mpNonLinearSolverLabel = new Label(tr("Non Linear Solver (Optional):"));
  // get the non-linear solvers
  QStringList nonLinearSolverMethods, nonLinearSolverMethodsDesc;
  mpMainWindow->getOMCProxy()->getNonLinearSolvers(&nonLinearSolverMethods, &nonLinearSolverMethodsDesc);
  nonLinearSolverMethods.prepend("");
  nonLinearSolverMethodsDesc.prepend("");
  mpNonLinearSolverComboBox = new QComboBox;
  mpNonLinearSolverComboBox->addItems(nonLinearSolverMethods);
  for (int i = 0 ; i < nonLinearSolverMethodsDesc.size() ; i++) {
    mpNonLinearSolverComboBox->setItemData(i, nonLinearSolverMethodsDesc.at(i), Qt::ToolTipRole);
  }
  // time where the linearization of the model should be performed
  mpLinearizationTimeLabel = new Label(tr("Linearization Time (Optional):"));
  mpLinearizationTimeTextBox = new QLineEdit;
  // output variables
  mpOutputVariablesLabel = new Label(tr("Output Variables (Optional):"));
  mpOutputVariablesLabel->setToolTip(tr("Comma separated list of variables. Output the variables at the end of the simulation to the standard output."));
  mpOutputVariablesTextBox = new QLineEdit;
  // measure simulation time checkbox
  mpProfilingLabel = new Label(tr("Profiling (enable performance measurements)"));
  mpProfilingComboBox = new QComboBox;
  OMCInterface::getConfigFlagValidOptions_res profiling = mpMainWindow->getOMCProxy()->getConfigFlagValidOptions("profiling");
  mpProfilingComboBox->addItems(profiling.validOptions);
  mpProfilingComboBox->setCurrentIndex(0);
  mpProfilingComboBox->setToolTip(profiling.mainDescription);
  int i = 0;
  foreach (QString description, profiling.descriptions) {
    mpProfilingComboBox->setItemData(i, description, Qt::ToolTipRole);
    i++;
  }
  // cpu-time checkbox
  mpCPUTimeCheckBox = new QCheckBox(tr("CPU Time"));
  // enable all warnings
  mpEnableAllWarningsCheckBox = new QCheckBox(tr("Enable All Warnings"));
  mpEnableAllWarningsCheckBox->setChecked(true);
  // Logging
  mpLogDasslSolverCheckBox = new QCheckBox(tr("DASSL Solver Information"));
  mpLogDasslSolverCheckBox->setToolTip(tr("additional information about dassl solver"));
  mpLogDebugCheckBox = new QCheckBox(tr("Debug"));
  mpLogDebugCheckBox->setToolTip(tr("additional debug information"));
  mpLogDynamicStateSelectionCheckBox = new QCheckBox(tr("Dynamic State Selection Information"));
  mpLogDynamicStateSelectionCheckBox->setToolTip(tr("outputs information about dynamic state selection"));
  mpLogJacobianDynamicStateSelectionCheckBox = new QCheckBox(tr("Jacobians Dynamic State Selection Information"));
  mpLogJacobianDynamicStateSelectionCheckBox->setToolTip(tr("outputs jacobain of the dynamic state selection"));
  mpLogEventsCheckBox = new QCheckBox(tr("Event Iteration"));
  mpLogEventsCheckBox->setToolTip(tr("additional information during event iteration"));
  mpLogVerboseEventsCheckBox = new QCheckBox(tr("Verbose Event System"));
  mpLogVerboseEventsCheckBox->setToolTip(tr("verbose logging of event system"));
  mpLogInitializationCheckBox = new QCheckBox(tr("Initialization"));
  mpLogInitializationCheckBox->setToolTip(tr("additional information during initialization"));
  mpLogJacobianCheckBox = new QCheckBox(tr("Jacobian Matrix"));
  mpLogJacobianCheckBox->setToolTip(tr("outputs the jacobian matrix used by the integrator"));
  mpLogNonLinearSystemsCheckBox = new QCheckBox(tr("Non Linear Systems"));
  mpLogNonLinearSystemsCheckBox->setToolTip(tr("logging for nonlinear systems"));
  mpLogVerboseNonLinearSystemsCheckBox = new QCheckBox(tr("Verbose Non Linear Systems"));
  mpLogVerboseNonLinearSystemsCheckBox->setToolTip(tr("verbose logging of nonlinear systems"));
  mpLogJacobianNonLinearSystemsCheckBox = new QCheckBox(tr("Jacobian Non Linear Systems"));
  mpLogJacobianNonLinearSystemsCheckBox->setToolTip(tr("outputs the jacobian of nonlinear systems"));
  mpLogResidualsInitializationCheckBox = new QCheckBox(tr("Initialization Residuals"));
  mpLogResidualsInitializationCheckBox->setToolTip(tr("outputs residuals of the initialization"));
  mpLogSimulationCheckBox = new QCheckBox(tr("Simulation Process"));
  mpLogSimulationCheckBox->setToolTip(tr("additional information about simulation process"));
  mpLogSolverCheckBox = new QCheckBox(tr("Solver Process"));
  mpLogSolverCheckBox->setToolTip(tr("additional information about solver process"));
  mpLogFinalSolutionOfInitializationCheckBox = new QCheckBox(tr("Final Initialization Solution"));
  mpLogFinalSolutionOfInitializationCheckBox->setToolTip(tr("final solution of the initialization"));
  mpLogStatsCheckBox = new QCheckBox(tr("Timer/Events/Solver Statistics"));
  mpLogStatsCheckBox->setChecked(true);
  mpLogStatsCheckBox->setToolTip(tr("additional statistics about timer/events/solver"));
  mpLogUtilCheckBox = new QCheckBox(tr("Util"));
  mpLogUtilCheckBox->setToolTip(tr("outputs information about util"));
  mpLogZeroCrossingsCheckBox = new QCheckBox(tr("Zero Crossings"));
  mpLogZeroCrossingsCheckBox->setToolTip(tr("additional information about the zerocrossings"));
  // layout for logging group
  QGridLayout *pLoggingGroupLayout = new QGridLayout;
  pLoggingGroupLayout->addWidget(mpLogDasslSolverCheckBox, 0, 0);
  pLoggingGroupLayout->addWidget(mpLogDebugCheckBox, 0, 1);
  pLoggingGroupLayout->addWidget(mpLogDynamicStateSelectionCheckBox, 1, 0);
  pLoggingGroupLayout->addWidget(mpLogJacobianDynamicStateSelectionCheckBox, 1, 1);
  pLoggingGroupLayout->addWidget(mpLogEventsCheckBox, 2, 0);
  pLoggingGroupLayout->addWidget(mpLogVerboseEventsCheckBox, 2, 1);
  pLoggingGroupLayout->addWidget(mpLogInitializationCheckBox, 3, 0);
  pLoggingGroupLayout->addWidget(mpLogJacobianCheckBox, 3, 1);
  pLoggingGroupLayout->addWidget(mpLogNonLinearSystemsCheckBox, 4, 0);
  pLoggingGroupLayout->addWidget(mpLogVerboseNonLinearSystemsCheckBox, 4, 1);
  pLoggingGroupLayout->addWidget(mpLogJacobianNonLinearSystemsCheckBox, 5, 0);
  pLoggingGroupLayout->addWidget(mpLogResidualsInitializationCheckBox, 5, 1);
  pLoggingGroupLayout->addWidget(mpLogSimulationCheckBox, 6, 0);
  pLoggingGroupLayout->addWidget(mpLogSolverCheckBox, 6, 1);
  pLoggingGroupLayout->addWidget(mpLogFinalSolutionOfInitializationCheckBox, 7, 0);
  pLoggingGroupLayout->addWidget(mpLogStatsCheckBox, 7, 1);
  pLoggingGroupLayout->addWidget(mpLogUtilCheckBox, 8, 0);
  pLoggingGroupLayout->addWidget(mpLogZeroCrossingsCheckBox, 8, 1);
  mpLoggingGroupBox = new QGroupBox(tr("Logging (Optional)"));
  mpLoggingGroupBox->setLayout(pLoggingGroupLayout);
  mpAdditionalSimulationFlagsLabel = new Label(tr("Additional Simulation Flags (Optional):"));
  mpAdditionalSimulationFlagsLabel->setToolTip(tr("Space separated list of simulation flags"));
  mpAdditionalSimulationFlagsTextBox = new QLineEdit;
  // set SimulationFlags Tab Layout
  QGridLayout *pSimulationFlagsTabLayout = new QGridLayout;
  pSimulationFlagsTabLayout->setAlignment(Qt::AlignTop);
  pSimulationFlagsTabLayout->addWidget(mpModelSetupFileLabel, 0, 0);
  pSimulationFlagsTabLayout->addWidget(mpModelSetupFileTextBox, 0, 1);
  pSimulationFlagsTabLayout->addWidget(mpModelSetupFileBrowseButton, 0, 2);
  pSimulationFlagsTabLayout->addWidget(mpInitializationMethodLabel, 1, 0);
  pSimulationFlagsTabLayout->addWidget(mpInitializationMethodComboBox, 1, 1, 1, 2);
  pSimulationFlagsTabLayout->addWidget(mpEquationSystemInitializationFileLabel, 2, 0);
  pSimulationFlagsTabLayout->addWidget(mpEquationSystemInitializationFileTextBox, 2, 1);
  pSimulationFlagsTabLayout->addWidget(mpEquationSystemInitializationFileBrowseButton, 2, 2);
  pSimulationFlagsTabLayout->addWidget(mpEquationSystemInitializationTimeLabel, 3, 0);
  pSimulationFlagsTabLayout->addWidget(mpEquationSystemInitializationTimeTextBox, 3, 1, 1, 2);
  pSimulationFlagsTabLayout->addWidget(mpClockLabel, 4, 0);
  pSimulationFlagsTabLayout->addWidget(mpClockComboBox, 4, 1, 1, 2);
  pSimulationFlagsTabLayout->addWidget(mpLinearSolverLabel, 5, 0);
  pSimulationFlagsTabLayout->addWidget(mpLinearSolverComboBox, 5, 1, 1, 2);
  pSimulationFlagsTabLayout->addWidget(mpNonLinearSolverLabel, 6, 0);
  pSimulationFlagsTabLayout->addWidget(mpNonLinearSolverComboBox, 6, 1, 1, 2);
  pSimulationFlagsTabLayout->addWidget(mpLinearizationTimeLabel, 7, 0);
  pSimulationFlagsTabLayout->addWidget(mpLinearizationTimeTextBox, 7, 1, 1, 2);
  pSimulationFlagsTabLayout->addWidget(mpOutputVariablesLabel, 8, 0);
  pSimulationFlagsTabLayout->addWidget(mpOutputVariablesTextBox, 8, 1, 1, 2);
  pSimulationFlagsTabLayout->addWidget(mpProfilingLabel, 9, 0);
  pSimulationFlagsTabLayout->addWidget(mpProfilingComboBox, 9, 1, 1, 2);
  pSimulationFlagsTabLayout->addWidget(mpCPUTimeCheckBox, 10, 0);
  pSimulationFlagsTabLayout->addWidget(mpEnableAllWarningsCheckBox, 11, 0);
  pSimulationFlagsTabLayout->addWidget(mpLoggingGroupBox, 12, 0, 1, 3);
  pSimulationFlagsTabLayout->addWidget(mpAdditionalSimulationFlagsLabel, 13, 0);
  pSimulationFlagsTabLayout->addWidget(mpAdditionalSimulationFlagsTextBox, 13, 1, 1, 2);
  mpSimulationFlagsTab->setLayout(pSimulationFlagsTabLayout);
  // add Output Tab to Simulation TabWidget
  mpSimulationTabWidget->addTab(mpSimulationFlagsTabScrollArea, tr("Simulation Flags"));
  // Archived Simulations tab
  mpArchivedSimulationsTab = new QWidget;
  mpArchivedSimulationsTreeWidget = new QTreeWidget;
  mpArchivedSimulationsTreeWidget->setItemDelegate(new ItemDelegate(mpArchivedSimulationsTreeWidget));
  mpArchivedSimulationsTreeWidget->setTextElideMode(Qt::ElideMiddle);
  mpArchivedSimulationsTreeWidget->setColumnCount(4);
  QStringList headers;
  headers << tr("Class") << tr("DateTime") << tr("Start Time") << tr("Stop Time") << tr("Status");
  mpArchivedSimulationsTreeWidget->setHeaderLabels(headers);
  mpArchivedSimulationsTreeWidget->setIndentation(0);
  connect(mpArchivedSimulationsTreeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), SLOT(showArchivedSimulation(QTreeWidgetItem*)));
  QGridLayout *pArchivedSimulationsTabLayout = new QGridLayout;
  pArchivedSimulationsTabLayout->setAlignment(Qt::AlignTop);
  pArchivedSimulationsTabLayout->addWidget(mpArchivedSimulationsTreeWidget, 0, 0);
  mpArchivedSimulationsTab->setLayout(pArchivedSimulationsTabLayout);
  // add Archived simulations Tab to Simulation TabWidget
  mpSimulationTabWidget->addTab(mpArchivedSimulationsTab, tr("Archived Simulations"));
  // Add the validators
  QDoubleValidator *pDoubleValidator = new QDoubleValidator(this);
  mpStartTimeTextBox->setValidator(pDoubleValidator);
  mpStopTimeTextBox->setValidator(pDoubleValidator);
  mpToleranceTextBox->setValidator(pDoubleValidator);
  // Create the buttons
  mpSimulateButton = new QPushButton(Helper::simulate);
  mpSimulateButton->setAutoDefault(true);
  connect(mpSimulateButton, SIGNAL(clicked()), this, SLOT(simulate()));
  mpCancelButton = new QPushButton(Helper::cancel);
  mpCancelButton->setAutoDefault(false);
  connect(mpCancelButton, SIGNAL(clicked()), this, SLOT(reject()));
  // save simulations options
  mpSaveSimulationCheckbox = new QCheckBox(tr("Save simulation settings inside model"));
  // adds buttons to the button box
  mpButtonBox = new QDialogButtonBox(Qt::Horizontal);
  mpButtonBox->addButton(mpSimulateButton, QDialogButtonBox::ActionRole);
  mpButtonBox->addButton(mpCancelButton, QDialogButtonBox::ActionRole);
  // Create a layout
  QGridLayout *pMainLayout = new QGridLayout;
  pMainLayout->addWidget(mpSimulationHeading, 0, 0, 1, 2);
  pMainLayout->addWidget(mpHorizontalLine, 1, 0, 1, 2);
  pMainLayout->addWidget(mpSimulationTabWidget, 2, 0, 1, 2);
  pMainLayout->addWidget(mpSaveSimulationCheckbox, 3, 0);
  pMainLayout->addWidget(mpButtonBox, 3, 1);
  setLayout(pMainLayout);
}

/*!
  Validates the simulation values entered by the user.
  */
bool SimulationDialog::validate()
{
  if (mpStartTimeTextBox->text().isEmpty())
  {
    mpStartTimeTextBox->setText("0");
  }
  if (mpStopTimeTextBox->text().isEmpty())
  {
    mpStopTimeTextBox->setText("1");
  }
  if (mpStartTimeTextBox->text().toDouble() > mpStopTimeTextBox->text().toDouble())
  {
    QMessageBox::critical(mpMainWindow, QString(Helper::applicationName).append(" - ").append(Helper::error),
                          GUIMessages::getMessage(GUIMessages::SIMULATION_STARTTIME_LESSTHAN_STOPTIME), Helper::ok);
    return false;
  }
  return true;
}

/*!
  Initializes the simulation dialog with the default values.
  */
void SimulationDialog::initializeFields(bool isReSimulate, SimulationOptions simulationOptions)
{
  if (!isReSimulate) {
    mIsReSimulate = false;
    mClassName = mpLibraryTreeItem->getNameStructure();
    mFileName = mpLibraryTreeItem->getFileName();
    setWindowTitle(QString(Helper::applicationName).append(" - ").append(Helper::simulationSetup).append(" - ").append(mClassName));
    mpSimulationHeading->setText(QString(Helper::simulationSetup).append(" - ").append(mClassName));
    // if the class has experiment annotation then read it.
    if (mpMainWindow->getOMCProxy()->isExperiment(mClassName)) {
      // get the simulation options....
      QStringList result = mpMainWindow->getOMCProxy()->getSimulationOptions(mClassName);
      // since we always get simulationOptions so just get the values from array
      mpStartTimeTextBox->setText(QString::number(result.at(0).toFloat()));
      mpStopTimeTextBox->setText(QString::number(result.at(1).toFloat()));
      mpToleranceTextBox->setText(QString::number(result.at(2).toFloat()));
      mpNumberofIntervalsSpinBox->setValue(result.at(3).toLong());
    }
    mpCflagsTextBox->setEnabled(true);
    mpNumberofIntervalsSpinBox->setEnabled(true);
    mpFileNameTextBox->setEnabled(true);
    mpSaveSimulationCheckbox->setEnabled(true);
    mpSimulateButton->setText(Helper::simulate);
  } else {
    mIsReSimulate = true;
    mClassName = simulationOptions.getClassName();
    mFileName = simulationOptions.getFileName();
    setWindowTitle(QString(Helper::applicationName).append(" - ").append(Helper::reSimulation).append(" - ").append(mClassName));
    mpSimulationHeading->setText(QString(Helper::reSimulation).append(" - ").append(mClassName));
    // Simulation Interval
    mpStartTimeTextBox->setText(simulationOptions.getStartTime());
    mpStopTimeTextBox->setText(simulationOptions.getStopTime());
    // Integration
    int currentIndex = mpMethodComboBox->findText(simulationOptions.getMethod(), Qt::MatchExactly);
    if (currentIndex > -1) {
      mpMethodComboBox->setCurrentIndex(currentIndex);
    }
    // Tolerance
    mpToleranceTextBox->setText(simulationOptions.getTolerance());
    // dassl jacobian
    currentIndex = mpDasslJacobianComboBox->findText(simulationOptions.getDasslJacobian(), Qt::MatchExactly);
    if (currentIndex > -1) {
      mpDasslJacobianComboBox->setCurrentIndex(currentIndex);
    }
    // no root finding
    mpDasslRootFindingCheckBox->setChecked(simulationOptions.getDasslRootFinding());
    // no restart
    mpDasslRestartCheckBox->setChecked(simulationOptions.getDasslRestart());
    // initial step size
    mpDasslInitialStepSizeTextBox->setText(simulationOptions.getDasslInitialStepSize());
    // max step size
    mpDasslMaxStepSizeTextBox->setText(simulationOptions.getDasslMaxStepSize());
    // max integration order
    mpDasslMaxIntegrationOrderSpinBox->setValue(simulationOptions.getDasslMaxIntegration());
    // Compiler Flags
    mpCflagsTextBox->setDisabled(true);
    // Number of Processors
    mpNumberOfProcessorsSpinBox->setValue(simulationOptions.getNumberOfProcessors());
    // Launch Transformational Debugger checkbox
    mpLaunchTransformationalDebuggerCheckBox->setChecked(simulationOptions.getLaunchTransformationalDebugger());
    // Launch Algorithmic Debugger checkbox
    mpLaunchAlgorithmicDebuggerCheckBox->setChecked(simulationOptions.getLaunchAlgorithmicDebugger());
    // build only
    mpBuildOnlyCheckBox->setChecked(simulationOptions.getBuildOnly());
    // Output Interval
    mpNumberofIntervalsSpinBox->setValue(simulationOptions.getNumberofIntervals());
    // Output filename
    mpFileNameTextBox->setDisabled(true);
    // Variable filter
    mpVariableFilterTextBox->setText(simulationOptions.getVariableFilter());
    // Protected Variabels
    mpProtectedVariablesCheckBox->setChecked(simulationOptions.getProtectedVariables());
    // Equidistant time grid
    mpEquidistantTimeGridCheckBox->setChecked(simulationOptions.getEquidistantTimeGrid());
    // store variables at events
    mpStoreVariablesAtEventsCheckBox->setChecked(simulationOptions.getStoreVariablesAtEvents());
    // show generated files checkbox
    mpShowGeneratedFilesCheckBox->setChecked(simulationOptions.getShowGeneratedFiles());
    // Model Setup File
    mpModelSetupFileTextBox->setText(simulationOptions.getModelSetupFile());
    // Initialization Methods
    mpInitializationMethodLabel = new Label(tr("Initialization Method (Optional):"));
    mpInitializationMethodLabel->setToolTip(tr("Specifies the initialization method."));
    currentIndex = mpInitializationMethodComboBox->findText(simulationOptions.getInitializationMethod(), Qt::MatchExactly);
    if (currentIndex > -1) {
      mpInitializationMethodComboBox->setCurrentIndex(currentIndex);
    }
    // Equation System Initialization File
    mpEquationSystemInitializationFileTextBox->setText(simulationOptions.getEquationSystemInitializationFile());
    // Equation System time
    mpEquationSystemInitializationTimeTextBox->setText(simulationOptions.getEquationSystemInitializationTime());
    // clock
    currentIndex = mpClockComboBox->findText(simulationOptions.getClock(), Qt::MatchExactly);
    if (currentIndex > -1) {
      mpClockComboBox->setCurrentIndex(currentIndex);
    }
    // Linear Solvers
    currentIndex = mpLinearSolverComboBox->findText(simulationOptions.getLinearSolver(), Qt::MatchExactly);
    if (currentIndex > -1) {
      mpLinearSolverComboBox->setCurrentIndex(currentIndex);
    }
    // Non Linear Solvers
    currentIndex = mpNonLinearSolverComboBox->findText(simulationOptions.getNonLinearSolver(), Qt::MatchExactly);
    if (currentIndex > -1) {
      mpNonLinearSolverComboBox->setCurrentIndex(currentIndex);
    }
    // time where the linearization of the model should be performed
    mpLinearizationTimeTextBox->setText(simulationOptions.getLinearizationTime());
    // output variables
    mpOutputVariablesTextBox->setText(simulationOptions.getOutputVariables());
    // measure simulation time checkbox
    currentIndex = mpProfilingComboBox->findText(simulationOptions.getProfiling(), Qt::MatchExactly);
    if (currentIndex > -1) {
      mpProfilingComboBox->setCurrentIndex(currentIndex);
    }
    // cpu-time checkbox
    mpCPUTimeCheckBox->setChecked(simulationOptions.getCPUTime());
    // enable all warnings
    mpEnableAllWarningsCheckBox->setChecked(simulationOptions.getEnableAllWarnings());
    // Logging
    mpLogDasslSolverCheckBox->setChecked(simulationOptions.getLogDasslSolver());
    mpLogDebugCheckBox->setChecked(simulationOptions.getLogDebug());
    mpLogDynamicStateSelectionCheckBox->setChecked(simulationOptions.getLogDynamicStateSelection());
    mpLogJacobianDynamicStateSelectionCheckBox->setChecked(simulationOptions.getLogJacobianDynamicStateSelection());
    mpLogEventsCheckBox->setChecked(simulationOptions.getLogEvents());
    mpLogVerboseEventsCheckBox->setChecked(simulationOptions.getLogVerboseEvents());
    mpLogInitializationCheckBox->setChecked(simulationOptions.getLogInitialization());
    mpLogJacobianCheckBox->setChecked(simulationOptions.getLogJacobian());
    mpLogNonLinearSystemsCheckBox->setChecked(simulationOptions.getLogNonLinearSystems());
    mpLogVerboseNonLinearSystemsCheckBox->setChecked(simulationOptions.getLogVerboseNonLinearSystems());
    mpLogJacobianNonLinearSystemsCheckBox->setChecked(simulationOptions.getLogJacobianNonLinearSystems());
    mpLogResidualsInitializationCheckBox->setChecked(simulationOptions.getLogResidualsInitialization());
    mpLogSimulationCheckBox->setChecked(simulationOptions.getLogSimulation());
    mpLogSolverCheckBox->setChecked(simulationOptions.getLogSolver());
    mpLogFinalSolutionOfInitializationCheckBox->setChecked(simulationOptions.getLogFinalSolutionOfInitialization());
    mpLogStatsCheckBox->setChecked(simulationOptions.getLogStats());
    mpLogUtilCheckBox->setChecked(simulationOptions.getLogUtil());
    mpLogZeroCrossingsCheckBox->setChecked(simulationOptions.getLogZeroCrossings());
    mpAdditionalSimulationFlagsTextBox->setText(simulationOptions.getAdditionalSimulationFlags());
    // save simulation settings
    mpSaveSimulationCheckbox->setChecked(false);
    mpSaveSimulationCheckbox->setDisabled(true);
    mpSimulateButton->setText(Helper::reSimulate);
  }
}

/*!
  Used for non-interactive simulation.\n
  Sends the translateModel command to OMC.
  */
bool SimulationDialog::translateModel(QString simulationParameters)
{
  // check reset messages number before simulation option
  if (mpMainWindow->getOptionsDialog()->getMessagesPage()->getResetMessagesNumberBeforeSimulationCheckBox()->isChecked()) {
    mpMainWindow->getMessagesWidget()->resetMessagesNumber();
  }
  /* save the model before translating */
  if (mpMainWindow->getOptionsDialog()->getSimulationPage()->getSaveClassBeforeSimulationCheckBox()->isChecked() &&
      !mpLibraryTreeItem->isSaved() &&
      !mpMainWindow->getLibraryWidget()->saveLibraryTreeItem(mpLibraryTreeItem)) {
    return false;
  }
  /*
    set the debugging flag before translation
    we will remove it when gdb process is finished
    */
  if (mpLaunchAlgorithmicDebuggerCheckBox->isChecked()) {
    mpMainWindow->getOMCProxy()->setCommandLineOptions("+d=gendebugsymbols");
  }
  return mpMainWindow->getOMCProxy()->translateModel(mClassName, simulationParameters);
}

SimulationOptions SimulationDialog::createSimulationOptions()
{
  SimulationOptions simulationOptions;
  simulationOptions.setClassName(mClassName);
  simulationOptions.setStartTime(mpStartTimeTextBox->text());
  simulationOptions.setStopTime(mpStopTimeTextBox->text());
  simulationOptions.setMethod(mpMethodComboBox->currentText());
  simulationOptions.setTolerance(mpToleranceTextBox->text());
  simulationOptions.setDasslJacobian(mpDasslJacobianComboBox->itemData(mpDasslJacobianComboBox->currentIndex()).toString());
  simulationOptions.setDasslRootFinding(mpDasslRootFindingCheckBox->isChecked());
  simulationOptions.setDasslRestart(mpDasslRestartCheckBox->isChecked());
  simulationOptions.setDasslInitialStepSize(mpDasslInitialStepSizeTextBox->text());
  simulationOptions.setDasslMaxStepSize(mpDasslMaxStepSizeTextBox->text());
  simulationOptions.setDasslMaxIntegration(mpDasslMaxIntegrationOrderSpinBox->value());
  simulationOptions.setCflags(mpCflagsTextBox->text());
  simulationOptions.setNumberOfProcessors(mpNumberOfProcessorsSpinBox->value());
  simulationOptions.setBuildOnly(mpBuildOnlyCheckBox->isChecked());
  simulationOptions.setLaunchTransformationalDebugger(mpLaunchTransformationalDebuggerCheckBox->isChecked());
  simulationOptions.setLaunchAlgorithmicDebugger(mpLaunchAlgorithmicDebuggerCheckBox->isChecked());
  simulationOptions.setNumberofIntervals(mpNumberofIntervalsSpinBox->value());
  qreal startTime = mpStartTimeTextBox->text().toDouble();
  qreal stopTime = mpStopTimeTextBox->text().toDouble();
  simulationOptions.setStepSize((stopTime - startTime)/mpNumberofIntervalsSpinBox->value());
  simulationOptions.setOutputFormat(mpOutputFormatComboBox->currentText());
  if (!mpFileNameTextBox->text().isEmpty()) {
    simulationOptions.setFileNamePrefix(mpFileNameTextBox->text());
  } else if (mClassName.contains('\'')) {
    simulationOptions.setFileNamePrefix("_omcQuot_" + QByteArray(mClassName.toStdString().c_str()).toHex());
  }
  simulationOptions.setResultFileName(mpResultFileName->text());
  simulationOptions.setVariableFilter(mpVariableFilterTextBox->text());
  simulationOptions.setProtectedVariables(mpProtectedVariablesCheckBox->isChecked());
  simulationOptions.setEquidistantTimeGrid(mpEquidistantTimeGridCheckBox->isChecked());
  simulationOptions.setStoreVariablesAtEvents(mpStoreVariablesAtEventsCheckBox->isChecked());
  simulationOptions.setShowGeneratedFiles(mpShowGeneratedFilesCheckBox->isChecked());
  simulationOptions.setModelSetupFile(mpModelSetupFileTextBox->text());
  simulationOptions.setInitializationMethod(mpInitializationMethodComboBox->currentText());
  simulationOptions.setEquationSystemInitializationFile(mpEquationSystemInitializationFileTextBox->text());
  simulationOptions.setEquationSystemInitializationTime(mpEquationSystemInitializationTimeTextBox->text());
  simulationOptions.setClock(mpClockComboBox->currentText());
  simulationOptions.setLinearSolver(mpLinearSolverComboBox->currentText());
  simulationOptions.setNonLinearSolver(mpNonLinearSolverComboBox->currentText());
  simulationOptions.setLinearizationTime(mpLinearizationTimeTextBox->text());
  simulationOptions.setOutputVariables(mpOutputVariablesTextBox->text());
  simulationOptions.setProfiling(mpProfilingComboBox->currentText());
  simulationOptions.setCPUTime(mpCPUTimeCheckBox->isChecked());
  simulationOptions.setEnableAllWarnings(mpEnableAllWarningsCheckBox->isChecked());
  simulationOptions.setLogDasslSolver(mpLogDasslSolverCheckBox->isChecked());
  simulationOptions.setLogDebug(mpLogDebugCheckBox->isChecked());
  simulationOptions.setLogDynamicStateSelection(mpLogDynamicStateSelectionCheckBox->isChecked());
  simulationOptions.setLogJacobianDynamicStateSelection(mpLogJacobianDynamicStateSelectionCheckBox->isChecked());
  simulationOptions.setLogEvents(mpLogEventsCheckBox->isChecked());
  simulationOptions.setLogVerboseEvents(mpLogVerboseEventsCheckBox->isChecked());
  simulationOptions.setLogInitialization(mpLogInitializationCheckBox->isChecked());
  simulationOptions.setLogJacobian(mpLogJacobianCheckBox->isChecked());
  simulationOptions.setLogNonLinearSystems(mpLogNonLinearSystemsCheckBox->isChecked());
  simulationOptions.setLogVerboseNonLinearSystems(mpLogVerboseNonLinearSystemsCheckBox->isChecked());
  simulationOptions.setLogJacobianNonLinearSystems(mpLogJacobianNonLinearSystemsCheckBox->isChecked());
  simulationOptions.setLogResidualsInitialization(mpLogResidualsInitializationCheckBox->isChecked());
  simulationOptions.setLogSimulation(mpLogSimulationCheckBox->isChecked());
  simulationOptions.setLogSolver(mpLogSolverCheckBox->isChecked());
  simulationOptions.setLogFinalSolutionOfInitialization(mpLogFinalSolutionOfInitializationCheckBox->isChecked());
  simulationOptions.setLogStats(mpLogStatsCheckBox->isChecked());
  simulationOptions.setLogUtil(mpLogUtilCheckBox->isChecked());
  simulationOptions.setLogZeroCrossings(mpLogZeroCrossingsCheckBox->isChecked());
  simulationOptions.setAdditionalSimulationFlags(mpAdditionalSimulationFlagsTextBox->text());
  // setup simulation flags
  QStringList simulationFlags;
  simulationFlags.append(QString("-override=%1=%2,%3=%4,%5=%6,%7=%8,%9=%10,%11=%12,%13=%14")
                         .arg("startTime").arg(simulationOptions.getStartTime())
                         .arg("stopTime").arg(simulationOptions.getStopTime())
                         .arg("stepSize").arg(simulationOptions.getStepSize())
                         .arg("tolerance").arg(simulationOptions.getTolerance())
                         .arg("solver").arg(simulationOptions.getMethod())
                         .arg("outputFormat").arg(simulationOptions.getOutputFormat())
                         .arg("variableFilter").arg(simulationOptions.getVariableFilter()));
  simulationFlags.append(QString("-r=").append(simulationOptions.getResultFileName()));
  // dassl options
  if (mpDasslOptionsGroupBox->isEnabled()) {
    // dassl jacobian
    simulationFlags.append(QString("-dasslJacobian=").append(mpDasslJacobianComboBox->itemData(mpDasslJacobianComboBox->currentIndex()).toString()));
    // dassl root finding
    if (!mpDasslRootFindingCheckBox->isChecked()) {
      simulationFlags.append("-dasslnoRootFinding");
    }
    // dassl restart
    if (!mpDasslRestartCheckBox->isChecked()) {
      simulationFlags.append("-dasslnoRestart");
    }
    // dassl initial step size
    if (!mpDasslInitialStepSizeTextBox->text().isEmpty()) {
      simulationFlags.append(QString("-initialStepSize=").append(mpDasslInitialStepSizeTextBox->text()));
    }
    // dassl max step size
    if (!mpDasslMaxStepSizeTextBox->text().isEmpty()) {
      simulationFlags.append(QString("-maxStepSize=").append(mpDasslMaxStepSizeTextBox->text()));
    }
    // dassl max step size
    if (mpDasslMaxIntegrationOrderSpinBox->value() != 5) {
      simulationFlags.append(QString("-maxIntegrationOrder=").append(QString::number(mpDasslMaxIntegrationOrderSpinBox->value())));
    }
  }
  // emit protected variables
  if (mpProtectedVariablesCheckBox->isChecked()) {
    simulationFlags.append("-emit_protected");
  }
  // Equidistant time grid
  if (mpEquidistantTimeGridCheckBox->isEnabled() && !mpEquidistantTimeGridCheckBox->isChecked()) {
    simulationFlags.append("-noEquidistantTimeGrid");
  }
  // store variables at events
  if (!mpStoreVariablesAtEventsCheckBox->isChecked()) {
    simulationFlags.append("-noEventEmit");
  }
  // setup Model Setup file flag
  if (!mpModelSetupFileTextBox->text().isEmpty()) {
    simulationFlags.append(QString("-f=").append(mpModelSetupFileTextBox->text()));
  }
  // setup initiaization method flag
  if (!mpInitializationMethodComboBox->currentText().isEmpty()) {
    simulationFlags.append(QString("-iim=").append(mpInitializationMethodComboBox->currentText()));
  }
  // setup Equation System Initialization file flag
  if (!mpEquationSystemInitializationFileTextBox->text().isEmpty()) {
    simulationFlags.append(QString("-iif=").append(mpEquationSystemInitializationFileTextBox->text()));
  }
  // setup Equation System Initialization time flag
  if (!mpEquationSystemInitializationTimeTextBox->text().isEmpty()) {
    simulationFlags.append(QString("-iit=").append(mpEquationSystemInitializationTimeTextBox->text()));
  }
  // clock
  if (!mpClockComboBox->currentText().isEmpty()) {
    simulationFlags.append(QString("-clock=").append(mpClockComboBox->currentText()));
  }
  // linear solver
  if (!mpLinearSolverComboBox->currentText().isEmpty()) {
    simulationFlags.append(QString("-ls=").append(mpLinearSolverComboBox->currentText()));
  }
  // non linear solver
  if (!mpNonLinearSolverComboBox->currentText().isEmpty()) {
    simulationFlags.append(QString("-nls=").append(mpNonLinearSolverComboBox->currentText()));
  }
  // time where the linearization of the model should be performed
  if (!mpLinearizationTimeTextBox->text().isEmpty()) {
    simulationFlags.append(QString("-l=").append(mpLinearizationTimeTextBox->text()));
  }
  // output variables
  if (!mpOutputVariablesTextBox->text().isEmpty()) {
    simulationFlags.append(QString("-output=").append(mpOutputVariablesTextBox->text()));
  }
  // setup cpu time flag
  if (mpCPUTimeCheckBox->isChecked()) {
    simulationFlags.append("-cpu");
  }
  // setup enable all warnings flag
  if (mpEnableAllWarningsCheckBox->isChecked()) {
    simulationFlags.append("-w");
  }
  // setup Logging flags
  if (mpLogDasslSolverCheckBox->isChecked() ||
      mpLogDebugCheckBox->isChecked() ||
      mpLogDynamicStateSelectionCheckBox->isChecked() ||
      mpLogJacobianDynamicStateSelectionCheckBox->isChecked() ||
      mpLogEventsCheckBox->isChecked() ||
      mpLogVerboseEventsCheckBox->isChecked() ||
      mpLogInitializationCheckBox->isChecked() ||
      mpLogJacobianCheckBox->isChecked() ||
      mpLogNonLinearSystemsCheckBox->isChecked() ||
      mpLogVerboseNonLinearSystemsCheckBox->isChecked() ||
      mpLogJacobianNonLinearSystemsCheckBox->isChecked() ||
      mpLogResidualsInitializationCheckBox->isChecked() ||
      mpLogSimulationCheckBox->isChecked() ||
      mpLogSolverCheckBox->isChecked() ||
      mpLogFinalSolutionOfInitializationCheckBox->isChecked() ||
      mpLogStatsCheckBox->isChecked() ||
      mpLogStatsCheckBox->isChecked() ||
      mpLogZeroCrossingsCheckBox->isChecked())
  {
    QString loggingFlagName, loggingFlagValues;
    loggingFlagName.append("-lv=");
    if (mpLogDasslSolverCheckBox->isChecked())
      loggingFlagValues.append("LOG_DDASRT");
    if (mpLogDebugCheckBox->isChecked())
      loggingFlagValues.isEmpty() ? loggingFlagValues.append("LOG_DEBUG") : loggingFlagValues.append(",LOG_DEBUG");
    if (mpLogDynamicStateSelectionCheckBox->isChecked())
      loggingFlagValues.isEmpty() ? loggingFlagValues.append("LOG_DSS") : loggingFlagValues.append(",LOG_DSS");
    if (mpLogJacobianDynamicStateSelectionCheckBox->isChecked())
      loggingFlagValues.isEmpty() ? loggingFlagValues.append("LOG_DSS_JAC") : loggingFlagValues.append(",LOG_DSS_JAC");
    if (mpLogEventsCheckBox->isChecked())
      loggingFlagValues.isEmpty() ? loggingFlagValues.append("LOG_EVENTS") : loggingFlagValues.append(",LOG_EVENTS");
    if (mpLogVerboseEventsCheckBox->isChecked())
      loggingFlagValues.isEmpty() ? loggingFlagValues.append("LOG_EVENTS_V") : loggingFlagValues.append(",LOG_EVENTS_V");
    if (mpLogInitializationCheckBox->isChecked())
      loggingFlagValues.isEmpty() ? loggingFlagValues.append("LOG_INIT") : loggingFlagValues.append(",LOG_INIT");
    if (mpLogJacobianCheckBox->isChecked())
      loggingFlagValues.isEmpty() ? loggingFlagValues.append("LOG_JAC") : loggingFlagValues.append(",LOG_JAC");
    if (mpLogNonLinearSystemsCheckBox->isChecked())
      loggingFlagValues.isEmpty() ? loggingFlagValues.append("LOG_NLS") : loggingFlagValues.append(",LOG_NLS");
    if (mpLogVerboseNonLinearSystemsCheckBox->isChecked())
      loggingFlagValues.isEmpty() ? loggingFlagValues.append("LOG_NLS_V") : loggingFlagValues.append(",LOG_NLS_V");
    if (mpLogJacobianNonLinearSystemsCheckBox->isChecked())
      loggingFlagValues.isEmpty() ? loggingFlagValues.append("LOG_NLS_JAC") : loggingFlagValues.append(",LOG_NLS_JAC");
    if (mpLogResidualsInitializationCheckBox->isChecked())
      loggingFlagValues.isEmpty() ? loggingFlagValues.append("LOG_RES_INIT") : loggingFlagValues.append(",LOG_RES_INIT");
    if (mpLogSimulationCheckBox->isChecked())
      loggingFlagValues.isEmpty() ? loggingFlagValues.append("LOG_SIMULATION") : loggingFlagValues.append(",LOG_SIMULATION");
    if (mpLogSolverCheckBox->isChecked())
      loggingFlagValues.isEmpty() ? loggingFlagValues.append("LOG_SOLVER") : loggingFlagValues.append(",LOG_SOLVER");
    if (mpLogFinalSolutionOfInitializationCheckBox->isChecked())
      loggingFlagValues.isEmpty() ? loggingFlagValues.append("LOG_SOTI") : loggingFlagValues.append(",LOG_SOTI");
    if (mpLogStatsCheckBox->isChecked())
      loggingFlagValues.isEmpty() ? loggingFlagValues.append("LOG_STATS") : loggingFlagValues.append(",LOG_STATS");
    if (mpLogUtilCheckBox->isChecked())
      loggingFlagValues.isEmpty() ? loggingFlagValues.append("LOG_UTIL") : loggingFlagValues.append(",LOG_UTIL");
    if (mpLogZeroCrossingsCheckBox->isChecked())
      loggingFlagValues.isEmpty() ? loggingFlagValues.append("LOG_ZEROCROSSINGS") : loggingFlagValues.append(",LOG_ZEROCROSSINGS");

    simulationFlags.append(QString(loggingFlagName).append(loggingFlagValues));
  }
  if (!mpAdditionalSimulationFlagsTextBox->text().isEmpty()) {
    simulationFlags.append(StringHandler::splitStringWithSpaces(mpAdditionalSimulationFlagsTextBox->text()));
  }
  simulationOptions.setSimulationFlags(simulationFlags);
  simulationOptions.setIsValid(true);
  simulationOptions.setReSimulate(mIsReSimulate);
  simulationOptions.setWorkingDirectory(mpMainWindow->getOMCProxy()->changeDirectory());
  simulationOptions.setFileName(mFileName);
  return simulationOptions;
}

/*!
  Creates the SimulationOutputWidget.
  */
void SimulationDialog::createAndShowSimulationOutputWidget(SimulationOptions simulationOptions)
{
  /*
    If resimulation and show algorithmic debugger is checked then show algorithmic debugger.
    If show transformational debugger is checked then show transformational debugger.
    Otherwise run the normal resimulation.
    */
  if (simulationOptions.isReSimulate() && simulationOptions.getLaunchAlgorithmicDebugger()) {
    if (mpMainWindow->getOptionsDialog()->getDebuggerPage()->getAlwaysShowTransformationsCheckBox()->isChecked() ||
        simulationOptions.getLaunchTransformationalDebugger() || simulationOptions.getProfiling() != "none") {
      mpMainWindow->showTransformationsWidget(simulationOptions.getWorkingDirectory() + "/" + simulationOptions.getOutputFileName() + "_info.json");
    }
    showAlgorithmicDebugger(simulationOptions);
  } else {
    if (simulationOptions.isReSimulate()) {
      if (mpMainWindow->getOptionsDialog()->getDebuggerPage()->getAlwaysShowTransformationsCheckBox()->isChecked() ||
          simulationOptions.getLaunchTransformationalDebugger() || simulationOptions.getProfiling() != "none") {
        mpMainWindow->showTransformationsWidget(simulationOptions.getWorkingDirectory() + "/" + simulationOptions.getOutputFileName() + "_info.json");
      }
    }
    SimulationOutputWidget *pSimulationOutputWidget = new SimulationOutputWidget(simulationOptions, mpMainWindow);
    mSimulationOutputWidgetsList.append(pSimulationOutputWidget);
    int xPos = QApplication::desktop()->availableGeometry().width() - pSimulationOutputWidget->frameSize().width() - 20;
    int yPos = QApplication::desktop()->availableGeometry().height() - pSimulationOutputWidget->frameSize().height() - 20;
    pSimulationOutputWidget->setGeometry(xPos, yPos, pSimulationOutputWidget->width(), pSimulationOutputWidget->height());
    pSimulationOutputWidget->show();
  }
}

/*!
  Saves the simulation options in the model.
  */
void SimulationDialog::saveSimulationOptions()
{
  if (mIsReSimulate || !mpSaveSimulationCheckbox->isChecked())
    return;

  QString oldExperimentAnnotation = "annotate=experiment(";
  // if the class has experiment annotation then read it.
  if (mpMainWindow->getOMCProxy()->isExperiment(mpLibraryTreeItem->getNameStructure())) {
    // get the simulation options....
    QStringList result = mpMainWindow->getOMCProxy()->getSimulationOptions(mpLibraryTreeItem->getNameStructure());
    // since we always get simulationOptions so just get the values from array
    oldExperimentAnnotation.append("StartTime=").append(QString::number(result.at(0).toFloat())).append(",");
    oldExperimentAnnotation.append("StopTime=").append(QString::number(result.at(1).toFloat())).append(",");
    oldExperimentAnnotation.append("Tolerance=").append(QString::number(result.at(2).toFloat())).append(",");
    oldExperimentAnnotation.append("Interval=").append(QString::number(result.at(3).toFloat()));
  }
  oldExperimentAnnotation.append(")");
  QString newExperimentAnnotation;
  // create simulations options annotation
  newExperimentAnnotation.append("annotate=experiment(");
  newExperimentAnnotation.append("StartTime=").append(mpStartTimeTextBox->text()).append(",");
  newExperimentAnnotation.append("StopTime=").append(mpStopTimeTextBox->text()).append(",");
  newExperimentAnnotation.append("Tolerance=").append(mpToleranceTextBox->text()).append(",");
  double interval, stopTime, startTime;
  int numberOfIntervals;
  stopTime = mpStopTimeTextBox->text().toDouble();
  startTime = mpStartTimeTextBox->text().toDouble();
  numberOfIntervals = mpNumberofIntervalsSpinBox->value();
  interval = (numberOfIntervals == 0) ? 0 : (stopTime - startTime) / numberOfIntervals;
  newExperimentAnnotation.append("Interval=").append(QString::number(interval));
  newExperimentAnnotation.append(")");
  // if we have ModelWidget for class then put the change on undo stack.
  if (mpLibraryTreeItem->getModelWidget()) {
    UpdateClassExperimentAnnotationCommand *pUpdateClassExperimentAnnotationCommand;
    pUpdateClassExperimentAnnotationCommand = new UpdateClassExperimentAnnotationCommand(mpMainWindow, mpLibraryTreeItem,
                                                                                         oldExperimentAnnotation, newExperimentAnnotation);
    mpLibraryTreeItem->getModelWidget()->getUndoStack()->push(pUpdateClassExperimentAnnotationCommand);
    mpLibraryTreeItem->getModelWidget()->updateModelicaText();
  } else {
    // send the simulations options annotation to OMC
    mpMainWindow->getOMCProxy()->addClassAnnotation(mpLibraryTreeItem->getNameStructure(), newExperimentAnnotation);
    LibraryTreeModel *pLibraryTreeModel = mpMainWindow->getLibraryWidget()->getLibraryTreeModel();
    pLibraryTreeModel->updateLibraryTreeItemClassText(mpLibraryTreeItem);
  }
}

void SimulationDialog::reSimulate(SimulationOptions simulationOptions)
{
  createAndShowSimulationOutputWidget(simulationOptions);
}

void SimulationDialog::showAlgorithmicDebugger(SimulationOptions simulationOptions)
{
  // if not build only and launch the algorithmic debugger is true
  if (!simulationOptions.getBuildOnly() && simulationOptions.getLaunchAlgorithmicDebugger()) {
    QString fileName = simulationOptions.getOutputFileName();
    // start the executable
    fileName = QString(simulationOptions.getWorkingDirectory()).append("/").append(fileName);
    fileName = fileName.replace("//", "/");
    // run the simulation executable to create the result file
#ifdef WIN32
    fileName = fileName.append(".exe");
#endif
    // start the debugger
    if (mpMainWindow->getDebuggerMainWindow()->getGDBAdapter()->isGDBRunning()) {
      QMessageBox::information(this, QString(Helper::applicationName).append(" - ").append(Helper::information),
                               GUIMessages::getMessage(GUIMessages::DEBUGGER_ALREADY_RUNNING), Helper::ok);
    } else {
      QString GDBPath = mpMainWindow->getOptionsDialog()->getDebuggerPage()->getGDBPath();
      GDBAdapter *pGDBAdapter = mpMainWindow->getDebuggerMainWindow()->getGDBAdapter();
      pGDBAdapter->launch(fileName, simulationOptions.getWorkingDirectory(), simulationOptions.getSimulationFlags(), GDBPath, simulationOptions);
      mpMainWindow->showAlgorithmicDebugger();
    }
  }
}

void SimulationDialog::simulationProcessFinished(SimulationOptions simulationOptions, QDateTime resultFileLastModifiedDateTime)
{
  QString workingDirectory = simulationOptions.getWorkingDirectory();
  // read the result file
  QFileInfo resultFileInfo(QString(workingDirectory).append("/").append(simulationOptions.getResultFileName()));
  QRegExp regExp("\\b(mat|plt|csv)\\b");
  if (regExp.indexIn(simulationOptions.getResultFileName()) != -1 &&
      resultFileInfo.exists() && resultFileLastModifiedDateTime <= resultFileInfo.lastModified()) {
    VariablesWidget *pVariablesWidget = mpMainWindow->getVariablesWidget();
    OMCProxy *pOMCProxy = mpMainWindow->getOMCProxy();
    QStringList list = pOMCProxy->readSimulationResultVars(simulationOptions.getResultFileName());
    // close the simulation result file.
    pOMCProxy->closeSimulationResultFile();
    if (list.size() > 0) {
      mpMainWindow->getPerspectiveTabBar()->setCurrentIndex(2);
      pVariablesWidget->insertVariablesItemsToTree(simulationOptions.getResultFileName(), workingDirectory, list, simulationOptions);
    }
  }
}

/*!
 * \brief SimulationDialog::updateMethodToolTip
 * Updates the Method combobox tooltip.
 * \param index
 */
void SimulationDialog::updateMethodToolTip(int index)
{
  mpMethodComboBox->setToolTip(mpMethodComboBox->itemData(index, Qt::ToolTipRole).toString());
}

/*!
  Slot activated when mpMethodComboBox currentIndexChanged signal is raised.\n
  Enables/disables the Dassl options group box
  */
void SimulationDialog::enableDasslOptions(QString method)
{
  if (method.compare("dassl") == 0) {
    mpDasslOptionsGroupBox->setEnabled(true);
    mpEquidistantTimeGridCheckBox->setEnabled(true);
  } else {
    mpDasslOptionsGroupBox->setEnabled(false);
    mpEquidistantTimeGridCheckBox->setEnabled(false);
  }
}

/*!
  Slot activated when mpMehtodHelpButton clicked signal is raised.\n
  Opens the IntegrationAlgorithms.pdf file.
  */
void SimulationDialog::showIntegrationHelp()
{
  QUrl integrationAlgorithmsPath (QString("file:///").append(QString(Helper::OpenModelicaHome).replace("\\", "/"))
                                  .append("/share/doc/omc/OpenModelicaUsersGuide/simulationflags.html#integration-methods"));
  if (!QDesktopServices::openUrl(integrationAlgorithmsPath)) {
    QString errorMessage = GUIMessages::getMessage(GUIMessages::UNABLE_TO_OPEN_FILE).arg(integrationAlgorithmsPath.toString());
    mpMainWindow->getMessagesWidget()->addGUIMessage(MessageItem(MessageItem::Modelica, "", false, 0, 0, 0, 0, errorMessage,
                                                                 Helper::scriptingKind, Helper::errorLevel));
  }
}

/*!
  Slot activated when mpBuildOnlyCheckBox checkbox is checked.\n
  Makes sure that we only build the modelica model and don't run the simulation.
  */
void SimulationDialog::buildOnly(bool checked)
{
  mpLaunchAlgorithmicDebuggerCheckBox->setEnabled(!checked);
  mpSimulationFlagsTab->setEnabled(!checked);
}

/*!
  Slot activated when mpModelSetupFileBrowseButton clicked signal is raised.\n
  Allows user to select Model Setup File.
  */
void SimulationDialog::browseModelSetupFile()
{
  mpModelSetupFileTextBox->setText(StringHandler::getOpenFileName(this,QString(Helper::applicationName).append(" - ").append(Helper::chooseFile), NULL, Helper::xmlFileTypes, NULL));
}

/*!
  Slot activated when mpEquationSystemInitializationFileBrowseButton clicked signal is raised.\n
  Allows user to select Equation System Initialization File.
  */
void SimulationDialog::browseEquationSystemInitializationFile()
{
  mpEquationSystemInitializationFileTextBox->setText(StringHandler::getOpenFileName(this, QString(Helper::applicationName).append(" - ").append(Helper::chooseFile), NULL, Helper::matFileTypes, NULL));
}

/*!
  Slot activated when mpArchivedSimulationsListWidget itemDoubleClicked signal is raised.\n
  Shows the archived SimulationOutputWidget.
  */
void SimulationDialog::showArchivedSimulation(QTreeWidgetItem *pTreeWidgetItem)
{
  ArchivedSimulationItem *pArchivedSimulationItem = dynamic_cast<ArchivedSimulationItem*>(pTreeWidgetItem);
  if (pArchivedSimulationItem) {
    SimulationOutputWidget *pSimulationOutputWidget = pArchivedSimulationItem->getSimulationOutputWidget();
    pSimulationOutputWidget->show();
    pSimulationOutputWidget->raise();
    pSimulationOutputWidget->setWindowState(pSimulationOutputWidget->windowState() & (~Qt::WindowMinimized));
  }
}

/*!
  Slot activated when mpSimulateButton clicked signal is raised.\n
  Reads the simulation options set by the user and sends them to OMC by calling buildModel.
  */
void SimulationDialog::simulate()
{
  SimulationOptions simulationOptions;
  if (validate()) {
    QString simulationParameters;
    /* build the simulation parameters */
    simulationParameters.append("startTime=").append(mpStartTimeTextBox->text());
    simulationParameters.append(", stopTime=").append(mpStopTimeTextBox->text());
    simulationParameters.append(", numberOfIntervals=").append(QString::number(mpNumberofIntervalsSpinBox->value()));
    simulationParameters.append(", method=").append("\"").append(mpMethodComboBox->currentText()).append("\"");
    if (!mpToleranceTextBox->text().isEmpty()) {
      simulationParameters.append(", tolerance=").append(mpToleranceTextBox->text());
    }
    simulationParameters.append(", outputFormat=").append("\"").append(mpOutputFormatComboBox->currentText()).append("\"");
    if (!mpFileNameTextBox->text().isEmpty()) {
      simulationParameters.append(", fileNamePrefix=").append("\"").append(mpFileNameTextBox->text()).append("\"");
    } else if (mClassName.contains('\'')) {
      simulationParameters.append(", fileNamePrefix=").append("\"_omcQuot_").append(QByteArray(mClassName.toStdString().c_str()).toHex()).append("\"");
    }
    if (!mpVariableFilterTextBox->text().isEmpty()) {
      simulationParameters.append(", variableFilter=").append("\"").append(mpVariableFilterTextBox->text()).append("\"");
    }
    if (!mpCflagsTextBox->text().isEmpty()) {
      simulationParameters.append(", cflags=").append("\"").append(mpCflagsTextBox->text()).append("\"");
    }
    mpMainWindow->getOMCProxy()->setCommandLineOptions("+profiling=" + mpProfilingComboBox->currentText());
    simulationOptions = createSimulationOptions();
    // before simulating save the simulation options.
    saveSimulationOptions();
    // show the progress bar
    mpMainWindow->getStatusBar()->showMessage(tr("Translating %1.").arg(mClassName));
    mpMainWindow->getProgressBar()->setRange(0, 0);
    mpMainWindow->showProgressBar();
    bool isTranslationSuccessful = mIsReSimulate ? true : translateModel(simulationParameters);
    // hide the progress bar
    mpMainWindow->hideProgressBar();
    mpMainWindow->getStatusBar()->clearMessage();
    mIsReSimulate = false;
    accept();
    if (isTranslationSuccessful) {
      // check if we can compile using the target compiler
      SimulationPage *pSimulationPage = mpMainWindow->getOptionsDialog()->getSimulationPage();
      QString targetCompiler = pSimulationPage->getTargetCompilerComboBox()->currentText();
      if ((targetCompiler.compare("vxworks69") == 0) || (targetCompiler.compare("debugrt") == 0)) {
        QString msg = tr("Generated code for the target compiler <b>%1</b> at %2.").arg(targetCompiler)
            .arg(simulationOptions.getWorkingDirectory());
        mpMainWindow->getMessagesWidget()->addGUIMessage(MessageItem(MessageItem::Modelica, "", false, 0, 0, 0, 0, msg, Helper::scriptingKind,
                                                                     Helper::notificationLevel));
        return;
      }
      QString targetLanguage = pSimulationPage->getTargetLanguageComboBox()->currentText();
      // check if we can compile using the target language
      if ((targetLanguage.compare("C") == 0) || (targetLanguage.compare("Cpp") == 0)) {
        createAndShowSimulationOutputWidget(simulationOptions);
      } else {
        QString msg = tr("Generated code for the target language <b>%1</b> at %2.").arg(targetLanguage)
            .arg(simulationOptions.getWorkingDirectory());
        mpMainWindow->getMessagesWidget()->addGUIMessage(MessageItem(MessageItem::Modelica, "", false, 0, 0, 0, 0, msg, Helper::scriptingKind,
                                                                     Helper::notificationLevel));
        return;
      }
    }
  }
}

/*!
 * \brief SimulationDialog::resultFileNameChanged
 * \param text
 * Slot activated when mpResultFileNameTextBox textEdited OR mpOutputFormatComboBox currentIndexChanged signal is raised.\n
 * Sets the result file name label.
 */
void SimulationDialog::resultFileNameChanged(QString text)
{
  QLineEdit *pLineEditSender = qobject_cast<QLineEdit*>(sender());
  QComboBox *pComboBoxSender = qobject_cast<QComboBox*>(sender());

  if (pLineEditSender) {
    if (text.isEmpty()) {
      mpResultFileName->clear();
    } else {
      mpResultFileName->setText(QString("%1_res.%2").arg(text).arg(mpOutputFormatComboBox->currentText()));
    }
  } else if (pComboBoxSender && !mpResultFileNameTextBox->text().isEmpty()) {
    mpResultFileName->setText(QString("%1_res.%2").arg(mpResultFileNameTextBox->text()).arg(mpOutputFormatComboBox->currentText()));
  }
}
