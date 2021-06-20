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
	const char *name;

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
    active_slideshow.source = text_slideshows[index];
    active_slideshow.index = index;
}

void TextSlideshowDock::chooseNewActiveSource() {
    active_slideshow.source = NULL;
    // Find first that is not hidden
    for(int i = 0; i < text_slideshows.size() && !active_slideshow.source; i++) {
        if(!obs_source_is_hidden(text_slideshows[i])) {
            setActiveSource(i);
        }
    }

    // Default to first if all hidden
    if(!active_slideshow.source) {
        setActiveSource(0);
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

    for(int i = 0; i < text_slideshows.size(); i++) {
        const char *name = obs_source_get_name(text_slideshows[i]);
        ui->sourceBox->addItem(name);
    }

    if(scene_source) {
        obs_source_release(scene_source);
    }
}

void TextSlideshowDock::updateTexts() {
    texts.clear();
    obs_source_enum_full_tree(active_slideshow.source, enumChildSources, &texts);

    ui->textList->clear();
    for(int i = 0; i < texts.size(); i++) {
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

TextSlideshowDock::TextSlideshowDock(QWidget *parent)
	: QDockWidget(parent),
	  ui(new Ui::TextSlideshowDock) {
	ui->setupUi(this);

    connect(ui->sourceBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
        this, &TextSlideshowDock::changeActiveSource);
    connect(ui->refreshButton, &QPushButton::released, this, 
        &TextSlideshowDock::refresh);

	obs_frontend_add_event_callback(OBSFrontendEventWrapper, this);

	hide();
}

TextSlideshowDock::~TextSlideshowDock() {

}