/***
  This file is part of paprefs.

  Copyright 2006-2008 Lennart Poettering

  paprefs is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  paprefs is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with paprefs. If not, see <http://www.gnu.org/licenses/>.
***/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <signal.h>

#include <gtkmm.h>
#include <glibmm.h>
#include <glibmm/regex.h>
#include <libglademm.h>
#include <libintl.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus.h>
#include <gdk/gdkx.h>

#include <pulse/version.h>

#define PA_GSETTINGS_PATH_MODULES "/org/freedesktop/pulseaudio/modules"
#define MAX_MODULES 10

class MainWindow : public Gtk::Window {
public:
    MainWindow(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& x);
    static MainWindow* create();

    Gtk::Button
        *closeButton,
        *zeroconfDiscoverInstallButton,
        *zeroconfRaopDiscoverInstallButton,
        *remoteInstallButton,
        *zeroconfPublishInstallButton,
        *upnpInstallButton,
        *rtpRecvInstallButton,
        *rtpSendInstallButton;

    Gtk::CheckButton
        *remoteAccessCheckButton,
        *zeroconfPublishCheckButton,
        *zeroconfDiscoverCheckButton,
        *zeroconfRaopDiscoverCheckButton,
        *anonymousAuthCheckButton,
        *rtpReceiveCheckButton,
        *rtpSendCheckButton,
        *rtpLoopbackCheckButton,
        *combineCheckButton,
        *upnpMediaServerCheckButton,
        *upnpNullSinkCheckButton;

    Gtk::RadioButton
        *rtpMikeRadioButton,
        *rtpSpeakerRadioButton,
        *rtpNullSinkRadioButton;

    Glib::RefPtr<Gio::Settings> combineSettings;
    Glib::RefPtr<Gio::Settings> remoteAccessSettings;
    Glib::RefPtr<Gio::Settings> zeroconfSettings;
    Glib::RefPtr<Gio::Settings> raopSettings;
    Glib::RefPtr<Gio::Settings> rtpRecvSettings;
    Glib::RefPtr<Gio::Settings> rtpSendSettings;
    Glib::RefPtr<Gio::Settings> upnpSettings;

    bool ignoreChanges;

    void onCloseButtonClicked();

    void updateSensitive();

    void onChangeRemoteAccess();
    void onChangeZeroconfDiscover();
    void onChangeZeroconfRaopDiscover();
    void onChangeRtpReceive();
    void onChangeRtpSend();
    void onChangeCombine();
    void onChangeUpnp();

    void onZeroconfDiscoverInstallButtonClicked();
    void onZeroconfRaopDiscoverInstallButtonClicked();
    void onRemoteInstallButtonClicked();
    void onZeroconfPublishInstallButtonClicked();
    void upnpInstallButtonClicked();
    void rtpRecvInstallButtonClicked();
    void rtpSendInstallButtonClicked();

    void readFromGSettings();

    void checkForPackageKit();
    void checkForModules();

    void writeToGSettingsRemoteAccess();
    void writeToGSettingsZeroconfDiscover();
    void writeToGSettingsZeroconfRaopDiscover();
    void writeToGSettingsRtpReceive();
    void writeToGSettingsRtpSend();
    void writeToGSettingsCombine();
    void writeToGSettingsUPnP();

    void onGSettingsChange(const Glib::ustring& key);

    bool moduleHasArgument(Glib::RefPtr<Gio::Settings> gsettings, const Glib::ustring& module, const Glib::ustring& name, const Glib::ustring& value);

    void showInstallButton(Gtk::Button *button, bool available);
    void installFiles(const char *a, const char *b);
    void installModules(const char *a, const char *b);

    bool moduleExists(const gchar *name);
    gchar *modulePath(const gchar *name);

    bool
        packageKitAvailable,
        rtpRecvAvailable,
        rtpSendAvailable,
        zeroconfPublishAvailable,
        zeroconfDiscoverAvailable,
        zeroconfRaopDiscoverAvailable,
        remoteAvailable,
        upnpAvailable;
};

