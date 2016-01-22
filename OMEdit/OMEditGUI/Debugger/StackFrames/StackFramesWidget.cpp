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
 * RCS: $Id: StackFramesWidget.cpp 23058 2014-10-29 23:00:02Z adeas31 $
 *
 */

#include "StackFramesWidget.h"
#include "GDBAdapter.h"
#include "CommandFactory.h"

StackFrameItem::StackFrameItem(QString level, QString address, QString function, QString line, QString file, QString fullName, StackFramesTreeWidget *pStackFramesTreeWidget)
  : QTreeWidgetItem(pStackFramesTreeWidget)
{
  mpStackFramesTreeWidget = pStackFramesTreeWidget;
  mLevel = level;
  mAddress = address;
  mLine = line;
  mFile = cleanupFileName(file);
  mFullName = cleanupFileName(fullName);
  mFunction = cleanupFunction(function);
  setText(0, mFunction);
  setToolTip(0, mFunction);
  setText(1, mLine);
  setToolTip(1, mLine);
  setText(2, getFileName());
  setToolTip(2, getFileName());
  filterStackFrame();
}

QString StackFrameItem::getFileName()
{
  return getFullName().isEmpty() ? getFile() : getFullName();
}

void StackFrameItem::filterStackFrame()
{
  OptionsDialog *pOptionsDialog = mpStackFramesTreeWidget->getStackFramesWidget()->getDebuggerMainWindow()->getMainWindow()->getOptionsDialog();
  /* If file is not readable then disable the frame. */
  QFileInfo fileInfo(getFileName());
  if (StringHandler::isCFile(fileInfo.suffix()) || !fileInfo.isReadable()) {
    setDisabled(true);
  } else {
    setDisabled(false);
  }
  /* check display of C Frames */
  if (!pOptionsDialog->getDebuggerPage()->getDisplayCFramesCheckBox()->isChecked())
  {
    if (StringHandler::isCFile(fileInfo.suffix()))
    {
      setHidden(true);
      return;
    }
  }
  if (!pOptionsDialog->getDebuggerPage()->getDisplayUnknownFramesCheckBox()->isChecked())
  {
    if (!(StringHandler::isModelicaFile(fileInfo.suffix()) && StringHandler::isCFile(fileInfo.suffix())))
      setHidden(true);
  }
  setHidden(false);
}

QString StackFrameItem::cleanupFileName(const QString &fileName)
{
  QString cleanFilePath = fileName;
  // Gdb running on windows often delivers "fullnames" which
  // (a) have no drive letter and (b) are not normalized.
#ifdef Q_OS_WIN
  if (fileName.isEmpty())
    return "";
  QFileInfo fileInfo(fileName);
  if (fileInfo.isReadable())
    cleanFilePath = QDir::cleanPath(fileInfo.absoluteFilePath());
#endif
  return cleanFilePath;
}

QString StackFrameItem::cleanupFunction(const QString &function)
{
  QString cleanFunction = function;
  QFileInfo fileInfo(getFileName());
  if (StringHandler::isModelicaFile(fileInfo.suffix()))
  {
    /* if the function name starts with omc_ then remove the first 4 characters. */
    if (function.startsWith("omc_"))
    {
      cleanFunction = function.mid(4);
    }
    /* if the names are converted to hex values */
    else if (function.startsWith("_omcQuot_"))
    {
      char* hexString = omcHexToString(function.toStdString().c_str());
      cleanFunction = QString(hexString);
      if (hexString) free(hexString);
    }
  }
  return cleanFunction;
}

