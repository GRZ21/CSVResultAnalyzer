//
// Created by GRZ on 2026/5/2.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MainWindow.h" resolved

#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QFileDialog>
#include <QString>
#include <QColorDialog>

#include "../utils/qcustomplot.h"


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);


    tableModel = new QStandardItemModel(this);
    tableSelectionModel = new QItemSelectionModel(tableModel);
    ui->tableView->setModel(tableModel);
    ui->tableView->setSelectionModel(tableSelectionModel);
    ui->tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectItems);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 界面大小
    ui->Vsplitter->setSizes(QList<int>({700,300}));
    ui->Hsplitter->setSizes(QList<int>({200,600,200}));

    // 表头相关
    ui->treeWidget->header()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
    ui->treeWidget->header()->setSectionResizeMode(1,QHeaderView::ResizeToContents);

    // 自定义右键策略
    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    // 图相关
    ui->customPlot->legend->setVisible(true);
    ui->customPlot->legend->setBrush(QBrush(QColor(255,255,255,150)));
    ui->customPlot->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom|QCP::iSelectPlottables);

    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::actionOpen);
    connect(ui->treeWidget,&QTreeWidget::itemClicked,this,&MainWindow::onTreeItemClicked);
    connect(ui->treeWidget,&QTreeWidget::itemDoubleClicked,this,&MainWindow::onTreeItemDoubleClicked);
    connect(tableSelectionModel,&QItemSelectionModel::selectionChanged,this,
        [&](const QItemSelection& selected,const QItemSelection& deselected) {
            if (!selected.isEmpty()) {
                QModelIndex index = selected.indexes().first();
                highlightTableParam(index);
            }
        });
    connect(ui->treeWidget,&QTreeWidget::itemChanged,this,&MainWindow::onParmItemChanged);
    connect(ui->customPlot->xAxis, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged),
    [=](const QCPRange &newRange) {
        if (newRange.lower < 0) {
            ui->customPlot->xAxis->setRangeLower(0);
        }
    });

    connect(ui->customPlot->yAxis, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged),
        [=](const QCPRange &newRange) {
            if (newRange.lower < 0)
                ui->customPlot->yAxis->setRangeLower(0);
        });

    connect(ui->customPlot,&QCustomPlot::plottableClick,this,&MainWindow::onCurveClicked);

    connect(ui->editName,&QLineEdit::textChanged,this,&MainWindow::onEditName);
    connect(ui->spinWidth, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onSpinWidth);
    connect(ui->spinAlpha, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onSpinAlpha);
    connect(ui->btnColor, &QPushButton::clicked, this, &MainWindow::onPropColorClicked);
    connect(ui->comboStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onComboStyleChanged);
    connect(ui->comboScatter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onComboScatterChanged);
    connect(ui->btnBringTop,&QPushButton::clicked,this,&MainWindow::onBtnBringTopClicked);
    connect(ui->btnSendBottom,&QPushButton::clicked,this,&MainWindow::onBtnSendBottomClicked);
    connect(ui->comboFont,&QFontComboBox::currentFontChanged,this,&MainWindow::updateGlobalFont);
    connect(ui->spinFontSize, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::updateGlobalFont);
    connect(ui->checkFontBold,&QCheckBox::toggled,this,&MainWindow::updateGlobalFont);
    connect(ui->comboLegend, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onComboLegendChanged);
    connect(ui->comboGrid, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onComboGridChanged);
    connect(ui->editXTitle,&QLineEdit::textChanged,this,&MainWindow::onAxisTitleChanged);
    connect(ui->editYTitle,&QLineEdit::textChanged,this,&MainWindow::onAxisTitleChanged);
    connect(ui->spinScatterSize, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onSpinScatterSizeChanged);
    connect(ui->btnResetZoom,&QPushButton::clicked,this,&MainWindow::autoRescalePlot);
    connect(ui->btnExportImg,&QPushButton::clicked,this,&MainWindow::onBtnExportClicked);

    connect(ui->actionOpenDir, &QAction::triggered, this, &MainWindow::onActionOpenDir);
    connect(ui->action_Clear_Plot, &QAction::triggered, this, &MainWindow::onActionClearPlot);
    connect(ui->action_Remove_All_Files, &QAction::triggered, this, &MainWindow::onActionRemoveAllFiles);
    connect(ui->action_Toggle_File_Tree, &QAction::triggered, this, &MainWindow::onActionToggleTree);
    connect(ui->action_Toggle_Data_Table, &QAction::triggered, this, &MainWindow::onActionToggleTable);
    connect(ui->action_Toggle_Properties, &QAction::triggered, this, &MainWindow::onActionToggleProps);
    connect(ui->actionExport, &QAction::triggered, this, &MainWindow::onBtnExportClicked);
    connect(ui->action_Delete_CSV, &QAction::triggered, this, &MainWindow::onActionDeleteCSV);
    connect(ui->treeWidget,&QTreeWidget::customContextMenuRequested,this,&MainWindow::onTreeContextMenu);
    connect(ui->action_Q, &QAction::triggered, this, &MainWindow::close);
    connect(ui->action_Documentation, &QAction::triggered, this, &MainWindow::onActionGuide);
    connect(ui->action_About, &QAction::triggered, this, &MainWindow::onActionAbout);


    ui->action_Toggle_File_Tree->setChecked(true);
    ui->action_Toggle_Data_Table->setChecked(true);
    ui->action_Toggle_Properties->setChecked(true);

    buildTreeHeader();
    onComboGridChanged(0);
    onAxisTitleChanged();
    initTree();
    updateGlobalFont();
}


