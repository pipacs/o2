import QtQuick
import QtQuick.Controls
import QtWebView
import com.pipacs.o2

ApplicationWindow {
    id: app
    visible: true
    title: "Sialis"
    minimumWidth: 300
    minimumHeight: 200
    height: 700

    O1Twitter {
        id: o1Twitter
        clientId: "The app's API Key from developer.twitter.com"
        clientSecret: "The app's API Secret from developer.twitter.com "
        localPort: 8888

        onOpenBrowser: {
            browser.url = url
            browser.visible = true
        }

        onCloseBrowser: {
            browser.visible = false
        }

        onLinkedChanged: {
            loginButton.enabled = true
            twitterApi.requestTweets()
        }
    }

    TwitterApi {
        id: twitterApi
        authenticator: o1Twitter
    }

    footer: ToolBar {
        Label {
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
        id: listView
        anchors.fill: parent
        model: twitterApi.tweetModel
        delegate: listDelegate
        highlight: Rectangle {color: "#10000000"}
        focus: true

        Component {
            id: listDelegate
            Item {
                width: parent.width
                height: label.contentHeight + 10
                Row {
                    anchors.fill: parent
                    Label {
                        id: label
                        anchors.centerIn: parent
                        width: parent.width
                        text: rawText
                        wrapMode: Text.Wrap
                    }
                    Rectangle {
                        width: parent.width
                        height: 1
                        color: "#10000000"
                        border.color: "transparent"
                    }
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: listView.currentIndex = index
                }
            }
        }
    }

    ApplicationWindow {
        id: browser
        visible: false
        minimumHeight: 800
        minimumWidth: 500
        title: "Login"

        property url url: ""

        WebView {
            anchors.fill: parent
            url: browser.url
        }

        onClosing: {
            close.accepted = true
            loginButton.enabled = true
        }
    }

    Timer {
        interval: 30000
        repeat: true
        running: true
        triggeredOnStart: true
        onTriggered: twitterApi.requestTweets()
    }
}
