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

#include "VariablesWidget.h"
#include "util/read_matlab4.h"

using namespace OMPlot;

/*!
  \class VariablesTreeItem
  \brief Contains the information about the result variable.
  */
/*!
  \param variableItemData - a list of items.\n
  0 -> filePath\n
  1 -> fileName\n
  2 -> name\n
  3 -> displayName\n
  4 -> value\n
  5 -> unit\n
  6 -> displayUnit\n
  7 -> description\n
  8 -> tooltip
  */
VariablesTreeItem::VariablesTreeItem(const QVector<QVariant> &variableItemData, VariablesTreeItem *pParent, bool isRootItem)
{
  mpParentVariablesTreeItem = pParent;
  mIsRootItem = isRootItem;
  mFilePath = variableItemData[0].toString();
  mFileName = variableItemData[1].toString();
  mVariableName = variableItemData[2].toString();
  mDisplayVariableName = variableItemData[3].toString();
  mValue = variableItemData[4].toString();
  mValueChanged = false;
  mUnit = variableItemData[5].toString();
  mDisplayUnit = variableItemData[6].toString();
  mDescription = variableItemData[7].toString();
  mToolTip = variableItemData[8].toString();
  mChecked = false;
  mEditable = false;
}

VariablesTreeItem::~VariablesTreeItem()
{
  qDeleteAll(mChildren);
  mChildren.clear();
}

QString VariablesTreeItem::getPlotVariable()
{
  return QString(mVariableName).remove(0, mFileName.length() + 1);
}

QIcon VariablesTreeItem::getVariableTreeItemIcon(QString name) const
{
  if (name.endsWith(".mat"))
    return QIcon(":/Resources/icons/mat.svg");
  else if (name.endsWith(".plt"))
    return QIcon(":/Resources/icons/plt.svg");
  else if (name.endsWith(".csv"))
    return QIcon(":/Resources/icons/csv.svg");
  else
    return QIcon(":/Resources/icons/mat.svg");
}

void VariablesTreeItem::insertChild(int position, VariablesTreeItem *pVariablesTreeItem)
{
  mChildren.insert(position, pVariablesTreeItem);
}

VariablesTreeItem* VariablesTreeItem::child(int row)
{
  return mChildren.value(row);
}

void VariablesTreeItem::removeChildren()
{
  qDeleteAll(mChildren);
  mChildren.clear();
}

void VariablesTreeItem::removeChild(VariablesTreeItem *pVariablesTreeItem)
{
  mChildren.removeOne(pVariablesTreeItem);
}

int VariablesTreeItem::columnCount() const
{
  return 4;
}

bool VariablesTreeItem::setData(int column, const QVariant &value, int role)
{
  if (column == 0 && role == Qt::CheckStateRole) {
    if (value.toInt() == Qt::Checked) {
      setChecked(true);
    } else if (value.toInt() == Qt::Unchecked) {
      setChecked(false);
    }
    return true;
  } else if (column == 1 && role == Qt::EditRole) {
    if (mValue.compare(value.toString()) != 0) {
      mValueChanged = true;
      mValue = value.toString();
    }
    return true;
  }
  return false;
}

QVariant VariablesTreeItem::data(int column, int role) const
{
  switch (column) {
    case 0:
      switch (role) {
        case Qt::DisplayRole:
          return mDisplayVariableName;
        case Qt::DecorationRole:
          return mIsRootItem ? getVariableTreeItemIcon(mVariableName) : QIcon();
        case Qt::ToolTipRole:
          return mToolTip;
        case Qt::CheckStateRole:
          if (mChildren.size() > 0)
            return QVariant();
          else
            return isChecked() ? Qt::Checked : Qt::Unchecked;
        default:
          return QVariant();
      }
    case 1:
      switch (role) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
        case Qt::EditRole:
          return mValue;
        default:
          return QVariant();
      }
    case 2:
      switch (role) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
          return mUnit;
        default:
          return QVariant();
      }
    case 3:
      switch (role) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
          return mDescription;
        default:
          return QVariant();
      }
    default:
      return QVariant();
  }
}

int VariablesTreeItem::row() const
{
  if (mpParentVariablesTreeItem)
    return mpParentVariablesTreeItem->mChildren.indexOf(const_cast<VariablesTreeItem*>(this));

  return 0;
}

VariablesTreeItem* VariablesTreeItem::parent()
{
  return mpParentVariablesTreeItem;
}

VariablesTreeItem* VariablesTreeItem::rootParent()
{
  // since we have global mpRootVariablesTreeItem so we return one level down from this function in order to get the top level item.
  VariablesTreeItem *pVariablesTreeItem, *pVariablesTreeItem1;
  pVariablesTreeItem = this;
  pVariablesTreeItem1 = this;
  while (pVariablesTreeItem->mpParentVariablesTreeItem) {
    pVariablesTreeItem1 = pVariablesTreeItem;
    pVariablesTreeItem = pVariablesTreeItem->mpParentVariablesTreeItem;
  }
  return pVariablesTreeItem1;
}

VariablesTreeModel::VariablesTreeModel(VariablesTreeView *pVariablesTreeView)
  : QAbstractItemModel(pVariablesTreeView)
{
  mpVariablesTreeView = pVariablesTreeView;
  QVector<QVariant> headers;
  headers << "" << "" << Helper::variables << Helper::variables << tr("Value") << tr("Unit") << "" << Helper::description << "";
  mpRootVariablesTreeItem = new VariablesTreeItem(headers, 0, true);
}

int VariablesTreeModel::columnCount(const QModelIndex &parent) const
{
  if (parent.isValid())
    return static_cast<VariablesTreeItem*>(parent.internalPointer())->columnCount();
  else
    return mpRootVariablesTreeItem->columnCount();
}

int VariablesTreeModel::rowCount(const QModelIndex &parent) const
{
  VariablesTreeItem *pParentVariablesTreeItem;
  if (parent.column() > 0)
    return 0;

  if (!parent.isValid())
    pParentVariablesTreeItem = mpRootVariablesTreeItem;
  else
    pParentVariablesTreeItem = static_cast<VariablesTreeItem*>(parent.internalPointer());
  return pParentVariablesTreeItem->getChildren().size();
}

QVariant VariablesTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    return mpRootVariablesTreeItem->data(section);
  return QVariant();
}

QModelIndex VariablesTreeModel::index(int row, int column, const QModelIndex &parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  VariablesTreeItem *pParentVariablesTreeItem;

  if (!parent.isValid())
    pParentVariablesTreeItem = mpRootVariablesTreeItem;
  else
    pParentVariablesTreeItem = static_cast<VariablesTreeItem*>(parent.internalPointer());

  VariablesTreeItem *pChildVariablesTreeItem = pParentVariablesTreeItem->child(row);
  if (pChildVariablesTreeItem)
    return createIndex(row, column, pChildVariablesTreeItem);
  else
    return QModelIndex();
}

QModelIndex VariablesTreeModel::parent(const QModelIndex &index) const
{
  if (!index.isValid())
    return QModelIndex();

  VariablesTreeItem *pChildVariablesTreeItem = static_cast<VariablesTreeItem*>(index.internalPointer());
  VariablesTreeItem *pParentVariablesTreeItem = pChildVariablesTreeItem->parent();
  if (pParentVariablesTreeItem == mpRootVariablesTreeItem)
    return QModelIndex();

  return createIndex(pParentVariablesTreeItem->row(), 0, pParentVariablesTreeItem);
}

