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
            console.log("\nReceived message: " + message)
        }
        onStatusChanged: if (socket.status == WebSocket.Error) {
                             console.log("Error: " + socket.errorString)
                         } else if (socket.status == WebSocket.Open) {
                             // socket.sendTextMessage("Hello World")
                         } else if (socket.status == WebSocket.Closed) {
                             console.log("\nSocket closed")
                         }
        active: true
    }

    Component.onCompleted: {
        controller.detectLights()
    }

    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Discovered bulbs"
            color: "black"
        }

        Button {
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

        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Color:"
            color: "black"
        }

        TextInput {
            anchors.horizontalCenter: parent.horizontalCenter
            onEditingFinished: controller.changeColor(text)
            text: "black"
            width: 100
        }
    }
}