char* StackFrameItem::omcHexToString(const char* str)
{
  const char lookupTbl[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
  char *res,*cur;
  int len,i;
  const int offset = 10;
  const char _omcQuot[]="_omcQuot_";
  if (*str != '\'') return NULL;
  len = strlen(str)-2;
  res = (char*) malloc(2*len+offset+64);
  cur = res;
  cur += sprintf(cur,"%s",_omcQuot);
  for (i=0; i<len; i++) {
    unsigned char c = str[i+1];
    *cur = lookupTbl[c/16];
    cur++;
    *cur = lookupTbl[c%16];
    cur++;
  }
  *cur = '\0';
  return res;
}

/*!
  \class StackFramesTreeWidget
  \brief Shows the list of stack frames as a tree.
  */
/*!
  \param pStackFramesWidget - pointer to StackFramesWidget
  */
StackFramesTreeWidget::StackFramesTreeWidget(StackFramesWidget *pStackFramesWidget)
  : QTreeWidget(pStackFramesWidget)
{
  mpStackFramesWidget = pStackFramesWidget;
  setItemDelegate(new ItemDelegate(this));
  setTextElideMode(Qt::ElideMiddle);
  setIconSize(Helper::iconSize);
  setColumnCount(3);
  QStringList headers;
  headers << tr("Function") << Helper::line << tr("File");
  setHeaderLabels(headers);
  setIndentation(0);
  setExpandsOnDoubleClick(false);
  setContextMenuPolicy(Qt::CustomContextMenu);
  createActions();
  connect(mpStackFramesWidget->getDebuggerMainWindow()->getGDBAdapter(), SIGNAL(stackListFrames(GDBMIValue*)), SLOT(createStackFrames(GDBMIValue*)));
  connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), mpStackFramesWidget, SLOT(stackItemDoubleClicked(QTreeWidgetItem*)));
  connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
}

StackFramesWidget* StackFramesTreeWidget::getStackFramesWidget()
{
  return mpStackFramesWidget;
}

void StackFramesTreeWidget::clearStackFrames()
{
  int i = 0;
  while(i < topLevelItemCount())
  {
    qDeleteAll(topLevelItem(i)->takeChildren());
    delete topLevelItem(i);
    i = 0;   //Restart iteration
  }
}

void StackFramesTreeWidget::updateStackFrames()
{
  QTreeWidgetItemIterator it(this);
  while (*it)
  {
    StackFrameItem *pStackFrameItem = dynamic_cast<StackFrameItem*>((*it));
    if (pStackFrameItem)
    {
      pStackFrameItem->filterStackFrame();
    }
    ++it;
  }
}

void StackFramesTreeWidget::setCurrentStackFrame(QTreeWidgetItem *pQTreeWidgetItem)
{
  StackFrameItem *pStackFrameItem = dynamic_cast<StackFrameItem*>(pQTreeWidgetItem);
  if (pStackFrameItem)
  {
    /* Clear the icon of all stack frames items */
    QTreeWidgetItemIterator it(this);
    while (*it)
    {
      QPixmap pixmap(Helper::iconSize);
      pixmap.fill(QColor(Qt::transparent));
      (*it)->setIcon(0, QIcon(pixmap));
      ++it;
    }
    /* set the current item and add icon to it */
    QByteArray cmd = CommandFactory::stackSelectFrame(pStackFrameItem->getLevel().toInt());
    mpStackFramesWidget->getDebuggerMainWindow()->getGDBAdapter()->postCommand(cmd);
    setCurrentItem(pStackFrameItem);
    pStackFrameItem->setIcon(0, QIcon(":/Resources/icons/next.svg"));
  }
}

void StackFramesTreeWidget::createActions()
{
  mpCreateFullBacktraceAction = new QAction(tr("Create Full Backtrace"), this);
  mpCreateFullBacktraceAction->setStatusTip(tr("Creates a full backtrace of the program"));
  connect(mpCreateFullBacktraceAction, SIGNAL(triggered()), SLOT(createFullBacktrace()));
}

/*!
  Slot activated when stackListFrames signal of GDBAdapter is raised.
  */
/*
  -stack-list-frames
  ^done,
  stack=[
  frame={level="0",addr="0x00010734",func="callee4",
  file="../../../devo/gdb/testsuite/gdb.mi/basics.c",
  fullname="/home/foo/bar/devo/gdb/testsuite/gdb.mi/basics.c",line="8"},
  frame={level="1",addr="0x0001076c",func="callee3",
  file="../../../devo/gdb/testsuite/gdb.mi/basics.c",
  fullname="/home/foo/bar/devo/gdb/testsuite/gdb.mi/basics.c",line="17"}]
  */
