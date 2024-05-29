import QtQuick
import QtQuick.Controls
import QtWebSockets

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    WebSocket {
        id: socket
        url: "wss://eventsub.wss.twitch.tv/ws"
        onTextMessageReceived: function(message) {
            console.log("Received message: " + message);
            let data = JSON.parse(message);
            if (data.metadata.message_type === "session_welcome") {
                controller.registerSession(data.payload.session.id);
            } else if (data.metadata.message_type === "notification") {
                if (data.payload.event.title === controller.getRewardMessage()) {
                    console.log("Change color to: ", data.payload.event.prompt);
                    controller.changeColor(data.payload.event.prompt)
                }
            }
        }
        onStatusChanged: if (socket.status == WebSocket.Error) {
                             console.log("Error: " + socket.errorString)
                         } else if (socket.status == WebSocket.Open) {
                             console.log("Socket open")
                         } else if (socket.status == WebSocket.Closed) {
                             console.log("Socket closed")
                         }
        active: controller.linked
    }

    Component.onCompleted: {
        controller.detectLights()
    }

    Connections {
        target: controller
        function onOpenBrowser(url) {
            console.log(url);
            Qt.openUrlExternally(url);
        }
        function onConnectSocket() {
            socket.active = true;
        }
    }

    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        spacing: 20

        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Discovered bulbs"
            color: "black"
        }

        Button {
            width: 150
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Discover bulbs"
            onClicked: controller.detectLights()
        }

        Repeater {
            anchors.horizontalCenter: parent.horizontalCenter
            model: controller.bulbs

            Label {
                text: modelData
                color: "black"
            }
        }

        Button {
            width: 150
            text: controller.linked ? "Logout from twitch" : "Login to twitch"
            onClicked: controller.linked ? controller.unlink() : controller.link()
        }
    }
}