bool VariablesTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  VariablesTreeItem *pVariablesTreeItem = static_cast<VariablesTreeItem*>(index.internalPointer());
  if (!pVariablesTreeItem)
    return false;
  bool result = pVariablesTreeItem->setData(index.column(), value, role);
  if (index.column() == 0 && role == Qt::CheckStateRole)
  {
    if (!signalsBlocked())
    {
      PlottingPage *pPlottingPage = mpVariablesTreeView->getVariablesWidget()->getMainWindow()->getOptionsDialog()->getPlottingPage();
      emit itemChecked(index, pPlottingPage->getCurveThickness(), pPlottingPage->getCurvePattern());
    }
  }
  emit dataChanged(index, index);
  return result;
}

QVariant VariablesTreeModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  VariablesTreeItem *pVariablesTreeItem = static_cast<VariablesTreeItem*>(index.internalPointer());
  return pVariablesTreeItem->data(index.column(), role);
}

Qt::ItemFlags VariablesTreeModel::flags(const QModelIndex &index) const
{
  if (!index.isValid())
      return 0;

  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  VariablesTreeItem *pVariablesTreeItem = static_cast<VariablesTreeItem*>(index.internalPointer());
  if (index.column() == 0 && pVariablesTreeItem && pVariablesTreeItem->getChildren().size() == 0)
    flags |= Qt::ItemIsUserCheckable;
  else if (index.column() == 1 && pVariablesTreeItem && pVariablesTreeItem->getChildren().size() == 0 && pVariablesTreeItem->isEditable())
    flags |= Qt::ItemIsEditable;

  return flags;
}

VariablesTreeItem* VariablesTreeModel::findVariablesTreeItem(const QString &name, VariablesTreeItem *root) const
{
  if (root->getVariableName() == name)
    return root;
  for (int i = root->getChildren().size(); --i >= 0; )
    if (VariablesTreeItem *item = findVariablesTreeItem(name, root->getChildren().at(i)))
      return item;
  return 0;
}

QModelIndex VariablesTreeModel::variablesTreeItemIndex(const VariablesTreeItem *pVariablesTreeItem) const
{
  return variablesTreeItemIndexHelper(pVariablesTreeItem, mpRootVariablesTreeItem, QModelIndex());
}

QModelIndex VariablesTreeModel::variablesTreeItemIndexHelper(const VariablesTreeItem *pVariablesTreeItem,
                                                             const VariablesTreeItem *pParentVariablesTreeItem,
                                                             const QModelIndex &parentIndex) const
{
  if (pVariablesTreeItem == pParentVariablesTreeItem)
    return parentIndex;
  for (int i = pParentVariablesTreeItem->getChildren().size(); --i >= 0; ) {
    const VariablesTreeItem *childItem = pParentVariablesTreeItem->getChildren().at(i);
    QModelIndex childIndex = index(i, 0, parentIndex);
    QModelIndex index = variablesTreeItemIndexHelper(pVariablesTreeItem, childItem, childIndex);
    if (index.isValid())
      return index;
  }
  return QModelIndex();
}

void VariablesTreeModel::parseInitXml(QXmlStreamReader &xmlReader)
{
  mScalarVariablesList.clear();
  /* We'll parse the XML until we reach end of it.*/
  while(!xmlReader.atEnd() && !xmlReader.hasError())
  {
    /* Read next element.*/
    QXmlStreamReader::TokenType token = xmlReader.readNext();
    /* If token is just StartDocument, we'll go to next.*/
    if(token == QXmlStreamReader::StartDocument)
      continue;
    /* If token is StartElement, we'll see if we can read it.*/
    if(token == QXmlStreamReader::StartElement)
    {
      /* If it's named ScalarVariable, we'll dig the information from there.*/
      if(xmlReader.name() == "ScalarVariable")
      {
        QHash<QString, QString> scalarVariable = parseScalarVariable(xmlReader);
        mScalarVariablesList.insert(scalarVariable.value("name"),scalarVariable);
      }
    }
  }
  xmlReader.clear();
}

QHash<QString, QString> VariablesTreeModel::parseScalarVariable(QXmlStreamReader &xmlReader)
{
  QHash<QString, QString> scalarVariable;
  /* Let's check that we're really getting a ScalarVariable. */
  if(xmlReader.tokenType() != QXmlStreamReader::StartElement && xmlReader.name() == "ScalarVariable")
    return scalarVariable;
  /* Let's get the attributes for ScalarVariable */
  QXmlStreamAttributes attributes = xmlReader.attributes();
  /* Read the ScalarVariable attributes. */
  scalarVariable["name"] = attributes.value("name").toString();
  scalarVariable["description"] = attributes.value("description").toString();
  scalarVariable["isValueChangeable"] = attributes.value("isValueChangeable").toString();
  /* Read the next element i.e Real, Integer, Boolean etc. */
  xmlReader.readNext();
  while(!(xmlReader.tokenType() == QXmlStreamReader::EndElement && xmlReader.name() == "ScalarVariable"))
  {
    if(xmlReader.tokenType() == QXmlStreamReader::StartElement)
    {
      QXmlStreamAttributes attributes = xmlReader.attributes();
      scalarVariable["start"] = attributes.value("start").toString();
      scalarVariable["unit"] = attributes.value("unit").toString();
      scalarVariable["displayUnit"] = attributes.value("displayUnit").toString();
    }
    xmlReader.readNext();
  }
  return scalarVariable;
}

