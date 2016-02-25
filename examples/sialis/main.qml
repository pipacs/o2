import QtQuick 2.3
import QtQuick.Controls 1.4
import QtWebKit 3.0
import QtWebKit.experimental 1.0
import com.pipacs.o2 1.0

ApplicationWindow {
    id: app
    visible: true
    title: "Sialis"
    minimumWidth: 300

    O1Twitter {
        id: o1Twitter
        clientId: "2vHeyIxjywIadjEhvbDpg"
        clientSecret: "Xfwe195Kp3ZpcCKgkYs7RKfugTm8EfpLkQvsKfX2vvs"

        onOpenBrowser: {
            browser.url = url
            browser.visible = true
        }

        onCloseBrowser: {
            browser.visible = false
        }

        onLinkedChanged: {
            console.log("onLinkedChanged")
            loginButton.enabled = true
        }
    }

    statusBar: StatusBar {
        Label {
            id: statusLabel
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            text: o1Twitter.linked? ("Logged in as " + o1Twitter.extraTokens["screen_name"]): "Not logged in"
        }

        Button {
            id: loginButton
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            text: o1Twitter.linked? "Logout": "Login"
            onClicked: {
                enabled = false
                if (o1Twitter.linked) {
                    o1Twitter.unlink()
                } else {
                    o1Twitter.link()
                }
            }
        }

        height: loginButton.height + 5
    }

    ListView {
        // Tweets go here...
        anchors.fill: parent
    }

    ApplicationWindow {
        id: browser
        visible: false
        minimumHeight: 800
        minimumWidth: 500
        title: "Login"

        property url url: ""

        ScrollView {
            anchors.fill: parent
            WebView {
                anchors.fill: parent
                url: browser.url
                experimental.preferences.minimumFontSize: 14
            }
        }

        onClosing: {
            close.accepted = true
            loginButton.enabled = true
        }
    }
}

