#pragma once

#include <obs-module.h>
#include <QDialog>
#include <memory>

#include "ui_obsremote.h"

#define do_log(type, format, ...) blog(type, "[OBSRemote] " format, \
                ##__VA_ARGS__)

#define error(format, ...) do_log(LOG_ERROR, format, ##__VA_ARGS__)
#define info(format, ...) do_log(LOG_INFO, format, ##__VA_ARGS__)

using namespace std;

class OBSRemote : public QDialog
{
Q_OBJECT

public:
	OBSRemote(QWidget *parent);

	void closeEvent(QCloseEvent *event) override;

	unique_ptr<Ui_OBSRemote> ui;
public slots:

	void CheckStatus();

};