void VariablesTreeModel::insertVariablesItems(QString fileName, QString filePath, QStringList variablesList,
                                              SimulationOptions simulationOptions)
{
  QString toolTip = tr("Simulation Result File: %1\n%2: %3/%4").arg(fileName).arg(Helper::fileLocation).arg(filePath).arg(fileName);
  QRegExp resultTypeRegExp("(_res.mat|_res.plt|_res.csv)");
  QString text = QString(fileName).remove(resultTypeRegExp);
  QModelIndex index = variablesTreeItemIndex(mpRootVariablesTreeItem);
  QVector<QVariant> Variabledata;
  Variabledata << filePath << fileName << fileName << text << "" << "" << "" << "" << toolTip;
  VariablesTreeItem *pTopVariablesTreeItem = new VariablesTreeItem(Variabledata, mpRootVariablesTreeItem, true);
  pTopVariablesTreeItem->setSimulationOptions(simulationOptions);
  int row = rowCount();
  beginInsertRows(index, row, row);
  mpRootVariablesTreeItem->insertChild(row, pTopVariablesTreeItem);
  endInsertRows();
  /* open the model_init.xml file for reading */
  if (simulationOptions.isValid()) {
    QString initFileName = QString(simulationOptions.getOutputFileName()).append("_init.xml");
    QFile initFile(QString(filePath).append(QDir::separator()).append(initFileName));
    if (initFile.open(QIODevice::ReadOnly)) {
      QXmlStreamReader initXmlReader(&initFile);
      parseInitXml(initXmlReader);
      initFile.close();
    } else {
      MessagesWidget *pMessagesWidget = mpVariablesTreeView->getVariablesWidget()->getMainWindow()->getMessagesWidget();
      pMessagesWidget->addGUIMessage(MessageItem(MessageItem::Modelica, "", false, 0, 0, 0, 0,
                                                 GUIMessages::getMessage(GUIMessages::ERROR_OPENING_FILE).arg(initFile.fileName())
                                                 .arg(initFile.errorString()), Helper::scriptingKind, Helper::errorLevel));
    }
  }
  /* open the .mat file */
  ModelicaMatReader matReader;
  matReader.file = 0;
  const char *msg[] = {""};
  if (fileName.endsWith(".mat"))
  {
    //Read in mat file
    if (0 != (msg[0] = omc_new_matlab4_reader(QString(filePath + "/" + fileName).toStdString().c_str(), &matReader)))
    {
      MessagesWidget *pMessagesWidget = mpVariablesTreeView->getVariablesWidget()->getMainWindow()->getMessagesWidget();
      pMessagesWidget->addGUIMessage(MessageItem(MessageItem::Modelica, "", false, 0, 0, 0, 0,
                                                 GUIMessages::getMessage(GUIMessages::ERROR_OPENING_FILE).arg(fileName)
                                                 .arg(QString(msg[0])), Helper::scriptingKind, Helper::errorLevel));
    }
  }
  QStringList variables;
  foreach (QString plotVariable, variablesList)
  {
    QString parentVariable;
    if (plotVariable.startsWith("der("))
    {
      QString str = plotVariable;
      str.chop((str.lastIndexOf("der(")/4)+1);
      variables = StringHandler::makeVariableParts(str.mid(str.lastIndexOf("der(") + 4));
    }
    else
    {
      variables = StringHandler::makeVariableParts(plotVariable);
    }
    int count = 1;
    VariablesTreeItem *pParentVariablesTreeItem = 0;
    foreach (QString variable, variables)
    {
      if (count == 1) /* first loop iteration */
      {
        pParentVariablesTreeItem = pTopVariablesTreeItem;
      }
      QString findVariable;
      /* if last item */
      if (variables.size() == count && plotVariable.startsWith("der("))
        findVariable = parentVariable.isEmpty() ? fileName + ".der(" + variable + ")" : fileName + "." + parentVariable + ".der(" + variable + ")";
      else
        findVariable = parentVariable.isEmpty() ? fileName + "." + variable : fileName + "." + parentVariable + "." + variable;
      if ((pParentVariablesTreeItem = findVariablesTreeItem(findVariable, pParentVariablesTreeItem)) != NULL)
      {
        if (count == 1)
          parentVariable = variable;
        else
          parentVariable += "." + variable;
        count++;
        continue;
      }
      /*
        If pParentVariablesTreeItem is 0 and it is first loop iteration then use pTopVariablesTreeItem as parent.
        If loop iteration is not first and pParentVariablesTreeItem is 0 then find the parent item.
        */
      if (!pParentVariablesTreeItem && count > 1)
      {
        pParentVariablesTreeItem = findVariablesTreeItem(fileName + "." + parentVariable, pTopVariablesTreeItem);
      }
      else
      {
        pParentVariablesTreeItem = pTopVariablesTreeItem;
      }
      QModelIndex index = variablesTreeItemIndex(pParentVariablesTreeItem);
      QVector<QVariant> variableData;
      /* if last item */
      if (variables.size() == count && plotVariable.startsWith("der("))
        variableData << filePath << fileName << fileName + "." + plotVariable << "der(" + variable + ")";
      else
        variableData << filePath << fileName << pParentVariablesTreeItem->getVariableName() + "." + variable << variable;
      /* find the variable in the xml file */
      QString variableToFind = variableData[2].toString();
      variableToFind.remove(QRegExp(pTopVariablesTreeItem->getVariableName() + "."));
      /* get the variable information i.e value, unit, displayunit, description */
      QString value, unit, displayUnit, description;
      bool changeAble = false;
      getVariableInformation(&matReader, variableToFind, &value, &changeAble, &unit, &displayUnit, &description);
      variableData << StringHandler::unparse(QString("\"").append(value).append("\""));
      /* set the variable unit */
      variableData << StringHandler::unparse(QString("\"").append(unit).append("\""));
      /* set the variable displayUnit */
      variableData << StringHandler::unparse(QString("\"").append(displayUnit).append("\""));
      /* set the variable description */
      variableData << StringHandler::unparse(QString("\"").append(description).append("\""));
      /* construct tooltip text */
      variableData << tr("File: %1/%2\nVariable: %3").arg(filePath).arg(fileName).arg(variableToFind);
      VariablesTreeItem *pVariablesTreeItem = new VariablesTreeItem(variableData, pParentVariablesTreeItem);
      pVariablesTreeItem->setEditable(changeAble);
      int row = rowCount(index);
      beginInsertRows(index, row, row);
      pParentVariablesTreeItem->insertChild(row, pVariablesTreeItem);
      endInsertRows();
      if (count == 1)
        parentVariable = variable;
      else
        parentVariable += "." + variable;
      count++;
    }
  }
  /* close the .mat file */
  if (fileName.endsWith(".mat"))
  {
    if (matReader.file)
      omc_free_matlab4_reader(&matReader);
  }
  mpVariablesTreeView->collapseAll();
  QModelIndex idx = variablesTreeItemIndex(pTopVariablesTreeItem);
  idx = mpVariablesTreeView->getVariablesWidget()->getVariableTreeProxyModel()->mapFromSource(idx);
  mpVariablesTreeView->expand(idx);
  /*
    Ticket #3016.
    If you only have one model the message "You must select a class to re-simulate" is annoying.
    A default behavior of selecting the (single) model would be good.
    The following line selects the result tree top level item.
    */
  mpVariablesTreeView->setCurrentIndex(idx);
}

bool VariablesTreeModel::removeVariableTreeItem(QString variable)
{
  VariablesTreeItem *pVariablesTreeItem = findVariablesTreeItem(variable, mpRootVariablesTreeItem);
  if (pVariablesTreeItem)
  {
    beginRemoveRows(variablesTreeItemIndex(pVariablesTreeItem), 0, pVariablesTreeItem->getChildren().size());
    pVariablesTreeItem->removeChildren();
    VariablesTreeItem *pParentVariablesTreeItem = pVariablesTreeItem->parent();
    pParentVariablesTreeItem->removeChild(pVariablesTreeItem);
    delete pVariablesTreeItem;
    endRemoveRows();
    return true;
  }
  return false;
}

void VariablesTreeModel::unCheckVariables(VariablesTreeItem *pVariablesTreeItem)
{
  QList<VariablesTreeItem*> items = pVariablesTreeItem->getChildren();
  for (int i = 0 ; i < items.size() ; i++) {
    items[i]->setData(0, Qt::Unchecked, Qt::CheckStateRole);
    unCheckVariables(items[i]);
  }
}

