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
 *
 */

#ifndef LIBRARYTREEWIDGET_H
#define LIBRARYTREEWIDGET_H

#include "MainWindow.h"
#include "StringHandler.h"

class ItemDelegate : public QItemDelegate
{
  Q_OBJECT
private:
  bool mDrawRichText;
  QPoint mLastTextPos;
  bool mDrawGrid;
  QColor mGridColor;
  QObject *mpParent;
public:
  ItemDelegate(QObject *pParent = 0, bool drawRichText = false, bool drawGrid = false);
  QColor getGridColor() {return mGridColor;}
  void setGridColor(QColor color) {mGridColor = color;}
  QString formatDisplayText(QVariant variant) const;
  void initTextDocument(QTextDocument *pTextDocument, QFont font, int width) const;
  virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
  void drawHover(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
  virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
  virtual bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
};

class ModelWidget;
class ShapeAnnotation;
class Component;
class LineAnnotation;
class LibraryTreeModel;
class LibraryTreeItem : public QObject
{
  Q_OBJECT
public:
  enum LibraryType {
    Modelica,   /* Used to represent Modelica models. */
    Text,       /* Used to represent text based files. */
    TLM         /* Used to represent TLM files. */
  };
  enum SaveContentsType {
    SaveInOneFile,
    SaveFolderStructure
  };
  LibraryTreeItem();
  LibraryTreeItem(LibraryType type, QString text, QString nameStructure, OMCInterface::getClassInformation_res classInformation,
                  QString fileName, bool isSaved, LibraryTreeItem *pParent = 0);
  ~LibraryTreeItem();
  bool isRootItem() {return mIsRootItem;}
  QList<LibraryTreeItem*> getChildren() const {return mChildren;}
  LibraryType getLibraryType() {return mLibraryType;}
  void setLibraryType(LibraryType libraryType) {mLibraryType = libraryType;}
  void setSystemLibrary(bool systemLibrary) {mSystemLibrary = systemLibrary;}
  bool isSystemLibrary() {return mSystemLibrary;}
  void setModelWidget(ModelWidget *pModelWidget) {mpModelWidget = pModelWidget;}
  ModelWidget* getModelWidget() {return mpModelWidget;}
  void setName(QString name) {mName = name;}
  const QString& getName() const {return mName;}
  void setNameStructure(QString nameStructure) {mNameStructure = nameStructure;}
  const QString& getNameStructure() {return mNameStructure;}
  void setClassInformation(OMCInterface::getClassInformation_res classInformation);
  void setFileName(QString fileName) {mFileName = fileName;}
  const QString& getFileName() {return mFileName;}
  bool isFilePathValid();
  void setReadOnly(bool readOnly) {mReadOnly = readOnly;}
  bool isReadOnly() {return mReadOnly;}
  void setIsSaved(bool isSaved) {mIsSaved = isSaved;}
  bool isSaved() {return mIsSaved;}
  bool isProtected() {return mClassInformation.isProtectedClass;}
  bool isDocumentationClass();
  StringHandler::ModelicaClasses getRestriction() {return StringHandler::getModelicaClassType(mClassInformation.restriction);}
  bool isConnector() {return (getRestriction() == StringHandler::ExpandableConnector || getRestriction() == StringHandler::Connector);}
  bool isPartial() {return mClassInformation.partialPrefix;}
  void setSaveContentsType(LibraryTreeItem::SaveContentsType saveContentsType) {mSaveContentsType = saveContentsType;}
  SaveContentsType getSaveContentsType() {return mSaveContentsType;}
  void setToolTip(QString toolTip) {mToolTip = toolTip;}
  void setIcon(QIcon icon) {mIcon = icon;}
  void setPixmap(QPixmap pixmap) {mPixmap = pixmap;}
  QPixmap getPixmap() {return mPixmap;}
  void setDragPixmap(QPixmap dragPixmap) {mDragPixmap = dragPixmap;}
  QPixmap getDragPixmap() {return mDragPixmap;}
  void setClassTextBefore(QString classTextBefore) {mClassTextBefore = classTextBefore;}
  QString getClassTextBefore() {return mClassTextBefore;}
  void setClassText(QString classText) {mClassText = classText;}
  QString getClassText(LibraryTreeModel *pLibraryTreeModel);
  void setClassTextAfter(QString classTextAfter) {mClassTextAfter = classTextAfter;}
  QString getClassTextAfter() {return mClassTextAfter;}
  void setExpanded(bool expanded) {mExpanded = expanded;}
  bool isExpanded() const {return mExpanded;}
  void setNonExisting(bool nonExisting) {mNonExisting = nonExisting;}
  bool isNonExisting() const {return mNonExisting;}
  void updateAttributes();
  QIcon getLibraryTreeItemIcon();
  bool inRange(int lineNumber) {return (lineNumber >= mClassInformation.lineNumberStart) && (lineNumber <= mClassInformation.lineNumberEnd);}
  bool isInPackageOneFile();
  void insertChild(int position, LibraryTreeItem *pLibraryTreeItem);
  LibraryTreeItem* child(int row);
  void moveChild(int from, int to);
  void addInheritedClass(LibraryTreeItem *pLibraryTreeItem);
  void removeInheritedClasses();
  QList<LibraryTreeItem*> getInheritedClasses() const {return mInheritedClasses;}
  void removeChild(LibraryTreeItem *pLibraryTreeItem);
  QVariant data(int column, int role = Qt::DisplayRole) const;
  int row() const;
  void setParent(LibraryTreeItem *pParentLibraryTreeItem) {mpParentLibraryTreeItem = pParentLibraryTreeItem;}
  LibraryTreeItem* parent() {return mpParentLibraryTreeItem;}
  bool isTopLevel();
  bool isSimulationAllowed();
  void emitLoaded();
  void emitUnLoaded();
  void emitShapeAdded(ShapeAnnotation *pShapeAnnotation, GraphicsView *pGraphicsView);
  void emitComponentAdded(Component *pComponent);
  void emitConnectionAdded(LineAnnotation *pConnectionLineAnnotation) {emit connectionAdded(pConnectionLineAnnotation);}