MainWindow::~MainWindow() {
    delete ui;
}



void MainWindow::buildTreeHeader() {
    ui->treeWidget->clear();

    QTreeWidgetItem* header = new QTreeWidgetItem();
    header->setText(MainWindow::colItem,"文件名");
    header->setText(MainWindow::colDate,"最后修改日期");

    header->setTextAlignment(MainWindow::colItem,Qt::AlignHCenter|Qt::AlignVCenter);

    ui->treeWidget->setHeaderItem(header);
}


void MainWindow::initTree() {
    QTreeWidgetItem* root = new QTreeWidgetItem(MainWindow::itTopItem);
    root->setText(MainWindow::colItem,"csv name: ");
    root->setText(MainWindow::colDate,QDate::currentDate().toString());
    root->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemIsUserCheckable|Qt::ItemIsAutoTristate);
    root->setCheckState(MainWindow::colItem,Qt::Checked);
    ui->treeWidget->addTopLevelItem(root);
}


QTreeWidgetItem* MainWindow::addCSVItem(QTreeWidgetItem *parItem, QString fileName) {
    QFileInfo fileInfo(fileName);
    QString lastFileName = fileInfo.fileName();
    QDateTime fileDate = fileInfo.lastModified();

    QTreeWidgetItem* item = new QTreeWidgetItem(MainWindow::itCSVItem);
    item->setText(MainWindow::colItem,lastFileName);
    item->setText(MainWindow::colDate,fileDate.toString());
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    // item->setCheckState(MainWindow::colItem,Qt::Checked);

    item->setData(MainWindow::colItem,Qt::UserRole,QVariant(fileName));
    parItem->addChild(item);
    return item;
}


void MainWindow::addParmItem(QTreeWidgetItem *parItem, QString CSVHeaderParam,int colIndex) {
    QTreeWidgetItem* item = new QTreeWidgetItem(MainWindow::itParamItem);
    QString trimmedParam = CSVHeaderParam.trimmed();
    item->setText(MainWindow::colItem,trimmedParam);
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemIsUserCheckable|Qt::ItemIsAutoTristate);
    item->setCheckState(MainWindow::colItem,Qt::Unchecked);

    item->setData(MainWindow::colItem,Qt::UserRole,colIndex);
    parItem->addChild(item);
}


void MainWindow::loadCSVCache(QString fileName) {
    QStringList data;
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly|QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            data<<line;
        }
    }
    file.close();
    if (!data.isEmpty()) {
        csvCache[fileName] = data;
        csvTableDisplay(data);
    }
}


void MainWindow::highlightColumn(int columIndex) {
    if (columIndex<0||columIndex>=tableModel->columnCount())
        return;
    
    int rowCount = tableModel->rowCount();
    if (rowCount == 0)
        return;

    QModelIndex topLeft = tableModel->index(0, columIndex);
    QModelIndex bottomRight = tableModel->index(rowCount - 1, columIndex);

    QItemSelection selection(topLeft, bottomRight);

    tableSelectionModel->select(selection, QItemSelectionModel::ClearAndSelect);
}