void StackFramesTreeWidget::createStackFrames(GDBMIValue *pGDBMIValue)
{
  /* Clear the StackFramesTreeWidget items */
  clearStackFrames();
  /* create the StackFramesTreeWidget items */
  if(pGDBMIValue->type == GDBMIValue::ListValue)
  {
    GDBMIResultList::iterator resultListIterator;
    for (resultListIterator = pGDBMIValue->miList->miResultsList.begin(); resultListIterator != pGDBMIValue->miList->miResultsList.end(); ++resultListIterator)
    {
      GDBMIResult *pGDBMIResult = *resultListIterator;
      if (pGDBMIResult->variable.compare("frame") == 0)
      {
        QString level, address, function, line, file, fullName;
        if (pGDBMIResult->miValue->type == GDBMIValue::TupleValue)
        {
          GDBAdapter *pGDBAdapter = mpStackFramesWidget->getDebuggerMainWindow()->getGDBAdapter();
          GDBMIResultList resultsList = pGDBMIResult->miValue->miTuple->miResultsList;
          level = pGDBAdapter->getGDBMIConstantValue(pGDBAdapter->getGDBMIResult("level", resultsList));
          address = pGDBAdapter->getGDBMIConstantValue(pGDBAdapter->getGDBMIResult("addr", resultsList));
          function = pGDBAdapter->getGDBMIConstantValue(pGDBAdapter->getGDBMIResult("func", resultsList));
          line = pGDBAdapter->getGDBMIConstantValue(pGDBAdapter->getGDBMIResult("line", resultsList));
          file = pGDBAdapter->getGDBMIConstantValue(pGDBAdapter->getGDBMIResult("file", resultsList));
          fullName = pGDBAdapter->getGDBMIConstantValue(pGDBAdapter->getGDBMIResult("fullname", resultsList));
        }
        StackFrameItem *pStackFrameItem = new StackFrameItem(level, address, function, line, file, fullName, this);
        addTopLevelItem(pStackFrameItem);
      }
    }
  }
  /* if we get some frames then make the first one selected and fetch the variables for it. */
  if (topLevelItem(0)) {
    if (!mpStackFramesWidget->stackItemDoubleClicked(topLevelItem(0))) {
      /* if we reach here we should suspend the debugger. Otherwise the debugger is suspended in GDBAdapter::stackListVariablesCB */
      mpStackFramesWidget->getDebuggerMainWindow()->getGDBAdapter()->suspendDebugger();
    }
  }
}

void StackFramesTreeWidget::showContextMenu(QPoint point)
{
  int adjust = 24;
  StackFrameItem *pStackFrameItem = dynamic_cast<StackFrameItem*>(itemAt(point));
  if (pStackFrameItem) {
    QMenu menu(this);
    menu.addAction(mpCreateFullBacktraceAction);
    point.setY(point.y() + adjust);
    menu.exec(mapToGlobal(point));
  }
}

void StackFramesTreeWidget::createFullBacktrace()
{
  QByteArray cmd = CommandFactory::createFullBacktrace();
  GDBAdapter *pGDBAdapter = mpStackFramesWidget->getDebuggerMainWindow()->getGDBAdapter();
  pGDBAdapter->postCommand(cmd, GDBAdapter::ConsoleCommand, &GDBAdapter::createFullBacktraceCB);
}

/*!
  \class StackFramesWidget
  \brief A widget containing debugger manipulation buttons and StackFramesTreeWidget.
  */
/*!
  \param pMainWindow - pointer to MainWindow
  */
