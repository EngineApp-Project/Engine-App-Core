/***********************************************************************
***********Copyright (c) 2011-2014 The Bitcoin developers***************
*************Copyright (c) 2014-2015 The Dash developers****************
*************Copyright (c) 2015-2019 The PIVX developers****************
******************Copyright (c) 2010-2023 Nur1Labs**********************
>Distributed under the MIT/X11 software license, see the accompanying
>file COPYING or http://www.opensource.org/licenses/mit-license.php.
************************************************************************/

#if defined(HAVE_CONFIG_H)
#include "config/mubdi-config.h"
#endif

#include "addressbookpage.h"
#include "ui_addressbookpage.h"

#include "addresstablemodel.h"
#include "qt/mubdi/mubdigui.h"
#include "csvmodelwriter.h"
#include "editaddressdialog.h"
#include "guiutil.h"

#include <QIcon>
#include <QMenu>
#include <QMessageBox>
#include <QSortFilterProxyModel>

AddressBookPage::AddressBookPage(Mode mode, Tabs tab, QWidget* parent) : QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
                                                                         ui(new Ui::AddressBookPage),
                                                                         model(0),
                                                                         mode(mode),
                                                                         tab(tab)
{
    ui->setupUi(this);

#ifdef Q_OS_MAC // Icons on push buttons are very uncommon on Mac
    ui->newAddress->setIcon(QIcon());
    ui->copyAddress->setIcon(QIcon());
    ui->deleteAddress->setIcon(QIcon());
    ui->exportButton->setIcon(QIcon());
#endif

    switch (mode) {
    case ForSelection:
        switch (tab) {
        case SendingTab:
            setWindowTitle(tr("Choose the address to send coins to"));
            break;
        case ReceivingTab:
            setWindowTitle(tr("Choose the address to receive coins with"));
            break;
        }
        connect(ui->tableView, &QTableView::doubleClicked, this, &QDialog::accept);
        ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->tableView->setFocus();
        ui->closeButton->setText(tr("C&hoose"));
        ui->exportButton->hide();
        break;
    case ForEditing:
        switch (tab) {
        case SendingTab:
            setWindowTitle(tr("Sending addresses"));
            break;
        case ReceivingTab:
            setWindowTitle(tr("Receiving addresses"));
            break;
        }
        break;
    }
    switch (tab) {
    case SendingTab:
        ui->labelExplanation->setText(tr("These are your engineapp addresses for sending payments. Always check the amount and the receiving address before sending coins."));
        ui->deleteAddress->setVisible(true);
        break;
    case ReceivingTab:
        ui->labelExplanation->setText(tr("These are your engineapp addresses for receiving payments. It is recommended to use a new receiving address for each transaction."));
        ui->deleteAddress->setVisible(false);
        break;
    }

    // Context menu actions
    QAction* copyAddressAction = new QAction(tr("&Copy Address"), this);
    QAction* copyLabelAction = new QAction(tr("Copy &Label"), this);
    QAction* editAction = new QAction(tr("&Edit"), this);
    deleteAction = new QAction(ui->deleteAddress->text(), this);

    // Build context menu
    contextMenu = new QMenu();
    contextMenu->addAction(copyAddressAction);
    contextMenu->addAction(copyLabelAction);
    contextMenu->addAction(editAction);
    if (tab == SendingTab)
        contextMenu->addAction(deleteAction);
    contextMenu->addSeparator();

    // Connect signals for context menu actions
    connect(copyAddressAction, &QAction::triggered, this, &AddressBookPage::on_copyAddress_clicked);
    connect(copyLabelAction, &QAction::triggered, this, &AddressBookPage::onCopyLabelAction);
    connect(editAction, &QAction::triggered, this, &AddressBookPage::onEditAction);
    connect(deleteAction, &QAction::triggered, this, &AddressBookPage::on_deleteAddress_clicked);

    connect(ui->tableView, &QWidget::customContextMenuRequested, this, &AddressBookPage::contextualMenu);

    connect(ui->closeButton, &QPushButton::clicked, this, &QDialog::accept);
}

AddressBookPage::~AddressBookPage()
{
    delete ui;
}