MainWindow::MainWindow(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& x) :
    Gtk::Window(cobject), ignoreChanges(true) {

    x->get_widget("closeButton", closeButton);
    x->get_widget("zeroconfDiscoverInstallButton", zeroconfDiscoverInstallButton);
    x->get_widget("zeroconfRaopDiscoverInstallButton", zeroconfRaopDiscoverInstallButton);
    x->get_widget("remoteInstallButton", remoteInstallButton);
    x->get_widget("zeroconfPublishInstallButton", zeroconfPublishInstallButton);
    x->get_widget("upnpInstallButton", upnpInstallButton);
    x->get_widget("rtpRecvInstallButton", rtpRecvInstallButton);
    x->get_widget("rtpSendInstallButton", rtpSendInstallButton);

    x->get_widget("remoteAccessCheckButton", remoteAccessCheckButton);
    x->get_widget("zeroconfDiscoverCheckButton", zeroconfDiscoverCheckButton);
    x->get_widget("zeroconfRaopDiscoverCheckButton", zeroconfRaopDiscoverCheckButton);
    x->get_widget("zeroconfBrowseCheckButton", zeroconfPublishCheckButton);
    x->get_widget("anonymousAuthCheckButton", anonymousAuthCheckButton);
    x->get_widget("rtpReceiveCheckButton", rtpReceiveCheckButton);
    x->get_widget("rtpSendCheckButton", rtpSendCheckButton);
    x->get_widget("rtpLoopbackCheckButton", rtpLoopbackCheckButton);
    x->get_widget("combineCheckButton", combineCheckButton);
    x->get_widget("upnpMediaServerCheckButton", upnpMediaServerCheckButton);
    x->get_widget("upnpNullSinkCheckButton", upnpNullSinkCheckButton);

    x->get_widget("rtpMikeRadioButton", rtpMikeRadioButton);
    x->get_widget("rtpSpeakerRadioButton", rtpSpeakerRadioButton);
    x->get_widget("rtpNullSinkRadioButton", rtpNullSinkRadioButton);

    checkForPackageKit();
    checkForModules();

    combineSettings = Gio::Settings::create("org.freedesktop.pulseaudio.module",
                                            "/org/freedesktop/pulseaudio/modules/combine/");

    remoteAccessSettings = Gio::Settings::create("org.freedesktop.pulseaudio.module",
                                                 "/org/freedesktop/pulseaudio/modules/remote-access/");

    zeroconfSettings = Gio::Settings::create("org.freedesktop.pulseaudio.module",
                                             "/org/freedesktop/pulseaudio/modules/zeroconf-discover/");

    raopSettings = Gio::Settings::create("org.freedesktop.pulseaudio.module",
                                         "/org/freedesktop/pulseaudio/modules/raop-discover/");

    rtpRecvSettings = Gio::Settings::create("org.freedesktop.pulseaudio.module",
                                            "/org/freedesktop/pulseaudio/modules/rtp-recv/");

    rtpSendSettings = Gio::Settings::create("org.freedesktop.pulseaudio.module",
                                            "/org/freedesktop/pulseaudio/modules/rtp-send/");

    upnpSettings = Gio::Settings::create("org.freedesktop.pulseaudio.module",
                                         "/org/freedesktop/pulseaudio/modules/upnp-media-server/");

    combineSettings->signal_changed().connect(sigc::mem_fun(*this, &MainWindow::onGSettingsChange));
    remoteAccessSettings->signal_changed().connect(sigc::mem_fun(*this, &MainWindow::onGSettingsChange));
    zeroconfSettings->signal_changed().connect(sigc::mem_fun(*this, &MainWindow::onGSettingsChange));
    raopSettings->signal_changed().connect(sigc::mem_fun(*this, &MainWindow::onGSettingsChange));
    rtpRecvSettings->signal_changed().connect(sigc::mem_fun(*this, &MainWindow::onGSettingsChange));
    rtpSendSettings->signal_changed().connect(sigc::mem_fun(*this, &MainWindow::onGSettingsChange));
    upnpSettings->signal_changed().connect(sigc::mem_fun(*this, &MainWindow::onGSettingsChange));
    readFromGSettings();

    closeButton->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onCloseButtonClicked));

    remoteAccessCheckButton->signal_toggled().connect(sigc::mem_fun(*this, &MainWindow::onChangeRemoteAccess));
    zeroconfPublishCheckButton->signal_toggled().connect(sigc::mem_fun(*this, &MainWindow::onChangeRemoteAccess));
    anonymousAuthCheckButton->signal_toggled().connect(sigc::mem_fun(*this, &MainWindow::onChangeRemoteAccess));

    zeroconfDiscoverCheckButton->signal_toggled().connect(sigc::mem_fun(*this, &MainWindow::onChangeZeroconfDiscover));
    zeroconfRaopDiscoverCheckButton->signal_toggled().connect(sigc::mem_fun(*this, &MainWindow::onChangeZeroconfRaopDiscover));

    rtpReceiveCheckButton->signal_toggled().connect(sigc::mem_fun(*this, &MainWindow::onChangeRtpReceive));

    rtpSendCheckButton->signal_toggled().connect(sigc::mem_fun(*this, &MainWindow::onChangeRtpSend));
    rtpLoopbackCheckButton->signal_toggled().connect(sigc::mem_fun(*this, &MainWindow::onChangeRtpSend));
    rtpMikeRadioButton->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onChangeRtpSend));
    rtpSpeakerRadioButton->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onChangeRtpSend));
    rtpNullSinkRadioButton->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onChangeRtpSend));

    combineCheckButton->signal_toggled().connect(sigc::mem_fun(*this, &MainWindow::onChangeCombine));

    upnpMediaServerCheckButton->signal_toggled().connect(sigc::mem_fun(*this, &MainWindow::onChangeUpnp));
    upnpNullSinkCheckButton->signal_toggled().connect(sigc::mem_fun(*this, &MainWindow::onChangeUpnp));

    zeroconfDiscoverInstallButton->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onZeroconfDiscoverInstallButtonClicked));
    zeroconfRaopDiscoverInstallButton->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onZeroconfRaopDiscoverInstallButtonClicked));
    remoteInstallButton->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onRemoteInstallButtonClicked));
    zeroconfPublishInstallButton->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onZeroconfPublishInstallButtonClicked));
    upnpInstallButton->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::upnpInstallButtonClicked));
    rtpRecvInstallButton->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::rtpRecvInstallButtonClicked));
    rtpSendInstallButton->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::rtpSendInstallButtonClicked));
}