void MainWindow::highlightTableParam(QModelIndex& Index) {
    if (!Index.isValid())
        return;
    
    int columnIndex = Index.column();

    QString paramName = tableModel->headerData(columnIndex, Qt::Horizontal).toString().trimmed();
    
    if (paramName.isEmpty() || currentCSVFileName.isEmpty())
        return;

    QTreeWidgetItem* csvItem = nullptr;
    QList<QTreeWidgetItem*> allItems = ui->treeWidget->findItems(
        QFileInfo(currentCSVFileName).fileName(), 
        Qt::MatchExactly | Qt::MatchRecursive
    );
    
    for (QTreeWidgetItem* item : allItems) {
        if (item->type() == MainWindow::itCSVItem) {
            QString fileName = item->data(MainWindow::colItem, Qt::UserRole).toString();
            if (fileName == currentCSVFileName) {
                csvItem = item;
                break;
            }
        }
    }
    
    if (csvItem == nullptr)
        return;

    for (int i = 0; i < csvItem->childCount(); i++) {
        QTreeWidgetItem* child = csvItem->child(i);
        if (child->type() == MainWindow::itParamItem && 
            child->data(MainWindow::colItem, Qt::UserRole).toInt() == columnIndex) {
            ui->treeWidget->setCurrentItem(child);
            csvItem->setExpanded(true);

            ui->customPlot->deselectAll();
            for (int j = 0;j<ui->customPlot->graphCount();j++) {
                QCPGraph* graph = ui->customPlot->graph(j);
                if (curveToItemMap.value(graph, nullptr)== child) {
                    graph->setSelection(QCPDataSelection(graph->data()->dataRange()));
                    curGraph = graph;

                    if (!ui->customPlot->layer("top_layer")) {
                        ui->customPlot->addLayer("top_layer", ui->customPlot->layer("main"), QCustomPlot::limAbove);
                    }
                    if (graph->layer()->name() == "top_layer") {
                        graph->setLayer("main"); // 踢出重进机制
                    }
                    graph->setLayer("top_layer");

                    ui->editName->blockSignals(true);
                    ui->spinWidth->blockSignals(true);
                    ui->spinAlpha->blockSignals(true);
                    ui->comboStyle->blockSignals(true);
                    ui->comboScatter->blockSignals(true);
                    ui->spinScatterSize->blockSignals(true);

                    ui->editName->setText(graph->name());
                    ui->spinWidth->setValue(graph->pen().width());
                    ui->spinAlpha->setValue(graph->pen().color().alpha() * 100 / 255);
                    ui->spinScatterSize->setValue(graph->scatterStyle().size());

                    if (graph->pen().style() == Qt::SolidLine) ui->comboStyle->setCurrentIndex(0);
                    else if (graph->pen().style() == Qt::DashLine) ui->comboStyle->setCurrentIndex(1);

                    switch(graph->scatterStyle().shape()) {
                        case QCPScatterStyle::ssNone:     ui->comboScatter->setCurrentIndex(0); break;
                        case QCPScatterStyle::ssCircle:   ui->comboScatter->setCurrentIndex(1); break;
                        case QCPScatterStyle::ssSquare:   ui->comboScatter->setCurrentIndex(2); break;
                        case QCPScatterStyle::ssDiamond:    ui->comboScatter->setCurrentIndex(3); break;
                        case QCPScatterStyle::ssTriangle: ui->comboScatter->setCurrentIndex(4); break;
                        default:                          ui->comboScatter->setCurrentIndex(0); break;
                    }

                    ui->editName->blockSignals(false);
                    ui->spinWidth->blockSignals(false);
                    ui->spinAlpha->blockSignals(false);
                    ui->comboStyle->blockSignals(false);
                    ui->comboScatter->blockSignals(false);
                    ui->spinScatterSize->blockSignals(false);

                    // 5. 更新右下角的统计信息 (最高值、最终值等)
                    updateCurveStats(graph);

                    break; // 找到了就结束循环
                }
            }
            ui->customPlot->replot();
            return;
        }
    }
}

void MainWindow::poltCsvColumn(QTreeWidgetItem* item,int columnIndex) {
    if (!item || item->type() != MainWindow::itParamItem)
        return;

    QTreeWidgetItem* parItem = item->parent();
    if (!parItem)
        return;

    currentCSVFileName = parItem->data(MainWindow::colItem, Qt::UserRole).toString();
    QStringList data;
    if (csvCache.contains(currentCSVFileName)) {
        data = csvCache[currentCSVFileName];
    } else {
        return;
    }

    int valueIndex = item->data(MainWindow::colItem, Qt::UserRole).toInt();

    int epochIndex = -1;
    QStringList header = data.at(0).split(",");
    for (int i = 0; i < header.size(); i++) {
        if (header.at(i).trimmed() == "epoch") {
            epochIndex = i;
            break;
        }
    }

    if (epochIndex < 0 || valueIndex < 0 || valueIndex >= header.size()) return;

    QVector<double> xData, yData;
    for (int i = 1; i < data.size(); i++) {
        QString line = data.at(i);
        QStringList fields = line.split(",");
        if (fields.size() > epochIndex && fields.size() > valueIndex) {
            bool okX, okY;
            double x = fields.at(epochIndex).trimmed().toDouble(&okX);
            double y = fields.at(valueIndex).trimmed().toDouble(&okY);
            if (okX && okY) {
                xData.append(x);
                yData.append(y);
            }
        }
    }


    QString originalHeaderName = header.at(valueIndex).trimmed();
    QString currentItemName = item->text(MainWindow::colItem);
    QString pureName;
    if (currentItemName == originalHeaderName) {
        pureName = QFileInfo(currentCSVFileName).baseName() + " - " + currentItemName;
    } else {
        pureName = currentItemName;
    }

    if (!xData.isEmpty()) {
        drawOnCustomPlot(pureName, xData, yData, item);
    }
}

