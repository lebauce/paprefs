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
#include <libglademm.h>
#include <gconfmm.h>
#include <libintl.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus.h>
#include <gdk/gdkx.h>

#define PA_GCONF_ROOT "/system/pulseaudio"
#define PA_GCONF_PATH_MODULES PA_GCONF_ROOT"/modules"

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

    Glib::RefPtr<Gnome::Conf::Client> gconf;

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

    void readFromGConf();

    void checkForPackageKit();
    void checkForModules();

    void writeToGConfRemoteAccess();
    void writeToGConfZeroconfDiscover();
    void writeToGConfZeroconfRaopDiscover();
    void writeToGConfRtpReceive();
    void writeToGConfRtpSend();
    void writeToGConfCombine();
    void writeToGConfUPnP();

    void onGConfChange(const Glib::ustring& key, const Gnome::Conf::Value& value);

    void showInstallButton(Gtk::Button *button, bool available);
    void installFiles(const char *a, const char *b);

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

    gconf = Gnome::Conf::Client::get_default_client();
    gconf->set_error_handling(Gnome::Conf::CLIENT_HANDLE_ALL);
    gconf->add_dir(PA_GCONF_ROOT, Gnome::Conf::CLIENT_PRELOAD_RECURSIVE);

    gconf->signal_value_changed().connect(sigc::mem_fun(*this, &MainWindow::onGConfChange));
    readFromGConf();

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
    writeToGConfRemoteAccess();
}

void MainWindow::onChangeZeroconfDiscover() {

    if (ignoreChanges)
        return;

    updateSensitive();
    writeToGConfZeroconfDiscover();
}

void MainWindow::onChangeZeroconfRaopDiscover() {

    if (ignoreChanges)
        return;

    updateSensitive();
    writeToGConfZeroconfRaopDiscover();
}

void MainWindow::onChangeRtpReceive() {
    if (ignoreChanges)
        return;

    updateSensitive();
    writeToGConfRtpReceive();
    writeToGConfRtpSend();
}

void MainWindow::onChangeRtpSend() {
    if (ignoreChanges)
        return;

    updateSensitive();
    writeToGConfRtpSend();
}

void MainWindow::onChangeCombine() {
    if (ignoreChanges)
        return;

    writeToGConfCombine();
}