MainWindow* MainWindow::create() {
    MainWindow* w;
    Glib::RefPtr<Gnome::Glade::Xml> x = Gnome::Glade::Xml::create(GLADE_FILE, "mainWindow");
    x->get_widget_derived("mainWindow", w);
    return w;
}

void MainWindow::onCloseButtonClicked() {
    Gtk::Main::quit();
}

void MainWindow::updateSensitive() {
    bool b;

    remoteAccessCheckButton->set_sensitive(remoteAvailable);
    b = remoteAccessCheckButton->get_active();
    zeroconfPublishCheckButton->set_sensitive(b && zeroconfPublishAvailable);
    anonymousAuthCheckButton->set_sensitive(b && remoteAvailable);

    zeroconfDiscoverCheckButton->set_sensitive(zeroconfDiscoverAvailable);
    zeroconfRaopDiscoverCheckButton->set_sensitive(zeroconfRaopDiscoverAvailable);

    rtpReceiveCheckButton->set_sensitive(rtpRecvAvailable);
    rtpSendCheckButton->set_sensitive(rtpSendAvailable);
    b = rtpSendCheckButton->get_active();
    rtpLoopbackCheckButton->set_sensitive(b && !rtpSpeakerRadioButton->get_active() && rtpSendAvailable);
    rtpMikeRadioButton->set_sensitive(b && rtpSendAvailable);
    rtpSpeakerRadioButton->set_sensitive(b && rtpSendAvailable);
    rtpNullSinkRadioButton->set_sensitive(b && rtpSendAvailable);

    upnpMediaServerCheckButton->set_sensitive(upnpAvailable);
    upnpNullSinkCheckButton->set_sensitive(upnpAvailable && upnpMediaServerCheckButton->get_active());

    showInstallButton(zeroconfDiscoverInstallButton, zeroconfDiscoverAvailable);
    showInstallButton(zeroconfRaopDiscoverInstallButton, zeroconfRaopDiscoverAvailable);
    showInstallButton(remoteInstallButton, remoteAvailable);
    showInstallButton(zeroconfPublishInstallButton, zeroconfPublishAvailable);
    showInstallButton(upnpInstallButton, upnpAvailable);
    showInstallButton(rtpRecvInstallButton, rtpRecvAvailable);
    showInstallButton(rtpSendInstallButton, rtpSendAvailable);
}