void VariablesTreeModel::plotAllVariables(VariablesTreeItem *pVariablesTreeItem, PlotWindow *pPlotWindow)
{
  QList<VariablesTreeItem*> variablesTreeItems = pVariablesTreeItem->getChildren();
  if (variablesTreeItems.size() == 0) {
    QModelIndex index = variablesTreeItemIndex(pVariablesTreeItem);
    OMPlot::PlotCurve *pPlotCurve = 0;
    foreach (OMPlot::PlotCurve *curve, pPlotWindow->getPlot()->getPlotCurvesList()) {
      if (curve->getNameStructure().compare(pVariablesTreeItem->getVariableName()) == 0) {
        pPlotCurve = curve;
        break;
      }
    }
    setData(index, Qt::Checked, Qt::CheckStateRole);
    mpVariablesTreeView->getVariablesWidget()->plotVariables(index, pPlotWindow->getCurveWidth(), pPlotWindow->getCurveStyle(), pPlotCurve);
  } else {
    for (int i = 0 ; i < variablesTreeItems.size() ; i++) {
      plotAllVariables(variablesTreeItems[i], pPlotWindow);
    }
  }
}

void VariablesTreeModel::getVariableInformation(ModelicaMatReader *pMatReader, QString variableToFind, QString *value, bool *changeAble,
                                                QString *unit, QString *displayUnit, QString *description)
{
  QHash<QString, QString> hash = mScalarVariablesList.value(variableToFind);
  if (hash["name"].compare(variableToFind) == 0)
  {
    *changeAble = (hash["isValueChangeable"].compare("true") == 0) ? true : false;
    if (*changeAble)
    {
      *value = hash["start"];
    }
    /* if the variable is not a tunable parameter then read the final value of the variable. Only mat result files are supported. */
    else
    {
      if ((pMatReader->file != NULL) && strcmp(pMatReader->fileName, ""))
      {
        *value = "";
        if (variableToFind.compare("time") == 0)
        {
          *value = QString::number(omc_matlab4_stopTime(pMatReader));
        }
        else
        {
          ModelicaMatVariable_t *var;
          if (0 == (var = omc_matlab4_find_var(pMatReader, variableToFind.toStdString().c_str())))
          {
            qDebug() << QString("%1 not found in %2").arg(variableToFind).arg(pMatReader->fileName);
          }
          double res;
          if (var && !omc_matlab4_val(&res, pMatReader, var, omc_matlab4_stopTime(pMatReader)))
          {
            *value = QString::number(res);
          }
        }
      }
    }
    *unit = hash["unit"];
    *displayUnit = hash["displayUnit"];
    *description = hash["description"];
  }
  else if ((variableToFind.compare("time") == 0) && (pMatReader->file != NULL) && strcmp(pMatReader->fileName, ""))
  {
    *value = QString::number(omc_matlab4_stopTime(pMatReader));
  }
}

void VariablesTreeModel::removeVariableTreeItem()
{
  QAction *pAction = qobject_cast<QAction*>(sender());
  if (pAction)
  {
    removeVariableTreeItem(pAction->data().toString());
    emit variableTreeItemRemoved(pAction->data().toString());
  }
}

VariableTreeProxyModel::VariableTreeProxyModel(QObject *parent)
  : QSortFilterProxyModel(parent)
{
}

bool VariableTreeProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
  if (!filterRegExp().isEmpty())
  {
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    if (index.isValid())
    {
      // if any of children matches the filter, then current index matches the filter as well
      int rows = sourceModel()->rowCount(index);
      for (int i = 0 ; i < rows ; ++i)
      {
        if (filterAcceptsRow(i, index))
        {
          return true;
        }
      }
      // check current index itself
      VariablesTreeItem *pVariablesTreeItem = static_cast<VariablesTreeItem*>(index.internalPointer());
      if (pVariablesTreeItem)
      {
        QString variableName = pVariablesTreeItem->getVariableName();
        variableName.remove(QRegExp("(_res.mat|_res.plt|_res.csv)"));
        return variableName.contains(filterRegExp());
      }
      else
      {
        return sourceModel()->data(index).toString().contains(filterRegExp());
      }
      QString key = sourceModel()->data(index, filterRole()).toString();
      return key.contains(filterRegExp());
    }
  }
  return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}

bool VariableTreeProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
  QVariant l = (left.model() ? left.model()->data(left) : QVariant());
  QVariant r = (right.model() ? right.model()->data(right) : QVariant());
  return StringHandler::naturalSort(l.toString(), r.toString());
}

VariablesTreeView::VariablesTreeView(VariablesWidget *pVariablesWidget)
  : QTreeView(pVariablesWidget)
{
  mpVariablesWidget = pVariablesWidget;
  setObjectName("TreeWithBranches");
  setItemDelegate(new ItemDelegate(this));
  setTextElideMode(Qt::ElideMiddle);
  setIndentation(Helper::treeIndentation);
  setIconSize(Helper::iconSize);
  setContextMenuPolicy(Qt::CustomContextMenu);
  setExpandsOnDoubleClick(false);
}

/*!
  Reimplementation of QTreeView::mouseReleaseEvent\n
  Checks if user clicks on the first column then check/uncheck the corresponsing checkbox of the column.\n
  Otherwise calls the QTreeView::mouseReleaseEvent
  */
void VariablesTreeView::mouseReleaseEvent(QMouseEvent *event)
{
  QModelIndex index = indexAt(event->pos());
  if (index.isValid() &&
      index.column() == 0 &&
      index.parent().isValid() &&
      index.flags() & Qt::ItemIsUserCheckable &&
      event->button() == Qt::LeftButton)
  {
    if (visualRect(index).contains(event->pos()))
    {
      index = mpVariablesWidget->getVariableTreeProxyModel()->mapToSource(index);
      VariablesTreeItem *pVariablesTreeItem = static_cast<VariablesTreeItem*>(index.internalPointer());
      if (pVariablesTreeItem && pVariablesTreeItem)
      {
        if (pVariablesTreeItem->isChecked())
          mpVariablesWidget->getVariablesTreeModel()->setData(index, Qt::Unchecked, Qt::CheckStateRole);
        else
          mpVariablesWidget->getVariablesTreeModel()->setData(index, Qt::Checked, Qt::CheckStateRole);
      }
      return;
    }
  }
  QTreeView::mouseReleaseEvent(event);
}

