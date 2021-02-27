import QtQuick 2.12
import QtQuick.Controls 2.11
import QtQuick.Window 2.11
import QtQuick.Controls.Material 2.4
import QtQuick.VirtualKeyboard 2.4
import QtQuick.Layouts 1.3

import hmiPanelPkg 1.0

Item {
    id: testWindowUI
    anchors.fill: parent
    property alias slider: slider
    property alias button: button
    property alias textEdit: textEdit
    property alias swipeView: swipeView
    property alias shortcutA: shortcutA

    SwipeView {
        id: swipeView
        anchors.fill: parent

        Item {
            ColumnLayout {
                id: columnLayout
                anchors.fill: parent

                Shortcut {
                    id: shortcutA
                    context: Qt.ApplicationShortcut
                    sequence: "a"
                }

                Slider {
                    id: slider
                    height: 20
                    Layout.fillWidth: true
                    value: 0.5
                    property RegItem mbHR
                }

                RowLayout {
                    id: rowLayout1
                    anchors.fill: parent

                    TextEdit {
                        id: textEdit
                        Layout.fillWidth: true
                        text: qsTr("abcd")
                        font.pixelSize: 12
                        property RegItem mbHR
                    }

                    Button {
                        id: button
                        Layout.fillWidth: true
                        Layout.maximumWidth: 150
                        text: qsTr("Button")
                        property RegItem mbHR
                    }
                }
            }
        }

        Item {
            Dial {
                id: dial
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/