void MainWindow::onChangeRemoteAccess() {

    if (ignoreChanges)
        return;

    updateSensitive();
    writeToGSettingsRemoteAccess();
}

void MainWindow::onChangeZeroconfDiscover() {

    if (ignoreChanges)
        return;

    updateSensitive();
    writeToGSettingsZeroconfDiscover();
}

void MainWindow::onChangeZeroconfRaopDiscover() {

    if (ignoreChanges)
        return;

    updateSensitive();
    writeToGSettingsZeroconfRaopDiscover();
}

void MainWindow::onChangeRtpReceive() {
    if (ignoreChanges)
        return;

    updateSensitive();
    writeToGSettingsRtpReceive();
    writeToGSettingsRtpSend();
}

void MainWindow::onChangeRtpSend() {
    if (ignoreChanges)
        return;

    updateSensitive();
    writeToGSettingsRtpSend();
}

void MainWindow::onChangeCombine() {
    if (ignoreChanges)
        return;

    writeToGSettingsCombine();
}

void MainWindow::onChangeUpnp() {

    if (ignoreChanges)
        return;

    updateSensitive();
    writeToGSettingsUPnP();
}

void MainWindow::showInstallButton(Gtk::Button *button, bool available) {
  if (available || !packageKitAvailable)
    button->hide();
  else
    button->show();
}

void MainWindow::installFiles(const char *a, const char *b = NULL) {
    DBusGConnection *connection;
    DBusGProxy *proxy;
    gboolean ret;
    GError *error = NULL;
    const gchar *packages[] = {a, b, NULL};

    connection = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);

    proxy = dbus_g_proxy_new_for_name(connection,
                                      "org.freedesktop.PackageKit",
                                      "/org/freedesktop/PackageKit",
                                      "org.freedesktop.PackageKit.Modify");

    ret = dbus_g_proxy_call(
            proxy, "InstallProvideFiles", &error,
            G_TYPE_UINT, GDK_WINDOW_XID(get_window()->gobj()),
            G_TYPE_STRV, packages,
            G_TYPE_STRING, "show-confirm-search,hide-finished",
            G_TYPE_INVALID, G_TYPE_INVALID);

    if (!ret) {
        g_warning("Installation failed: %s", error->message);
        g_error_free(error);
    }

    g_object_unref(proxy);
    dbus_g_connection_unref(connection);

    checkForModules();
    updateSensitive();
}

void MainWindow::installModules(const char *a, const char *b = NULL) {
  gchar *ma, *mb = NULL;

  ma = modulePath (a);
  if (b != NULL)
    mb = modulePath (b);

  installFiles (ma, mb);

  g_free (ma);
  g_free (mb);
}

void MainWindow::onZeroconfDiscoverInstallButtonClicked() {
    installModules("module-zeroconf-discover" SHREXT);
}

void MainWindow::onZeroconfRaopDiscoverInstallButtonClicked() {
    installModules("module-raop-discover" SHREXT);
}

void MainWindow::onRemoteInstallButtonClicked() {
    installModules("module-esound-protocol-tcp" SHREXT,
        "module-native-protocol-tcp" SHREXT);
}

void MainWindow::onZeroconfPublishInstallButtonClicked() {
    installModules("module-zeroconf-publish" SHREXT);
}

void MainWindow::upnpInstallButtonClicked() {
    gchar *mpath = modulePath ("module-rygel-media-server" SHREXT);

    installFiles("/usr/bin/rygel", mpath);
    g_free (mpath);
}

void MainWindow::rtpRecvInstallButtonClicked() {
    installModules("module-rtp-recv" SHREXT);
}

void MainWindow::rtpSendInstallButtonClicked() {
    installModules("module-rtp-send" SHREXT);
}

void MainWindow::writeToGSettingsCombine() {
    combineSettings->delay();

    if (combineCheckButton->get_active()) {
        combineSettings->set_string("name0", Glib::ustring("module-combine"));
        combineSettings->set_string("args0", Glib::ustring(""));

        combineSettings->set_boolean("enabled", true);
    } else
        combineSettings->set_boolean("enabled", false);

    combineSettings->apply();
}

