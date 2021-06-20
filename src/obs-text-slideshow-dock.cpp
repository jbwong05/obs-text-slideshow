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

static bool findTextSlideshowSources(obs_scene_t *scene, 
        obs_sceneitem_t *item, void *param) {
    
    obs_source_t *source = NULL;
    source = obs_sceneitem_get_source(item);

    if(source) {
        const char *id = obs_source_get_id(source);

        if(strcmp(id, "text-freetype2-slideshow") == 0
#ifdef _WIN32
            || strcmp(id, "text-gdiplus-slideshow") == 0
#endif
        ) {
            vector<obs_source_t *> *text_slideshows = reinterpret_cast<vector<obs_source_t *> *>(param);
            text_slideshows->push_back(source);
        }
    }

    return true;
}

static void enumChildSources(obs_source_t *parent,
	    obs_source_t *child, void *param) {
    vector<const char *> *texts = reinterpret_cast<vector<const char *> *>(param);
    const char *text = obs_source_get_name(child);
    texts->push_back(text);
}


void TextSlideshowDock::OBSFrontendEvent(enum obs_frontend_event event) {
	obs_source_t *scene_source = NULL;
    obs_scene_t *scene = NULL;
	const char *name;

	switch (event) {
        case OBS_FRONTEND_EVENT_SCENE_CHANGED:
            ui->sourceBox->clear();
            text_slideshows.clear();

            scene_source = obs_frontend_get_current_scene();
            scene = obs_scene_from_source(scene_source);
            obs_scene_enum_items(scene, findTextSlideshowSources, &text_slideshows);

            for(int i = 0; i < text_slideshows.size(); i++) {
                const char *name = obs_source_get_name(text_slideshows[i]);
                ui->sourceBox->addItem(name);
            }

            if(text_slideshows.size() == 0) {
                ui->sourceBox->addItem("No Text Slide Show sources found on current scene");
            } else {
                active_slideshow.source = NULL;
                // Find first that is not hidden
                for(int i = 0; i < text_slideshows.size() && !active_slideshow.source; i++) {
                    if(!obs_source_is_hidden(text_slideshows[i])) {
                        active_slideshow.source = text_slideshows[i];
                        active_slideshow.index = i;
                    }
                }

                // Default to first if all hidden
                if(!active_slideshow.source) {
                    active_slideshow.source = text_slideshows[0];
                    active_slideshow.index = 0;
                }

                updateTexts();
            }

            break;
        default:
            break;
	}

    if(scene_source) {
        obs_source_release(scene_source);
    }
}

void TextSlideshowDock::changeActiveSource(int index) {

}

void TextSlideshowDock::updateTexts() {
    texts.clear();
    obs_source_enum_full_tree(active_slideshow.source, enumChildSources, &texts);

    ui->textList->clear();
    for(int i = 0; i < texts.size(); i++) {
        ui->textList->addItem(texts[i]);
    }
}

TextSlideshowDock::TextSlideshowDock(QWidget *parent)
	: QDockWidget(parent),
	  ui(new Ui::TextSlideshowDock) {
	ui->setupUi(this);

    connect(ui->sourceBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
        this, &TextSlideshowDock::changeActiveSource);

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