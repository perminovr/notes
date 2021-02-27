import QtQuick 2.11
import QtQuick.Window 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4

ApplicationWindow {
    id: window
    visible: true
    width: 800
    height: 480

    Material.theme: Material.Dark
    Material.accent: Material.Red

    CustomDialog {
        id: dialog
        contentSource: Item {
            Rectangle {
                color: "blue"
                width: 50
                height: 50
                x: parent.width/2
                y: parent.height/2
                MouseArea {
                    anchors.fill: parent
                    onClicked: dialog.close()
                }
            }
        }
        explicitClose: true
    }

    Button {
        text: "push"
        onClicked: dialog.open()
    }
}