void MainWindow::drawOnCustomPlot(QString name, QVector<double> x, QVector<double> y,QTreeWidgetItem* item) {
    if (curveToItemMap.key(item, nullptr) != nullptr) {
        return;
    }
    QCPGraph* graph = ui->customPlot->addGraph();
    curveToItemMap.insert(graph, item);

    graph->setName(name);

    graph->setData(x,y);
    QPen pen;
    pen.setColor(QColor(0,120,212));
    graph->setPen(pen);

    autoRescalePlot();

    ui->customPlot->setInteractions(QCP::Interaction::iRangeDrag|QCP::Interaction::iRangeZoom|QCP::Interaction::iSelectPlottables);
    ui->customPlot->replot();
}

void MainWindow::onParmItemChanged(QTreeWidgetItem *item, int columIndex) {
    if (item->type() != MainWindow::itParamItem)
        return;
    QTreeWidgetItem* parItem = item->parent();
    if (item->checkState(columIndex)==Qt::Checked)
        poltCsvColumn(item,columIndex);
    else
        removeCurveFromPlot(item);
}

void MainWindow::removeCurveFromPlot(QTreeWidgetItem *item) {
    bool hasRemoved = false;
    for (int i = ui->customPlot->graphCount()-1;i>=0;i--) {
        QCPGraph* graph = ui->customPlot->graph(i);
        if (curveToItemMap.value(graph,nullptr)==item) {
            if (curGraph == graph)
                curGraph = nullptr;
            graph->setSelection(QCPDataSelection());
            curveToItemMap.remove(graph);
            ui->customPlot->removeGraph(graph);
            hasRemoved = true;
        }
    }
    if (hasRemoved) {
        autoRescalePlot();
        ui->customPlot->replot();
    }
}

void MainWindow::updateCurveStats(QCPGraph *graph) {
    if (!graph || graph->data()->isEmpty()) {
        ui->labelMaxValue->setText("--");
        ui->labelMinValue->setText("--");
        ui->labelFinalValue->setText("--");
        return;
    }

    double maxVal = -std::numeric_limits<double>::max();
    double minVal = std::numeric_limits<double>::max();
    double finalVal = 0;

    // 遍历数据点寻找最值
    auto it = graph->data()->constBegin();
    auto itEnd = graph->data()->constEnd();
    for (; it != itEnd; ++it) {
        double val = it->mainValue();
        if (val > maxVal) maxVal = val;
        if (val < minVal) minVal = val;
    }

    finalVal = (itEnd - 1)->mainValue();

    ui->labelMaxValue->setText(QString::number(maxVal, 'f', 5));
    ui->labelMinValue->setText(QString::number(minVal, 'f', 5));
    ui->labelFinalValue->setText(QString::number(finalVal, 'f', 5));
}

void MainWindow::actionOpen() {
    QStringList data;
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open CSV File"), QDir::currentPath(), tr("CSV Files (*.csv)"));
    if (!fileName.isEmpty()) {
        QTreeWidgetItem* parItem,*item;
        item = ui->treeWidget->currentItem();

        if (item != nullptr) {
            if (item->type() == MainWindow::itCSVItem || item->type() == MainWindow::itParamItem) {
                while (item->parent() != nullptr && item->type() != MainWindow::itTopItem) {
                    item = item->parent();
                }
            }
            parItem = item;
        } else {
            ui->treeWidget->addTopLevelItem(new QTreeWidgetItem(MainWindow::itTopItem));
            parItem = ui->treeWidget->topLevelItem(ui->treeWidget->topLevelItemCount() - 1);
        }

        QTreeWidgetItem* csvItem = addCSVItem(parItem,fileName);
        parItem->setExpanded(true);

        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly|QIODevice::Text)) {
            QTextStream in(&file);

            if (!in.atEnd()) {
                QString headLine = in.readLine();
                QStringList headerData = headLine.split(",");

                    for (int i = 0; i < headerData.size(); i++) {
                        QString paramName = headerData.at(i).trimmed();
                        if (paramName=="epoch"||paramName=="time")
                            continue;
                        if (paramName.startsWith("x/lr") && paramName != "x/lr0")
                            continue;
                        if (paramName.startsWith("lr/pg")&&paramName != "lr/pg0")
                            continue;
                        addParmItem(csvItem,headerData.at(i),i);
                    }
                data << headLine;
            }

            while (!in.atEnd()) {
                QString line = in.readLine().trimmed();
                if (!line.isEmpty())
                    data<<line;
            }
        }
        file.close();
        csvCache[fileName] = data;
        currentCSVFileName = fileName;  // 记录当前文件名
    }
    csvTableDisplay(data);
}