void MainWindow::onChangeUpnp() {

    if (ignoreChanges)
        return;

    updateSensitive();
    writeToGConfUPnP();
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

void MainWindow::onZeroconfDiscoverInstallButtonClicked() {
    installFiles(MODULESDIR "module-zeroconf-discover" SHREXT);
}

void MainWindow::onZeroconfRaopDiscoverInstallButtonClicked() {
    installFiles(MODULESDIR "module-raop-discover" SHREXT);
}

void MainWindow::onRemoteInstallButtonClicked() {
    installFiles(MODULESDIR "module-esound-protocol-tcp" SHREXT,
                 MODULESDIR "module-native-protocol-tcp" SHREXT);
}

void MainWindow::onZeroconfPublishInstallButtonClicked() {
    installFiles(MODULESDIR "module-zeroconf-publish" SHREXT);
}

void MainWindow::upnpInstallButtonClicked() {
    installFiles("/usr/bin/rygel", MODULESDIR "module-rygel-media-server" SHREXT);
}

void MainWindow::rtpRecvInstallButtonClicked() {
    installFiles(MODULESDIR "module-rtp-recv" SHREXT);
}

void MainWindow::rtpSendInstallButtonClicked() {
    installFiles(MODULESDIR "module-rtp-send" SHREXT);
}

void MainWindow::writeToGConfCombine() {
    Gnome::Conf::ChangeSet changeSet;
    changeSet.set(PA_GCONF_PATH_MODULES"/combine/locked", true);
    gconf->change_set_commit(changeSet, true);

    if (combineCheckButton->get_active()) {
        changeSet.set(PA_GCONF_PATH_MODULES"/combine/name0", Glib::ustring("module-combine"));
        changeSet.set(PA_GCONF_PATH_MODULES"/combine/args0", Glib::ustring(""));

        changeSet.set(PA_GCONF_PATH_MODULES"/combine/enabled", true);
    } else
        changeSet.set(PA_GCONF_PATH_MODULES"/combine/enabled", false);

    gconf->change_set_commit(changeSet, true);

    changeSet.set(PA_GCONF_PATH_MODULES"/combine/locked", false);
    gconf->change_set_commit(changeSet, true);

    gconf->suggest_sync();
}

void MainWindow::writeToGConfRemoteAccess() {
    Gnome::Conf::ChangeSet changeSet;
    bool zeroconfEnabled, anonymousEnabled;

    changeSet.set(PA_GCONF_PATH_MODULES"/remote-access/locked", true);
    gconf->change_set_commit(changeSet, true);

    changeSet.set(PA_GCONF_PATH_MODULES"/remote-access/zeroconf_enabled", zeroconfEnabled = zeroconfPublishCheckButton->get_active());
    changeSet.set(PA_GCONF_PATH_MODULES"/remote-access/anonymous_enabled", anonymousEnabled = anonymousAuthCheckButton->get_active());

    if (remoteAccessCheckButton->get_active()) {
        changeSet.set(PA_GCONF_PATH_MODULES"/remote-access/name0", Glib::ustring("module-native-protocol-tcp"));
        changeSet.set(PA_GCONF_PATH_MODULES"/remote-access/name1", Glib::ustring("module-esound-protocol-tcp"));

        if (anonymousEnabled) {
            changeSet.set(PA_GCONF_PATH_MODULES"/remote-access/args0", Glib::ustring("auth-anonymous=1"));
            changeSet.set(PA_GCONF_PATH_MODULES"/remote-access/args1", Glib::ustring("auth-anonymous=1"));
        } else {
            changeSet.set(PA_GCONF_PATH_MODULES"/remote-access/args0", Glib::ustring(""));
            changeSet.set(PA_GCONF_PATH_MODULES"/remote-access/args1", Glib::ustring(""));
        }

        if (zeroconfEnabled) {
            changeSet.set(PA_GCONF_PATH_MODULES"/remote-access/name2", Glib::ustring("module-zeroconf-publish"));
            changeSet.set(PA_GCONF_PATH_MODULES"/remote-access/args2", Glib::ustring(""));
        } else {
            changeSet.unset(PA_GCONF_PATH_MODULES"/remote-access/name2");
            changeSet.unset(PA_GCONF_PATH_MODULES"/remote-access/args2");
        }

        changeSet.set(PA_GCONF_PATH_MODULES"/remote-access/enabled", true);
    } else
        changeSet.set(PA_GCONF_PATH_MODULES"/remote-access/enabled", false);

    gconf->change_set_commit(changeSet, true);

    changeSet.set(PA_GCONF_PATH_MODULES"/remote-access/locked", false);
    gconf->change_set_commit(changeSet, true);

    gconf->suggest_sync();
}

void MainWindow::writeToGConfZeroconfDiscover() {
    Gnome::Conf::ChangeSet changeSet;

    changeSet.set(PA_GCONF_PATH_MODULES"/zeroconf-discover/locked", true);
    gconf->change_set_commit(changeSet, true);

    if (zeroconfDiscoverCheckButton->get_active()) {
        changeSet.set(PA_GCONF_PATH_MODULES"/zeroconf-discover/name0", Glib::ustring("module-zeroconf-discover"));
        changeSet.set(PA_GCONF_PATH_MODULES"/zeroconf-discover/args0", Glib::ustring(""));

        changeSet.set(PA_GCONF_PATH_MODULES"/zeroconf-discover/enabled", true);
    } else
        changeSet.set(PA_GCONF_PATH_MODULES"/zeroconf-discover/enabled", false);

    gconf->change_set_commit(changeSet, true);

    changeSet.set(PA_GCONF_PATH_MODULES"/zeroconf-discover/locked", false);
    gconf->change_set_commit(changeSet, true);

    gconf->suggest_sync();
}

void MainWindow::writeToGConfZeroconfRaopDiscover() {
    Gnome::Conf::ChangeSet changeSet;

    changeSet.set(PA_GCONF_PATH_MODULES"/raop-discover/locked", true);
    gconf->change_set_commit(changeSet, true);

    if (zeroconfRaopDiscoverCheckButton->get_active()) {
        changeSet.set(PA_GCONF_PATH_MODULES"/raop-discover/name0", Glib::ustring("module-raop-discover"));
        changeSet.set(PA_GCONF_PATH_MODULES"/raop-discover/args0", Glib::ustring(""));

        changeSet.set(PA_GCONF_PATH_MODULES"/raop-discover/enabled", true);
    } else
        changeSet.set(PA_GCONF_PATH_MODULES"/raop-discover/enabled", false);

    gconf->change_set_commit(changeSet, true);

    changeSet.set(PA_GCONF_PATH_MODULES"/raop-discover/locked", false);
    gconf->change_set_commit(changeSet, true);

    gconf->suggest_sync();
}

void MainWindow::writeToGConfRtpReceive() {
    Gnome::Conf::ChangeSet changeSet;

    changeSet.set(PA_GCONF_PATH_MODULES"/rtp-recv/locked", true);
    gconf->change_set_commit(changeSet, true);

    if (rtpReceiveCheckButton->get_active()) {
        changeSet.set(PA_GCONF_PATH_MODULES"/rtp-recv/name0", Glib::ustring("module-rtp-recv"));
        changeSet.set(PA_GCONF_PATH_MODULES"/rtp-recv/args0", Glib::ustring(""));

        changeSet.set(PA_GCONF_PATH_MODULES"/rtp-recv/enabled", true);
    }  else
        changeSet.set(PA_GCONF_PATH_MODULES"/rtp-recv/enabled", false);

    gconf->change_set_commit(changeSet, true);

    changeSet.set(PA_GCONF_PATH_MODULES"/rtp-recv/locked", false);
    gconf->change_set_commit(changeSet, true);

    gconf->suggest_sync();
}

void MainWindow::writeToGConfRtpSend() {
    Gnome::Conf::ChangeSet changeSet;
    bool loopbackEnabled, mikeEnabled, speakerEnabled = false;

    changeSet.set(PA_GCONF_PATH_MODULES"/rtp-send/locked", true);
    gconf->change_set_commit(changeSet, true);

    changeSet.set(PA_GCONF_PATH_MODULES"/rtp-send/loopback_enabled", loopbackEnabled = rtpLoopbackCheckButton->get_active());

    changeSet.set(PA_GCONF_PATH_MODULES"/rtp-send/mode", Glib::ustring(
                          (mikeEnabled = rtpMikeRadioButton->get_active()) ? "microphone" :
                          ((speakerEnabled = rtpSpeakerRadioButton->get_active()) ? "speaker" : "null-sink")));

    if (rtpSendCheckButton->get_active()) {
        if (!mikeEnabled && !speakerEnabled) {
            changeSet.set(PA_GCONF_PATH_MODULES"/rtp-send/name0", Glib::ustring("module-null-sink"));
            changeSet.set(PA_GCONF_PATH_MODULES"/rtp-send/args0", Glib::ustring("sink_name=rtp "
                                                                                "format=s16be "
                                                                                "channels=2 "
                                                                                "rate=44100 "
                                                                                "sink_properties=\"device.description='RTP Multicast' device.bus='network' device.icon_name='network-server'\""));

            changeSet.set(PA_GCONF_PATH_MODULES"/rtp-send/name1", Glib::ustring("module-rtp-send"));
            changeSet.set(PA_GCONF_PATH_MODULES"/rtp-send/args1", Glib::ustring(loopbackEnabled ? "source=rtp.monitor loop=1" : "source=rtp.monitor loop=0"));
        } else {
            char tmp[256];

            snprintf(tmp, sizeof(tmp), "%s %s",
                     mikeEnabled ? "source=@DEFAULT_SOURCE@" : "source=@DEFAULT_MONITOR@",
                     mikeEnabled && loopbackEnabled ? "loop=1" : "loop=0");

            changeSet.set(PA_GCONF_PATH_MODULES"/rtp-send/name0", Glib::ustring("module-rtp-send"));
            changeSet.set(PA_GCONF_PATH_MODULES"/rtp-send/args0", Glib::ustring(tmp));

            changeSet.unset(PA_GCONF_PATH_MODULES"/rtp-send/name1");
            changeSet.unset(PA_GCONF_PATH_MODULES"/rtp-send/args1");
        }

        changeSet.set(PA_GCONF_PATH_MODULES"/rtp-send/enabled", true);
    }  else
        changeSet.set(PA_GCONF_PATH_MODULES"/rtp-send/enabled", false);

    gconf->change_set_commit(changeSet, true);

    changeSet.set(PA_GCONF_PATH_MODULES"/rtp-send/locked", false);
    gconf->change_set_commit(changeSet, true);

    gconf->suggest_sync();
}

void MainWindow::writeToGConfUPnP() {
    Gnome::Conf::ChangeSet changeSet;

    changeSet.set(PA_GCONF_PATH_MODULES"/upnp-media-server/locked", true);
    gconf->change_set_commit(changeSet, true);

    if (upnpMediaServerCheckButton->get_active()) {
        changeSet.set(PA_GCONF_PATH_MODULES"/upnp-media-server/name0", Glib::ustring("module-rygel-media-server"));
        changeSet.set(PA_GCONF_PATH_MODULES"/upnp-media-server/args0", Glib::ustring(""));

        if (upnpNullSinkCheckButton->get_active()) {
            changeSet.set(PA_GCONF_PATH_MODULES"/upnp-media-server/name1", Glib::ustring("module-null-sink"));
            changeSet.set(PA_GCONF_PATH_MODULES"/upnp-media-server/args1", Glib::ustring("sink_name=upnp "
                                                                                         "format=s16be "
                                                                                         "channels=2 "
                                                                                         "rate=44100 "
                                                                                         "sink_properties=\"device.description='DLNA/UPnP Streaming' device.bus='network' device.icon_name='network-server'\""));
            changeSet.set(PA_GCONF_PATH_MODULES"/upnp-media-server/null-sink-enabled", true);
        } else {
            changeSet.unset(PA_GCONF_PATH_MODULES"/upnp-media-server/name1");
            changeSet.unset(PA_GCONF_PATH_MODULES"/upnp-media-server/args1");
            changeSet.set(PA_GCONF_PATH_MODULES"/upnp-media-server/null-sink-enabled", false);
        }

        changeSet.set(PA_GCONF_PATH_MODULES"/upnp-media-server/enabled", true);
    }  else
        changeSet.set(PA_GCONF_PATH_MODULES"/upnp-media-server/enabled", false);

    gconf->change_set_commit(changeSet, true);

    changeSet.set(PA_GCONF_PATH_MODULES"/upnp-media-server/locked", false);
    gconf->change_set_commit(changeSet, true);

    gconf->suggest_sync();
}

void MainWindow::onGConfChange(const Glib::ustring&, const Gnome::Conf::Value&) {
    readFromGConf();
}

void MainWindow::readFromGConf() {
    Glib::ustring mode;

    ignoreChanges = TRUE;

    remoteAccessCheckButton->set_active(gconf->get_bool(PA_GCONF_PATH_MODULES"/remote-access/enabled"));
    zeroconfPublishCheckButton->set_active(gconf->get_bool(PA_GCONF_PATH_MODULES"/remote-access/zeroconf_enabled"));
    anonymousAuthCheckButton->set_active(gconf->get_bool(PA_GCONF_PATH_MODULES"/remote-access/anonymous_enabled"));

    zeroconfDiscoverCheckButton->set_active(gconf->get_bool(PA_GCONF_PATH_MODULES"/zeroconf-discover/enabled"));
    zeroconfRaopDiscoverCheckButton->set_active(gconf->get_bool(PA_GCONF_PATH_MODULES"/raop-discover/enabled"));

    rtpReceiveCheckButton->set_active(gconf->get_bool(PA_GCONF_PATH_MODULES"/rtp-recv/enabled"));

    rtpSendCheckButton->set_active(gconf->get_bool(PA_GCONF_PATH_MODULES"/rtp-send/enabled"));
    rtpLoopbackCheckButton->set_active(gconf->get_bool(PA_GCONF_PATH_MODULES"/rtp-send/loopback_enabled"));

    mode = gconf->get_string(PA_GCONF_PATH_MODULES"/rtp-send/mode");
    if (mode == "microphone")
        rtpMikeRadioButton->set_active(TRUE);
    else if (mode == "speaker")
        rtpSpeakerRadioButton->set_active(TRUE);
    else
        rtpNullSinkRadioButton->set_active(TRUE);

    combineCheckButton->set_active(gconf->get_bool(PA_GCONF_PATH_MODULES"/combine/enabled"));

    upnpMediaServerCheckButton->set_active(gconf->get_bool(PA_GCONF_PATH_MODULES"/upnp-media-server/enabled"));
    upnpNullSinkCheckButton->set_active(gconf->get_bool(PA_GCONF_PATH_MODULES"/upnp-media-server/null-sink-enabled"));

    ignoreChanges = FALSE;

    updateSensitive();
}

void MainWindow::checkForModules() {

    remoteAvailable =
        access(MODULESDIR "module-esound-protocol-tcp" SHREXT, F_OK) == 0 ||
        access(MODULESDIR "module-native-protocol-tcp" SHREXT, F_OK) == 0;

    zeroconfPublishAvailable = access(MODULESDIR "module-zeroconf-publish" SHREXT, F_OK) == 0;
    zeroconfDiscoverAvailable = access(MODULESDIR "module-zeroconf-discover" SHREXT, F_OK) == 0;

    zeroconfRaopDiscoverAvailable = access(MODULESDIR "module-raop-discover" SHREXT, F_OK) == 0;

    rtpRecvAvailable = access(MODULESDIR "module-rtp-recv" SHREXT, F_OK) == 0;
    rtpSendAvailable = access(MODULESDIR "module-rtp-send" SHREXT, F_OK) == 0;

    upnpAvailable =
        access(MODULESDIR "module-rygel-media-server" SHREXT, F_OK) == 0 &&
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

    Gnome::Conf::init();

    Gtk::Window* mainWindow = MainWindow::create();

    Gtk::Main::run(*mainWindow);
    delete mainWindow;
}
