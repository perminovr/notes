import QtQuick 2.11
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.12
import QtQuick.Window 2.11

Item {
    id: element
    width: 400
    height: 400
    property alias butThird: butThird
    property alias butBackEnd: butBackEnd
    property alias ledImg: ledImg
    property alias led: led
    property alias ledButton: ledButton
    property alias exitButton: exitButton

    SwipeView {
        id: swipeView
        x: 0
        y: 0
        width: parent.width
        height: parent.height

        Item {
            id: firstPage

            Label {
                id: label
                x: 112
                y: 262
                text: "press me to close this window"
            }

            RoundButton {
                id: exitButton
                y: 285
                text: "exit"
                anchors.horizontalCenterOffset: 0
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 67
                anchors.horizontalCenter: parent.horizontalCenter
            }

            RoundButton {
                id: ledButton
                y: 21
                text: "LED"
                anchors.horizontalCenterOffset: -118
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 331
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Label {
                id: led
                x: 13
                y: 27
                width: 38
                height: 37
                text: ""
                background: Image {
                    id: ledImg
                    source: "redled.png"
                }
            }

            Button {
                id: butBackEnd
                x: 311
                y: 27
                text: qsTr("Button")
                signal signalClicked
            }
        }
        Item {
            id: secondPage

            Slider {
                id: slider
                value: 0.5
            }

            Button {
                id: butThird
                x: 173
                y: 197
                text: qsTr("Button")
            }
        }
        Item {
            id: thirdPage
        }
    }
}