void MainWindow::writeToGSettingsRemoteAccess() {
    bool zeroconfEnabled, anonymousEnabled;

    remoteAccessSettings->delay();

    zeroconfEnabled = zeroconfPublishCheckButton->get_active();
    anonymousEnabled = anonymousAuthCheckButton->get_active();

    if (remoteAccessCheckButton->get_active()) {
        remoteAccessSettings->set_string("name0", Glib::ustring("module-native-protocol-tcp"));
        remoteAccessSettings->set_string("name1", Glib::ustring("module-esound-protocol-tcp"));

        if (anonymousEnabled) {
            remoteAccessSettings->set_string("args0", Glib::ustring("auth-anonymous=1"));
            remoteAccessSettings->set_string("args1", Glib::ustring("auth-anonymous=1"));
        } else {
            remoteAccessSettings->set_string("args0", Glib::ustring(""));
            remoteAccessSettings->set_string("args1", Glib::ustring(""));
        }

        if (zeroconfEnabled) {
            remoteAccessSettings->set_string("name2", Glib::ustring("module-zeroconf-publish"));
            remoteAccessSettings->set_string("args2", Glib::ustring(""));
        } else {
            remoteAccessSettings->reset("name2");
            remoteAccessSettings->reset("args2");
        }

        remoteAccessSettings->set_boolean("enabled", true);
    } else
        remoteAccessSettings->set_boolean("enabled", false);

    remoteAccessSettings->apply();
}

void MainWindow::writeToGSettingsZeroconfDiscover() {
    zeroconfSettings->delay();

    if (zeroconfDiscoverCheckButton->get_active()) {
        zeroconfSettings->set_string("name0", Glib::ustring("module-zeroconf-discover"));
        zeroconfSettings->set_string("args0", Glib::ustring(""));

        zeroconfSettings->set_boolean("enabled", true);
    } else
        zeroconfSettings->set_boolean("enabled", false);

    zeroconfSettings->apply();
}

void MainWindow::writeToGSettingsZeroconfRaopDiscover() {
    raopSettings->delay();

    if (zeroconfRaopDiscoverCheckButton->get_active()) {
        raopSettings->set_string("name0", Glib::ustring("module-raop-discover"));
        raopSettings->set_string("args0", Glib::ustring(""));

        raopSettings->set_boolean("enabled", true);
    } else
        raopSettings->set_boolean("enabled", false);

    raopSettings->apply();
}

void MainWindow::writeToGSettingsRtpReceive() {
    rtpRecvSettings->delay();

    if (rtpReceiveCheckButton->get_active()) {
        rtpRecvSettings->set_string("name0", Glib::ustring("module-rtp-recv"));
        rtpRecvSettings->set_string("args0", Glib::ustring(""));

        rtpRecvSettings->set_boolean("enabled", true);
    }  else
        rtpRecvSettings->set_boolean("enabled", false);

    rtpRecvSettings->apply();
}

void MainWindow::writeToGSettingsRtpSend() {
    bool loopbackEnabled, mikeEnabled, speakerEnabled = false;

    rtpSendSettings->delay();

    loopbackEnabled = rtpLoopbackCheckButton->get_active();
    mikeEnabled = rtpMikeRadioButton->get_active();
    speakerEnabled = rtpSpeakerRadioButton->get_active();

    if (rtpSendCheckButton->get_active()) {
        if (!mikeEnabled && !speakerEnabled) {
            rtpSendSettings->set_string("name0", Glib::ustring("module-null-sink"));
            rtpSendSettings->set_string("args0", Glib::ustring("sink_name=rtp "
                                                               "format=s16be "
                                                               "channels=2 "
                                                               "rate=44100 "
                                                               "sink_properties=\"device.description='RTP Multicast' device.bus='network' device.icon_name='network-server'\""));

            rtpSendSettings->set_string("name1", Glib::ustring("module-rtp-send"));
            rtpSendSettings->set_string("args1", Glib::ustring(loopbackEnabled ? "source=rtp.monitor loop=1" : "source=rtp.monitor loop=0"));
        } else {
            char tmp[256];

            snprintf(tmp, sizeof(tmp), "%s %s",
                     mikeEnabled ? "source=@DEFAULT_SOURCE@" : "source=@DEFAULT_MONITOR@",
                     mikeEnabled && loopbackEnabled ? "loop=1" : "loop=0");

            rtpSendSettings->set_string("name0", Glib::ustring("module-rtp-send"));
            rtpSendSettings->set_string("args0", Glib::ustring(tmp));

            rtpSendSettings->reset("name1");
            rtpSendSettings->reset("args1");
        }

        rtpSendSettings->set_boolean("enabled", true);
    } else
        rtpSendSettings->set_boolean("enabled", false);

    rtpSendSettings->apply();
}