VariablesWidget::VariablesWidget(MainWindow *pMainWindow)
  : QWidget(pMainWindow)
{
  setMinimumWidth(175);
  mpMainWindow = pMainWindow;
  // tree search filters
  mpTreeSearchFilters = new TreeSearchFilters(this);
  mpTreeSearchFilters->getSearchTextBox()->setPlaceholderText(Helper::findVariables);
  connect(mpTreeSearchFilters->getSearchTextBox(), SIGNAL(returnPressed()), SLOT(findVariables()));
  connect(mpTreeSearchFilters->getSearchTextBox(), SIGNAL(textEdited(QString)), SLOT(findVariables()));
  connect(mpTreeSearchFilters->getCaseSensitiveCheckBox(), SIGNAL(toggled(bool)), SLOT(findVariables()));
  connect(mpTreeSearchFilters->getSyntaxComboBox(), SIGNAL(currentIndexChanged(int)), SLOT(findVariables()));
  // create variables tree widget
  mpVariablesTreeView = new VariablesTreeView(this);
  mpVariablesTreeModel = new VariablesTreeModel(mpVariablesTreeView);
  mpVariableTreeProxyModel = new VariableTreeProxyModel;
  mpVariableTreeProxyModel->setDynamicSortFilter(true);
  mpVariableTreeProxyModel->setSourceModel(mpVariablesTreeModel);
  mpVariablesTreeView->setModel(mpVariableTreeProxyModel);
  mpVariablesTreeView->setColumnWidth(0, 150);
  mpVariablesTreeView->setColumnWidth(2, 50);
  mpLastActiveSubWindow = 0;
  // create the layout
  QGridLayout *pMainLayout = new QGridLayout;
  pMainLayout->setContentsMargins(0, 0, 0, 0);
  pMainLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  pMainLayout->addWidget(mpTreeSearchFilters, 0, 0);
  pMainLayout->addWidget(mpVariablesTreeView, 1, 0);
  setLayout(pMainLayout);
  connect(mpTreeSearchFilters->getExpandAllButton(), SIGNAL(clicked()), mpVariablesTreeView, SLOT(expandAll()));
  connect(mpTreeSearchFilters->getCollapseAllButton(), SIGNAL(clicked()), mpVariablesTreeView, SLOT(collapseAll()));
  connect(mpVariablesTreeModel, SIGNAL(rowsInserted(QModelIndex,int,int)), mpVariableTreeProxyModel, SLOT(invalidate()));
  connect(mpVariablesTreeModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), mpVariableTreeProxyModel, SLOT(invalidate()));
  connect(mpVariablesTreeModel, SIGNAL(itemChecked(QModelIndex,qreal,int)), SLOT(plotVariables(QModelIndex,qreal,int)));
  connect(mpVariablesTreeView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(showContextMenu(QPoint)));
  connect(pMainWindow->getPlotWindowContainer(), SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(updateVariablesTree(QMdiSubWindow*)));
  connect(mpVariablesTreeModel, SIGNAL(variableTreeItemRemoved(QString)), pMainWindow->getPlotWindowContainer(), SLOT(updatePlotWindows(QString)));
}

void VariablesWidget::insertVariablesItemsToTree(QString fileName, QString filePath, QStringList variablesList,
                                                 SimulationOptions simulationOptions)
{
  mpVariablesTreeView->setSortingEnabled(false);
  /* In order to improve the response time of insertVariablesItems function we should clear the filter and collapse all the items. */
  mpVariableTreeProxyModel->setFilterRegExp(QRegExp(""));
  mpVariablesTreeView->collapseAll();
  /* Remove the simulation result if we already had it in tree */
  bool variableItemDeleted = mpVariablesTreeModel->removeVariableTreeItem(fileName);
  /* add the plot variables */
  mpVariablesTreeModel->insertVariablesItems(fileName, filePath, variablesList, simulationOptions);
  /* update the plot variables tree */
  if (variableItemDeleted) {
    variablesUpdated();
  }
  mpVariablesTreeView->setSortingEnabled(true);
  mpVariablesTreeView->sortByColumn(0, Qt::AscendingOrder);
  /* since we cleared the filter above so we need to apply it back. */
  findVariables();
}

void VariablesWidget::variablesUpdated()
{
  foreach (QMdiSubWindow *pSubWindow, mpMainWindow->getPlotWindowContainer()->subWindowList(QMdiArea::StackingOrder))
  {
    PlotWindow *pPlotWindow = qobject_cast<PlotWindow*>(pSubWindow->widget());
    foreach (PlotCurve *pPlotCurve, pPlotWindow->getPlot()->getPlotCurvesList())
    {
      if (pPlotWindow->getPlotType() == PlotWindow::PLOT)
      {
        QString curveNameStructure = pPlotCurve->getNameStructure();
        VariablesTreeItem *pVariableTreeItem;
        pVariableTreeItem = mpVariablesTreeModel->findVariablesTreeItem(curveNameStructure, mpVariablesTreeModel->getRootVariablesTreeItem());
        pPlotCurve->detach();
        if (pVariableTreeItem)
        {
          bool state = mpVariablesTreeModel->blockSignals(true);
          QModelIndex index = mpVariablesTreeModel->variablesTreeItemIndex(pVariableTreeItem);
          mpVariablesTreeModel->setData(index, Qt::Checked, Qt::CheckStateRole);
          plotVariables(index, pPlotCurve->getCurveWidth(), pPlotCurve->getCurveStyle(), pPlotCurve, pPlotWindow);
          mpVariablesTreeModel->blockSignals(state);
        }
        else
        {
          pPlotWindow->getPlot()->removeCurve(pPlotCurve);
        }
      }
      else if (pPlotWindow->getPlotType() == PlotWindow::PLOTPARAMETRIC)
      {
        QString xVariable = QString(pPlotCurve->getFileName()).append(".").append(pPlotCurve->getXVariable());
        VariablesTreeItem *pXVariableTreeItem;
        pXVariableTreeItem = mpVariablesTreeModel->findVariablesTreeItem(xVariable, mpVariablesTreeModel->getRootVariablesTreeItem());
        QString yVariable = QString(pPlotCurve->getFileName()).append(".").append(pPlotCurve->getYVariable());
        VariablesTreeItem *pYVariableTreeItem;
        pYVariableTreeItem = mpVariablesTreeModel->findVariablesTreeItem(yVariable, mpVariablesTreeModel->getRootVariablesTreeItem());
        pPlotCurve->detach();
        if (pXVariableTreeItem && pYVariableTreeItem)
        {
          bool state = mpVariablesTreeModel->blockSignals(true);
          QModelIndex xIndex = mpVariablesTreeModel->variablesTreeItemIndex(pXVariableTreeItem);
          mpVariablesTreeModel->setData(xIndex, Qt::Checked, Qt::CheckStateRole);
          plotVariables(xIndex, pPlotCurve->getCurveWidth(), pPlotCurve->getCurveStyle(), pPlotCurve, pPlotWindow);
          QModelIndex yIndex = mpVariablesTreeModel->variablesTreeItemIndex(pYVariableTreeItem);
          mpVariablesTreeModel->setData(yIndex, Qt::Checked, Qt::CheckStateRole);
          plotVariables(yIndex, pPlotCurve->getCurveWidth(), pPlotCurve->getCurveStyle(), pPlotCurve, pPlotWindow);
          mpVariablesTreeModel->blockSignals(state);
        }
        else
        {
          pPlotWindow->getPlot()->removeCurve(pPlotCurve);
          pPlotCurve->detach();
        }
      }
    }
    if (pPlotWindow->getAutoScaleButton()->isChecked()) {
      pPlotWindow->fitInView();
    } else {
      pPlotWindow->getPlot()->replot();
    }
  }
  updateVariablesTreeHelper(mpMainWindow->getPlotWindowContainer()->currentSubWindow());
}

