//
// Created by GRZ.
//

#ifndef CSVREADER_MAINWINDOW_H
#define CSVREADER_MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QTreeWidgetItem>
#include <QMap>

#include "../utils/qcustomplot.h"

QT_BEGIN_NAMESPACE

namespace Ui {
    class MainWindow;
}

QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

private:
    enum treeItemType{itTopItem=1001,itCSVItem,itParamItem};
    enum treeColNum{colItem=0,colDate};

    void buildTreeHeader();
    void initTree();

    QTreeWidgetItem* addCSVItem(QTreeWidgetItem* parItem,QString fileName);
    void addParmItem(QTreeWidgetItem* parItem,QString CSVHeader,int colIndex);
    void loadCSVCache(QString fileName);
    void highlightColumn(int columIndex);
    void highlightTableParam(QModelIndex& columIndex);

    void poltCsvColumn(QTreeWidgetItem* item,int columIndex);
    void drawOnCustomPlot(QString name,QVector<double>x,QVector<double>y,QTreeWidgetItem* item);
    void onParmItemChanged(QTreeWidgetItem* item,int columIndex);
    void removeCurveFromPlot(QTreeWidgetItem*  item);

    void updateCurveStats(QCPGraph *graph);

    QStandardItemModel* tableModel;
    QItemSelectionModel* tableSelectionModel;
    QMap<QString,QStringList>csvCache;
    QString currentCSVFileName;  // 记录当前显示的CSV文件名
    QHash<QCPGraph*,QTreeWidgetItem*> curveToItemMap;
    QCPGraph* curGraph = nullptr;

private slots:
    void csvTableDisplay(QStringList&  data);
    void onTreeItemClicked(QTreeWidgetItem* item,int column);
    void onTreeItemDoubleClicked(QTreeWidgetItem* item,int column);
    void onCurveClicked(QCPAbstractPlottable *plottable, int dataIndex, QMouseEvent *event);
    void autoRescalePlot();
    void onEditName(const QString& name);
    void onSpinWidth(int width);
    void onSpinAlpha(int alpha);
    void onPropColorClicked();
    void onComboStyleChanged(int index);
    void onComboScatterChanged(int index);
    void onBtnBringTopClicked();
    void onBtnSendBottomClicked();
    void onComboLegendChanged(int index);
    void updateGlobalFont();
    void onComboGridChanged(int index);
    void onBtnExportClicked();
    void onSpinScatterSizeChanged(int size);
    void onAxisTitleChanged();
    void removeSingleCSV(QTreeWidgetItem* csvItem);

    //ACtion
    void actionOpen();
    // 全局清理与数据操作
    void onActionClearPlot();
    void onActionRemoveAllFiles();
    void onActionOpenDir();
    void onTreeContextMenu(const QPoint &pos);
    void onActionDeleteCSV();
    // 视图控制 (接收 action 的 Check 状态)
    void onActionToggleTree(bool checked);
    void onActionToggleTable(bool checked);
    void onActionToggleProps(bool checked);

    void onActionGuide();
    void onActionAbout();
private:
    Ui::MainWindow *ui;
};


#endif //CSVREADER_MAINWINDOW_H
