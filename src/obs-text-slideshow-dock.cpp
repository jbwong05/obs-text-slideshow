/*
obs-text-slideshow
Copyright (C) 2021 Joshua Wong jbwong05@gmail.com

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include "obs-text-slideshow-dock.h"

void TextSlideshowDock::OBSFrontendEventWrapper(enum obs_frontend_event event, 
        void *ptr) {
	TextSlideshowDock *dock = reinterpret_cast<TextSlideshowDock *>(ptr);
	dock->OBSFrontendEvent(event);
}

void TextSlideshowDock::OBSFrontendEvent(enum obs_frontend_event event) {
	obs_source_t *scene = NULL;
	const char *name;

	switch (event) {
        case OBS_FRONTEND_EVENT_SCENE_CHANGED:
            /*if (ui->targetButton_program->isChecked())
                scene = obs_frontend_get_current_scene();*/
            break;
        case OBS_FRONTEND_EVENT_PREVIEW_SCENE_CHANGED:
            /*if (ui->targetButton_preview->isChecked())
                scene = obs_frontend_get_current_preview_scene();*/
            break;
        default:
            break;
	}

	if (!scene)
		return;

	/*name = obs_source_get_name(scene);
	for (unsigned long int i = 0; i < PTZDevice::device_count(); i++) {
		PTZDevice *ptz = PTZDevice::get_device(i);
		if (ptz->objectName() == name) {
			setCurrent(i);
			break;
		}
	}*/

	//obs_source_release(scene);
}

TextSlideshowDock::TextSlideshowDock(QWidget *parent)
	: QDockWidget(parent),
	  ui(new Ui::TextSlideshowDock) {
	ui->setupUi(this);
	/*ui->cameraList->setModel(PTZDevice::model());

	QItemSelectionModel *selectionModel = ui->cameraList->selectionModel();
	connect(selectionModel,
		SIGNAL(currentChanged(QModelIndex, QModelIndex)),
		this, SLOT(currentChanged(QModelIndex, QModelIndex)));

	LoadConfig();

	QGamepadManager* gamepad_manager = QGamepadManager::instance();

	// from https://stackoverflow.com/questions/62668629/qgamepadmanager-connecteddevices-empty-but-windows-detects-gamepad
	QWindow* wnd = new QWindow();
	wnd->show();
	delete wnd;
	qApp->processEvents();

	QList<int> gamepads = gamepad_manager->connectedGamepads();

	blog(LOG_INFO, "gamepads found %d", gamepads.size());

	if (!gamepads.isEmpty()) {
		gamepad = new QGamepad(*gamepads.begin(), this);

		connect(gamepad, &QGamepad::axisLeftXChanged, this,
				&PTZControls::on_panTiltGamepad);
		connect(gamepad, &QGamepad::axisLeftYChanged, this,
				&PTZControls::on_panTiltGamepad);
	}

	ui->speedSlider->setValue(10);
	ui->speedSlider->setMinimum(0);
	ui->speedSlider->setMaximum(0x14);*/

	obs_frontend_add_event_callback(OBSFrontendEventWrapper, this);

	hide();
}

TextSlideshowDock::~TextSlideshowDock() {
	//signal_handler_disconnect_global(obs_get_signal_handler(), OBSSignal, this);
	/*SaveConfig();
	PTZDevice::delete_all();
	deleteLater();*/
}