/* $Id$ */

/***
  This file is part of paprefs.
 
  paprefs is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published
  by the Free Software Foundation; either version 2 of the License,
  or (at your option) any later version.
 
  paprefs is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.
 
  You should have received a copy of the GNU General Public License
  along with paprefs; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.
***/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <signal.h>

#include <gtkmm.h>
#include <libglademm.h>
#include <gconfmm.h>

#define PA_GCONF_ROOT "/system/pulseaudio"
#define PA_GCONF_PATH_MODULES PA_GCONF_ROOT"/modules"

class MainWindow : public Gtk::Window {
public:
    MainWindow(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& x);
    static MainWindow* create();

    Gtk::EventBox *titleEventBox;
    Gtk::Button *closeButton;

    Gtk::CheckButton
        *remoteAccessCheckButton,
        *zeroconfCheckButton,
        *anonymousAuthCheckButton,
        *rtpReceiveCheckButton,
        *rtpSendCheckButton,
        *rtpLoopbackCheckButton;

    Gtk::RadioButton
        *rtpMikeRadioButton,
        *rtpSpeakerRadioButton,
        *rtpNullSinkRadioButton;

    Glib::RefPtr<Gnome::Conf::Client> gconf;

    bool ignoreChanges;
    
    void onCloseButtonClicked();
    void updateSensitive();
    void onChangeRemoteAccess();
    void onChangeRtpReceive();
    void onChangeRtpSend();
    void readFromGConf();
    void writeToGConfRemoteAccess();
    void writeToGConfRtpReceive();
    void writeToGConfRtpSend();
    void onGConfChange(const Glib::ustring& key, const Gnome::Conf::Value& value);
};

MainWindow::MainWindow(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& x) :
    Gtk::Window(cobject), ignoreChanges(true) {

    x->get_widget("titleEventBox", titleEventBox);
    x->get_widget("closeButton", closeButton);

    x->get_widget("remoteAccessCheckButton", remoteAccessCheckButton);
    x->get_widget("zeroconfCheckButton", zeroconfCheckButton);
    x->get_widget("anonymousAuthCheckButton", anonymousAuthCheckButton);
    x->get_widget("rtpReceiveCheckButton", rtpReceiveCheckButton);
    x->get_widget("rtpSendCheckButton", rtpSendCheckButton);
    x->get_widget("rtpLoopbackCheckButton", rtpLoopbackCheckButton);
    
    x->get_widget("rtpMikeRadioButton", rtpMikeRadioButton);
    x->get_widget("rtpSpeakerRadioButton", rtpSpeakerRadioButton);
    x->get_widget("rtpNullSinkRadioButton", rtpNullSinkRadioButton);

    Gdk::Color c("white");
    titleEventBox->modify_bg(Gtk::STATE_NORMAL, c);

    gconf = Gnome::Conf::Client::get_default_client();
    gconf->set_error_handling(Gnome::Conf::CLIENT_HANDLE_ALL);
    gconf->add_dir(PA_GCONF_ROOT, Gnome::Conf::CLIENT_PRELOAD_RECURSIVE);

    gconf->signal_value_changed().connect(sigc::mem_fun(*this, &MainWindow::onGConfChange));
    readFromGConf();

    closeButton->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onCloseButtonClicked));
    
    remoteAccessCheckButton->signal_toggled().connect(sigc::mem_fun(*this, &MainWindow::onChangeRemoteAccess));
    zeroconfCheckButton->signal_toggled().connect(sigc::mem_fun(*this, &MainWindow::onChangeRemoteAccess));
    anonymousAuthCheckButton->signal_toggled().connect(sigc::mem_fun(*this, &MainWindow::onChangeRemoteAccess));

    rtpReceiveCheckButton->signal_toggled().connect(sigc::mem_fun(*this, &MainWindow::onChangeRtpReceive));
    
    rtpSendCheckButton->signal_toggled().connect(sigc::mem_fun(*this, &MainWindow::onChangeRtpSend));
    rtpLoopbackCheckButton->signal_toggled().connect(sigc::mem_fun(*this, &MainWindow::onChangeRtpSend));
    rtpMikeRadioButton->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onChangeRtpSend));
    rtpSpeakerRadioButton->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onChangeRtpSend));
    rtpNullSinkRadioButton->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onChangeRtpSend));
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

    b = remoteAccessCheckButton->get_active();
    zeroconfCheckButton->set_sensitive(b);
    anonymousAuthCheckButton->set_sensitive(b);

    b = rtpSendCheckButton->get_active();
    rtpLoopbackCheckButton->set_sensitive(b && !rtpSpeakerRadioButton->get_active());
    rtpMikeRadioButton->set_sensitive(b);
    rtpSpeakerRadioButton->set_sensitive(b);
    rtpNullSinkRadioButton->set_sensitive(b);
}

void MainWindow::onChangeRemoteAccess() {

    if (ignoreChanges)
        return;
    
    updateSensitive();
    writeToGConfRemoteAccess();
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

void MainWindow::writeToGConfRemoteAccess() {
    Gnome::Conf::ChangeSet changeSet;
    bool zeroconfEnabled, anonymousEnabled;

    changeSet.set(PA_GCONF_PATH_MODULES"/remote-access/locked", true);
    gconf->change_set_commit(changeSet, true);

    changeSet.set(PA_GCONF_PATH_MODULES"/remote-access/zeroconf_enabled", zeroconfEnabled = zeroconfCheckButton->get_active());
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
            changeSet.set(PA_GCONF_PATH_MODULES"/rtp-send/args0", Glib::ustring("sink_name=rtp format=s16be channels=2 rate=44100 description=\"RTP Multicast Sink\""));

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

void MainWindow::onGConfChange(const Glib::ustring&, const Gnome::Conf::Value&) {
    readFromGConf();   
}

void MainWindow::readFromGConf() {
    Glib::ustring mode;
    
    ignoreChanges = TRUE;

    remoteAccessCheckButton->set_active(gconf->get_bool(PA_GCONF_PATH_MODULES"/remote-access/enabled"));
    zeroconfCheckButton->set_active(gconf->get_bool(PA_GCONF_PATH_MODULES"/remote-access/zeroconf_enabled"));
    anonymousAuthCheckButton->set_active(gconf->get_bool(PA_GCONF_PATH_MODULES"/remote-access/anonymous_enabled"));
    
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

    ignoreChanges = FALSE;

    updateSensitive();
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
