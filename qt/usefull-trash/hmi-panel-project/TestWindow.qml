import QtQuick 2.12
import QtQuick.Controls 2.11
import QtQuick.Window 2.11
import QtQuick.Controls.Material 2.4

import hmiPanelPkg 1.0

ApplicationWindow {
    id : mainWindow

    width: 400
    height: 400
    visible: true

    Material.theme: Material.Dark
    Material.accent: Material.Purple

//    flags: Qt.Window | Qt.Frameles
//    visibility: Window.FullScreensWindowHint

    TestWindowForm {

        shortcutA.onActivated: {
            swipeView.currentIndex = (swipeView.currentIndex == 0)? 1 : 0
        }

        Component.onCompleted: {
            // AUTO REGISTER HERE
            textEdit.mbHR = uiMbProvider.regItem(RegItemType.ReadWrite, textEdit, "text", 0, 2)
            slider.mbHR = uiMbProvider.regItem(RegItemType.ReadWrite, slider, "value", 2, 2)
            uiMbProvider.endRegistration()
        }

        textEdit.onTextChanged: {
            uiMbProvider.changeValue(textEdit.mbHR, textEdit.text)
        }

        slider.onValueChanged: {
            uiMbProvider.changeValue(slider.mbHR, slider.value)
        }

        button.onClicked: {
            swipeView.currentIndex = (swipeView.currentIndex == 0)? 1 : 0
        }
    }
}
