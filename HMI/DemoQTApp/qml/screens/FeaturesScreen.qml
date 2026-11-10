import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Page {
    title: "Features"

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 20

        Label { text: "Available Features:"; font.pixelSize: 28 }
        Label { text: "• BSD"; font.pixelSize: 22 }
        Label { text: "• BSD2"; font.pixelSize: 22 }
        Label { text: "• BSD3"; font.pixelSize: 22 }
        Label { text: "• BSD4"; font.pixelSize: 22 }

        Button {
            text: "Next: ECU / Backend"
            onClicked: {
                stack.push(Qt.resolvedUrl("ECUScreen.qml"))
            }
        }
        
        Button {
            text: "Main page"
            onClicked: {
                stack.push(Qt.resolvedUrl("qrc:/qml/main.qml"))
            }
        }
    }
}