void MainWindow::writeToGSettingsUPnP() {
    upnpSettings->delay();

    bool mediaServer = upnpMediaServerCheckButton->get_active();
    bool nullSink = upnpNullSinkCheckButton->get_active();

    if (mediaServer) {
        upnpSettings->set_string("name0", Glib::ustring("module-rygel-media-server"));
        upnpSettings->set_string("args0", Glib::ustring(""));

        if (nullSink) {
            upnpSettings->set_string("name1", Glib::ustring("module-null-sink"));
            upnpSettings->set_string("args1", Glib::ustring("sink_name=upnp "
                                                            "format=s16be "
                                                            "channels=2 "
                                                            "rate=44100 "
                                                            "sink_properties=\"device.description='DLNA/UPnP Streaming' device.bus='network' device.icon_name='network-server'\""));
        } else {
            upnpSettings->reset("name1");
            upnpSettings->reset("args1");
        }

        upnpSettings->set_boolean("enabled", true);
    } else
        upnpSettings->set_boolean("enabled", false);

    upnpSettings->apply();
}

void MainWindow::onGSettingsChange(const Glib::ustring&) {
    readFromGSettings();
}

bool MainWindow::moduleHasArgument(Glib::RefPtr<Gio::Settings> gsettings, const Glib::ustring& module, const Glib::ustring& name = "", const Glib::ustring& value = "") {
    Glib::ustring args;
    std::vector<std::string> moduleArgs, keyValue;

    for (int i=0; i<MAX_MODULES; i++) {
        char tmp[256];
        snprintf(tmp, sizeof(tmp), "name%d", i);
        if (gsettings->get_string(tmp) == module) {
            if (name.empty() && value.empty()) {
                return gsettings->get_boolean("enabled");
            }
            snprintf(tmp, sizeof(tmp), "args%d", i);
            args = gsettings->get_string(tmp);
            moduleArgs = Glib::Regex::split_simple(" ", args);
            for (std::vector<std::string>::iterator it = moduleArgs.begin(); it != moduleArgs.end(); ++it) {
                keyValue = Glib::Regex::split_simple("=", *it);
                if (keyValue.size() >= 2 && keyValue[0] == name) {
                    return keyValue[1] == value;
                }
            }
            break;
        }
    }
    return false;
}

void MainWindow::readFromGSettings() {
    Glib::ustring mode, args;
    std::vector<std::string> moduleArgs, keyValue;
    bool loopbackEnabled = false;
    bool anonymousEnabled = false;
    bool zeroconfEnabled = false;
    bool mikeEnabled = false;
    bool speakerEnabled = false;
    bool nullSink = false;
    ignoreChanges = TRUE;

    remoteAccessCheckButton->set_active(remoteAccessSettings->get_boolean("enabled"));

    zeroconfDiscoverCheckButton->set_active(zeroconfSettings->get_boolean("enabled"));
    zeroconfRaopDiscoverCheckButton->set_active(raopSettings->get_boolean("enabled"));

    rtpReceiveCheckButton->set_active(rtpRecvSettings->get_boolean("enabled"));

    rtpSendCheckButton->set_active(rtpSendSettings->get_boolean("enabled"));

    loopbackEnabled = moduleHasArgument(rtpSendSettings, "module-rtp-send", "loop", "1");
    rtpLoopbackCheckButton->set_active(loopbackEnabled);

    anonymousEnabled = moduleHasArgument(remoteAccessSettings, "module-native-protocol-tcp", "auth-anonymous", "1") &&
                       moduleHasArgument(remoteAccessSettings, "module-esound-protocol-tcp", "auth-anonymous", "1");
    anonymousAuthCheckButton->set_active(anonymousEnabled);

    zeroconfEnabled = moduleHasArgument(remoteAccessSettings, "module-zeroconf-publish");
    zeroconfPublishCheckButton->set_active(zeroconfEnabled);

    mikeEnabled = moduleHasArgument(rtpSendSettings, "module-rtp-send", "source", "@DEFAULT_SOURCE@");
    speakerEnabled = moduleHasArgument(rtpSendSettings, "module-rtp-send", "source", "@DEFAULT_MONITOR@");

    if (mikeEnabled)
        rtpMikeRadioButton->set_active(TRUE);
    else if (speakerEnabled)
        rtpSpeakerRadioButton->set_active(TRUE);
    else
        rtpNullSinkRadioButton->set_active(TRUE);

    combineCheckButton->set_active(combineSettings->get_boolean("enabled"));

    upnpMediaServerCheckButton->set_active(upnpSettings->get_boolean("enabled"));

    nullSink = moduleHasArgument(upnpSettings, "module-null-sink", "sink_name", "upnp");
    upnpNullSinkCheckButton->set_active(nullSink);

    ignoreChanges = FALSE;

    updateSensitive();
}

