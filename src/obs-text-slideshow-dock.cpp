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
            text_slideshows->insert(text_slideshows->begin(), source);
        }
    }

    return true;
}

void TextSlideshowDock::OBSFrontendEvent(enum obs_frontend_event event) {
	switch(event) {
        case OBS_FRONTEND_EVENT_FINISHED_LOADING:
        case OBS_FRONTEND_EVENT_SCENE_CHANGED:
            refresh();
            break;
        default:
            break;
	}
}

void TextSlideshowDock::changeActiveSource(int index) {
    if(index >= 0) {
        setActiveSource(index);
        updateTexts();
    }
}

void TextSlideshowDock::setActiveSource(int index) {
    if(index >= 0 && index < text_slideshows.size()) {
        active_slideshow.source = text_slideshows[index];
        active_slideshow.index = index;
        ui->sourceBox->setCurrentIndex(index);
    } else {
        active_slideshow.source = NULL;
        active_slideshow.index = -1;
    }
}

void TextSlideshowDock::chooseNewActiveSource() {
    if(active_slideshow.index != -1) {
        setActiveSource(active_slideshow.index);

    } else {
        active_slideshow.source = NULL;
        // Find first that is not hidden
        for(unsigned int i = 0; i < text_slideshows.size() && 
                !active_slideshow.source; i++) {
            if(!obs_source_is_hidden(text_slideshows[i])) {
                setActiveSource(i);
                return;
            }
        }

        // Default to first if all hidden
        if(!active_slideshow.source) {
            setActiveSource(0);
        }
    }
}

void TextSlideshowDock::updateSources() {
    obs_source_t *scene_source = NULL;
    obs_scene_t *scene = NULL;

    ui->sourceBox->clear();
    text_slideshows.clear();

    scene_source = obs_frontend_get_current_scene();
    scene = obs_scene_from_source(scene_source);
    obs_scene_enum_items(scene, findTextSlideshowSources, &text_slideshows);

    active_slideshow.index = -1;

    for(unsigned int i = 0; i < text_slideshows.size(); i++) {
        const char *name = obs_source_get_name(text_slideshows[i]);
        ui->sourceBox->addItem(name);

        if(active_slideshow.source == text_slideshows[i]) {
            active_slideshow.index = i;
        }
    }

    if(scene_source) {
        obs_source_release(scene_source);
    }
}

void TextSlideshowDock::updateTexts() {
    texts.clear();
    proc_handler_t *handler = obs_source_get_proc_handler(active_slideshow.source);
    calldata_t cd = {0};
    calldata_set_ptr(&cd, "texts", &texts);
    proc_handler_call(handler, "get_texts", &cd);
    calldata_free(&cd);

    ui->textList->clear();
    for(unsigned int i = 0; i < texts.size(); i++) {
        ui->textList->addItem(texts[i]);
    }
}

void TextSlideshowDock::refresh() {
    updateSources();
            
    if(text_slideshows.size() == 0) {
        ui->sourceBox->addItem("No Text Slide Show sources found on current scene");
        ui->textList->clear();
    } else {
        chooseNewActiveSource();
        updateTexts();
    }
}

void TextSlideshowDock::transition(QListWidgetItem *item) {
    int index = ui->textList->row(item);

    if(index >= 0) {
        proc_handler_t *handler = obs_source_get_proc_handler(active_slideshow.source);
        calldata_t cd = {0};
        calldata_set_int(&cd, "index", index);
        proc_handler_call(handler, "dock_transition", &cd);
        calldata_free(&cd);
    }
}

static void callback(void *data, calldata_t *cd) {
    TextSlideshowDock *dock = reinterpret_cast<TextSlideshowDock *>(data);
    dock->refresh();
}

TextSlideshowDock::TextSlideshowDock(QWidget *parent)
	: QDockWidget(parent),
	  ui(new Ui::TextSlideshowDock) {
	ui->setupUi(this);
    setActiveSource(-1);

    const char *source_signals[] = {
        "source_create",
        "source_destroy",
        "source_rename",
        "source_save"
    };

    signal_handler_t * obs_handler = obs_get_signal_handler();
    for(int i = 0; i < 4; i++) {
        signal_handler_connect(obs_handler, source_signals[i], callback, this);
    }

    connect(ui->sourceBox, QOverload<int>::of(&QComboBox::activated), 
        this, &TextSlideshowDock::changeActiveSource);
    connect(ui->textList, &QListWidget::itemClicked, this,
        &TextSlideshowDock::transition);

	obs_frontend_add_event_callback(OBSFrontendEventWrapper, this);

	hide();
}

TextSlideshowDock::~TextSlideshowDock() {

}