StackFramesWidget::StackFramesWidget(DebuggerMainWindow *pDebuggerMainWindow)
  : QWidget(pDebuggerMainWindow)
{
  mpDebuggerMainWindow = pDebuggerMainWindow;
  /* continue tool button */
  mpResumeToolButton = new QToolButton;
  mpResumeToolButton->setEnabled(false);
  mpResumeToolButton->setText(Helper::resume);
  mpResumeToolButton->setIcon(QIcon(":/Resources/icons/resume.svg"));
  mpResumeToolButton->setIconSize(Helper::buttonIconSize);
  mpResumeToolButton->setToolTip(Helper::resume);
  mpResumeToolButton->setAutoRaise(true);
  connect(mpResumeToolButton, SIGNAL(clicked()), SLOT(resumeButtonClicked()));
  /* continue tool button */
  mpInterruptToolButton = new QToolButton;
  mpInterruptToolButton->setEnabled(false);
  mpInterruptToolButton->setText(Helper::interrupt);
  mpInterruptToolButton->setIcon(QIcon(":/Resources/icons/interrupt.svg"));
  mpInterruptToolButton->setIconSize(Helper::buttonIconSize);
  mpInterruptToolButton->setToolTip(Helper::interrupt);
  mpInterruptToolButton->setAutoRaise(true);
  connect(mpInterruptToolButton, SIGNAL(clicked()), SLOT(interruptButtonClicked()));
  /* continue tool button */
  mpExitToolButton = new QToolButton;
  mpExitToolButton->setEnabled(false);
  mpExitToolButton->setText(Helper::exit);
  mpExitToolButton->setIcon(QIcon(":/Resources/icons/exit.svg"));
  mpExitToolButton->setIconSize(Helper::buttonIconSize);
  mpExitToolButton->setToolTip(Helper::exit);
  mpExitToolButton->setAutoRaise(true);
  connect(mpExitToolButton, SIGNAL(clicked()), SLOT(exitButtonClicked()));
  /* Control buttons frame */
  QFrame *pControlButtonsFrame = new QFrame;
  QHBoxLayout *pControlButtonsHorizontalLayout = new QHBoxLayout;
  pControlButtonsHorizontalLayout->setContentsMargins(0, 0, 0, 0);
  pControlButtonsHorizontalLayout->setSpacing(0);
  pControlButtonsHorizontalLayout->addWidget(mpResumeToolButton);
  pControlButtonsHorizontalLayout->addWidget(mpInterruptToolButton);
  pControlButtonsHorizontalLayout->addWidget(mpExitToolButton);
  pControlButtonsFrame->setLayout(pControlButtonsHorizontalLayout);
  /* continue tool button */
  mpStepOverToolButton = new QToolButton;
  mpStepOverToolButton->setEnabled(false);
  mpStepOverToolButton->setText(Helper::stepOver);
  mpStepOverToolButton->setIcon(QIcon(":/Resources/icons/step-over.svg"));
  mpStepOverToolButton->setIconSize(Helper::buttonIconSize);
  mpStepOverToolButton->setToolTip(Helper::stepOver);
  mpStepOverToolButton->setAutoRaise(true);
  connect(mpStepOverToolButton, SIGNAL(clicked()), SLOT(stepOverButtonClicked()));
  /* continue tool button */
  mpStepIntoToolButton = new QToolButton;
  mpStepIntoToolButton->setEnabled(false);
  mpStepIntoToolButton->setText(Helper::stepInto);
  mpStepIntoToolButton->setIcon(QIcon(":/Resources/icons/step-into.svg"));
  mpStepIntoToolButton->setIconSize(Helper::buttonIconSize);
  mpStepIntoToolButton->setToolTip(Helper::stepInto);
  mpStepIntoToolButton->setAutoRaise(true);
  connect(mpStepIntoToolButton, SIGNAL(clicked()), SLOT(stepIntoButtonClicked()));
  /* continue tool button */
  mpStepReturnToolButton = new QToolButton;
  mpStepReturnToolButton->setEnabled(false);
  mpStepReturnToolButton->setText(Helper::stepReturn);
  mpStepReturnToolButton->setIcon(QIcon(":/Resources/icons/step-return.svg"));
  mpStepReturnToolButton->setIconSize(Helper::buttonIconSize);
  mpStepReturnToolButton->setToolTip(Helper::stepReturn);
  mpStepReturnToolButton->setAutoRaise(true);
  connect(mpStepReturnToolButton, SIGNAL(clicked()), SLOT(stepReturnButtonClicked()));
  /* Flow buttons frame */
  QFrame *pFlowButtonsFrame = new QFrame;
  QHBoxLayout *pFlowButtonsHorizontalLayout = new QHBoxLayout;
  pFlowButtonsHorizontalLayout->setContentsMargins(0, 0, 0, 0);
  pFlowButtonsHorizontalLayout->setSpacing(0);
  pFlowButtonsHorizontalLayout->addWidget(mpStepOverToolButton);
  pFlowButtonsHorizontalLayout->addWidget(mpStepIntoToolButton);
  pFlowButtonsHorizontalLayout->addWidget(mpStepReturnToolButton);
  pFlowButtonsFrame->setLayout(pFlowButtonsHorizontalLayout);
  /* Threads Label */
  mpThreadsLabel = new Label(tr("Threads:"));
  /* Threads combo box */
  mpThreadsComboBox = new QComboBox;
  mpThreadsComboBox->setEnabled(false);
  connect(mpThreadsComboBox, SIGNAL(currentIndexChanged(int)), SLOT(threadChanged(int)));
  connect(mpDebuggerMainWindow->getGDBAdapter(), SIGNAL(threadInfo(GDBMIValue*,QString)), SLOT(fillThreadComboBox(GDBMIValue*,QString)));
  /* Thread frame */
  QFrame *pThreadFrame = new QFrame;
  QHBoxLayout *pThreadHorizontalLayout = new QHBoxLayout;
  pThreadHorizontalLayout->setContentsMargins(0, 0, 0, 0);
  pThreadHorizontalLayout->setSpacing(0);
  pThreadHorizontalLayout->addWidget(mpThreadsLabel);
  pThreadHorizontalLayout->addWidget(mpThreadsComboBox);
  pThreadFrame->setLayout(pThreadHorizontalLayout);
  // create project status bar
  mpStatusBar = new QStatusBar;
  mpStatusBar->setObjectName("ModelStatusBar");
  mpStatusBar->setStyleSheet("QStatusBar{border-bottom: none;} QStatusBar::item{margin-bottom: -1px;}");
  mpStatusBar->setSizeGripEnabled(false);
  mpStatusBar->addPermanentWidget(pControlButtonsFrame, 0);
  mpStatusBar->addPermanentWidget(pFlowButtonsFrame, 0);
  mpStatusBar->addPermanentWidget(pThreadFrame, 0);
  /* Message Label */
  mpStatusLabel = new Label;
  mpStatusLabel->setElideMode(Qt::ElideMiddle);
  mpStatusBar->addPermanentWidget(mpStatusLabel, 1);
  /* Stack Frames Tree Widget */
  mpStackFramesTreeWidget = new StackFramesTreeWidget(this);
  /* set layout */
  QGridLayout *pMainLayout = new QGridLayout;
  pMainLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  pMainLayout->setContentsMargins(0, 0, 0, 0);
  pMainLayout->setSpacing(0);
  pMainLayout->addWidget(mpStatusBar, 0, 0);
  pMainLayout->addWidget(mpStackFramesTreeWidget, 1, 0);
  setLayout(pMainLayout);
  /* Make connections to control the enable/disable of tool buttons */
  connect(mpDebuggerMainWindow->getGDBAdapter(), SIGNAL(GDBProcessStarted()), SLOT(handleGDBProcessStarted()));
  connect(mpDebuggerMainWindow->getGDBAdapter(), SIGNAL(GDBProcessFinished()), SLOT(handleGDBProcessFinished()));
  connect(mpDebuggerMainWindow->getGDBAdapter(), SIGNAL(inferiorSuspended()), SLOT(handleInferiorSuspended()));
  connect(mpDebuggerMainWindow->getGDBAdapter(), SIGNAL(inferiorResumed()), SLOT(handleInferiorResumed()));
}