void VariablesWidget::updateVariablesTreeHelper(QMdiSubWindow *pSubWindow)
{
  if (!pSubWindow)
    return;
  // first clear all the check boxes in the tree
  bool state = mpVariablesTreeModel->blockSignals(true);
  mpVariablesTreeModel->unCheckVariables(mpVariablesTreeModel->getRootVariablesTreeItem());
  mpVariablesTreeModel->blockSignals(state);
  // all plotwindows are closed down then simply return
  if (mpMainWindow->getPlotWindowContainer()->subWindowList().size() == 0)
    return;

  PlotWindow *pPlotWindow = qobject_cast<PlotWindow*>(pSubWindow->widget());
  // now loop through the curves and tick variables in the tree whose curves are on the plot
  state = mpVariablesTreeModel->blockSignals(true);
  foreach (PlotCurve *pPlotCurve, pPlotWindow->getPlot()->getPlotCurvesList())
  {
    VariablesTreeItem *pVariablesTreeItem;
    if (pPlotWindow->getPlotType() == PlotWindow::PLOT)
    {
      QString variable = pPlotCurve->getNameStructure();
      pVariablesTreeItem = mpVariablesTreeModel->findVariablesTreeItem(variable, mpVariablesTreeModel->getRootVariablesTreeItem());
      if (pVariablesTreeItem)
        mpVariablesTreeModel->setData(mpVariablesTreeModel->variablesTreeItemIndex(pVariablesTreeItem), Qt::Checked, Qt::CheckStateRole);
    }
    else if (pPlotWindow->getPlotType() == PlotWindow::PLOTPARAMETRIC)
    {
      // check the xvariable
      QString xVariable = QString(pPlotCurve->getFileName()).append(".").append(pPlotCurve->getXVariable());
      pVariablesTreeItem = mpVariablesTreeModel->findVariablesTreeItem(xVariable, mpVariablesTreeModel->getRootVariablesTreeItem());
      if (pVariablesTreeItem)
        mpVariablesTreeModel->setData(mpVariablesTreeModel->variablesTreeItemIndex(pVariablesTreeItem), Qt::Checked, Qt::CheckStateRole);
      // check the y variable
      QString yVariable = QString(pPlotCurve->getFileName()).append(".").append(pPlotCurve->getYVariable());
      pVariablesTreeItem = mpVariablesTreeModel->findVariablesTreeItem(yVariable, mpVariablesTreeModel->getRootVariablesTreeItem());
      if (pVariablesTreeItem)
        mpVariablesTreeModel->setData(mpVariablesTreeModel->variablesTreeItemIndex(pVariablesTreeItem), Qt::Checked, Qt::CheckStateRole);
    }
  }
  mpVariablesTreeModel->blockSignals(state);
  /* invalidate the view so that the items show the updated values. */
  mpVariableTreeProxyModel->invalidate();
}

void VariablesWidget::readVariablesAndUpdateXML(VariablesTreeItem *pVariablesTreeItem, QString outputFileName,
                                                QHash<QString, QHash<QString, QString> > *variables)
{
  for (int i = 0 ; i < pVariablesTreeItem->getChildren().size() ; i++)
  {
    VariablesTreeItem *pChildVariablesTreeItem = pVariablesTreeItem->child(i);
    if (pChildVariablesTreeItem->isEditable() && pChildVariablesTreeItem->isValueChanged())
    {
      QString value = pChildVariablesTreeItem->data(1, Qt::DisplayRole).toString();
      QString variableToFind = pChildVariablesTreeItem->getVariableName();
      variableToFind.remove(QRegExp(outputFileName + "."));
      QHash<QString, QString> hash;
      hash["name"] = variableToFind;
      hash["value"] = value;
      variables->insert(variableToFind, hash);
    }
    readVariablesAndUpdateXML(pChildVariablesTreeItem, outputFileName, variables);
  }
}

void VariablesWidget::findVariableAndUpdateValue(QDomDocument xmlDocument, QHash<QString, QHash<QString, QString> > variables)
{
  /* if no variables are changed. */
  if (variables.empty())
    return;
  /* update the variables */
  int count = 0;
  QDomNodeList scalarVariable = xmlDocument.elementsByTagName("ScalarVariable");
  for (int i = 0; i < scalarVariable.size(); i++)
  {
    if (count >= variables.size())
      break;
    QDomElement element = scalarVariable.at(i).toElement();
    if (!element.isNull())
    {
      QHash<QString, QString> hash = variables.value(element.attribute("name"));
      if (element.attribute("name").compare(hash["name"]) == 0)
      {
        count++;
        QDomElement el = scalarVariable.at(i).firstChild().toElement();
        if (!el.isNull())
        {
          el.setAttribute("start", hash["value"]);
        }
      }
    }
  }
}

void VariablesWidget::reSimulate(bool showSetup)
{
  QModelIndexList indexes = mpVariablesTreeView->selectionModel()->selectedIndexes();
  if (indexes.isEmpty()) {
    QMessageBox::information(this, QString(Helper::applicationName).append(" - ").append(Helper::information),
                             tr("You must select a class to re-simulate."), Helper::ok);
    return;
  }
  QModelIndex index = indexes.at(0);
  index = mpVariableTreeProxyModel->mapToSource(index);
  VariablesTreeItem *pVariablesTreeItem = static_cast<VariablesTreeItem*>(index.internalPointer());
  pVariablesTreeItem = pVariablesTreeItem->rootParent();
  SimulationOptions simulationOptions = pVariablesTreeItem->getSimulationOptions();
  if (simulationOptions.isValid()) {
    simulationOptions.setReSimulate(true);
    updateInitXmlFile(simulationOptions);
    if (showSetup) {
      mpMainWindow->getSimulationDialog()->show(0, true, simulationOptions);
    } else {
      mpMainWindow->getSimulationDialog()->reSimulate(simulationOptions);
    }
  } else {
    QMessageBox::information(this, QString(Helper::applicationName).append(" - ").append(Helper::information),
                             tr("You cannot re-simulate this class.<br />This is just a result file loaded via menu <b>File->Open Result File(s)</b>."), Helper::ok);
  }
}

void VariablesWidget::updateInitXmlFile(SimulationOptions simulationOptions)
{
  /* Update the _init.xml file with new values. */
  /* open the model_init.xml file for writing */
  QString initFileName = QString(simulationOptions.getOutputFileName()).append("_init.xml");
  QFile initFile(QString(simulationOptions.getWorkingDirectory()).append(QDir::separator()).append(initFileName));
  QDomDocument initXmlDocument;
  if (initFile.open(QIODevice::ReadOnly)) {
    if (initXmlDocument.setContent(&initFile)) {
      VariablesTreeItem *pTopVariableTreeItem;
      pTopVariableTreeItem = mpVariablesTreeModel->findVariablesTreeItem(simulationOptions.getResultFileName(),
                                                                         mpVariablesTreeModel->getRootVariablesTreeItem());
      if (pTopVariableTreeItem) {
        QHash<QString, QHash<QString, QString> > variables;
        readVariablesAndUpdateXML(pTopVariableTreeItem, simulationOptions.getResultFileName(), &variables);
        findVariableAndUpdateValue(initXmlDocument, variables);
      }
    } else {
      MessagesWidget *pMessagesWidget = mpVariablesTreeView->getVariablesWidget()->getMainWindow()->getMessagesWidget();
      pMessagesWidget->addGUIMessage(MessageItem(MessageItem::Modelica, "", false, 0, 0, 0, 0,
                                                 tr("Unable to set the content of QDomDocument from file %1")
                                                 .arg(initFile.fileName()), Helper::scriptingKind, Helper::errorLevel));
    }
    initFile.close();
    initFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
    QTextStream textStream(&initFile);
    textStream.setCodec(Helper::utf8.toStdString().data());
    textStream.setGenerateByteOrderMark(false);
    textStream << initXmlDocument.toString();
    initFile.close();
  } else {
    MessagesWidget *pMessagesWidget = mpVariablesTreeView->getVariablesWidget()->getMainWindow()->getMessagesWidget();
    pMessagesWidget->addGUIMessage(MessageItem(MessageItem::Modelica, "", false, 0, 0, 0, 0,
                                               GUIMessages::getMessage(GUIMessages::ERROR_OPENING_FILE).arg(initFile.fileName())
                                               .arg(initFile.errorString()), Helper::scriptingKind, Helper::errorLevel));
  }
}

