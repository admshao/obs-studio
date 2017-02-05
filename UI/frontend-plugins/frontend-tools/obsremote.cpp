#include <obs-frontend-api.h>
#include <obs-module.h>
#include <obs.hpp>
#include <util/util.hpp>
#include <QAction>
#include <QMainWindow>
#include <QTimer>
#include <QObject>
#include "obsremote.hpp"

using namespace std;

OBSRemote *remote;

OBSRemote::OBSRemote(QWidget *parent) :
    QDialog(parent),
    ui(new Ui_OBSRemote)
{
    ui->setupUi(this);
}

void OBSRemote::ShowHideDialog()
{
    if (!isVisible()) {
        setVisible(true);
        QTimer::singleShot(250, this, SLOT(show()));
    } else {
        setVisible(false);
        QTimer::singleShot(250, this, SLOT(hide()));
    }
}

void OBSRemote::closeEvent(QCloseEvent*)
{

}

extern "C" void EndOBSRemote()
{
}

extern "C" void InitOBSRemote()
{
    QAction *action = (QAction*)obs_frontend_add_tools_menu_qaction(
            obs_module_text("OBSRemote"));

    obs_frontend_push_ui_translation(obs_module_get_string);

    QMainWindow *window = (QMainWindow*)obs_frontend_get_main_window();

    remote = new OBSRemote(window);

    auto cb = [] ()
    {
        remote->ShowHideDialog();
    };

    obs_frontend_pop_ui_translation();

    //obs_frontend_add_save_callback(SaveOutputTimer, nullptr);
    //obs_frontend_add_event_callback(OBSEvent, nullptr);

    action->connect(action, &QAction::triggered, cb);
}