void StackFramesWidget::setStatusMessage(QString statusMessage)
{
  mpStatusLabel->setText(statusMessage);
}

/*!
  Slot activated when clicked signal of StackFramesWidget::mpResumeToolButton is raised.
  Resumes the GDB inferior process. Sends the -exec-continue command to GDB.
  */
void StackFramesWidget::resumeButtonClicked()
{
  mpDebuggerMainWindow->getGDBAdapter()->postCommand(CommandFactory::execContinue());
}

/*!
  Slot activated when clicked signal of StackFramesWidget::mpInterruptToolButton is raised.
  Sends the interrupt signal to GDB inferior process. Sends the -exec-interrupt command to GDB.
  */
void StackFramesWidget::interruptButtonClicked()
{

}

/*!
  Slot activated when clicked signal of StackFramesWidget::mpExitToolButton is raised.
  Stops the GDB inferior process. Sends the -gdb-exit command to GDB.
  */
void StackFramesWidget::exitButtonClicked()
{
  mpDebuggerMainWindow->getGDBAdapter()->deleteCatchOMCBreakpoint();
  mpDebuggerMainWindow->getGDBAdapter()->postCommand(CommandFactory::GDBExit());
  mpDebuggerMainWindow->getGDBAdapter()->setGDBKilled(true);
}

