/*
    Bauer stereophonic-to-binaural plugin for DeaDBeeF
    Copyright (C) 2010 Steven McDonald <steven.mcdonald@libremail.me>
    Uses libstereo_widener by Boris Mikhaylov <http://stereo_widener.sourceforge.net/>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <deadbeef/deadbeef.h>
#include <math.h>

//#define trace(...) { fprintf(stderr, __VA_ARGS__); }
#define trace(fmt,...)

static DB_dsp_t plugin;
DB_functions_t *deadbeef;

static short enabled = 0;
static float width = 1;
static float lsample, rsample, mid, side;

static void
stereo_widener_reset (void);

static int
stereo_widener_on_configchanged (DB_event_t *ev, uintptr_t data) {
    int e = deadbeef->conf_get_int ("stereo_widener.enable", 0);
    if (e != enabled) {
        if (e) {
            stereo_widener_reset ();
        }
        enabled = e;
    }

    float w = deadbeef->conf_get_float ("stereo_widener.width", 1);
    if (w != width) {
        width = tanh(w / 100) + 1;
    }
    
    return 0;
}

static int
stereo_widener_plugin_start (void) {
    enabled = deadbeef->conf_get_int ("stereo_widener.enable", 0);
    width = deadbeef->conf_get_float ("stereo_widener.width", 1);
    deadbeef->ev_subscribe (DB_PLUGIN (&plugin), DB_EV_CONFIGCHANGED, DB_CALLBACK (stereo_widener_on_configchanged), 0);
}

static int
stereo_widener_plugin_stop (void) {
    deadbeef->ev_unsubscribe (DB_PLUGIN (&plugin), DB_EV_CONFIGCHANGED, DB_CALLBACK (stereo_widener_on_configchanged), 0);
}

static int
stereo_widener_process_int16 (int16_t *samples, int nsamples, int nch, int bps, int srate) {
    if (nch != 2 || width == 1) return nsamples;
    for (unsigned i = 0; i < nsamples; i++) {
        lsample = (float)samples[i*2];
        rsample = (float)samples[(i*2) + 1];
        mid = lsample + rsample;
        side = lsample - rsample;
        samples[i*2] = (int16_t)((1-(width/2)) * mid + (width/2) * side);
        samples[i*2 + 1] = (int16_t)((1-(width/2)) * mid - (width/2) * side);
    }
    return nsamples;
}

static void
stereo_widener_reset (void) {
    return;
}

static void
stereo_widener_enable (int e) {
    if (e != enabled) {
        deadbeef->conf_set_int ("stereo_widener.enable", e);
        if (e && !enabled) {
            stereo_widener_reset ();
        }
        enabled = e;
    }
    return;
}

static int
stereo_widener_enabled (void) {
    return enabled;
}

static const char settings_dlg[] =
    "property \"Enable\" checkbox stereo_widener.enable 0;\n"

    // FIXME: This should ideally be a horizontal slider, not text entry.
    //        plugins/gtkui/pluginconf.c needs to be modified in order to
    //        support this behaviour.
    "property \"Effect strength (negative values permitted, sensible range [-100, 100])\" entry stereo_widener.width 0;\n"
;

static DB_dsp_t plugin = {
    .plugin.api_vmajor = DB_API_VERSION_MAJOR,
    .plugin.api_vminor = DB_API_VERSION_MINOR,
    .plugin.type = DB_PLUGIN_DSP,
    .plugin.id = "stereo_widener",
    .plugin.name = "Stereo widener",
    .plugin.descr = "Stereo widener plugin",
    .plugin.author = "Steven McDonald",
    .plugin.email = "steven.mcdonald@libremail.me",
    .plugin.website = "http://deadbeef.sf.net",
    .plugin.start = stereo_widener_plugin_start,
    .plugin.stop = stereo_widener_plugin_stop,
    .plugin.configdialog = settings_dlg,
    .process_int16 = stereo_widener_process_int16,
    .reset = stereo_widener_reset,
    .enable = stereo_widener_enable,
    .enabled = stereo_widener_enabled,
};

DB_plugin_t *
stereo_widener_load (DB_functions_t *api) {
    deadbeef = api;
    return DB_PLUGIN (&plugin);
}