void MainWindow::onActionClearPlot() {
    curveToItemMap.clear();
    curGraph = nullptr;

    ui->customPlot->clearGraphs();
    ui->customPlot->replot();

    ui->treeWidget->blockSignals(true);
    QTreeWidgetItemIterator it(ui->treeWidget);
    while (*it) {
        if ((*it)->type() == MainWindow::itParamItem) {
            (*it)->setCheckState(MainWindow::colItem, Qt::Unchecked);
        }
        ++it;
    }
    ui->treeWidget->blockSignals(false);
}

void MainWindow::onActionRemoveAllFiles() {
    onActionClearPlot();
    csvCache.clear();
    currentCSVFileName.clear();
    for (int i = 0;i<ui->treeWidget->topLevelItemCount();i++) {
        QTreeWidgetItem* item = ui->treeWidget->topLevelItem(i);
        qDeleteAll(item->takeChildren());
    }
    if (tableModel)
        tableModel->clear();
}

void MainWindow::onActionOpenDir() {
   QString dirPath = QFileDialog::getExistingDirectory(this, tr("Open Directory"), QDir::currentPath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dirPath.isEmpty())
        return;

    QDirIterator it(dirPath, QStringList() << "*.csv", QDir::Files, QDirIterator::Subdirectories);

    while (it.hasNext()) {
        QString fileName = it.next();
        if (!fileName.isEmpty()) {
            QStringList data;

            QTreeWidgetItem* parItem, *item;
            item = ui->treeWidget->currentItem();

            if (item != nullptr) {
                if (item->type() == MainWindow::itCSVItem || item->type() == MainWindow::itParamItem) {
                    while (item->parent() != nullptr && item->type() != MainWindow::itTopItem) {
                        item = item->parent();
                    }
                }
                parItem = item;
            } else {
                ui->treeWidget->addTopLevelItem(new QTreeWidgetItem(MainWindow::itTopItem));
                parItem = ui->treeWidget->topLevelItem(ui->treeWidget->topLevelItemCount() - 1);
            }

            QTreeWidgetItem* csvItem = addCSVItem(parItem, fileName);
            parItem->setExpanded(true);

            QFile file(fileName);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);

                if (!in.atEnd()) {
                    QString headLine = in.readLine();
                    QStringList headerData = headLine.split(",");

                    for (int i = 0; i < headerData.size(); i++) {
                        QString paramName = headerData.at(i).trimmed();
                        if (paramName == "epoch" || paramName == "time")
                            continue;
                        if (paramName.startsWith("x/lr") && paramName != "x/lr0")
                            continue;
                        if (paramName.startsWith("lr/pg") && paramName != "lr/pg0")
                            continue;
                        addParmItem(csvItem, headerData.at(i), i);
                    }
                    data << headLine;
                }

                while (!in.atEnd()) {
                    QString line = in.readLine().trimmed();
                    if (!line.isEmpty())
                        data << line;
                }
            }
            file.close();

            csvCache[fileName] = data;
            currentCSVFileName = fileName;
        }
    }

    if (!currentCSVFileName.isEmpty() && csvCache.contains(currentCSVFileName)) {
        csvTableDisplay(csvCache[currentCSVFileName]);
    }
}

void MainWindow::onTreeContextMenu(const QPoint &pos) {
    QTreeWidgetItem* item = ui->treeWidget->itemAt(pos);
    if (!item)
        return;
    if (item->type() == MainWindow::itCSVItem) {
        QMenu menu;
        QAction* action = menu.addAction("移除该文件");
        connect(action, &QAction::triggered, [this, item]() {removeSingleCSV(item);});
        menu.exec(ui->treeWidget->mapToGlobal(pos));
    }
}

void MainWindow::onActionDeleteCSV() {
    QTreeWidgetItem* item = ui->treeWidget->currentItem();
    if (!item)
        return;
    if (item->type() == MainWindow::itCSVItem)
        removeSingleCSV(item);
    else if (item->type() == MainWindow::itParamItem)
        removeSingleCSV(item->parent());
}


void MainWindow::onActionToggleTree(bool checked) {
    ui->treeWidget->setVisible(checked);
}

void MainWindow::onActionToggleTable(bool checked) {
    ui->tableView->setVisible(checked);
}

void MainWindow::onActionToggleProps(bool checked) {
    if (ui->rightPanelWidget) {
        ui->rightPanelWidget->setVisible(checked);
    }
}

void MainWindow::onActionGuide() {
    QDesktopServices::openUrl(QUrl("https://github.com/GRZ21/CSVResultAnalyzer"));
}

void MainWindow::onActionAbout() {
    QString Text = "<h3>CSV Result Analyzer</h3>"
                "<p><b>开发者：</b>GRZ</p>"
                "<p><b>联系邮箱：</b>grzgrzgrz21@foxmail.com</p>"
                "<p><b>版本：</b>v1.0</p>"
                "<p>本软件专为深度学习（如YOLO和RT-DETR系列）训练日志可视化设计。</p>"
                "<hr>"
                "<p>基于 Qt(C++) 构建。</p>"
                "<p><b>开源协议：</b>本项目遵循 <a href='https://opensource.org/licenses/MIT'>MIT License</a>。</p>"
                "<p><span style='font-size:10px; color:gray;'>Copyright &copy; 2026 GRZ. All rights reserved.</span></p>";
    QMessageBox::about(this, "关于", Text);
}

