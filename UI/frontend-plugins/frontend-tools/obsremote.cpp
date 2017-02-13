#include <obs-frontend-api.h>
#include <obs-module.h>
#include <obs.hpp>
#include <util/util.hpp>
#include <QAction>
#include <QMainWindow>
#include <QTimer>
#include <QObject>
#include "obsremote.hpp"

#include <condition_variable>
#include <chrono>
#include <string>
#include <thread>
#include <mutex>

#define do_log(type, format, ...) blog(type, "[OBSRemote] " format, \
                ##__VA_ARGS__)

#define error(format, ...) do_log(LOG_ERROR, format, ##__VA_ARGS__)
#define info(format, ...) do_log(LOG_INFO, format, ##__VA_ARGS__)

using namespace std;

#define DEFAULT_PORT 4444
#define DEFAULT_INTERVAL 1000

struct OBSRemoteData
{
	thread th;
	condition_variable cv;
	mutex m;

	bool enabled = false;
	int interval = DEFAULT_INTERVAL;
	int port = DEFAULT_PORT;
	string password;

	void Loop();

	void Start();

	void Stop();

	inline ~OBSRemoteData()
	{
		Stop();
	}
};

static OBSRemoteData *obsremote = nullptr;
static OBSRemote *obsr = nullptr;

OBSRemote::OBSRemote(QWidget *parent) :
	QDialog(parent),
	ui(new Ui_OBSRemote)
{
	ui->setupUi(this);

	ui->edit_port->setValidator(new QIntValidator(1000, 65535, this));

	QObject::connect(ui->enabled, SIGNAL(clicked()), this,
	                 SLOT(EnableClicked()));
}

void OBSRemoteData::Loop()
{
	info("Loop");
	chrono::duration<long long, milli> duration =
		chrono::milliseconds(interval);

	for (;;)
	{
		unique_lock<mutex> lock(m);

		cv.wait_for(lock, duration);
		if (!obsremote->enabled)
		{
			break;
		}

		duration = chrono::milliseconds(interval);

		info("Teste");
	}
}

void OBSRemoteData::Start()
{
	info("Start");
	if (!obsremote->th.joinable())
	{
		info("Joinable");
		obsremote->th = thread([]()
		                       { obsremote->Loop(); });
	}
}

void OBSRemoteData::Stop()
{
	info("Stop");
	if (th.joinable())
	{
		info("Joinable");
		{
			lock_guard<mutex> lock(m);
			enabled = false;
		}
		cv.notify_one();
		th.join();
	}
}

void OBSRemote::EnableClicked()
{
	if (ui->enabled->isChecked())
	{
		obsremote->enabled = true;
		obsremote->Start();
		ui->edit_password->setDisabled(true);
		ui->edit_port->setDisabled(true);
	} else
	{
		obsremote->Stop();
		ui->edit_password->setDisabled(false);
		ui->edit_port->setDisabled(false);
	}
}

static void SaveOBSRemote(obs_data_t *save_data, bool saving, void *)
{
	if (saving)
	{
		lock_guard<mutex> lock(obsremote->m);
		obs_data_t *obj = obs_data_create();

		obs_data_set_bool(obj, "enabled",
		                  obsr->ui->enabled->isChecked());
		int port = obsr->ui->edit_port->text().toInt();
		if (port > 0)
		{
			obs_data_set_int(obj, "port", port);
		} else
		{
			obs_data_set_int(obj, "port", DEFAULT_PORT);
		}

		obs_data_set_string(obj, "password",
		                    obsr->ui->edit_password->text()
			                    .toStdString().c_str());

		obs_data_set_obj(save_data, "obsremote", obj);
		obs_data_release(obj);
	} else
	{
		obsremote->Stop();

		obsremote->m.lock();

		obs_data_t *obj = obs_data_get_obj(save_data, "obsremote");

		if (!obj)
			obj = obs_data_create();

		obs_data_set_default_int(obj, "port", DEFAULT_PORT);

		obsremote->enabled = obs_data_get_bool(obj, "enabled");
		obsr->ui->enabled->setChecked(obsremote->enabled);
		obsr->EnableClicked();
		obsremote->port = (int) obs_data_get_int(obj, "port");
		obsr->ui->edit_port->setText(QString::number(obsremote->port));
		obsremote->password = obs_data_get_string(obj, "password");
		obsr->ui->edit_password->setText(obsremote->password.c_str());

		obs_data_release(obj);
		obsremote->m.unlock();

		if (obsremote->enabled)
			obsremote->Start();
	}
}

void OBSRemote::closeEvent(QCloseEvent *)
{
	info("Window Close");
	obs_frontend_save();
}

extern "C" void EndOBSRemote()
{
	info("End");
	delete obsremote;
	obsremote = nullptr;
	delete obsr;
	obsr = nullptr;
}

static void OBSEvent(enum obs_frontend_event event, void *)
{
	info("Event");
	if (event == OBS_FRONTEND_EVENT_EXIT)
	{
		info("Exit");
		EndOBSRemote();
	}
}

extern "C" void InitOBSRemote()
{

	info("Init");
	QAction *action = (QAction *) obs_frontend_add_tools_menu_qaction(
		obs_module_text("OBSRemote"));

	obs_frontend_push_ui_translation(obs_module_get_string);

	QWidget *window = (QWidget *) obs_frontend_get_main_window();

	obsremote = new OBSRemoteData;
	obsr = new OBSRemote(window);

	auto cb = []()
	{
		info("OBSRemote Window Exec");
		obsr->exec();
	};

	obs_frontend_pop_ui_translation();

	obs_frontend_add_save_callback(SaveOBSRemote, nullptr);
	obs_frontend_add_event_callback(OBSEvent, nullptr);

	action->connect(action, &QAction::triggered, cb);
}
