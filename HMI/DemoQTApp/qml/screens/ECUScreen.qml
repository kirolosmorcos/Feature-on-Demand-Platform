import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15


Rectangle {

    anchors.fill: parent

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 10

        Label { text: "Safety ECU"; font.pixelSize: 28}
        Button { text: "Send Command" }

        Label { text: "Dashboard ECU"; font.pixelSize: 28 }
        Button { text: "Send Command" }

        Label { text: "Backend Server"; font.pixelSize: 28 }
        Button { text: "Send Request" }
        
        Button {
            text: "Main page"
            onClicked: {
                stack.push(Qt.resolvedUrl("qrc:/qml/main.qml"))
            }
            
    	}
    }
}