void VariablesWidget::plotVariables(const QModelIndex &index, qreal curveThickness, int curveStyle, PlotCurve *pPlotCurve,
                                    PlotWindow *pPlotWindow)
{
  if (index.column() > 0) {
    return;
  }
  VariablesTreeItem *pVariablesTreeItem = static_cast<VariablesTreeItem*>(index.internalPointer());
  if (!pVariablesTreeItem) {
    return;
  }
  try {
    // if pPlotWindow is 0 then get the current window.
    if (!pPlotWindow) {
      pPlotWindow = mpMainWindow->getPlotWindowContainer()->getCurrentWindow();
    }
    // if pPlotWindow is 0 then create a new plot window.
    if (!pPlotWindow) {
      bool state = mpMainWindow->getPlotWindowContainer()->blockSignals(true);
      mpMainWindow->getPlotWindowContainer()->addPlotWindow();
      mpMainWindow->getPlotWindowContainer()->blockSignals(state);
      pPlotWindow = mpMainWindow->getPlotWindowContainer()->getCurrentWindow();
    }
    // if still pPlotWindow is 0 then return.
    if (!pPlotWindow) {
      bool state = mpVariablesTreeModel->blockSignals(true);
      mpVariablesTreeModel->setData(index, Qt::Unchecked, Qt::CheckStateRole);
      mpVariablesTreeModel->blockSignals(state);
      QMessageBox::information(this, QString(Helper::applicationName).append(" - ").append(Helper::information),
                               tr("No plot window is active for plotting. Please select a plot window or open a new."), Helper::ok);
      return;
    }
    // if plottype is PLOT then
    if (pPlotWindow->getPlotType() == PlotWindow::PLOT) {
      // check the item checkstate
      if (pVariablesTreeItem->isChecked()) {
        pPlotWindow->initializeFile(QString(pVariablesTreeItem->getFilePath()).append("/").append(pVariablesTreeItem->getFileName()));
        pPlotWindow->setCurveWidth(curveThickness);
        pPlotWindow->setCurveStyle(curveStyle);
        pPlotWindow->setVariablesList(QStringList(pVariablesTreeItem->getPlotVariable()));
        pPlotWindow->setUnit(pVariablesTreeItem->getUnit());
        pPlotWindow->setDisplayUnit(pVariablesTreeItem->getDisplayUnit());
        pPlotWindow->plot(pPlotCurve);
        pPlotCurve = pPlotWindow->getPlot()->getPlotCurvesList().last();
        if (pPlotCurve && pVariablesTreeItem->getUnit().compare(pVariablesTreeItem->getDisplayUnit()) != 0) {
          OMCInterface::convertUnits_res convertUnit = mpMainWindow->getOMCProxy()->convertUnits(pVariablesTreeItem->getUnit(),
                                                                                                 pVariablesTreeItem->getDisplayUnit());
          if (convertUnit.unitsCompatible) {
            for (int i = 0 ; i < pPlotCurve->getYAxisData().size() ; i++) {
              pPlotCurve->updateYAxisValue(i, Utilities::convertUnit(pPlotCurve->getYAxisData().at(i), convertUnit.offset, convertUnit.scaleFactor));
            }
            pPlotCurve->setData(pPlotCurve->getXAxisVector(), pPlotCurve->getYAxisVector(), pPlotCurve->getSize());
            pPlotWindow->getPlot()->replot();
          }
        }
        if (pPlotWindow->getAutoScaleButton()->isChecked()) {
          pPlotWindow->fitInView();
        } else {
          pPlotWindow->getPlot()->replot();
          if (pPlotWindow->getPlot()->getPlotZoomer()->zoomStack().size() == 1) {
            pPlotWindow->getPlot()->getPlotZoomer()->setZoomBase(false);
          }
        }
      } else if (!pVariablesTreeItem->isChecked()) {  // if user unchecks the variable then remove it from the plot
        foreach (PlotCurve *pPlotCurve, pPlotWindow->getPlot()->getPlotCurvesList()) {
          QString curveTitle = pPlotCurve->getNameStructure();
          if (curveTitle.compare(pVariablesTreeItem->getVariableName()) == 0) {
            pPlotWindow->getPlot()->removeCurve(pPlotCurve);
            pPlotCurve->detach();
            if (pPlotWindow->getAutoScaleButton()->isChecked()) {
              pPlotWindow->fitInView();
            } else {
              pPlotWindow->getPlot()->replot();
            }
          }
        }
      }
    } else {  // if plottype is PLOTPARAMETRIC then
      // check the item checkstate
      if (pVariablesTreeItem->isChecked()) {
        // if mPlotParametricVariables is empty just add one QStringlist with 1 varibale to it
        if (mPlotParametricVariables.isEmpty()) {
          mPlotParametricVariables.append(QStringList() << pVariablesTreeItem->getPlotVariable() << pVariablesTreeItem->getUnit());
          mFileName = pVariablesTreeItem->getFileName();
        } else {  // if mPlotParametricVariables is not empty then add one string to its last element
          if (mPlotParametricVariables.last().size() < 4) {
            if (mFileName.compare(pVariablesTreeItem->getFileName()) != 0) {
              bool state = mpVariablesTreeModel->blockSignals(true);
              mpVariablesTreeModel->setData(index, Qt::Unchecked, Qt::CheckStateRole);
              QMessageBox::critical(this, QString(Helper::applicationName).append(" - ").append(Helper::error),
                                    GUIMessages::getMessage(GUIMessages::PLOT_PARAMETRIC_DIFF_FILES), Helper::ok);
              mpVariablesTreeModel->blockSignals(state);
              return;
            }
            mPlotParametricVariables.last().append(QStringList() << pVariablesTreeItem->getPlotVariable() << pVariablesTreeItem->getUnit());
            pPlotWindow->initializeFile(QString(pVariablesTreeItem->getFilePath()).append("/").append(pVariablesTreeItem->getFileName()));
            pPlotWindow->setCurveWidth(curveThickness);
            pPlotWindow->setCurveStyle(curveStyle);
            pPlotWindow->setVariablesList(QStringList() << mPlotParametricVariables.last().at(0) << mPlotParametricVariables.last().at(2));
            pPlotWindow->plotParametric(pPlotCurve);
            if (pPlotWindow->getPlot()->getPlotCurvesList().size() > 1) {
              pPlotWindow->setXLabel("");
              pPlotWindow->setYLabel("");
            } else {
              QString xVariable = mPlotParametricVariables.last().at(0);
              QString xUnit = mPlotParametricVariables.last().at(1);
              QString yVariable = mPlotParametricVariables.last().at(2);
              QString yUnit = mPlotParametricVariables.last().at(3);
              if (xUnit.isEmpty()) {
                pPlotWindow->setXLabel(xVariable);
              } else {
                pPlotWindow->setXLabel(xVariable + " [" + xUnit + "]");
              }

              if (yUnit.isEmpty()) {
                pPlotWindow->setYLabel(yVariable);
              } else {
                pPlotWindow->setYLabel(yVariable + " [" + yUnit + "]");
              }
            }
            if (pPlotWindow->getAutoScaleButton()->isChecked()) {
              pPlotWindow->fitInView();
            } else {
              pPlotWindow->getPlot()->replot();
              if (pPlotWindow->getPlot()->getPlotZoomer()->zoomStack().size() == 1) {
                pPlotWindow->getPlot()->getPlotZoomer()->setZoomBase(false);
              }
            }
          } else {
            mPlotParametricVariables.append(QStringList() << pVariablesTreeItem->getPlotVariable() << pVariablesTreeItem->getUnit());
            mFileName = pVariablesTreeItem->getFileName();
          }
        }
      } else if (!pVariablesTreeItem->isChecked()) {  // if user unchecks the variable then remove it from the plot
        // remove the variable from mPlotParametricVariables list
        foreach (QStringList list, mPlotParametricVariables) {
          if (list.contains(pVariablesTreeItem->getPlotVariable())) {
            // if list has only one variable then clear the list and return;
            if (list.size() < 4) {
              mPlotParametricVariables.removeOne(list);
              break;
            } else {  // if list has more than two variables then remove both and remove the curve
              QString itemTitle = QString(pVariablesTreeItem->getFileName()).append(".").append(list.at(2)).append(" vs ").append(list.at(0));
              foreach (PlotCurve *pPlotCurve, pPlotWindow->getPlot()->getPlotCurvesList()) {
                QString curveTitle = pPlotCurve->getNameStructure();
                if ((curveTitle.compare(itemTitle) == 0) && (pVariablesTreeItem->getFileName().compare(pPlotCurve->getFileName()) == 0)) {
                  bool state = mpVariablesTreeModel->blockSignals(true);
                  // uncheck the x variable
                  QString xVariable = QString(pPlotCurve->getFileName()).append(".").append(pPlotCurve->getXVariable());
                  VariablesTreeItem *pVariablesTreeItem;
                  pVariablesTreeItem = mpVariablesTreeModel->findVariablesTreeItem(xVariable, mpVariablesTreeModel->getRootVariablesTreeItem());
                  if (pVariablesTreeItem) {
                    mpVariablesTreeModel->setData(mpVariablesTreeModel->variablesTreeItemIndex(pVariablesTreeItem), Qt::Unchecked, Qt::CheckStateRole);
                  }
                  // uncheck the y variable
                  QString yVariable = QString(pPlotCurve->getFileName()).append(".").append(pPlotCurve->getYVariable());
                  pVariablesTreeItem = mpVariablesTreeModel->findVariablesTreeItem(yVariable, mpVariablesTreeModel->getRootVariablesTreeItem());
                  if (pVariablesTreeItem) {
                    mpVariablesTreeModel->setData(mpVariablesTreeModel->variablesTreeItemIndex(pVariablesTreeItem), Qt::Unchecked, Qt::CheckStateRole);
                  }
                  mpVariablesTreeModel->blockSignals(state);
                  pPlotWindow->getPlot()->removeCurve(pPlotCurve);
                  pPlotCurve->detach();
                  if (pPlotWindow->getAutoScaleButton()->isChecked()) {
                    pPlotWindow->fitInView();
                  } else {
                    pPlotWindow->getPlot()->replot();
                  }
                }
              }
              mPlotParametricVariables.removeOne(list);
              if (pPlotWindow->getPlot()->getPlotCurvesList().size() == 1) {
                if (mPlotParametricVariables.last().size() > 3) {
                  QString xVariable = mPlotParametricVariables.last().at(0);
                  QString xUnit = mPlotParametricVariables.last().at(1);
                  QString yVariable = mPlotParametricVariables.last().at(2);
                  QString yUnit = mPlotParametricVariables.last().at(3);
                  if (xUnit.isEmpty()) {
                    pPlotWindow->setXLabel(xVariable);
                  } else {
                    pPlotWindow->setXLabel(xVariable + " [" + xUnit + "]");
                  }

                  if (yUnit.isEmpty()) {
                    pPlotWindow->setYLabel(yVariable);
                  } else {
                    pPlotWindow->setYLabel(yVariable + " [" + yUnit + "]");
                  }
                } else {
                  pPlotWindow->setXLabel("");
                  pPlotWindow->setYLabel("");
                }
              } else {
                pPlotWindow->setXLabel("");
                pPlotWindow->setYLabel("");
              }
            }
          }
        }
      }
    }
  } catch (PlotException &e) {
    QMessageBox::critical(this, QString(Helper::applicationName).append(" - ").append(Helper::error), e.what(), Helper::ok);
  }
}