  OMCInterface::getClassInformation_res mClassInformation;
private:
  bool mIsRootItem;
  LibraryTreeItem *mpParentLibraryTreeItem;
  QList<LibraryTreeItem*> mChildren;
  QList<LibraryTreeItem*> mInheritedClasses;
  LibraryType mLibraryType;
  bool mSystemLibrary;
  ModelWidget *mpModelWidget;
  QString mName;
  QString mParentName;
  QString mNameStructure;
  QString mFileName;
  bool mReadOnly;
  bool mIsSaved;
  SaveContentsType mSaveContentsType;
  QString mToolTip;
  QIcon mIcon;
  QPixmap mPixmap;
  QPixmap mDragPixmap;
  QString mClassTextBefore;
  QString mClassText;
  QString mClassTextAfter;
  bool mExpanded;
  bool mNonExisting;
signals:
  void loaded(LibraryTreeItem *pLibraryTreeItem);
  void loadedForComponent();
  void unLoaded();
  void unLoadedForComponent();
  void shapeAdded(ShapeAnnotation *pShapeAnnotation, GraphicsView *pGraphicsView);
  void shapeAddedForComponent();
  void componentAdded(Component *pComponent);
  void componentAddedForComponent();
  void connectionAdded(LineAnnotation *pConnectionLineAnnotation);
  void iconUpdated();
public slots:
  void handleLoaded(LibraryTreeItem *pLibraryTreeItem);
  void handleUnloaded();
  void handleShapeAdded(ShapeAnnotation *pShapeAnnotation, GraphicsView *pGraphicsView);
  void handleComponentAdded(Component *pComponent);
  void handleConnectionAdded(LineAnnotation *pConnectionLineAnnotation);
  void handleIconUpdated();
};

class LibraryWidget;
class LibraryTreeProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT
public:
  LibraryTreeProxyModel(LibraryWidget *pLibraryWidget);
private:
  LibraryWidget *mpLibraryWidget;
protected:
  virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
};

class LibraryTreeModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  LibraryTreeModel(LibraryWidget *pLibraryWidget);
  LibraryTreeItem* getRootLibraryTreeItem() {return mpRootLibraryTreeItem;}
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
  QModelIndex parent(const QModelIndex & index) const;
  QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
  Qt::ItemFlags flags(const QModelIndex &index) const;
  LibraryTreeItem* findLibraryTreeItem(const QString &name, LibraryTreeItem *root = 0, Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitive) const;
  LibraryTreeItem* findLibraryTreeItem(const QRegExp &regExp, LibraryTreeItem *root = 0) const;
  LibraryTreeItem* findNonExistingLibraryTreeItem(const QString &name, Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitive) const;
  QModelIndex libraryTreeItemIndex(const LibraryTreeItem *pLibraryTreeItem) const;
  void addModelicaLibraries(QSplashScreen *pSplashScreen);
  void createLibraryTreeItems(LibraryTreeItem *pLibraryTreeItem);
  LibraryTreeItem* createLibraryTreeItem(QString name, LibraryTreeItem *pParentLibraryTreeItem, bool isSaved = true,
                                         bool isSystemLibrary = false, bool load = false, int row = -1);
  LibraryTreeItem* createLibraryTreeItem(LibraryTreeItem::LibraryType type, QString name, bool isSaved);
  LibraryTreeItem* createNonExistingLibraryTreeItem(QString nameStructure);
  void createNonExistingLibraryTreeItem(LibraryTreeItem *pLibraryTreeItem, LibraryTreeItem *pParentLibraryTreeItem, bool isSaved = true,
                                        int row = -1);
  void loadNonExistingLibraryTreeItem(LibraryTreeItem *pLibraryTreeItem);
  void checkIfAnyNonExistingClassLoaded();
  void addNonExistingLibraryTreeItem(LibraryTreeItem *pLibraryTreeItem) {mNonExistingLibraryTreeItemsList.append(pLibraryTreeItem);}
  void removeNonExistingLibraryTreeItem(LibraryTreeItem *pLibraryTreeItem) {mNonExistingLibraryTreeItemsList.removeOne(pLibraryTreeItem);}
  void updateLibraryTreeItem(LibraryTreeItem *pLibraryTreeItem);
  void updateLibraryTreeItemClassText(LibraryTreeItem *pLibraryTreeItem);
  void updateLibraryTreeItemClassTextManually(LibraryTreeItem *pLibraryTreeItem, QString contents);
  void readLibraryTreeItemClassText(LibraryTreeItem *pLibraryTreeItem);
  LibraryTreeItem* getContainingFileParentLibraryTreeItem(LibraryTreeItem *pLibraryTreeItem);
  void loadLibraryTreeItemPixmap(LibraryTreeItem *pLibraryTreeItem);
  void loadDependentLibraries(QStringList libraries);
  LibraryTreeItem* getLibraryTreeItemFromFile(QString fileName, int lineNumber);
  void showModelWidget(LibraryTreeItem *pLibraryTreeItem, QString text = QString(""), bool show = true);
  void showHideProtectedClasses();
  bool unloadClass(LibraryTreeItem *pLibraryTreeItem, bool askQuestion = true);
  bool unloadTLMOrTextFile(LibraryTreeItem *pLibraryTreeItem, bool askQuestion = true);
  bool unloadLibraryTreeItem(LibraryTreeItem *pLibraryTreeItem, bool doDeleteClass);
  bool removeLibraryTreeItem(LibraryTreeItem *pLibraryTreeItem);
  void moveClassUpDown(LibraryTreeItem *pLibraryTreeItem, bool up);
  void moveClassTopBottom(LibraryTreeItem *pLibraryTreeItem, bool top);
  void updateBindings(LibraryTreeItem *pLibraryTreeItem);
  QString getUniqueTopLevelItemName(QString name, int number = 1);
  void emitDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight) {emit dataChanged(topLeft, bottomRight);}
private:
  LibraryWidget *mpLibraryWidget;
  LibraryTreeItem *mpRootLibraryTreeItem;
  QList<LibraryTreeItem*> mNonExistingLibraryTreeItemsList;
  QModelIndex libraryTreeItemIndexHelper(const LibraryTreeItem *pLibraryTreeItem, const LibraryTreeItem *pParentLibraryTreeItem,
                                         const QModelIndex &parentIndex) const;
  LibraryTreeItem* getLibraryTreeItemFromFileHelper(LibraryTreeItem *pLibraryTreeItem, QString fileName, int lineNumber);
  void updateChildLibraryTreeItemClassText(LibraryTreeItem *pLibraryTreeItem, QString contents, QString fileName);
  void readLibraryTreeItemClassTextFromText(LibraryTreeItem *pLibraryTreeItem, QString contents);
  QString readLibraryTreeItemClassTextFromFile(LibraryTreeItem *pLibraryTreeItem);
  void unloadClassHelper(LibraryTreeItem *pLibraryTreeItem, LibraryTreeItem *pParentLibraryTreeItem);
  void unloadClassChildren(LibraryTreeItem *pLibraryTreeItem);
protected:
  Qt::DropActions supportedDropActions() const;
};