/*!
  Slot activated when clicked signal of StackFramesWidget::mpStepOverToolButton is raised.
  Sends the -exec-next command to GDB.
  */
void StackFramesWidget::stepOverButtonClicked()
{
  mpResumeToolButton->setEnabled(false);
  mpInterruptToolButton->setEnabled(false);
  mpStepOverToolButton->setEnabled(false);
  mpStepIntoToolButton->setEnabled(false);
  mpStepReturnToolButton->setEnabled(false);
  mpThreadsComboBox->setEnabled(false);
  mpDebuggerMainWindow->getGDBAdapter()->enableCatchOMCBreakpoint();
  mpDebuggerMainWindow->getGDBAdapter()->postCommand(CommandFactory::execNext());
}

/*!
  Slot activated when clicked signal of StackFramesWidget::mpStepIntoToolButton is raised.
  Sends the -exec-step command to GDB.
  */
void StackFramesWidget::stepIntoButtonClicked()
{
  mpResumeToolButton->setEnabled(false);
  mpInterruptToolButton->setEnabled(false);
  mpStepOverToolButton->setEnabled(false);
  mpStepIntoToolButton->setEnabled(false);
  mpStepReturnToolButton->setEnabled(false);
  mpThreadsComboBox->setEnabled(false);
  mpDebuggerMainWindow->getGDBAdapter()->enableCatchOMCBreakpoint();
  mpDebuggerMainWindow->getGDBAdapter()->postCommand(CommandFactory::execStep());
}

/*!
  Slot activated when clicked signal of StackFramesWidget::mpStepReturnToolButton is raised.
  Sends the -exec-finish command to GDB.
  */
void StackFramesWidget::stepReturnButtonClicked()
{
  mpResumeToolButton->setEnabled(false);
  mpInterruptToolButton->setEnabled(false);
  mpStepOverToolButton->setEnabled(false);
  mpStepIntoToolButton->setEnabled(false);
  mpStepReturnToolButton->setEnabled(false);
  mpThreadsComboBox->setEnabled(false);
  mpDebuggerMainWindow->getGDBAdapter()->postCommand(CommandFactory::execFinish());
}

/*!
  Slot activated when GDBProcessStarted signal of GDBAdapter is raised.
  Enables the GDB exit button.
  */
void StackFramesWidget::handleGDBProcessStarted()
{
  mpExitToolButton->setEnabled(true);
  setStatusMessage("Debugging started");
}

/*!
  Slot activated when GDBProcessFinished signal of GDBAdapter is raised.
  Disables the program manipulation buttons.
  */
void StackFramesWidget::handleGDBProcessFinished()
{
  mpResumeToolButton->setEnabled(false);
  mpInterruptToolButton->setEnabled(false);
  mpExitToolButton->setEnabled(false);
  mpStepOverToolButton->setEnabled(false);
  mpStepIntoToolButton->setEnabled(false);
  mpStepReturnToolButton->setEnabled(false);
  mpStackFramesTreeWidget->clearStackFrames();
  mpThreadsComboBox->clear();
  setStatusMessage("Debugging finished");
}

/*!
  Slot activated when inferiorSuspended signal of GDBAdapter is raised.
  Enables the program stepping buttons.
  */
void StackFramesWidget::handleInferiorSuspended()
{
  mpResumeToolButton->setEnabled(true);
  mpInterruptToolButton->setEnabled(false);
  mpExitToolButton->setEnabled(true);
  mpStepOverToolButton->setEnabled(true);
  mpStepIntoToolButton->setEnabled(true);
  mpStepReturnToolButton->setEnabled(true);
  mpThreadsComboBox->setEnabled(true);
}

/*!
  Slot activated when inferiorResumed signal of GDBAdapter is raised.
  Disables the program stepping buttons.
  */
