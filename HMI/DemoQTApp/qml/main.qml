import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: appWindow
    visible: true
    width: 800
    height: 480
    title: "Feature on Demand"


    // Background Image
    Image {
        anchors.fill: parent
        source: "qrc:/images/background.png"   
        fillMode: Image.PreserveAspectFit
    }

    StackView {
        id: stack
        anchors.fill: parent
        initialItem: Qt.resolvedUrl("qrc:/qml/main.qml")
    }

    // Navigation buttons
    Row {
        spacing: 10
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 20

        Button {
            text: "Features"
            onClicked: stack.replace(Qt.resolvedUrl("qrc:/qml/screens/FeaturesScreen.qml"))
        }

        Button {
            text: "ECUs & Backend"
            onClicked: stack.replace(Qt.resolvedUrl("qrc:/qml/screens/ECUScreen.qml"))
        }
    }
}