void MainWindow::csvTableDisplay(QStringList& data) {
    if (data.isEmpty())
        return;

    int rowCount = data.length()-1;
    int columnCount = data[0].split(",").size();

    tableModel->clear();
    tableModel->setRowCount(rowCount);
    tableModel->setColumnCount(columnCount);

    QString header = data.at(0);
    QStringList headerData = header.split(",");

    for (int i = 0; i < columnCount; i++) {
        QStandardItem* item = new QStandardItem(headerData.at(i));
        item->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);
        tableModel->setHorizontalHeaderItem(i,item);
    }
    tableModel->setHorizontalHeaderLabels(headerData);

    QStandardItem* item;
    for (int i = 1; i < data.length(); i++) {
        QString str = data.at(i);
        QStringList rowData = str.split(",");
        for (int j = 0; j < columnCount; j++) {
            item = new QStandardItem(rowData.at(j));
            item->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);
            tableModel->setItem(i-1, j, item);
        }
    }
    ui->tableView->horizontalHeader()->setVisible(true);
    ui->tableView->resizeColumnsToContents();
}

void MainWindow::onTreeItemClicked(QTreeWidgetItem *item, int column) {
    if (item== nullptr)
        return;

    if (item->type() == MainWindow::itCSVItem) {
        QString fileName = item->data(MainWindow::colItem,Qt::UserRole).toString();
        currentCSVFileName = fileName;  // 记录当前文件名
        if (csvCache.contains(fileName))
            csvTableDisplay(csvCache[fileName]);
        else
            loadCSVCache(fileName);
    }else if (item->type() == MainWindow::itParamItem) {
        int paramIndex = item->data(MainWindow::colItem,Qt::UserRole).toInt();

        QTreeWidgetItem* parItem = item->parent();
        currentCSVFileName = parItem->data(MainWindow::colItem,Qt::UserRole).toString();  // 记录当前文件名
        csvTableDisplay(csvCache[currentCSVFileName]);
        highlightColumn(paramIndex);
    }
}

void MainWindow::onTreeItemDoubleClicked(QTreeWidgetItem *item, int column) {
    if (item== nullptr)
        return;
    if (item->type()!=MainWindow::itParamItem)
        return;
    if (item->checkState(MainWindow::colItem)==Qt::Checked)
        item->setCheckState(column,Qt::Unchecked);
    else
        item->setCheckState(column,Qt::Checked);
}

void MainWindow::onCurveClicked(QCPAbstractPlottable *plottable, int dataIndex, QMouseEvent *event) {
    curGraph = qobject_cast<QCPGraph*>(plottable);
    if (!curGraph)
        return;
    QTreeWidgetItem* item = curveToItemMap.value(curGraph,nullptr);
    if (item) {
        ui->treeWidget->setCurrentItem(item);
        ui->treeWidget->scrollToItem(item);
        highlightColumn(item->data(MainWindow::colItem,Qt::UserRole).toInt());
    }

    QString name = curGraph->name();
    QPen pen = curGraph->pen();
    QColor color = pen.color();
    int width = pen.width();
    int alpha = color.alpha()*100/255;

    ui->comboScatter->blockSignals(true);
    ui->editName->blockSignals(true);
    ui->spinWidth->blockSignals(true);
    ui->spinAlpha->blockSignals(true);
    ui->comboStyle->blockSignals(true);

    QCPScatterStyle::ScatterShape shape = curGraph->scatterStyle().shape();
    switch(shape) {
        case QCPScatterStyle::ssNone:     ui->comboScatter->setCurrentIndex(0); break;
        case QCPScatterStyle::ssCircle:   ui->comboScatter->setCurrentIndex(1); break;
        case QCPScatterStyle::ssSquare:   ui->comboScatter->setCurrentIndex(2); break;
        case QCPScatterStyle::ssCross:    ui->comboScatter->setCurrentIndex(3); break;
        case QCPScatterStyle::ssTriangle: ui->comboScatter->setCurrentIndex(4); break;
        default:                          ui->comboScatter->setCurrentIndex(0); break;
    }

    ui->editName->setText(name);
    ui->spinWidth->setValue(width);
    ui->spinAlpha->setValue(alpha);

    if (pen.style() == Qt::SolidLine)
        ui->comboStyle->setCurrentIndex(0);
    else if (pen.style() == Qt::DashLine)
        ui->comboStyle->setCurrentIndex(1);

    ui->editName->blockSignals(false);
    ui->spinWidth->blockSignals(false);
    ui->spinAlpha->blockSignals(false);
    ui->comboStyle->blockSignals(false);
    ui->comboScatter->blockSignals(false);

    updateCurveStats(curGraph);
}

