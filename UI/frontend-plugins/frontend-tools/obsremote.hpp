#pragma once

#include <QDialog>
#include <memory>

#include "ui_obsremote.h"

class QCloseEvent;

class OBSRemote : public QDialog
{
    Q_OBJECT

public:
    std::unique_ptr<Ui_OBSRemote> ui;
    OBSRemote(QWidget *parent);

    void closeEvent(QCloseEvent *event) override;

public slots:
    void ShowHideDialog();

};