void VariablesWidget::updateVariablesTree(QMdiSubWindow *pSubWindow)
{
  if (!pSubWindow && mpMainWindow->getPlotWindowContainer()->subWindowList().size() != 0)
    return;
  /* if the same sub window is activated again then just return */
  if (mpLastActiveSubWindow == pSubWindow)
  {
    mpLastActiveSubWindow = pSubWindow;
    return;
  }
  mpLastActiveSubWindow = pSubWindow;
  updateVariablesTreeHelper(pSubWindow);
}

void VariablesWidget::showContextMenu(QPoint point)
{
  int adjust = 24;
  QModelIndex index = mpVariablesTreeView->indexAt(point);
  index = mpVariableTreeProxyModel->mapToSource(index);
  VariablesTreeItem *pVariablesTreeItem = static_cast<VariablesTreeItem*>(index.internalPointer());
  if (pVariablesTreeItem && pVariablesTreeItem->isRootItem())
  {
    /* delete result action */
    QAction *pDeleteResultAction = new QAction(QIcon(":/Resources/icons/delete.svg"), tr("Delete Result"), this);
    pDeleteResultAction->setData(pVariablesTreeItem->getVariableName());
    pDeleteResultAction->setStatusTip(tr("Delete the result"));
    connect(pDeleteResultAction, SIGNAL(triggered()), mpVariablesTreeModel, SLOT(removeVariableTreeItem()));

    QMenu menu(this);
    menu.addAction(pDeleteResultAction);
    menu.addSeparator();
    menu.addAction(mpMainWindow->getReSimulateModelAction());
    menu.addAction(mpMainWindow->getReSimulateSetupAction());
    point.setY(point.y() + adjust);
    menu.exec(mpVariablesTreeView->mapToGlobal(point));
  }
}

/*!
 * \brief VariablesWidget::findVariables
 * Finds the variables in the Variables Browser.
 */
void VariablesWidget::findVariables()
{
  QString findText = mpTreeSearchFilters->getSearchTextBox()->text();
  QRegExp::PatternSyntax syntax = QRegExp::PatternSyntax(mpTreeSearchFilters->getSyntaxComboBox()->itemData(mpTreeSearchFilters->getSyntaxComboBox()->currentIndex()).toInt());
  Qt::CaseSensitivity caseSensitivity = mpTreeSearchFilters->getCaseSensitiveCheckBox()->isChecked() ? Qt::CaseSensitive: Qt::CaseInsensitive;
  QRegExp regExp(findText, caseSensitivity, syntax);
  mpVariableTreeProxyModel->setFilterRegExp(regExp);
  /* expand all so that the filtered items can be seen. */
  if (!findText.isEmpty()) {
    mpVariablesTreeView->expandAll();
  }
}

void VariablesWidget::directReSimulate()
{
  reSimulate(false);
}

void VariablesWidget::showReSimulateSetup()
{
  reSimulate(true);
}
