import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4

Item {
    id : root
    property alias contentSource: contentFrame.sourceComponent
    property alias content: contentFrame.item
    property int durationIn: 300
    property int durationOut: 300
    property bool explicitClose: false

    parent: Overlay.overlay
    z: 99
    width: parent.width
    height: parent.height

    property int explicitWidth: root.width/2
    property int explicitHeight: root.height/2
    property int posX: Math.round((root.width - root.explicitWidth) / 2)
    property int posY: Math.round((root.height - root.explicitHeight) / 2)

    signal loaded()
    signal opened()
    signal closed()
    function open() {
        root.state = "show"
    }
    function close() {
        root.state = "hide"
    }

    Rectangle {
        id: shadow
        z: 100
        width: parent.width
        height: parent.height
        MouseArea {
            id: shadowMa
            enabled: root.state == "show"
            anchors.fill: parent
            z: 101
            onClicked: {
                if (explicitClose) { shaking.start() }
                else { root.close() }
            }
        }
    }

    Rectangle {
        id: dialogFrame
        property real rad: 0
        property real angle: 0
        x: root.posX + (Math.cos(angle)*rad /* for shaking animation (prop explicitClose) */ )
        y: root.posY + (Math.sin(angle)*rad /* for shaking animation (prop explicitClose) */ )
        z: 102
        width: root.explicitWidth
        height: root.explicitHeight
        color: parent.Material.background
        Loader {
            id: contentFrame
            anchors.fill: parent
            onLoaded: root.loaded()
        }
    }

    state: "hide"
    states: [
        State {
            name: "hide"
            PropertyChanges { target: shadow; color: "#00000000" }
            PropertyChanges { target: shadow; enabled: false }
            PropertyChanges { target: dialogFrame; scale: 0.0 }
            PropertyChanges { target: dialogFrame; enabled: false }
        },
        State {
            name: "show"
            PropertyChanges { target: shadow; color: "#80000000" }
            PropertyChanges { target: shadow; enabled: true }
            PropertyChanges { target: dialogFrame; scale: 1.0 }
            PropertyChanges { target: dialogFrame; enabled: true }
        }
    ]
    transitions: [
        Transition {
            from: "show"
            to: "hide"
            ColorAnimation {
                target: shadow
                properties: "color"
                duration: root.durationOut
            }
            PropertyAnimation {
                target: dialogFrame
                properties: "scale"
                duration: root.durationOut
            }
            onRunningChanged: {
                if (root.state == "hide") {
                    root.closed()
                }
            }
        },
        Transition {
            from: "hide"
            to: "show"
            ColorAnimation {
                target: shadow
                properties: "color"
                duration: root.durationIn
            }
            PropertyAnimation {
                target: dialogFrame
                properties: "scale"
                duration: root.durationIn
            }
            onRunningChanged: {
                if (root.state == "show") {
                    root.opened()
                }
            }
        }
    ]

    ParallelAnimation {
        id: shaking
        PropertyAnimation {
            target: dialogFrame
            properties: "rad"
            from: 0
            to: 5
            duration: 600
            easing.type: Easing.SineCurve // from..to..from
        }
        PropertyAnimation {
            target: dialogFrame
            properties: "angle"
            from: 0
            to: 15
            duration: 600
            easing.type: Easing.SineCurve // from..to..from
        }
    }
}