void MainWindow::autoRescalePlot() {
    if (ui->customPlot->graphCount() == 0) {
        ui->customPlot->xAxis->setRange(0, 1);
        ui->customPlot->yAxis->setRange(0, 1);
        ui->customPlot->replot();

        ui->editName->clear();
        ui->labelMaxValue->setText("--");
        ui->labelMinValue->setText("--");
        ui->labelFinalValue->setText("--");
        return;
    }
    ui->customPlot->rescaleAxes();

    double xUpper = ui->customPlot->xAxis->range().upper;
    double yUpper = ui->customPlot->yAxis->range().upper;

    ui->customPlot->xAxis->setRange(0, xUpper * 1.05);
    ui->customPlot->yAxis->setRange(0, yUpper * 1.05);

    ui->customPlot->replot();
}


void MainWindow::onEditName(const QString &name) {
    if (curGraph==nullptr)
        return;
    curGraph->setName(name);
    QTreeWidgetItem* item = curveToItemMap.value(curGraph,nullptr);
    if (item)
        item->setText(MainWindow::colItem,name);
    ui->customPlot->replot();
}

void MainWindow::onSpinWidth(int width) {
    if (curGraph==nullptr)
        return;
    QPen pen = curGraph->pen();
    pen.setWidth(width);
    curGraph->setPen(pen);
    ui->customPlot->replot();
}

void MainWindow::onSpinAlpha(int alpha) {
    if (curGraph==nullptr)
        return;
    QPen pen = curGraph->pen();
    QColor color = pen.color();
    color.setAlpha(alpha*255/100);
    pen.setColor(color);
    curGraph->setPen(pen);

    QCPScatterStyle scatter = curGraph->scatterStyle();
    if (scatter.shape() != QCPScatterStyle::ssNone) {
        scatter.setBrush(QBrush(color));
        curGraph->setScatterStyle(scatter);
    }

    ui->customPlot->replot();
}

void MainWindow::onPropColorClicked() {
    if (curGraph== nullptr)
        return;
    QColor color = curGraph->pen().color();
    QColor newColor = QColorDialog::getColor(color,this,"选择曲线颜色");

    if (newColor.isValid()) {
        QPen pen = curGraph->pen();
        newColor.setAlpha(pen.color().alpha());
        pen.setColor(newColor);
        curGraph->setPen(pen);

        QCPScatterStyle scatter = curGraph->scatterStyle();
        if (scatter.shape() != QCPScatterStyle::ssNone) {
            scatter.setBrush(QBrush(newColor));
            curGraph->setScatterStyle(scatter);
        }

        ui->customPlot->replot();
    }
}

void MainWindow::onComboStyleChanged(int index) {
    if (curGraph== nullptr)
        return;
    QPen pen = curGraph->pen();
    if (index == 0)
        pen.setStyle(Qt::SolidLine);
    else if (index == 1)
        pen.setStyle(Qt::DashLine);

    curGraph->setPen(pen);
    ui->customPlot->replot();
}

void MainWindow::onComboScatterChanged(int index) {
    if (curGraph== nullptr)
        return;

    QCPScatterStyle::ScatterShape shape = QCPScatterStyle::ssNone;
    switch (index) {
        case 0:shape = QCPScatterStyle::ssNone;
            break;
        case 1:shape = QCPScatterStyle::ssCircle;
            break;
        case 2:shape = QCPScatterStyle::ssSquare;
            break;
        case 3:shape = QCPScatterStyle::ssDiamond;
            break;
        case 4:shape = QCPScatterStyle::ssTriangle;
            break;
    }
    QCPScatterStyle scatterStyle(shape,ui->spinScatterSize->value());

    if (shape!=QCPScatterStyle::ssNone) {
        scatterStyle.setBrush(curGraph->pen().color());
        scatterStyle.setPen(Qt::NoPen);
    }
    curGraph->setScatterStyle(scatterStyle);
    ui->customPlot->replot();
}

void MainWindow::onBtnBringTopClicked() {
    if (curGraph== nullptr)
        return;
    if (!ui->customPlot->layer("top_layer"))
        ui->customPlot->addLayer("top_layer",ui->customPlot->layer("main"), QCustomPlot::limAbove);
    if (curGraph->layer()->name() == "top_layer")
        curGraph->setLayer("main");
    curGraph->setLayer("top_layer");
    ui->customPlot->replot();
}

void MainWindow::onBtnSendBottomClicked() {
    if (curGraph== nullptr)
        return;
    if (!ui->customPlot->layer("bottom_layer"))
        ui->customPlot->addLayer("bottom_layer",ui->customPlot->layer("main"), QCustomPlot::limBelow);
    curGraph->setLayer("bottom_layer");
    QList<QCPLayerable *>items = ui->customPlot->layer("bottom_layer")->children();
    for (QCPLayerable *item:items) {
        if (item!=curGraph) {
            item->setLayer("main");
            item->setLayer("bottom_layer");
        }
    }
    ui->customPlot->replot();
}