void AddressBookPage::setModel(AddressTableModel* model)
{
    this->model = model;
    if (!model)
        return;

    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    proxyModel->setDynamicSortFilter(true);
    proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    switch (tab) {
    case ReceivingTab:
        // Receive filter
        proxyModel->setFilterRole(AddressTableModel::TypeRole);
        proxyModel->setFilterFixedString(AddressTableModel::Receive);
        break;
    case SendingTab:
        // Send filter
        proxyModel->setFilterRole(AddressTableModel::TypeRole);
        proxyModel->setFilterFixedString(AddressTableModel::Send);
        break;
    }
    ui->tableView->setModel(proxyModel);
    ui->tableView->sortByColumn(0, Qt::AscendingOrder);

    // Set column widths
    ui->tableView->horizontalHeader()->setSectionResizeMode(AddressTableModel::Label, QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(AddressTableModel::Address, QHeaderView::ResizeToContents);

    connect(ui->tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &AddressBookPage::selectionChanged);

    // Select row for newly created address
    connect(model, &AddressTableModel::rowsInserted, this, &AddressBookPage::selectNewAddress);

    selectionChanged();
}

void AddressBookPage::on_copyAddress_clicked()
{
    GUIUtil::copyEntryData(ui->tableView, AddressTableModel::Address);
}

void AddressBookPage::onCopyLabelAction()
{
    GUIUtil::copyEntryData(ui->tableView, AddressTableModel::Label);
}

void AddressBookPage::onEditAction()
{
    if (!model)
        return;

    if (!ui->tableView->selectionModel())
        return;
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedRows();
    if (indexes.isEmpty())
        return;

    EditAddressDialog dlg(
        tab == SendingTab ?
            EditAddressDialog::EditSendingAddress :
            EditAddressDialog::EditReceivingAddress,
        this);
    dlg.setModel(model);
    QModelIndex origIndex = proxyModel->mapToSource(indexes.at(0));
    dlg.loadRow(origIndex.row());
    dlg.exec();
}

void AddressBookPage::on_newAddress_clicked()
{
    if (!model)
        return;

    EditAddressDialog dlg(
        tab == SendingTab ?
            EditAddressDialog::NewSendingAddress :
            EditAddressDialog::NewReceivingAddress,
        this);
    dlg.setModel(model);
    if (dlg.exec()) {
        newAddressToSelect = dlg.getAddress();
    }
}

void AddressBookPage::on_deleteAddress_clicked()
{
    QTableView* table = ui->tableView;
    if (!table->selectionModel())
        return;

    QModelIndexList indexes = table->selectionModel()->selectedRows();
    if (!indexes.isEmpty()) {
        table->model()->removeRow(indexes.at(0).row());
    }
}

void AddressBookPage::selectionChanged()
{
    // Set button states based on selected tab and selection
    QTableView* table = ui->tableView;
    if (!table->selectionModel())
        return;

    if (table->selectionModel()->hasSelection()) {
        switch (tab) {
        case SendingTab:
            // In sending tab, allow deletion of selection
            ui->deleteAddress->setEnabled(true);
            ui->deleteAddress->setVisible(true);
            deleteAction->setEnabled(true);
            break;
        case ReceivingTab:
            // Deleting receiving addresses, however, is not allowed
            ui->deleteAddress->setEnabled(false);
            ui->deleteAddress->setVisible(false);
            deleteAction->setEnabled(false);
            break;
        }
        ui->copyAddress->setEnabled(true);
    } else {
        ui->deleteAddress->setEnabled(false);
        ui->copyAddress->setEnabled(false);
    }
}

void AddressBookPage::done(int retval)
{
    QTableView* table = ui->tableView;
    if (!table->selectionModel() || !table->model())
        return;

    // Figure out which address was selected, and return it
    QModelIndexList indexes = table->selectionModel()->selectedRows(AddressTableModel::Address);

    for (QModelIndex index : indexes) {
        QVariant address = table->model()->data(index);
        returnValue = address.toString();
    }

    if (returnValue.isEmpty()) {
        // If no address entry selected, return rejected
        retval = Rejected;
    }

    QDialog::done(retval);
}

void AddressBookPage::on_exportButton_clicked()
{
    // CSV is currently the only supported format
    QString filename = GUIUtil::getSaveFileName(this,
        tr("Export Address List"), QString(),
        tr("Comma separated file (*.csv)"), NULL);

    if (filename.isNull())
        return;

    CSVModelWriter writer(filename);

    // name, column, role
    writer.setModel(proxyModel);
    writer.addColumn("Label", AddressTableModel::Label, Qt::EditRole);
    writer.addColumn("Address", AddressTableModel::Address, Qt::EditRole);

    if (!writer.write()) {
        QMessageBox::critical(this, tr("Exporting Failed"),
            tr("There was an error trying to save the address list to %1. Please try again.").arg(filename));
    }
}

void AddressBookPage::contextualMenu(const QPoint& point)
{
    QModelIndex index = ui->tableView->indexAt(point);
    if (index.isValid()) {
        contextMenu->exec(QCursor::pos());
    }
}

void AddressBookPage::selectNewAddress(const QModelIndex& parent, int begin, int /*end*/)
{
    QModelIndex idx = proxyModel->mapFromSource(model->index(begin, AddressTableModel::Address, parent));
    if (idx.isValid() && (idx.data(Qt::EditRole).toString() == newAddressToSelect)) {
        // Select row of newly created address, once
        ui->tableView->setFocus();
        ui->tableView->selectRow(idx.row());
        newAddressToSelect.clear();
    }
}