gchar * MainWindow::modulePath(const gchar *name) {
  gchar *path, **versions;

  versions = g_strsplit(pa_get_library_version(), ".", 3);
  if (versions[0] && versions[1]) {
      gchar *pulsedir, *search;

      /* Remove the "/pulse-x.y/modules" suffix so we can dynamically inject
       * it again with runtime library version numbers */
      pulsedir = g_strdup_printf ("%s", MODDIR);
      if ((search = g_strrstr (pulsedir, G_DIR_SEPARATOR_S))) {
          *search = '\0';
          if ((search = g_strrstr (pulsedir, G_DIR_SEPARATOR_S)))
              *search = '\0';
      }
      path = g_strdup_printf ("%s" G_DIR_SEPARATOR_S "pulse-%s.%s" G_DIR_SEPARATOR_S "modules" G_DIR_SEPARATOR_S "%s", pulsedir, versions[0], versions[1], name);
      g_free (pulsedir);
  } else
      path = g_build_filename (MODDIR, name, NULL);
  g_strfreev(versions);

  return path;
}

bool MainWindow::moduleExists(const gchar *name) {
  gchar *path = modulePath (name);
  bool ret;

  ret = g_file_test (path, G_FILE_TEST_EXISTS);

  g_free (path);

  return ret;
}

void MainWindow::checkForModules() {

    remoteAvailable =
        moduleExists("module-esound-protocol-tcp" SHREXT) ||
        moduleExists("module-native-protocol-tcp" SHREXT);

    zeroconfPublishAvailable = moduleExists("module-zeroconf-publish" SHREXT);
    zeroconfDiscoverAvailable = moduleExists("module-zeroconf-discover" SHREXT);

    zeroconfRaopDiscoverAvailable = moduleExists("module-raop-discover" SHREXT);

    rtpRecvAvailable = moduleExists("module-rtp-recv" SHREXT);
    rtpSendAvailable = moduleExists("module-rtp-send" SHREXT);

    upnpAvailable = moduleExists("module-rygel-media-server" SHREXT) &&
        g_find_program_in_path("rygel");
}

void MainWindow::checkForPackageKit() {

    DBusError err;
    dbus_error_init(&err);
    DBusConnection *sessionBus = dbus_bus_get(DBUS_BUS_SESSION, &err);

    if(dbus_error_is_set(&err)) {
        g_warning("Error connecting to DBus: %s", err.message);
        packageKitAvailable = FALSE;
    } else {
        packageKitAvailable = dbus_bus_name_has_owner(sessionBus, "org.freedesktop.PackageKit", NULL);
        dbus_connection_unref(sessionBus);
    }
    dbus_error_free(&err);
}


int main(int argc, char *argv[]) {

    /* Initialize the i18n stuff */
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);

    signal(SIGPIPE, SIG_IGN);

    Gtk::Main kit(argc, argv);

    Gtk::Window* mainWindow = MainWindow::create();

    Gtk::Main::run(*mainWindow);
    delete mainWindow;
}