void StackFramesWidget::handleInferiorResumed()
{
  mpResumeToolButton->setEnabled(false);
  mpInterruptToolButton->setEnabled(true);
  mpExitToolButton->setEnabled(true);
  mpStepOverToolButton->setEnabled(false);
  mpStepIntoToolButton->setEnabled(false);
  mpStepReturnToolButton->setEnabled(false);
  mpThreadsComboBox->setEnabled(false);
}

/*!
  Slot activated when currentIndexChanged signal of mpThreadsComboBox is raised.
  Marks the selected thread as current thread in GDB.
  */
void StackFramesWidget::threadChanged(int threadIndex)
{
  if (threadIndex < 0)
    return;

  QByteArray cmd = CommandFactory::threadSelect(mpThreadsComboBox->itemText(threadIndex).toInt());
  mpDebuggerMainWindow->getGDBAdapter()->postCommand(cmd, &GDBAdapter::threadSelectCB);
}

/*!
  Slot activated when threadInfo signal of GDBAdapter is raised.
  Populates the mpThreadsComboBox with the number of threads.
  */
/*
  -thread-info
   ^done,threads=[
   {id="2",target-id="Thread 0xb7e14b90 (LWP 21257)",
      frame={level="0",addr="0xffffe410",func="__kernel_vsyscall",
              args=[]},state="running"},
   {id="1",target-id="Thread 0xb7e156b0 (LWP 21254)",
      frame={level="0",addr="0x0804891f",func="foo",
              args=[{name="i",value="10"}],
              file="/tmp/a.c",fullname="/tmp/a.c",line="158"},
              state="running"}],
   current-thread-id="1"
  */
void StackFramesWidget::fillThreadComboBox(GDBMIValue *pThreadsGDBMIValue, QString currentThreadId)
{
  const bool state = mpThreadsComboBox->blockSignals(true);
  /* Clear the mpThreadsComboBox items */
  mpThreadsComboBox->clear();
  /* fill the mpThreadsComboBox the threads */
  if(pThreadsGDBMIValue->type == GDBMIValue::ListValue)
  {
    GDBMIValueList::iterator valuesListiterator;
    for (valuesListiterator = pThreadsGDBMIValue->miList->miValuesList.begin(); valuesListiterator != pThreadsGDBMIValue->miList->miValuesList.end(); ++valuesListiterator)
    {
      GDBMIValue *pGDBMIValue = *valuesListiterator;
      QString threadId;
      if (pGDBMIValue->type == GDBMIValue::TupleValue)
      {
        GDBMIResultList resultsList = pGDBMIValue->miTuple->miResultsList;
        threadId = mpDebuggerMainWindow->getGDBAdapter()->getGDBMIConstantValue(mpDebuggerMainWindow->getGDBAdapter()->getGDBMIResult("id", resultsList));
        if (!threadId.isEmpty())
        {
          mpThreadsComboBox->addItem(threadId);
        }
      }
    }
    mpThreadsComboBox->model()->sort(0);
  }
  /* select the current thread id */
  int currentIndex = mpThreadsComboBox->findText(currentThreadId, Qt::MatchExactly);
  if (currentIndex > -1)
    mpThreadsComboBox->setCurrentIndex(currentIndex);
  mpThreadsComboBox->blockSignals(state);
}

/*!
  Slot activated when itemDoubleClicked signal of StackFramesTreeWidget is raised.
  Sends the -stack-list-variables command.
  */
bool StackFramesWidget::stackItemDoubleClicked(QTreeWidgetItem *pQTreeWidgetItem)
{
  StackFrameItem *pStackFrameItem = dynamic_cast<StackFrameItem*>(pQTreeWidgetItem);
  if (pStackFrameItem)
  {
    if (pStackFrameItem->isDisabled())
      return false;
    mpStackFramesTreeWidget->setCurrentStackFrame(pStackFrameItem);
    mpDebuggerMainWindow->getGDBAdapter()->postCommand(CommandFactory::stackListVariables("--simple-values"), &GDBAdapter::stackListVariablesCB);
    /* Get the stack frame location and mark the line highlighted. */
    mpDebuggerMainWindow->readFileAndNavigateToLine(pStackFrameItem->getFile(), pStackFrameItem->getLine());
    return true;
  }
  return false;
}
