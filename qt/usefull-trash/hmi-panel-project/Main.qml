import QtQuick 2.12
import QtQuick.Window 2.2
import QtQuick.VirtualKeyboard 2.1
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4

import hmiPanelPkg 1.0

ApplicationWindow {
    id: window
    visible: true
    width: 400
    height: 400
    title: qsTr("Default window")

    Material.theme: Material.Dark
    Material.accent: Material.Purple

    MainForm {


	}

    InputPanel {
        id: inputPanel
        z: 99
        x: 0
        y: window.height
        width: window.width

        states: State {
            name: "visible"
            when: inputPanel.active
            PropertyChanges {
                target: inputPanel
                y: window.height - inputPanel.height
            }
        }
        transitions: Transition {
            from: ""
            to: "visible"
            reversible: true
            ParallelAnimation {
                NumberAnimation {
                    properties: "y"
                    duration: 250
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }
}
