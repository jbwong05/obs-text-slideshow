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

#pragma once

#include <obs-frontend-api.h>
#include <QDockWidget>
#include <memory>
#include <vector>

#include "ui_obs-text-slideshow-dock.h"

using std::vector;

struct slideshow_t {
    obs_source_t *source;
    int index;
};

class TextSlideshowDock : public QDockWidget {
    Q_OBJECT

    private:
        static void OBSFrontendEventWrapper(enum obs_frontend_event event, 
            void *ptr);
        void OBSFrontendEvent(enum obs_frontend_event event);
        void changeActiveSource(int index);
        void setActiveSource(int index);
        void chooseNewActiveSource();
        void updateSources();
        void updateTexts();
        void transition(QListWidgetItem *item);

        std::unique_ptr<Ui::TextSlideshowDock> ui;
        vector<obs_source_t *> text_slideshows;
        vector<const char *> texts;
        struct slideshow_t active_slideshow;

    public:
        TextSlideshowDock(QWidget *parent = nullptr);
        ~TextSlideshowDock();
        void refresh();
};