class LibraryTreeView : public QTreeView
{
  Q_OBJECT
public:
  LibraryTreeView(LibraryWidget *pLibraryWidget);
  LibraryWidget* getLibraryWidget() {return mpLibraryWidget;}
private:
  LibraryWidget *mpLibraryWidget;
  QAction *mpOpenClassAction;
  QAction *mpViewDocumentationAction;
  QAction *mpNewModelicaClassAction;
  QAction *mpSaveAction;
  QAction *mpSaveAsAction;
  QAction *mpSaveTotalAction;
  QAction *mpMoveUpAction;
  QAction *mpMoveDownAction;
  QAction *mpMoveTopAction;
  QAction *mpMoveBottomAction;
  QMenu *mpOrderMenu;
  QAction *mpInstantiateModelAction;
  QAction *mpCheckModelAction;
  QAction *mpCheckAllModelsAction;
  QAction *mpSimulateAction;
  QAction *mpSimulateWithTransformationalDebuggerAction;
  QAction *mpSimulateWithAlgorithmicDebuggerAction;
  QAction *mpSimulationSetupAction;
  QAction *mpDuplicateClassAction;
  QAction *mpUnloadClassAction;
  QAction *mpUnloadTLMFileAction;
  QAction *mpExportFMUAction;
  QAction *mpExportXMLAction;
  QAction *mpExportFigaroAction;
  QAction *mpUpdateBindingsAction;
  QAction *mpFetchInterfaceDataAction;
  QAction *mpTLMCoSimulationAction;
  void createActions();
  LibraryTreeItem* getSelectedLibraryTreeItem();
  void libraryTreeItemExpanded(LibraryTreeItem* pLibraryTreeItem);
public slots:
  void libraryTreeItemExpanded(QModelIndex index);
  void showContextMenu(QPoint point);
  void openClass();
  void viewDocumentation();
  void createNewModelicaClass();
  void saveClass();
  void saveAsClass();
  void saveTotalClass();
  void moveClassUp();
  void moveClassDown();
  void moveClassTop();
  void moveClassBottom();
  void instantiateModel();
  void checkModel();
  void checkAllModels();
  void simulate();
  void simulateWithTransformationalDebugger();
  void simulateWithAlgorithmicDebugger();
  void simulationSetup();
  void duplicateClass();
  void unloadClass();
  void unloadTLMOrTextFile();
  void exportModelFMU();
  void exportModelXML();
  void exportModelFigaro();
  void updateBindings();
  void fetchInterfaceData();
  void TLMSimulate();
protected:
  virtual void mouseDoubleClickEvent(QMouseEvent *event);
  virtual void startDrag(Qt::DropActions supportedActions);
  virtual void keyPressEvent(QKeyEvent *event);
};

class MainWindow;
class LibraryWidget : public QWidget
{
  Q_OBJECT
public:
  LibraryWidget(MainWindow *pMainWindow);
  MainWindow* getMainWindow() {return mpMainWindow;}
  TreeSearchFilters* getTreeSearchFilters() {return mpTreeSearchFilters;}
  LibraryTreeModel* getLibraryTreeModel() {return mpLibraryTreeModel;}
  LibraryTreeProxyModel* getLibraryTreeProxyModel() {return mpLibraryTreeProxyModel;}
  LibraryTreeView* getLibraryTreeView() {return mpLibraryTreeView;}
  void openFile(QString fileName, QString encoding = Helper::utf8, bool showProgress = true, bool checkFileExists = false);
  void openModelicaFile(QString fileName, QString encoding = Helper::utf8, bool showProgress = true);
  void openTLMOrTextFile(QFileInfo fileInfo, bool showProgress = true);
  void parseAndLoadModelicaText(QString modelText);
  bool saveLibraryTreeItem(LibraryTreeItem *pLibraryTreeItem);
  void saveAsLibraryTreeItem(LibraryTreeItem *pLibraryTreeItem);
  bool saveTotalLibraryTreeItem(LibraryTreeItem *pLibraryTreeItem);
  void openLibraryTreeItem(QString nameStructure);
private:
  MainWindow *mpMainWindow;
  TreeSearchFilters *mpTreeSearchFilters;
  LibraryTreeModel *mpLibraryTreeModel;
  LibraryTreeProxyModel *mpLibraryTreeProxyModel;
  LibraryTreeView *mpLibraryTreeView;
  bool saveFile(QString fileName, QString contents);
  bool saveModelicaLibraryTreeItem(LibraryTreeItem *pLibraryTreeItem);
  bool saveModelicaLibraryTreeItemHelper(LibraryTreeItem *pLibraryTreeItem);
  bool saveModelicaLibraryTreeItemOneFile(LibraryTreeItem *pLibraryTreeItem);
  void saveChildLibraryTreeItemsOneFile(LibraryTreeItem *pLibraryTreeItem);
  void saveChildLibraryTreeItemsOneFileHelper(LibraryTreeItem *pLibraryTreeItem);
  bool saveModelicaLibraryTreeItemFolder(LibraryTreeItem *pLibraryTreeItem);
  bool saveTextLibraryTreeItem(LibraryTreeItem *pLibraryTreeItem);
  bool saveTLMLibraryTreeItem(LibraryTreeItem *pLibraryTreeItem);
  bool saveTotalLibraryTreeItemHelper(LibraryTreeItem *pLibraryTreeItem);
public slots:
  void searchClasses();
};

#endif // LIBRARYTREEWIDGET_H
