#include <QAction>
#include <QTimer>
#include "obsremote.hpp"
#include "obsremote-message-handler.hpp"
#include "obsremote-event-handler.hpp"

#include <thread>
#include <mutex>
#include <libwebsockets.h>

using namespace std;

struct OBSRemoteData
{
	thread th;
	mutex m;

	volatile bool enabled = false;

	void Loop();

	void Start();

	void Stop();

	inline ~OBSRemoteData()
	{
		Stop();
	}
};

static OBSRemoteData *obsremote_data = nullptr;
static OBSRemote *obsremote = nullptr;
static OBSRemoteConfig *obsremote_config = nullptr;
static OBSRemoteEventHandler *obsremote_event_handler = nullptr;

int callback_obsapi(struct lws *wsi, enum lws_callback_reasons reason,
                    void *user, void *in, size_t len)
{
	if (user == NULL)
		return 0;

	OBSAPIMessageHandler **userp = (OBSAPIMessageHandler **) user;
	OBSAPIMessageHandler *messageHandler = *(userp);

	switch (reason) {
	case LWS_CALLBACK_ESTABLISHED: {
		*userp = new OBSAPIMessageHandler();
	}
		break;
	case LWS_CALLBACK_SERVER_WRITEABLE: {
		if (!messageHandler->messagesToSend.empty()) {
			obs_data_t *message = messageHandler->messagesToSend
				.front();
			messageHandler->messagesToSend.pop_front();

			const char *messageText = obs_data_get_json(message);

			if (messageText) {
				size_t sendLength = strlen(messageText);

				char *messageBuf =
					(char *) malloc(LWS_PRE + sendLength);
				memcpy(messageBuf + LWS_PRE, messageText,
				       sendLength);

				if (lws_write(
					wsi, (unsigned char *) messageBuf +
					     LWS_PRE, sendLength,
					LWS_WRITE_TEXT) < 0) {
					error("ERROR writing to socket");
				}
				free(messageBuf);
			}
			obs_data_release(message);

			lws_callback_on_writable(wsi);
		}
	}
		break;
	case LWS_CALLBACK_RECEIVE: {
		if (messageHandler->HandleReceivedMessage(in, len)) {
			lws_callback_on_writable(wsi);
		}
	}
		break;
	case LWS_CALLBACK_CLOSED: {
		delete (*userp);
	}
		break;
	default:
		break;
	}

	return 0;
}

static struct lws_protocols protocols[] = {
	{
		"obsapi",
		callback_obsapi,
		     sizeof(OBSAPIMessageHandler *),
		        1024,
	},
	{NULL, NULL, 0, 0}
};

OBSRemote::OBSRemote(QWidget *parent) :
	QDialog(parent),
	ui(new Ui_OBSRemote)
{
	ui->setupUi(this);

	ui->edit_port->setValidator(new QIntValidator(1000, 65535, this));

	QObject::connect(ui->enabled, SIGNAL(clicked()), this,
	                 SLOT(CheckStatus()));
}

void OBSRemoteData::Loop()
{
	struct lws_context *context;
	struct lws_context_creation_info info;

	memset(&info, 0, sizeof info);
	info.port = obsremote_config->port;
	info.protocols = protocols;
	context = lws_create_context(&info);
	if (context == NULL) {
		error("Failed to create lws context);");
		return;
	}
	info("lws context created");

	while (enabled) {
		lws_service(context, 30);
	}

	lws_context_destroy(context);
	info("lws context finished");
}

void OBSRemoteData::Start()
{
	if (!th.joinable()) {
		enabled = true;
		th = thread([]()
		            { obsremote_data->Loop(); });
	}
}

void OBSRemoteData::Stop()
{
	if (th.joinable()) {
		enabled = false;
		th.join();
	}
}

void OBSRemote::CheckStatus()
{
	obsremote_data->m.lock();
	if (ui->enabled->isChecked()) {
		ui->edit_password->setDisabled(true);
		ui->edit_port->setDisabled(true);
		obsremote_data->Start();
	} else {
		ui->edit_password->setDisabled(false);
		ui->edit_port->setDisabled(false);
		obsremote_data->Stop();
	}
	obsremote_data->m.unlock();
}

void OBSRemote::closeEvent(QCloseEvent *event)
{
	if (obsremote_config && obsremote_config->listenToChanges) {
		obsremote_config->enabled = ui->enabled->isChecked();
		obsremote_config->port = ui->edit_port->text().toInt();
		if (obsremote_config->port == 0)
			obsremote_config->port = DEFAULT_PORT;

		obsremote_config->SetPassword(ui->edit_password->text()
			                              .toStdString());
		obsremote_config->listenToChanges = false;
	}
}

static void LoadSaveOBSRemote(obs_data_t *save_data, bool saving, void *)
{
	//obs_data_set_obj(save_data, CONFIG_NAME, NULL);

	if (saving) {
		obs_data_t *obj = obs_data_create();

		obs_data_set_bool(obj, PARAM_ENABLED,
		                  obsremote_config->enabled);
		obs_data_set_int(obj, PARAM_PORT, obsremote_config->port);
		obs_data_set_string(obj, PARAM_PASSWORD,
		                    obsremote_config->password.c_str());
		obs_data_set_string(obj, PARAM_SECRET,
		                    obsremote_config->secret.c_str());
		obs_data_set_string(obj, PARAM_SALT,
		                    obsremote_config->salt.c_str());

		obs_data_set_obj(save_data, CONFIG_NAME, obj);
		obs_data_release(obj);
	} else {
		obs_data_t *obj = obs_data_get_obj(save_data, CONFIG_NAME);

		if (obj) {
			obsremote_config->enabled = obs_data_get_bool(
				obj, PARAM_ENABLED);
			obsremote_config->port = (int) obs_data_get_int(
				obj, PARAM_PORT);
			obsremote_config->password = obs_data_get_string(
				obj, PARAM_PASSWORD);
			obsremote_config->secret = obs_data_get_string(
				obj, PARAM_SECRET);
			obsremote_config->salt = obs_data_get_string(
				obj, PARAM_SALT);

			obs_data_release(obj);
		}

		obsremote->ui->enabled->setChecked(obsremote_config->enabled);
		obsremote->ui->edit_port->setText
			(QString::number(obsremote_config->port));
		obsremote->ui->edit_password->setText
			(obsremote_config->password.c_str());

		obsremote->CheckStatus();
	}
}

extern "C" void EndOBSRemote()
{
	delete obsremote_event_handler;
	obsremote_event_handler = nullptr;
	delete obsremote_data;
	obsremote_data = nullptr;
	delete obsremote_config;
	obsremote_config = nullptr;
}

extern "C" void InitOBSRemote()
{
	QAction *action = (QAction *) obs_frontend_add_tools_menu_qaction(
		obs_module_text("OBSRemote"));

	obs_frontend_push_ui_translation(obs_module_get_string);
	QWidget *window = (QWidget *) obs_frontend_get_main_window();

	obsremote_config = OBSRemoteConfig::GetInstance();
	obsremote = new OBSRemote(window);
	obsremote_data = new OBSRemoteData;
	obsremote_event_handler = new OBSRemoteEventHandler(obsremote);

	auto cb = []()
	{
		obsremote_config->listenToChanges = true;
		obsremote->exec();
	};

	obs_frontend_pop_ui_translation();

	obs_frontend_add_save_callback(LoadSaveOBSRemote, nullptr);

	action->connect(action, &QAction::triggered, cb);
}