void MainWindow::onComboLegendChanged(int index) {
    if (index==2)
        ui->customPlot->legend->setVisible(false);
    else {
        ui->customPlot->legend->setVisible(true);

        if (index==1)
            ui->customPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignRight|Qt::AlignBottom);
        else if (index==0)
            ui->customPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignRight|Qt::AlignTop);
    }
    ui->customPlot->replot();
}

void MainWindow::updateGlobalFont() {
    QFont baseFont = ui->comboFont->currentFont();
    int fontSize = ui->spinFontSize->value();
    bool isBold = ui->checkFontBold->isChecked();

    QFont labelFont = baseFont;
    labelFont.setPointSize(fontSize);
    labelFont.setBold(isBold);

    QFont secondaryFont = baseFont;
    secondaryFont.setPointSize(qMax(6, fontSize - 2)); // qMax 防止字号减成负数或太小看不清
    secondaryFont.setBold(false);

    ui->customPlot->xAxis->setLabelFont(labelFont);
    ui->customPlot->xAxis->setTickLabelFont(secondaryFont);

    ui->customPlot->yAxis->setLabelFont(labelFont);
    ui->customPlot->yAxis->setTickLabelFont(secondaryFont);

    ui->customPlot->legend->setFont(secondaryFont);

    ui->customPlot->replot();
}

void MainWindow::onComboGridChanged(int index) {
    QPen gridPen(QColor(220, 220, 220), 1, Qt::DashLine);

    if (index == 0) {
        ui->customPlot->xAxis->grid()->setVisible(false);
        ui->customPlot->yAxis->grid()->setVisible(true);
        ui->customPlot->yAxis->grid()->setPen(gridPen);
    } else if (index == 1) {
        ui->customPlot->xAxis->grid()->setVisible(true);
        ui->customPlot->yAxis->grid()->setVisible(true);
        ui->customPlot->xAxis->grid()->setPen(gridPen);
        ui->customPlot->yAxis->grid()->setPen(gridPen);
    } else if (index == 2) {
        ui->customPlot->xAxis->grid()->setVisible(false);
        ui->customPlot->yAxis->grid()->setVisible(false);
    }

    ui->customPlot->replot();
}

void MainWindow::onBtnExportClicked() {
    QString filter = "PNG 图片 (*.png);;JPEG 图片 (*.jpg);;PDF 矢量图 (LaTeX推荐) (*.pdf)";

    QString fileName = QFileDialog::getSaveFileName(this, "导出图表", "Result_Chart", filter);

    if (fileName.isEmpty())
        return;

    int width = 0, height = 0;
    int ratioIndex = ui->comboExportRatio->currentIndex();
    if (ratioIndex==1) {
        width = 1200;
        height = 900;
    }else if (ratioIndex==2) {
        width = 2400;
        height = 1200;
    }

    if (fileName.endsWith(".png", Qt::CaseInsensitive))
        ui->customPlot->savePng(fileName, width, height, 2.0, 100);
    else if (fileName.endsWith(".jpg", Qt::CaseInsensitive))
        ui->customPlot->saveJpg(fileName, width, height, 2.0, 100);
    else if (fileName.endsWith(".pdf", Qt::CaseInsensitive))
        ui->customPlot->savePdf(fileName,width, height);
}

void MainWindow::onSpinScatterSizeChanged(int size) {
    if (curGraph== nullptr)
        return;
    QCPScatterStyle scatter_style = curGraph->scatterStyle();
    if (curGraph->scatterStyle().shape()!=QCPScatterStyle::ssNone) {
        scatter_style.setSize(size);
        curGraph->setScatterStyle(scatter_style);
        ui->customPlot->replot();
    }
}

void MainWindow::onAxisTitleChanged() {
    ui->customPlot->xAxis->setLabel(ui->editXTitle->text());
    ui->customPlot->yAxis->setLabel(ui->editYTitle->text());
    ui->customPlot->replot();
}

void MainWindow::removeSingleCSV(QTreeWidgetItem *csvItem) {
    if (!csvItem || csvItem->type() != MainWindow::itCSVItem) return;
    for (int i = 0; i < csvItem->childCount(); ++i) {
        QTreeWidgetItem* paramItem = csvItem->child(i);
        removeCurveFromPlot(paramItem);
    }
    QString fileName = csvItem->data(MainWindow::colItem, Qt::UserRole).toString();
    csvCache.remove(fileName);

    if (currentCSVFileName == fileName) {
        currentCSVFileName.clear();
        if (tableModel) tableModel->clear();
    }
    delete csvItem;
}





